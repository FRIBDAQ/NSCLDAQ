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
* @file CRingFragmentSource.h
* @brief application logic of ring fragment source.
*/
#ifndef CRINGFRAGMENTSOURCE_H
#define CRINGFRAGMENTSOURCE_H
#include <DataFormat.h>
#include <stdint.h>
#include <list>
#include <set>
#include <CRingBufferChunkAccess.h>
#include <fragment.h>
#include <time.h>

class CEventOrderClient;
class CRingBuffer;

/**
 * @class CRingFragmentSource
 *    This class contains the main application logic of the ring
 *    fragment source application
 */
class CRingFragmentSource
{
public:
    typedef uint64_t  (*timestampExtractor)(pPhysicsEventItem);
private:
    CEventOrderClient& m_client;
    CRingBuffer&       m_dataSource;
    std::set<uint32_t> m_validSids;
    timestampExtractor m_tsExtractor;
    bool               m_expectBodyHeaders;
    bool               m_isOneShot;
    int                m_endsExpected;
    int                m_endsSeen;
    int                m_endRunTimeout;
    uint64_t           m_timestampOffset;
    unsigned           m_timestampDivisor;
    int                m_nDefaultSid;
    size_t             m_nFragments;  // size of the array below.
    ufmt::EVB::pFragment     m_pFragments;
    
    time_t             m_endRunTime;  // Most recent end run time.
    
public:
    CRingFragmentSource(
        CEventOrderClient& client, CRingBuffer& dataSource, std::list<int> validIds,
        const char* tsExtractorLib, int haveHeaders, int endRunsExpected,
        int endTimeoutSeconds, int timestampOffset,  unsigned divisor, int defaultId
    );
    virtual ~CRingFragmentSource();
    
    void operator()();

    // private utilties
private:
    void setValidIds(std::list<int>& ids);
    void setTsExtractor(const char* tsExtractorLib);
    bool processSegment(CRingBufferChunkAccess& a, size_t chuckSize);
    bool sendChunk(CRingBufferChunkAccess::Chunk& c);
    void resizeFragments();
    void setFragment(
        int n, uint64_t stamp, uint32_t sid, uint32_t size, uint32_t barrier,
        void* pItem
    );
    uint64_t getTimestampFromUserCode(RingItem& item);
    uint32_t barrierType(RingItem& item);
    
    std::pair<size_t, ufmt::EVB::pFragment> makeFragments(CRingBufferChunkAccess::Chunk& c);
    bool timedOut();
    void throwIfNotExpectingBodyHeaders(const char* msg);
};


#endif