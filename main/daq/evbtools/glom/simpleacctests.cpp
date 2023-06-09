/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  simpleacctests.cpp
 *  @brief: 
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

// Support white box testing.

#define private public
#include "CEventAccumulatorSimple.h"
#undef private
#include <string>
#include <sys/mman.h>    // where memfd_create lives in buster.
#include <unistd.h>
#include <sys/types.h>
#include <fragment.h>
#include <DataFormat.h>

// memory file name - we need something

const std::string memoryFilename("output");

// Default event accumulator settings.
// Note there are tests that will delete the default one
// and create a new one with different settings.
// These are the settings that will be used in setUp

const time_t      maxFlushTime(1);
const size_t      bSize(1024);
const size_t      maxfrags(10);
const CEventAccumulatorSimple::TimestampPolicy policy(
            CEventAccumulatorSimple::first
);
// Data structures:

// This is really a flat fragment with some large fixed capacity

#pragma pack(push, 1)
typedef struct _TestFragment {
    EVB::FragmentHeader s_header;
    uint8_t             s_payload[bSize];
} TestFragment, * pTestFragment;
#pragma pack(pop)


// This is what an event looks like (one fragment).

#pragma pack(push, 1)

typedef struct _Event {
    CEventAccumulatorSimple::EventHeader s_evHeader;
    TestFragment                         s_frag;
} Event, *pEvent;

#pragma pack(pop)


class simpleacctest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(simpleacctest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(empty_1);
    CPPUNIT_TEST(empty_2);
    CPPUNIT_TEST(add_1);
    CPPUNIT_TEST(add_2);
    CPPUNIT_TEST(add_3);
    
    CPPUNIT_TEST(finish_1);
    CPPUNIT_TEST(finish_2);
    
    CPPUNIT_TEST(finish_3);
    CPPUNIT_TEST(finish_4);
    CPPUNIT_TEST(finish_5);
    
    CPPUNIT_TEST(flush_1);
    CPPUNIT_TEST(flush_2);
    CPPUNIT_TEST(flush_3);
    CPPUNIT_TEST(flush_4);
    CPPUNIT_TEST(flush_5);
    
    CPPUNIT_TEST(multievt_1);
    
    CPPUNIT_TEST(oob_1);
    CPPUNIT_TEST(oob_2);
    CPPUNIT_TEST(oob_3);
    
    CPPUNIT_TEST(autofinish_1);
    CPPUNIT_TEST(autofinish_2);
    CPPUNIT_TEST(autofinish_3);
    CPPUNIT_TEST(autofinish_4);
    
    CPPUNIT_TEST(autoflush_1);
    CPPUNIT_TEST(autoflush_2);
    CPPUNIT_TEST(flush_6);
    CPPUNIT_TEST_SUITE_END();
protected:
    void construct_1();
    
    void empty_1();
    void empty_2();
    
    void add_1();
    void add_2();
    void add_3();

    void finish_1();
    void finish_2();
    
    void finish_3();
    void finish_4();
    void finish_5();
    
    void flush_1();
    void flush_2();
    void flush_3();
    void flush_4();
    void flush_5();
    
    void multievt_1();
    
    void oob_1();
    void oob_2();
    void oob_3();
    // Tests that force the completion of an event.
    
    void autofinish_1();
    void autofinish_2();
    void autofinish_3();
    void autofinish_4();
    // Tests that force a flush of the buffer.
    
    void autoflush_1();
    void autoflush_2();
    void flush_6();
private:
    int m_fd;
    CEventAccumulatorSimple* m_pacc;
public:
    void setUp() {
        m_fd = memfd_create(memoryFilename.c_str(), 0);
        ASSERT(m_fd >= 0);
        m_pacc = new CEventAccumulatorSimple(
            m_fd, maxFlushTime, bSize, maxfrags, policy
        );
    }
    void tearDown() {
        delete m_pacc;
        close(m_fd);
        
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(simpleacctest);
// Whitebox check all the attributes are as I think they should be:
void simpleacctest::construct_1()
{
    EQ(m_fd, m_pacc->m_nFd);
    EQ(maxFlushTime, m_pacc->m_maxFlushTime);
    EQ(policy, m_pacc->m_tsPolicy);
    EQ(bSize, m_pacc->m_nBufferSize);
    EQ(maxfrags, m_pacc->m_nMaxFrags);
    ASSERT(m_pacc->m_pBuffer);
    EQ(size_t(0), m_pacc->m_nBytesInBuffer);
    EQ((uint8_t*)(m_pacc->m_pBuffer), m_pacc->m_pCursor);
    ASSERT(!m_pacc->m_pCurrentEvent);
}
// If there's no data we must not need to flush:

void simpleacctest::empty_1()
{
    m_pacc->finishEvent();
    m_pacc->flushEvents();
    
    // The file should be empty_1...that meens off_t of current position
    // is the same as the rewound fd.
    
    off_t current = lseek(m_fd, 0, SEEK_CUR);
    off_t start   = lseek(m_fd, 0, SEEK_SET);
    EQ(current, start); 
}
// Just putting a fragment in does not output anything.
// only finish/flushing does:
void simpleacctest::empty_2()
{
    // For this we don't need a fragment payload of any specific content:
    TestFragment f;
    f.s_header.s_timestamp = 0x124356789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
    
    m_pacc->addFragment(
        reinterpret_cast<EVB::pFlatFragment>(&f), 2
    );
    m_pacc->flushEvents();         // output should be empty
    
    off_t current = lseek(m_fd, 0, SEEK_CUR);
    off_t start   = lseek(m_fd, 0, SEEK_SET);
    EQ(current, start); 
}

// Adding that empty fragment should set the current event fields
// correctly.  Note we're still not setting a payload.

void simpleacctest::add_1()
{
    TestFragment f;
    f.s_header.s_timestamp = 0x124356789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
    
    m_pacc->addFragment(
        reinterpret_cast<EVB::pFlatFragment>(&f), 2
    );
    // Note m_pCurrent event should (and we verified did) point to the
    // m_currentEvent member.  m_pCurrentEvent is only null to show that
    // no event is being built.
    ASSERT(m_pacc->m_pCurrentEvent);
    EQ(f.s_header.s_timestamp, m_pacc->m_currentEvent.s_lastTimestamp);
    EQ(f.s_header.s_timestamp, m_pacc->m_currentEvent.s_timestampTotal);
    EQ(size_t(1), m_pacc->m_currentEvent.s_nFragments);
}
// adding the first item gets the item header, body header and
// fragment byte count set up in the new event.
// For this we need to set up the ring item headers for the
// event but not the actual body payload:

void simpleacctest::add_2()
{
    // Make the fragment:
    
    TestFragment f;
    f.s_header.s_timestamp = 0x124356789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    CEventAccumulatorSimple::pEventHeader p = m_pacc->m_currentEvent.s_header;
    EQ(PHYSICS_EVENT, p->s_itemHeader.s_type);
    uint32_t totalsize = sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t) +
        sizeof(EVB::FragmentHeader) + 100;
    EQ(totalsize, p->s_itemHeader.s_size);
    EQ(f.s_header.s_timestamp, p->s_bodyHeader.s_timestamp);
    EQ(sizeof(BodyHeader), size_t(p->s_bodyHeader.s_size));
    EQ(uint32_t(10), p->s_bodyHeader.s_sourceId);    // output sourceid.
    EQ(uint32_t(0), p->s_bodyHeader.s_barrier);
    
    // Value of the fragbytes:
    
    uint32_t fragsize = sizeof(uint32_t) + sizeof(EVB::FragmentHeader) + 100;
    EQ(fragsize, p->s_fragBytes);
    
    
}
// adding another fragment should still not force an event to end. we'll
// add the same fragment 2x.

void simpleacctest::add_3()
{
    // Make the fragment:
    
    TestFragment f;
    f.s_header.s_timestamp = 0x124356789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    off_t begin = lseek(m_fd, 0, SEEK_CUR);   
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    f.s_header.s_timestamp = 0x124356800;   // change up the ts and 
    f.s_header.s_sourceId  = 2;             // sid.
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    
    m_pacc->flushEvents();             // Should be no output.
    off_t end = lseek(m_fd, 0, SEEK_CUR);
    EQ(begin, end);
    
    // Whitebox assertions:
    
    CEventAccumulatorSimple::Event& e((m_pacc->m_currentEvent));
    EQ(f.s_header.s_timestamp, e.s_lastTimestamp);
    EQ(uint64_t(0x124356789 + 0x124356800), e.s_timestampTotal);
    EQ(size_t(2), e.s_nFragments);
    
    CEventAccumulatorSimple::EventHeader& eh(*(e.s_header));
    
    uint32_t totalsize = sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t) +
        2*(sizeof(EVB::FragmentHeader) + 100);
    EQ(totalsize, eh.s_itemHeader.s_size);
    EQ(PHYSICS_EVENT, eh.s_itemHeader.s_type);
    EQ(uint64_t( 0x124356789), eh.s_bodyHeader.s_timestamp);  // sb from first.
    EQ(uint32_t(10), eh.s_bodyHeader.s_sourceId);    // STill output sid.
    
    uint32_t payloadSize = sizeof(uint32_t) + 2*(sizeof(EVB::FragmentHeader) + 100);
    EQ(payloadSize, eh.s_fragBytes);
    
    // The cursor should have advanced by all those bytes too:
    
    EQ(
       ptrdiff_t(totalsize),
       (m_pacc->m_pCursor - reinterpret_cast<uint8_t*>(m_pacc->m_pBuffer))
    );
    
    
}
// Make a single fragment event and do a forced flush.  there's no current
// event but the cursor should still be advanced.

void simpleacctest::finish_1()
{
    TestFragment f;
    f.s_header.s_timestamp = 0x124356789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    off_t begin = lseek(m_fd, 0, SEEK_CUR);   
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    m_pacc->finishEvent();           // Close off the event.
    
    // There's no current event but the cursor is still advanced.
    
    ASSERT(!m_pacc->m_pCurrentEvent);
    off_t end = lseek(m_fd, 0, SEEK_CUR);
    EQ(begin, end);               // NOthing got written.
    
    ptrdiff_t size = sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t) +
        (sizeof(EVB::FragmentHeader) + 100);
    EQ(size,
       (m_pacc->m_pCursor - reinterpret_cast<uint8_t*>(m_pacc->m_pBuffer))
    );
    
}
// Let's make sure the headers got finished properly when an event is finished.


void simpleacctest::finish_2()
{
    TestFragment f;
    f.s_header.s_timestamp = 0x124356789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    
    CEventAccumulatorSimple::pEventHeader p =
        reinterpret_cast<CEventAccumulatorSimple::pEventHeader>(m_pacc->m_pBuffer);
    uint32_t payload = sizeof(uint32_t) + 100 + sizeof(EVB::FragmentHeader);
    uint32_t total   = sizeof(RingItemHeader) + sizeof(BodyHeader) + payload;
    
    EQ(total, p->s_itemHeader.s_size);
    EQ(PHYSICS_EVENT, p->s_itemHeader.s_type);
    
    EQ(f.s_header.s_timestamp, p->s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), p->s_bodyHeader.s_sourceId);
    EQ(sizeof(BodyHeader), size_t(p->s_bodyHeader.s_size));
    EQ(uint32_t(0), p->s_bodyHeader.s_barrier);
    
    EQ(payload, p->s_fragBytes);
}
// two frags with first policy gives first timestamp.

void simpleacctest::finish_3()
{
    m_pacc->m_tsPolicy = CEventAccumulatorSimple::first;
    TestFragment f;
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    f.s_header.s_timestamp = 0x123456800;
    pbHeader->s_timestamp    = 0x123456800;
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    
    CEventAccumulatorSimple::pEventHeader p =
        reinterpret_cast<CEventAccumulatorSimple::pEventHeader>(m_pacc->m_pBuffer);
    EQ(uint64_t(0x123456789), p->s_bodyHeader.s_timestamp);
    
}

// two frags with last policy gives last timestamp.
void simpleacctest::finish_4()
{
    m_pacc->m_tsPolicy = CEventAccumulatorSimple::last;
    TestFragment f;
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    f.s_header.s_timestamp = 0x123456800;
    pbHeader->s_timestamp    = 0x123456800;
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    
    CEventAccumulatorSimple::pEventHeader p =
        reinterpret_cast<CEventAccumulatorSimple::pEventHeader>(m_pacc->m_pBuffer);
    EQ(uint64_t(0x123456800), p->s_bodyHeader.s_timestamp);
}

// two frags with average pollicy gives average timestamp
void simpleacctest::finish_5()
{
    m_pacc->m_tsPolicy = CEventAccumulatorSimple::average;
    TestFragment f;
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    f.s_header.s_timestamp = 0x123456800;
    pbHeader->s_timestamp    = 0x123456800;
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    
    CEventAccumulatorSimple::pEventHeader p =
        reinterpret_cast<CEventAccumulatorSimple::pEventHeader>(m_pacc->m_pBuffer);
    EQ(uint64_t((0x123456789 + 0x123456800)/2), p->s_bodyHeader.s_timestamp);
}
// Flushing if we have no current event gets the file size right
// and resets all the pointer stuff
void simpleacctest::flush_1()
{
    m_pacc->m_tsPolicy = CEventAccumulatorSimple::average;
    TestFragment f;
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    off_t begin = lseek(m_fd, 0, SEEK_CUR);
    m_pacc->flushEvents();
    off_t end   = lseek(m_fd, 0, SEEK_CUR);
    
    off_t size = sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t) +
        (sizeof(EVB::FragmentHeader) + 100);
    EQ(size, end - begin);
    
    EQ((uint8_t*)(m_pacc->m_pBuffer), m_pacc->m_pCursor);
    EQ(size_t(0), m_pacc->m_nBytesInBuffer);
    ASSERT(!m_pacc->m_pCurrentEvent);    // May have already tested :=)
}

// flushing should give a file with the data for the event:

void simpleacctest::flush_2()
{
    m_pacc->m_tsPolicy = CEventAccumulatorSimple::average;
    TestFragment f;
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    
    m_pacc->flushEvents();
    off_t beg   = lseek(m_fd, 0, SEEK_SET);    // Rewind the file...
    ssize_t size = sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t) +
        (sizeof(EVB::FragmentHeader) + 100);
    
        
    // Read the event:
    
    Event ev;
    ssize_t nRead = read(m_fd, &ev, sizeof(Event));
    EQ(size, nRead);
 
    // Check event contents:
    
    // Header:
    
    EQ(uint32_t(size), ev.s_evHeader.s_itemHeader.s_size);
    EQ(PHYSICS_EVENT, ev.s_evHeader.s_itemHeader.s_type);
    EQ(f.s_header.s_timestamp, ev.s_evHeader.s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), ev.s_evHeader.s_bodyHeader.s_sourceId);
    EQ(uint32_t(0), ev.s_evHeader.s_bodyHeader.s_barrier);
    EQ(uint32_t(sizeof(uint32_t) + sizeof(EVB::FragmentHeader) + 100),
       ev.s_evHeader.s_fragBytes
    );
    // Fragment body:
    
    EQ(f.s_header.s_timestamp, ev.s_frag.s_header.s_timestamp);
    EQ(f.s_header.s_sourceId, ev.s_frag.s_header.s_sourceId);
    EQ(f.s_header.s_size, ev.s_frag.s_header.s_size);
    EQ(f.s_header.s_barrier, ev.s_frag.s_header.s_barrier);
    
    pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(f.s_payload);
    EQ(PHYSICS_EVENT, pH->s_type);
    EQ(uint32_t(100), pH->s_size);
    
}
// Now ensure the payload contents are good.

void simpleacctest::flush_3()
{
    m_pacc->m_tsPolicy = CEventAccumulatorSimple::average;
    TestFragment f;
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    for (int i =0; i < 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader)); i++) {
        pData[i] = i;
    }
    // insert the fragment, finish the event and flush to file:
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    m_pacc->flushEvents();
    
    // Now read the built event from file:
    
    
    lseek(m_fd, 0, SEEK_SET);    // Rewind the file...
    Event ev;
    ssize_t nRead = read(m_fd, &ev, sizeof(Event));  // We know from flush_2 nRead is correct.
    
    // We also know that all the stuf leading up to the payload is correct
    // from flush_2.
    
    pHeader = reinterpret_cast<pRingItemHeader>(ev.s_frag.s_payload);
    pBodyHeader     pBH     = reinterpret_cast<pBodyHeader>(pHeader+1);
    uint8_t*       pPayload = reinterpret_cast<uint8_t*>(pBH+1);
    
    // Should be a counting pattern:
    
    for (uint8_t i =0; i < 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader)); i++) {
        EQ(i, pPayload[i]);
    }
}
// flush 1 event with two fragments.
// Check the metadata for correctness

void simpleacctest::flush_4()
{
    
    TestFragment f;
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    size_t bodySize = 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader));
    for (int i =0; i < bodySize; i++) {
        pData[i] = i;
    }
    // insert the fragment, 
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    // Make the second fragment slightly different:
    
    
    f.s_header.s_timestamp += 10;
    f.s_header.s_sourceId   = 2;
    pbHeader->s_timestamp   = f.s_header.s_timestamp;
    pbHeader->s_sourceId    = f.s_header.s_sourceId;
    
    for (int i =0;  i < bodySize; i++) {
        pData[i] = bodySize - i;
    }
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    // Finish the fragment and flush to file:
    
    m_pacc->finishEvent();
    m_pacc->flushEvents();
    
    // Now rewind and read the event from file.
    
    Event ev;
    lseek(m_fd, 0, SEEK_SET);
    ssize_t nRead = read(m_fd, &ev, sizeof(ev));
    // check nRead against the anticipated size:
    // Each fragment is a fragment header, 
    // and 100 bytes.  The front of the event is a
    // CEventAccumulatorSimple::EventHeader:
    
    uint32_t nBodyBytes = 2*(sizeof(EVB::FragmentHeader) + 100);
    ssize_t evSize = sizeof(CEventAccumulatorSimple::EventHeader) +nBodyBytes;
        
        
    EQ(evSize, nRead);
    
    // Now let's look at the event header  note that the timestamp policy is 1st.
    
    EQ(uint32_t(evSize), ev.s_evHeader.s_itemHeader.s_size);
    EQ(PHYSICS_EVENT, ev.s_evHeader.s_itemHeader.s_type);
    EQ(uint32_t(sizeof(BodyHeader)), ev.s_evHeader.s_bodyHeader.s_size);
    EQ(uint32_t(10), ev.s_evHeader.s_bodyHeader.s_sourceId);
    EQ(uint64_t(0x123456789), ev.s_evHeader.s_bodyHeader.s_timestamp);
    EQ(uint32_t(nBodyBytes + sizeof(uint32_t)), ev.s_evHeader.s_fragBytes);
    
    // Now the metadata for each fragment - the fragment header, the ring item
    // header and the body headers of each fragment.
    
    EVB::pFragmentHeader pfh = reinterpret_cast<EVB::pFragmentHeader>(&ev.s_frag);
    EQ(uint64_t(0x123456789), pfh->s_timestamp);
    EQ(uint32_t(1), pfh->s_sourceId);
    EQ(uint32_t(0), pfh->s_barrier);
    EQ(uint32_t(100), pfh->s_size);
    
    pRingItemHeader ph1 = reinterpret_cast<pRingItemHeader>(pfh+1);
    EQ(PHYSICS_EVENT, ph1->s_type);
    EQ(uint32_t(100), ph1->s_size);
    pBodyHeader pbh1   = reinterpret_cast<pBodyHeader>(ph1+1);
    EQ(uint64_t(0x123456789), pbh1->s_timestamp);
    EQ(uint32_t(1), pbh1->s_sourceId);
    EQ(uint32_t(0), pbh1->s_barrier);
    
    uint8_t* p = reinterpret_cast<uint8_t*>(pfh);
    p += sizeof(EVB::FragmentHeader) + pfh->s_size;   // next fragment heaer:
    
    pfh = reinterpret_cast<EVB::pFragmentHeader>(p);
    EQ(uint64_t(0x123456789+10), pfh->s_timestamp);
    EQ(uint32_t(2), pfh->s_sourceId);
    EQ(uint32_t(100), pfh->s_size);
    EQ(uint32_t(0), pfh->s_barrier);
    
    pRingItemHeader ph2 = reinterpret_cast<pRingItemHeader>(pfh+1);
    EQ(PHYSICS_EVENT, ph2->s_type);
    EQ(uint32_t(100), ph2->s_size);
    pBodyHeader pbh2 = reinterpret_cast<pBodyHeader>(ph2+1);
    EQ(uint64_t(0x123456789+ 10), pbh2->s_timestamp);
    EQ(uint32_t(2), pbh2->s_sourceId);
    EQ(uint32_t(0), pbh2->s_barrier);
    
}
// Now ensure both payloads are good

void simpleacctest::flush_5()
{
    TestFragment f;
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    size_t bodySize = 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader));
    for (int i =0; i < bodySize; i++) {
        pData[i] = i;
    }
    // insert the fragment, 
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    // Make the second fragment slightly different:
    
    
    f.s_header.s_timestamp += 10;
    f.s_header.s_sourceId   = 2;
    pbHeader->s_timestamp   = f.s_header.s_timestamp;
    pbHeader->s_sourceId    = f.s_header.s_sourceId;
    
    for (int i =0;  i < bodySize; i++) {
        pData[i] = bodySize - i;
    }
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    // Finish the fragment and flush to file:
    
    m_pacc->finishEvent();
    m_pacc->flushEvents();
    
    // Now rewind and read the event from file.
    
    Event ev;
    lseek(m_fd, 0, SEEK_SET);
    ssize_t nRead = read(m_fd, &ev, sizeof(ev));
    
    // Locate the first payload which is a count up pattern of bodySize bytes:
    
    EVB::pFragmentHeader pf = reinterpret_cast<EVB::pFragmentHeader>(&(ev.s_frag));
    pRingItemHeader pRh = reinterpret_cast<pRingItemHeader>(pf+1);
    pBodyHeader     pBh = reinterpret_cast<pBodyHeader>(pRh+1);
    uint8_t*        p   = reinterpret_cast<uint8_t*>(pBh+1);
    for (uint8_t i = 0; i < bodySize; i++) {
        EQ(i, *p);
        p++;
    }
    // p points to the next fragment header:
    
    p += sizeof(EVB::FragmentHeader) + sizeof(RingItemHeader) + sizeof(BodyHeader);
    for (uint8_t i = 0; i < bodySize; i++) {
        EQ(uint8_t(bodySize - i), *p);
        p++;
    }
}
// Two fragments that are forced to be two events:
void simpleacctest::multievt_1()
{
    TestFragment f;
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    size_t bodySize = 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader));
    for (int i =0; i < bodySize; i++) {
        pData[i] = i;
    }
    // insert the fragment, 
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    
    // Make the second fragment slightly different:
    
    
    f.s_header.s_timestamp += 10;
    f.s_header.s_sourceId   = 2;
    pbHeader->s_timestamp   = f.s_header.s_timestamp;
    pbHeader->s_sourceId    = f.s_header.s_sourceId;
    
    for (int i =0;  i < bodySize; i++) {
        pData[i] = bodySize - i;
    }
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    // Finish the fragment and flush to file:
    
    m_pacc->finishEvent();
    m_pacc->flushEvents();
    
    // The file should contain two events.  Each event has one fragment
    // size should be
    
    uint32_t eventSize =
        sizeof(CEventAccumulatorSimple::EventHeader) +
        sizeof(EVB::FragmentHeader) + 100;
    ssize_t totalSize =2*(eventSize);
        
    
    Event ev;
    lseek(m_fd, 0, SEEK_SET);
    ssize_t n = read(m_fd, &ev, sizeof(ev));
    EQ(totalSize, n);
    
    // Check the first event:
    
    EQ(PHYSICS_EVENT, ev.s_evHeader.s_itemHeader.s_type);
    EQ(eventSize, ev.s_evHeader.s_itemHeader.s_size);
    EQ(uint32_t(sizeof(BodyHeader)), ev.s_evHeader.s_bodyHeader.s_size);
    EQ(uint64_t(0x123456789), ev.s_evHeader.s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), ev.s_evHeader.s_bodyHeader.s_sourceId);
    EQ(uint32_t(0), ev.s_evHeader.s_bodyHeader.s_barrier);
    EQ(uint32_t(
        eventSize - sizeof(CEventAccumulatorSimple::EventHeader) + sizeof(uint32_t)
    ) , ev.s_evHeader.s_fragBytes);
    
    pTestFragment frag = &(ev.s_frag);
    EVB::pFragmentHeader fh= reinterpret_cast<EVB::pFragmentHeader>(&frag->s_header);
    EQ(uint64_t(0x123456789), fh->s_timestamp);
    EQ(uint32_t(1), fh->s_sourceId);
    EQ(uint32_t(0), fh->s_barrier);
    EQ(uint32_t(100), fh->s_size);
    pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(&frag->s_payload);
    EQ(uint32_t(PHYSICS_EVENT), pH->s_type);
    EQ(uint32_t(100), pH->s_size);
    pBodyHeader pBh = reinterpret_cast<pBodyHeader>(pH+1);
    EQ(uint64_t(0x123456789), pBh->s_timestamp);
    EQ(uint32_t(1), pBh->s_sourceId);
    EQ(uint32_t(0), pBh->s_barrier);
    EQ(uint32_t(sizeof(BodyHeader)), pBh->s_size);
    
    uint32_t payloadSize = 100 - sizeof(RingItemHeader) - sizeof(BodyHeader);
    uint8_t* p = reinterpret_cast<uint8_t*>(pBh+1);
    for (uint8_t i =0; i < payloadSize; i++) {
        EQ(i, *p);
        p++;
    }
    
    pEvent pev = reinterpret_cast<pEvent>(p);
    EQ(PHYSICS_EVENT, pev->s_evHeader.s_itemHeader.s_type);
    EQ(eventSize, pev->s_evHeader.s_itemHeader.s_size);
    EQ(uint32_t(sizeof(BodyHeader)), pev->s_evHeader.s_bodyHeader.s_size);
    EQ(uint64_t(0x123456789+10), pev->s_evHeader.s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), pev->s_evHeader.s_bodyHeader.s_sourceId);
    EQ(uint32_t(0), pev->s_evHeader.s_bodyHeader.s_barrier);
    EQ(uint32_t(
        eventSize - sizeof(CEventAccumulatorSimple::EventHeader) + sizeof(uint32_t)
    ) , pev->s_evHeader.s_fragBytes);
    frag = &(pev->s_frag);
    fh= reinterpret_cast<EVB::pFragmentHeader>(&frag->s_header);
    EQ(uint64_t(0x123456789+10), fh->s_timestamp);
    EQ(uint32_t(2), fh->s_sourceId);
    EQ(uint32_t(0), fh->s_barrier);
    EQ(uint32_t(100), fh->s_size);
    pH = reinterpret_cast<pRingItemHeader>(&frag->s_payload);
    EQ(uint32_t(PHYSICS_EVENT), pH->s_type);
    EQ(uint32_t(100), pH->s_size);
    pBh = reinterpret_cast<pBodyHeader>(pH+1);
    EQ(uint64_t(0x123456789+10), pBh->s_timestamp);
    EQ(uint32_t(2), pBh->s_sourceId);
    EQ(uint32_t(0), pBh->s_barrier);
    EQ(uint32_t(sizeof(BodyHeader)), pBh->s_size);
    p = reinterpret_cast<uint8_t*>(pBh+1);
    for (uint8_t i =0; i < payloadSize; i++) {
        EQ(uint8_t(payloadSize-i), *p);
        p++;
    }
}
// Out of band item goes out immediately to file.
void simpleacctest::oob_1()
{
    TestFragment scaler;     // Will be the OOB fragment:
    
    scaler.s_header.s_timestamp = 0x111111111L;
    scaler.s_header.s_barrier   = 0;
    scaler.s_header.s_size =
        sizeof(RingItemHeader) +     // Ring item with
        sizeof(BodyHeader) +         // A body header
        sizeof(ScalerItemBody) +32*sizeof(uint32_t);  // A scaler body with32 scalers.
    pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(scaler.s_payload);
    pH->s_type = PERIODIC_SCALERS;
    pH->s_size = scaler.s_header.s_size;
    pBodyHeader pB = reinterpret_cast<pBodyHeader>(pH+1);
    pB->s_size = sizeof(BodyHeader);
    pB->s_timestamp = scaler.s_header.s_timestamp;
    pB->s_barrier    = 0;
    pB->s_sourceId   = 1;
    pScalerItemBody pS = reinterpret_cast<pScalerItemBody>(pB+1);
    pS->s_intervalStartOffset = 0;
    pS->s_intervalEndOffset   = 2;
    pS->s_timestamp = time(nullptr);
    pS->s_intervalDivisor     = 1;
    pS->s_scalerCount = 32;
    pS->s_isIncremental       = 1;
    pS->s_originalSid    = 1;
    for (int i = 0; i < 32; i++) {
        pS->s_scalers[i] = i;
    }
    
    // This should write the scaler to the 'file'.
    
    m_pacc->addOOBFragment(reinterpret_cast<EVB::pFlatFragment>(&scaler), 10);
    
    off_t totalSize = scaler.s_header.s_size;
    off_t here = lseek(m_fd, 0, SEEK_CUR);
    off_t begin= lseek(m_fd, 0, SEEK_SET);    // Rewind.
    EQ(totalSize, here-begin);
    
    // The output should be same as input but the body header will have
    // the sid rewritten to 10:
    
    uint8_t buffer[1000];
    ssize_t nRead = read(m_fd, buffer, sizeof(buffer));
    EQ(nRead, ssize_t(totalSize));
    
    // Now the contents:
    
    pScalerItem pScaler = reinterpret_cast<pScalerItem>(buffer);
    EQ(pH->s_type, pScaler->s_header.s_type);
    EQ(pH->s_size, pScaler->s_header.s_size);
    EQ(sizeof(BodyHeader), size_t(pScaler->s_body.u_hasBodyHeader.s_bodyHeader.s_size));
    EQ(pB->s_timestamp, pScaler->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp);
    EQ(pB->s_barrier, pScaler->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier);
    EQ(uint32_t(10), pScaler->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId);
    EQ(pS->s_intervalStartOffset, pScaler->s_body.u_hasBodyHeader.s_body.s_intervalStartOffset);
    EQ(pS->s_intervalEndOffset, pScaler->s_body.u_hasBodyHeader.s_body.s_intervalEndOffset);
    EQ(pS->s_timestamp, pScaler->s_body.u_hasBodyHeader.s_body.s_timestamp);
    EQ(pS->s_intervalDivisor, pScaler->s_body.u_hasBodyHeader.s_body.s_intervalDivisor);
    EQ(pS->s_scalerCount, pScaler->s_body.u_hasBodyHeader.s_body.s_scalerCount);
    EQ(pS->s_isIncremental, pScaler->s_body.u_hasBodyHeader.s_body.s_isIncremental);
    EQ(pS->s_originalSid, pScaler->s_body.u_hasBodyHeader.s_body.s_originalSid);
    for (int i = 0; i < 32; i++) {
        EQ(pS->s_scalers[i], pScaler->s_body.u_hasBodyHeader.s_body.s_scalers[i]);
    }
}
// out of band item prior to complete events goes before them.
void simpleacctest::oob_2()
{
    TestFragment scaler;     // Will be the OOB fragment:
    
    scaler.s_header.s_timestamp = 0x111111111L;
    scaler.s_header.s_barrier   = 0;
    scaler.s_header.s_size =
        sizeof(RingItemHeader) +     // Ring item with
        sizeof(BodyHeader) +         // A body header
        sizeof(ScalerItemBody) +32*sizeof(uint32_t);  // A scaler body with32 scalers.
    pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(scaler.s_payload);
    pH->s_type = PERIODIC_SCALERS;
    pH->s_size = scaler.s_header.s_size;
    pBodyHeader pB = reinterpret_cast<pBodyHeader>(pH+1);
    pB->s_size = sizeof(BodyHeader);
    pB->s_timestamp = scaler.s_header.s_timestamp;
    pB->s_barrier    = 0;
    pB->s_sourceId   = 1;
    pScalerItemBody pS = reinterpret_cast<pScalerItemBody>(pB+1);
    pS->s_intervalStartOffset = 0;
    pS->s_intervalEndOffset   = 2;
    pS->s_timestamp = time(nullptr);
    pS->s_intervalDivisor     = 1;
    pS->s_scalerCount = 32;
    pS->s_isIncremental       = 1;
    pS->s_originalSid    = 1;
    for (int i = 0; i < 32; i++) {
        pS->s_scalers[i] = i;
    }
    
    TestFragment f;    // The fragment that is the event.
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    size_t bodySize = 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader));
    for (int i =0; i < bodySize; i++) {
        pData[i] = i;
    }
    // insert the fragment, 
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    // Since the event has not been finished the OOB event goes before it:
    
    m_pacc->addOOBFragment(reinterpret_cast<EVB::pFlatFragment>(&scaler), 10);
    m_pacc->finishEvent();
    m_pacc->flushEvents();       // Now f is written.
    
    off_t totalsize = scaler.s_header.s_size +
        f.s_header.s_size + sizeof(EVB::FragmentHeader) +
        sizeof (CEventAccumulatorSimple::Event);
    off_t end = lseek(m_fd, 0, SEEK_CUR);
    off_t begin = lseek(m_fd, 0, SEEK_SET);
    EQ(totalsize, end - begin);
    
    // First event is the scaler, second event is PHYSICS_EVENT we trust
    // everything else given our prior tests:
    // (whitebox  - OOB uses flushEvents which has been tested).
    
    uint8_t data[1000];
    ssize_t nBytes = read(m_fd, data, sizeof(data));
    EQ(nBytes, ssize_t(totalsize));
    
    pRingItemHeader pScHeader = reinterpret_cast<pRingItemHeader>(data);
    EQ(PERIODIC_SCALERS, pScHeader->s_type);
    pRingItemHeader pPhHeader = reinterpret_cast<pRingItemHeader>(&(data[pScHeader->s_size]));
    EQ(PHYSICS_EVENT, pPhHeader->s_type);
    
}

// out of band items flush completed events:

void simpleacctest::oob_3()
{
    TestFragment scaler;     // Will be the OOB fragment:
    
    scaler.s_header.s_timestamp = 0x111111111L;
    scaler.s_header.s_barrier   = 0;
    scaler.s_header.s_size =
        sizeof(RingItemHeader) +     // Ring item with
        sizeof(BodyHeader) +         // A body header
        sizeof(ScalerItemBody) +32*sizeof(uint32_t);  // A scaler body with32 scalers.
    pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(scaler.s_payload);
    pH->s_type = PERIODIC_SCALERS;
    pH->s_size = scaler.s_header.s_size;
    pBodyHeader pB = reinterpret_cast<pBodyHeader>(pH+1);
    pB->s_size = sizeof(BodyHeader);
    pB->s_timestamp = scaler.s_header.s_timestamp;
    pB->s_barrier    = 0;
    pB->s_sourceId   = 1;
    pScalerItemBody pS = reinterpret_cast<pScalerItemBody>(pB+1);
    pS->s_intervalStartOffset = 0;
    pS->s_intervalEndOffset   = 2;
    pS->s_timestamp = time(nullptr);
    pS->s_intervalDivisor     = 1;
    pS->s_scalerCount = 32;
    pS->s_isIncremental       = 1;
    pS->s_originalSid    = 1;
    for (int i = 0; i < 32; i++) {
        pS->s_scalers[i] = i;
    }
    
    TestFragment f;    // The fragment that is the event.
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    size_t bodySize = 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader));
    for (int i =0; i < bodySize; i++) {
        pData[i] = i;
    }
    // insert the fragment, 
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    
    // This writes both to file f and scaler in that order.
    
    m_pacc->addOOBFragment(reinterpret_cast<EVB::pFlatFragment>(&scaler), 10);
    
    off_t totalsize = scaler.s_header.s_size +
        f.s_header.s_size + sizeof(EVB::FragmentHeader) +
        sizeof (CEventAccumulatorSimple::Event);
    off_t end = lseek(m_fd, 0, SEEK_CUR);
    off_t begin = lseek(m_fd, 0, SEEK_SET);
    EQ(totalsize, end - begin);
    
    uint8_t data[1000];
    ssize_t nBytes = read(m_fd, data, sizeof(data));
    EQ(nBytes, ssize_t(totalsize));
    
    pRingItemHeader ph1 = reinterpret_cast<pRingItemHeader>(data);
    EQ(PHYSICS_EVENT, ph1->s_type);
    pRingItemHeader ph2 = reinterpret_cast<pRingItemHeader>(&(data[ph1->s_size]));
    EQ(PERIODIC_SCALERS, ph2->s_type);
}
// Changing item types forces events to finish.

void simpleacctest::autofinish_1()
{
    TestFragment f;    // The fragment that is the event.
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    size_t bodySize = 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader));
    for (int i =0; i < bodySize; i++) {
        pData[i] = i;
    }
    // insert the fragment, 
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    // Now we change the type to some user type, adding the fragment should
    // force completion of the event which we can then flush to file to check
    
    pHeader->s_type = FIRST_USER_ITEM_CODE;
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->flushEvents();     // first fragment gets flushed as an event:
    
    off_t totalsize = sizeof(CEventAccumulatorSimple::Event) +
        sizeof(EVB::FragmentHeader) + f.s_header.s_size;
    off_t here = lseek(m_fd, 0, SEEK_CUR);
    off_t begin = lseek(m_fd, 0, SEEK_SET);
    EQ(totalsize, here-begin);
    
    // It's a physics event:
    
    Event e;
    ssize_t nRead=read(m_fd, &e, sizeof(e));
    EQ(nRead, ssize_t(totalsize));
    EQ(PHYSICS_EVENT, e.s_evHeader.s_itemHeader.s_type);
    
}
// Changing the output source id finishes the event in progress:

void simpleacctest::autofinish_2()
{
    TestFragment f;    // The fragment that is the event.
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    size_t bodySize = 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader));
    for (int i =0; i < bodySize; i++) {
        pData[i] = i;
    }
    // insert the fragment, 
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 11); // forces finish
    m_pacc->flushEvents();
    
    off_t totalsize = sizeof(CEventAccumulatorSimple::Event) +
        sizeof(EVB::FragmentHeader) + f.s_header.s_size;
    off_t here = lseek(m_fd, 0, SEEK_CUR);
    off_t begin = lseek(m_fd, 0, SEEK_SET);
    EQ(totalsize, here-begin);
    
    // It's a physics event:
    
    Event e;
    ssize_t nRead=read(m_fd, &e, sizeof(e));
    EQ(nRead, ssize_t(totalsize));
    EQ(PHYSICS_EVENT, e.s_evHeader.s_itemHeader.s_type);
    EQ(uint32_t(10), e.s_evHeader.s_bodyHeader.s_sourceId);
}
// Exceeding max frags forces a finish:

void simpleacctest::autofinish_3()
{
    m_pacc->m_nMaxFrags = 1;        // Limit events to one fragment:
    
    TestFragment f;    // The fragment that is the event.
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    size_t bodySize = 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader));
    for (int i =0; i < bodySize; i++) {
        pData[i] = i;
    }
    // insert the fragment, 
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10); // finish
    m_pacc->flushEvents();
    
    off_t totalsize = sizeof(CEventAccumulatorSimple::Event) +
        sizeof(EVB::FragmentHeader) + f.s_header.s_size;
    off_t here = lseek(m_fd, 0, SEEK_CUR);
    off_t begin = lseek(m_fd, 0, SEEK_SET);
    EQ(totalsize, here-begin);
    
    // It's a physics event:
    
    Event e;
    ssize_t nRead=read(m_fd, &e, sizeof(e));
    EQ(nRead, ssize_t(totalsize));               // Ensures there's only one.
    EQ(PHYSICS_EVENT, e.s_evHeader.s_itemHeader.s_type);
    EQ(uint32_t(10), e.s_evHeader.s_bodyHeader.s_sourceId);
    
}
// If the next fragment extends past the buffer that also forces a finish.

void simpleacctest::autofinish_4()
{

    
    TestFragment f;    // The fragment that is the event.
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    size_t bodySize = 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader));
    for (int i =0; i < bodySize; i++) {
        pData[i] = i;
    }
    off_t totalsize = sizeof(CEventAccumulatorSimple::Event) +
        sizeof(EVB::FragmentHeader) + f.s_header.s_size;
    m_pacc->m_nBufferSize = totalsize + 1;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10); // finish
    // That second fragment should have foreced a flush as well to fit the first
    // frag of the second event:
    
    off_t here = lseek(m_fd, 0, SEEK_CUR);
    off_t begin = lseek(m_fd, 0, SEEK_SET);
    EQ(totalsize, here-begin);
    
    // It's a physics event:
    
    Event e;
    ssize_t nRead=read(m_fd, &e, sizeof(e));
    EQ(nRead, ssize_t(totalsize));               // Ensures there's only one.
    EQ(PHYSICS_EVENT, e.s_evHeader.s_itemHeader.s_type);
    EQ(uint32_t(10), e.s_evHeader.s_bodyHeader.s_sourceId);
    
}
// If there's buffere with an event in it and the next fragment overflows
// a flush is forced.

void simpleacctest::autoflush_1()
{
    TestFragment f;    // The fragment that is the event.
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    size_t bodySize = 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader));
    for (int i =0; i < bodySize; i++) {
        pData[i] = i;
    }
    off_t totalsize = sizeof(CEventAccumulatorSimple::Event) +
        sizeof(EVB::FragmentHeader) + f.s_header.s_size;
    m_pacc->m_nBufferSize = totalsize + 1;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10); //autoflush.
    
    off_t here = lseek(m_fd, 0, SEEK_CUR);
    off_t begin = lseek(m_fd, 0, SEEK_SET);
    EQ(totalsize, here-begin);
    
    // It's a physics event:
    
    Event e;
    ssize_t nRead=read(m_fd, &e, sizeof(e));
    EQ(nRead, ssize_t(totalsize));               // Ensures there's only one.
    EQ(PHYSICS_EVENT, e.s_evHeader.s_itemHeader.s_type);
    EQ(uint32_t(10), e.s_evHeader.s_bodyHeader.s_sourceId);
    
}
// fragment after an autoflush is placed properly.

void simpleacctest::autoflush_2()
{
    TestFragment f;    // The fragment that is the event.
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    size_t bodySize = 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader));
    for (int i =0; i < bodySize; i++) {
        pData[i] = i;
    }
    off_t totalsize = sizeof(CEventAccumulatorSimple::Event) +
        sizeof(EVB::FragmentHeader) + f.s_header.s_size;
    m_pacc->m_nBufferSize = totalsize + 1;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10); //autoflush.
    m_pacc->finishEvent();
    m_pacc->flushEvents();
    
    off_t here = lseek(m_fd, 0, SEEK_CUR);
    off_t begin = lseek(m_fd, 0, SEEK_SET);
    EQ(2*totalsize, here-begin);
    
    
    
}
// If a flush is forced when an event is in progress it can still be built
// properly.

void simpleacctest::flush_6()
{
    TestFragment f;    // The fragment that is the event.
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    // Fill in a payload - the remainder of the ring item.
    
    uint8_t* pData = reinterpret_cast<uint8_t*>(pbHeader+1);
    size_t bodySize = 100 -(sizeof(RingItemHeader) + sizeof(BodyHeader));
    for (int i =0; i < bodySize; i++) {
        pData[i] = i;
    }
    off_t totalsize = sizeof(CEventAccumulatorSimple::Event) +
        sizeof(EVB::FragmentHeader) + f.s_header.s_size;
    m_pacc->m_nBufferSize = 2*totalsize + 1;  // Can hold 2 frags.
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();     // Start second event.
    
    // Second event with 2 frags:
    
    f.s_header.s_timestamp += 10;
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    f.s_header.s_timestamp += 10;
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    f.s_header.s_sourceId = 2;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    // This insertion forces a flush and slide of the partial event.
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    m_pacc->flushEvents();
    
    // We should have 2 events, one with a single fragment, the other with
    // two.  We're really interested to know that the contents of the
    // second event are good.
    
    off_t here = lseek(m_fd, 0, SEEK_CUR);
    off_t begin = lseek(m_fd, 0, SEEK_SET);
    off_t filesize =  2*totalsize + sizeof(EVB::FragmentHeader) + f.s_header.s_size;
    EQ(filesize, here-begin);
    
    uint8_t buffer[1000];
    ssize_t nRead = read(m_fd, buffer, sizeof(buffer));
    EQ(nRead, ssize_t(filesize));
    
    // We're interested in the second ring item:
    
    pRingItemHeader ph = reinterpret_cast<pRingItemHeader>(buffer);
    CEventAccumulatorSimple::pEventHeader pEvent=
        reinterpret_cast<CEventAccumulatorSimple::pEventHeader>(&(buffer[ph->s_size]));
    uint32_t evSize = sizeof(CEventAccumulatorSimple::EventHeader)
        + 2*(sizeof(EVB::FragmentHeader) + 100);
    EQ(evSize, pEvent->s_itemHeader.s_size);
    EQ(PHYSICS_EVENT, pEvent->s_itemHeader.s_type);
    EQ(uint64_t(0x123456789+10), pEvent->s_bodyHeader.s_timestamp);  // first.
    EQ(uint32_t(10), pEvent->s_bodyHeader.s_sourceId);
    EQ(uint32_t(0), pEvent->s_bodyHeader.s_barrier);
    EQ(uint32_t(sizeof(uint32_t) + 2*(sizeof(EVB::FragmentHeader) + 100)), pEvent->s_fragBytes);
    // first fragment:
    
    pTestFragment f1 = reinterpret_cast<pTestFragment>(pEvent+1);
    EQ(uint64_t(0x123456789+10), f1->s_header.s_timestamp);
    EQ(uint32_t(100), f1->s_header.s_size);
    EQ(uint32_t(0), f1->s_header.s_barrier);
    EQ(uint32_t(1), f1->s_header.s_sourceId);
    pRingItemHeader pH1 = reinterpret_cast<pRingItemHeader>(f1->s_payload);
    EQ(uint32_t(100), pH1->s_size);
    EQ(PHYSICS_EVENT, pH1->s_type);
    pBodyHeader pB1 = reinterpret_cast<pBodyHeader>(pH1+1);
    EQ(uint64_t(0x123456789+10), pB1->s_timestamp);
    EQ(uint32_t(0), pB1->s_barrier);
    EQ(uint32_t(1), pB1->s_sourceId);
    uint8_t* p1 = reinterpret_cast<uint8_t*>(pB1+1);
    unsigned payloadSize= 100 - sizeof(BodyHeader) - sizeof(RingItemHeader);
    for (uint8_t i =0; i < payloadSize; i++) {
       EQ(i, *p1);
       p1++;
    }
    
    // Second fragment:
    
    pTestFragment f2 = reinterpret_cast<pTestFragment>(p1);
    EQ(uint64_t(0x123456789+20), f2->s_header.s_timestamp);
    EQ(uint32_t(100), f2->s_header.s_size);
    EQ(uint32_t(0), f2->s_header.s_barrier);
    EQ(uint32_t(2), f2->s_header.s_sourceId);
    pRingItemHeader pH2 = reinterpret_cast<pRingItemHeader>(f2->s_payload);
    EQ(uint32_t(100), pH2->s_size);
    EQ(PHYSICS_EVENT, pH2->s_type);
    pBodyHeader pB2 = reinterpret_cast<pBodyHeader>(pH2+1);
    EQ(uint64_t(0x123456789+20), pB2->s_timestamp);
    EQ(uint32_t(0), pB2->s_barrier);
    EQ(uint32_t(2), pB2->s_sourceId);
    uint8_t* p2 = reinterpret_cast<uint8_t*>(pB2+1);
   
    for (uint8_t i =0; i < payloadSize; i++) {
       EQ(i, *p2);
       p2++;
    }
    
}