/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
* @file CRingFragmentSource.cpp
* @brief Implement the main logic of the ring fragment source.
*/
#include "CRingFragmentSource.h"
#include <CEventOrderClient.h>
#include <CRingBuffer.h>
#include <CRingBufferChunkAccess.h>
#include <DataFormat.h>


#include <iostream>
#include <stdexcept>
#include <dlfcn.h>
#include <system_error>
#include <new>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static const size_t FRAG_RESIZE_AMOUNT(1024);   // Number of frags to add on resize

/**
 * constructor:
 *    Just sets up the data - the only interesting logic
 *    is turning a nonempty string of a timestamp extractor into a
 *    function pointer to the timestamp extraction function.
 *
 * @param client - Event builder client object.
 * @param dataSource - Ring buffer from which we're going to get data.
 * @param validIds   - List of ids we told the event builder we'll expect.
 * @param tsExtractorLib - Path to the shared object that has the timestamp
 *                     extraction function to be used if no header is seen.
 *                     This is an empty string? null pointer? if not used.
 * @param haveHeaders - The ring items have body headers.
 * @param endRunsExpected - number of end runs after which to exit
 *                       note that zero means don't exit.
 * @param endTimeoutSeconds -number of seconds after the first end run at which
 *                      to give up waiting for more.
 * @param timestampOffset - Offset to add to timestamps when creating the event builder
 *                      headers.  This allows for startup skew compensation while
 *                      keeping the glom --dt small.
 * @param divisor - Divisor for timestamp. The final timstamp is 
 *       extracted-timestamp/divisor + timestampOffset
 * @param defaultId - default source id.
 */
CRingFragmentSource::CRingFragmentSource(
    CEventOrderClient& client, CRingBuffer& dataSource, std::list<int> validIds,
    const char* tsExtractorLib, int haveHeaders, int endRunsExpected,
    int endTimeoutSeconds, int timestampOffset, unsigned divisor, int defaultId
) :
    m_client(client), m_dataSource(dataSource), m_tsExtractor(nullptr),
    m_expectBodyHeaders(haveHeaders != 0), m_isOneShot(endRunsExpected != 0),
    m_endsExpected(endRunsExpected), m_endsSeen(0),
    m_endRunTimeout(endTimeoutSeconds),
    m_timestampOffset(timestampOffset), m_timestampDivisor(divisor),
    m_nDefaultSid(defaultId),
    m_nFragments(0), m_pFragments(nullptr), m_endRunTime(0)
{
    setValidIds(validIds);
    setTsExtractor(tsExtractorLib);
}
/**
 * destructor
 *    Free the fragments.
 */
CRingFragmentSource::~CRingFragmentSource()
{
    free(m_pFragments);
}
/**
 * operator()
 * The application logic.
*/
void
CRingFragmentSource::operator()()
{
    CRingBufferChunkAccess accessor(&m_dataSource);
    size_t chunkSize = accessor.m_nRingBufferBytes/4;  // go for 1/4'th the buffer.
    while (processSegment(accessor, chunkSize) && !(timedOut()))
        ;
}
//////////////////////////////////////////////////////////////////////
// Private methods.

/**
 * setValidIds
 *    given the list of valid source ids, creates the m_validSids set
 * @param ids - list of valid sourceids.
 */
void
CRingFragmentSource::setValidIds(std::list<int>& ids)
{
    for (auto i :ids ) {
        m_validSids.insert(i);
    }
}
/**
 * setTsExtractor
 *   If the timestamp extractor library is supplied, load it and
 *   locate the timestamp function.  If that's found set m_tsExtrator.
 *   If not an error is emitted and we throw an exception.
 *
 *   Note that it's not an error not to supply a timestamp extractor unless
 *   m_expectBodyHeaders is false.
 *
 * @param tsExtractorLib -path to the .so that has the timestamp extractor.
 */
void
CRingFragmentSource::setTsExtractor(const char* tsExtractorLib)
{
    if (tsExtractorLib == nullptr) {
        throwIfNotExpectingBodyHeaders("If --expectbodyheaders is not supplied --timestampextractor is required");
        return;                      // NO timestamp extractor is ok if we have body headers.
    }
    std::string tsLib(tsExtractorLib);
    if (tsLib == "") {
        throwIfNotExpectingBodyHeaders("If --expectbodyheaders is not supplied --timestampextractor is required");
        return;   
    }
    // Access the timestamp extractor.  From here on in,
    // all errors are also fatal:
    
    void* pDll = dlopen(tsLib.c_str(), RTLD_NOW);
    if(!pDll) {
        std::string msg("Failed to open shared timestamp extractor library: ");
        msg += tsLib;
        msg += " ";
        msg += dlerror();
        std::cerr << msg << std::endl;
        throw std::logic_error(msg);        
    }
    void* timestamp = dlsym(pDll, "timestamp");
    if (!timestamp) {
        std::string msg(
            "Failed to find the 'timestamp' function in the extractor library: ");
        msg += tsLib;
        msg += " ";
        msg += dlerror();
        throw std::logic_error(msg);
    }
    
    // Set m_tsExtractor:
    
    m_tsExtractor = reinterpret_cast<timestampExtractor>(timestamp);
}
/**
 * processSegment
 *    Processes a segment of the ring buffer in as copyfree a manner as
 *    possible.
 *
 *  @param a - the chunk accessor that gives us copy free access to ring items.
 *  @param chunkSize - the biggest chunksize we're going to try for.
 *  @return bool - true if we're not done yhet.
 */
bool
CRingFragmentSource::processSegment(CRingBufferChunkAccess& a, size_t chunkSize)
{
    size_t chunkBytesGotten;
    auto chunk = a.nextChunk();
    chunkBytesGotten = chunk.size();
    if (chunkBytesGotten == 0) {
        while(a.waitChunk(chunkSize, 1000, 10) == 0) {
            if(timedOut()) return false;              // Done processing.
        }
        chunk = a.nextChunk();
        chunkBytesGotten = chunk.size();
        if(chunkBytesGotten  == 0) {
            return false;                          // Odd impossible(?) case.
        }
    }
    return sendChunk(chunk);
}
/**
 * sendChunk
 *   send a chunk of data to the event builder.
 *
 *  @param c - chunk
 *  @return bool -true if we should continue to process.
 */
bool
CRingFragmentSource::sendChunk(CRingBufferChunkAccess::Chunk& c)
{
    // Create the fragment headers and count the end runs.
    
    std::pair<size_t, ufmt::EVB::pFragment> frags = makeFragments(c);
    
    // Send the fragments to the event builder.
    
    if (frags.first > 0) {
        m_client.submitFragments(frags.first, frags.second);
    }
    
    // Figure out if we should continue processing or not.
    // @todo - timeout on end runs seen.
    
    if (!m_isOneShot) return true;           // always keep on going if not 1shot
    if (m_endsExpected <= m_endsSeen) return false;

    return true;

}
/**
 * resizeFragments
 *   Adds storage to the fragment array.
 */
void
CRingFragmentSource::resizeFragments()
{
    m_nFragments += FRAG_RESIZE_AMOUNT;
    m_pFragments = static_cast<ufmt::EVB::pFragment>(
        realloc(m_pFragments, m_nFragments * sizeof(ufmt::EVB::Fragment))
    );
    if (!m_pFragments) throw std::bad_alloc();
}
/**
 * setFragment
 *    Sets a fragment in m_pFragments.
 * @param n - index.
 * @param stamp  - fragment timestamp
 * @param sid    - Source id.
 * @param size   - Number of bytes in the fragment.
 * @param barrier- item barrier code.
 * @param pItem  - pointer to the payload.
 */
void
CRingFragmentSource::setFragment(
    int n, uint64_t stamp, uint32_t sid, uint32_t size, uint32_t barrier,
    void* pItem  
)
{
    if (n >= m_nFragments) resizeFragments();  // ensure we can accomodate that.
    auto f = m_pFragments + n;
    f->s_header.s_timestamp = stamp;
    f->s_header.s_sourceId  = sid;
    f->s_header.s_size      = size;
    f->s_header.s_barrier   = barrier;
    f->s_pBody     = pItem;
}
/**
 * getTimestampFromUserCode
 *   Called to return a timestamp if there is no body header.
 *   -  If this is a physics event _and_ the user has supplied a
 *      timestamp extractor, the event is passed to that extractor
 *      to get the timestamp.
 *   -  Otherwise NULL_TIMESTAMP is returned.
 *
 * @param item      - References the ring item.
 * @return uint64_t - the timestamp
 * @note The framework applies the timestamp offset to the stamp returned by the
 *        user code.
 */
uint64_t
CRingFragmentSource::getTimestampFromUserCode(RingItem& item)
{
    if ((itemType(&item) == PHYSICS_EVENT) && m_tsExtractor) {
        pPhysicsEventItem pEvent = reinterpret_cast<pPhysicsEventItem>(&item);
        return (*m_tsExtractor)(pEvent)/m_timestampDivisor + m_timestampOffset;
    } else {
        return NULL_TIMESTAMP;
    }
}
/**
 * barrierType
 *   Returns the event's barrier type if there is no body header.
 *   The barrier type depends only on the ring item type.
 *
 *   @param item - references the ring item.
 *   @return uint32_t barrier type which is 0 for most ring item types.
 *   @retval 1 - Type is BEGIN_RUN.
 *   @retval 2 - Type is END_RUN.
 */
uint32_t
CRingFragmentSource::barrierType(RingItem& item)
{
    uint32_t result(0);                // Default result.
    switch (itemType(&item)) {
    case BEGIN_RUN:
        result = 1;
        break;
    case END_RUN:
        result = 2;
        break;
    // Don't need default case because of initialization.
    }
    
    return result;
}

/**
 *
 *  makeFragments
 *     Given a chunk, creates the array of EVB::Fragment-s that describe
 *     that chunk.
 *
 * @param c - the chunk to process.
 * @return std::pair<size_t, EVB::pFragment> - first is number of fragments,
 *                                             second is pointer to the fragment array.
 */
std::pair<size_t, ufmt::EVB::pFragment>
CRingFragmentSource::makeFragments(CRingBufferChunkAccess::Chunk& c)
{
    int n = 0;
    for (auto p = c.begin(); !(p == c.end()); p++) {
        RingItemHeader& header(*p);
        RingItem&       item(reinterpret_cast<RingItem&>(header));
        
        
        if (!hasBodyHeader(&item)) {
            setFragment(
                n, getTimestampFromUserCode(item),
                m_nDefaultSid,
                item.s_header.s_size, barrierType(item), &item
                
            );
        } else {
            // Note the divisor and offset are only applied to items when the timestamp is not nul..
            pBodyHeader pB =
                reinterpret_cast<pBodyHeader>(bodyHeader(&item));
            setFragment(
                n,
                pB->s_timestamp != NULL_TIMESTAMP ? pB->s_timestamp/m_timestampDivisor + m_timestampOffset :
                    pB->s_timestamp,
                pB->s_sourceId,
                itemSize(&item),
                pB->s_barrier,
                &item
            );
        }
        
        // Count end runs and when they happened for one-shot and timeout
        
        if (itemType(&item) == END_RUN) {
            m_endRunTime = time_t(nullptr);
            m_endsSeen++;
        }
        
        n++;
    }
    return {n, m_pFragments};
}
/**
 * timedOut
 *    @return bool - True if we timed out waiting for all the end runs.
 */
bool
CRingFragmentSource::timedOut()
{
    // We timed out if:
    // -  We're one shot
    // -  THe endRunTimeout is not zero.
    // -  The time difference between now and the last time we saw an end run
    //    is bigger than the timeout:
    
    if (!m_isOneShot || (m_endRunTimeout == 0)) return false;  // can't timeout.
    
    if (!m_endsSeen) return false;                 // Need at least 1 end to timeout
    
    time_t now = time(nullptr);
    if ((now - m_endRunTime) > m_endRunTimeout) return true;
        
    return false;
}
/**
 * throwIfNotExpectingBodyHeaders
 *    Outputs an error to cerr and throws it as a string
 *    if we're not expecting body headers.
 *    This method was motivated by daqdev/NSCLDAQ#700
 *
 *  @p[aram msg - the message to output/throw.
 */
void
CRingFragmentSource::throwIfNotExpectingBodyHeaders(const char* msg)
{
    if (!m_expectBodyHeaders) {
        std::cerr << msg << std::endl;
        throw std::logic_error(msg);
    }
}
