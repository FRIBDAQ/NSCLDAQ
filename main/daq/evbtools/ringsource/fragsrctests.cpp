// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#define private public
#include "CRingFragmentSource.h"
#undef private
#include <fragment.h>
#include <CRingBufferChunkAccess.h>
#include <CRingItem.h>
#include <DataFormat.h>

#include <time.h>
#include <stdint.h>
#include <string.h>
#include <vector>

// Fake timestamp extractor:

uint64_t fakeExtractor(pPhysicsEventItem pItem)
{
  return 0x123456789abcdef;     // Just a fake stamp.
}

// Stub classes:

class CEventOrderClient  {
public:
  std::vector<std::pair<size_t, ufmt::EVB::pFragment>> m_submissions;
  void submitFragments(size_t nFrags, ufmt::EVB::pFragment frags);
};
class CRingBuffer {};

void
CEventOrderClient::submitFragments(size_t nFrags, ufmt::EVB::pFragment frags)
{
  m_submissions.push_back({nFrags, frags});
}


class fragsrctest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(fragsrctest);
  CPPUNIT_TEST(timeout_1);
  CPPUNIT_TEST(timeout_2);
  CPPUNIT_TEST(timeout_3);
  CPPUNIT_TEST(timeout_4);
  
  CPPUNIT_TEST(makefrags_1);
  CPPUNIT_TEST(makefrags_2);
  CPPUNIT_TEST(makefrags_3);
  CPPUNIT_TEST(makefrags_4);
  
  CPPUNIT_TEST(btype_1);
  CPPUNIT_TEST(btype_2);
  CPPUNIT_TEST(btype_3);
  
  CPPUNIT_TEST(tstamp_1);
  CPPUNIT_TEST(tstamp_2);
  CPPUNIT_TEST(tstamp_3);
  CPPUNIT_TEST(tstamp_4);  // Issue #430 - test divisor.
  
  CPPUNIT_TEST(sendchunk_1);
  CPPUNIT_TEST(sendchunk_2);
  CPPUNIT_TEST_SUITE_END();


private:
  CEventOrderClient* m_pClient;
  CRingBuffer*       m_pSrc;
  CRingFragmentSource* m_pTestObj;
public:
  void setUp() {
    m_pClient = new CEventOrderClient;
    m_pSrc    = new CRingBuffer;
    
    std::list<int> validIds = {1,2};
    m_pTestObj = new CRingFragmentSource(
      *m_pClient, *m_pSrc, validIds, nullptr, true, 1, 0, 0, 1, 1
    );
  }
  void tearDown() {
    delete m_pTestObj;
    delete m_pSrc;
    delete m_pClient;
  }
protected:
  void timeout_1();
  void timeout_2();
  void timeout_3();
  void timeout_4();
  
  void makefrags_1();
  void makefrags_2();
  void makefrags_3();
  void makefrags_4();
  
  void btype_1();
  void btype_2();
  void btype_3();
  
  void tstamp_1();
  void tstamp_2();
  void tstamp_3();
  void tstamp_4();
  
  void sendchunk_1();
  void sendchunk_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(fragsrctest);

void fragsrctest::timeout_1() {
  // Not a oneshot never timesout:
  
  m_pTestObj->m_endRunTime = time(nullptr) +10;
  m_pTestObj->m_endRunTimeout = 1;
  m_pTestObj->m_endsSeen = 1;
  m_pTestObj->m_endsExpected = 2;
  m_pTestObj->m_isOneShot = false;
  
  ASSERT(!m_pTestObj->timedOut());
}

void fragsrctest::timeout_2()
{
  // No timeout if we've not seen an end run:
  
  time_t now = time(nullptr);
  
  m_pTestObj->m_endRunTime = now - 10;
  m_pTestObj->m_endRunTimeout = 1;
  m_pTestObj->m_endsSeen = 0;
  m_pTestObj->m_endsExpected = 1;
  m_pTestObj->m_isOneShot = true;
  
  ASSERT(!m_pTestObj->timedOut());
}
void fragsrctest::timeout_3()
{
  // No timeout specified is also false:
  
  time_t now = time(nullptr);
  
  m_pTestObj->m_endRunTime = now - 10;
  m_pTestObj->m_endRunTimeout = 0;
  m_pTestObj->m_endsSeen = 1;
  m_pTestObj->m_endsExpected = 2;
  m_pTestObj->m_isOneShot = true;
  
  ASSERT(!m_pTestObj->timedOut());
}
void fragsrctest::timeout_4()
{
  // Times out:

time_t now = time(nullptr);
  
  m_pTestObj->m_endRunTime = now - 10;
  m_pTestObj->m_endRunTimeout = 1;
  m_pTestObj->m_endsSeen = 1;
  m_pTestObj->m_endsExpected = 2;
  m_pTestObj->m_isOneShot = true;
  
  ASSERT(m_pTestObj->timedOut());
}
// empty chunk results in no frags

void fragsrctest::makefrags_1()
{
  CRingBufferChunkAccess::Chunk c;
  c.setChunk(0, nullptr);
  
  auto result = m_pTestObj->makeFragments(c);
  EQ(size_t(0), result.first);
}
// A chunk with a single ring item makes a single, correct, fragment
// header.

void fragsrctest::makefrags_2()
{
  ::CRingItem r(PHYSICS_EVENT, 0x1234, 1, 0);
  uint32_t* p = static_cast<uint32_t*>(r.getBodyCursor());
  for (int i =0; i < 10; i++) {
    *p++ = i;
  }
  r.setBodyCursor(p);
  r.updateSize();
  ::pRingItem pRawItem = r.getItemPointer();
  
  CRingBufferChunkAccess::Chunk c;
  c.setChunk(pRawItem->s_header.s_size, pRawItem);
  
  auto result = m_pTestObj->makeFragments(c);
  EQ(size_t(1), result.first);
  
  ufmt::EVB::pFragment f(result.second);
  
  EQ(f->s_header.s_timestamp, r.getEventTimestamp());
  EQ(f->s_header.s_sourceId, r.getSourceId());
  EQ(f->s_header.s_size,      pRawItem->s_header.s_size);
  EQ(f->s_header.s_barrier,   r.getBarrierType());
  EQ((void*)(pRawItem), f->s_pBody);
}
void fragsrctest::makefrags_3()
{
  // Make a chunk with a bunch of ring items:
  
  ::CRingItem* pItems[10];
  uint32_t   d = 0;
  uint8_t*   pChunkStorage;
  size_t totalSize(0);
  
  for (int i =0; i < 10; i++) {
    pItems[i] = new CRingItem(PHYSICS_EVENT, i, 1, 0);
    uint32_t* p = static_cast<uint32_t*>(pItems[i]->getBodyCursor());
    for (int i =0; i < 10; i++) {
      *p++ = i;
    }
    pItems[i]->setBodyCursor(p);
    pItems[i]->updateSize();
    totalSize += itemSize(pItems[i]->getItemPointer());  
  }
  
  pChunkStorage = new uint8_t[totalSize];
  
  // Fill the chunk with raw ring items.
  
  uint8_t* p = pChunkStorage;
  
  for (int i =0; i < 10; i++) {
    ::pRingItem pi = pItems[i]->getItemPointer();
    memcpy(p, pi, itemSize(pi));
    p += itemSize(pi);
  }
  CRingBufferChunkAccess::Chunk c;
  c.setChunk(totalSize, pChunkStorage);
  
  auto result = m_pTestObj->makeFragments(c);
  EQ(size_t(10), result.first);
  
  auto pFrags = result.second;
  
  auto f = pFrags;
  for(int i =0; i < 10; i++) {
  
    ::pRingItem pOriginal = pItems[i]->getItemPointer();
    EQ(f->s_header.s_timestamp, pItems[i]->getEventTimestamp());
    EQ(f->s_header.s_sourceId, pItems[i]->getSourceId());
    EQ(f->s_header.s_size,      pOriginal->s_header.s_size);
    EQ(f->s_header.s_barrier,   pItems[i]->getBarrierType());
  
    // The body should look like pItems[i].
    
    void* pBody = f->s_pBody;
    
    ASSERT(memcmp(pOriginal, pBody, pOriginal->s_header.s_size) == 0);
    
    delete pItems[i];
    f++;
  }
  
  
}
// Make fragments for bodyheader-less items:

void fragsrctest::makefrags_4()
{
  ::CRingItem item(PHYSICS_EVENT);
  uint32_t* p = static_cast<uint32_t*>(item.getBodyCursor());
  for (int i =0; i < 10; i++) {
    *p++ = i;
  }
  item.setBodyCursor(p);
  item.updateSize();
  
  pRingItem pRawItem = item.getItemPointer();
  CRingBufferChunkAccess::Chunk c;
  c.setChunk(pRawItem->s_header.s_size, pRawItem);
  
  auto result = m_pTestObj->makeFragments(c);
  EQ(size_t(1), result.first);
  auto f = result.second;
  EQ(NULL_TIMESTAMP, f->s_header.s_timestamp);   // From extractor which is null
  EQ(itemSize(pRawItem), f->s_header.s_size);
  EQ(uint32_t(0), f->s_header.s_barrier);
  EQ(uint32_t(1), f->s_header.s_sourceId);   // Default id.
  EQ((void*)(pRawItem), f->s_pBody);
  
  
  
}

// Barrier type for BEGIN_RUN -> 1

void fragsrctest::btype_1()
{
  ::CRingItem item(BEGIN_RUN);
  
  EQ(uint32_t(1), m_pTestObj->barrierType(*(item.getItemPointer())));
}
// Barrier type for END_RUN -> 2

void fragsrctest::btype_2()
{
  ::CRingItem item(END_RUN);
  EQ(uint32_t(2), m_pTestObj->barrierType(*(item.getItemPointer())));
}
void fragsrctest::btype_3()         // Rest are 0.
{
  ::CRingItem item(PHYSICS_EVENT);
  EQ(uint32_t(0), m_pTestObj->barrierType(*item.getItemPointer()));
}

// No timestamp extractor gives NULL_TIMESTAMP:

void fragsrctest::tstamp_1()
{
  ::RingItem item;
  EQ(NULL_TIMESTAMP, m_pTestObj->getTimestampFromUserCode(item));
}
// Call the extractor if there is one:
void fragsrctest::tstamp_2()
{
  ::RingItem item;
  item.s_header.s_type = PHYSICS_EVENT;                // extractor for phys items.
  m_pTestObj->m_tsExtractor = fakeExtractor;
  EQ(uint64_t(0x123456789abcdef), m_pTestObj->getTimestampFromUserCode(item));
}
void fragsrctest::tstamp_3()
{
  ::RingItem item;
  item.s_header.s_type = BEGIN_RUN;
  m_pTestObj->m_tsExtractor = fakeExtractor; // only called on physics items.
  EQ(NULL_TIMESTAMP, m_pTestObj->getTimestampFromUserCode(item));
}
void fragsrctest::tstamp_4() {
  // need to replace the test object with one that has a divisor - we'll pick 2 :
  
  delete m_pTestObj;
  std::list<int> validIds = {1,2};
  m_pTestObj = new CRingFragmentSource(
      *m_pClient, *m_pSrc, validIds, nullptr, true, 1, 0, 0, 2, 1
    );
  ::RingItem item;
  item.s_header.s_type = PHYSICS_EVENT;                // extractor for phys items.
  m_pTestObj->m_tsExtractor = fakeExtractor;
  EQ(uint64_t(0x123456789abcdef)/2, m_pTestObj->getTimestampFromUserCode(item));


}
//

void fragsrctest::sendchunk_1()
{
  // Empty chunk sends nothing. false return:
  
  CRingBufferChunkAccess::Chunk c;
  c.setChunk(0, nullptr);
  bool result = m_pTestObj->sendChunk(c);
  ASSERT(result);
  
  ASSERT(m_pClient->m_submissions.empty());
}
void fragsrctest::sendchunk_2()
{
  // Make a chunk with a bunch of ring items:
  
  ::CRingItem* pItems[10];
  uint32_t   d = 0;
  uint8_t*   pChunkStorage;
  size_t totalSize(0);
  
  for (int i =0; i < 10; i++) {
    pItems[i] = new ::CRingItem(PHYSICS_EVENT, i, 1, 0);
    uint32_t* p = static_cast<uint32_t*>(pItems[i]->getBodyCursor());
    for (int i =0; i < 10; i++) {
      *p++ = i;
    }
    pItems[i]->setBodyCursor(p);
    pItems[i]->updateSize();
    totalSize += itemSize(pItems[i]->getItemPointer());
  }
  
  pChunkStorage = new uint8_t[totalSize];
  
  // Fill the chunk with raw ring items.
  
  uint8_t* p = pChunkStorage;
  
  for (int i =0; i < 10; i++) {
    ::pRingItem pi = pItems[i]->getItemPointer();
    memcpy(p, pi, itemSize(pi));
    p += itemSize(pi);
  }
  CRingBufferChunkAccess::Chunk c;
  c.setChunk(totalSize, pChunkStorage);
  
  m_pTestObj->sendChunk(c);
  
  EQ(size_t(1), m_pClient->m_submissions.size());
  auto sub = m_pClient->m_submissions[0];
  EQ(size_t(10), sub.first);           // # of fragments.
  ufmt::EVB::pFragment pFrags = sub.second;
  for (int i =0; i < 10; i++) {
    ::pRingItem pItem = pItems[i]->getItemPointer();
    EQ(pItems[i]->getEventTimestamp(), pFrags->s_header.s_timestamp);
    EQ(pItems[i]->getSourceId(), pFrags->s_header.s_sourceId);
    EQ(pItems[i]->getBarrierType(), pFrags->s_header.s_barrier);
    EQ(itemSize(pItem), pFrags->s_header.s_size);
    
    ASSERT(memcmp(pItem, pFrags->s_pBody, pItem->s_header.s_size) == 0);
    
    delete pItems[i];
    pFrags++;
  }
  free(pChunkStorage);
  
}