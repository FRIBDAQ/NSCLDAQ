#include <DebugUtils.h>
#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>

#include <deque>
#include <iostream>

#include "Asserts.h"
#define private public
#include "HitManager.h"
#undef private
#include "BufferArena.h"
#include "ReferenceCountedBuffer.h"
#include "ZeroCopyHit.h"

using namespace DDASReadout;

namespace std {
std::ostream &operator<<(std::ostream &o,
                         const std::deque<ZeroCopyHit *> &hits) {
  for (int i = 0; i < hits.size(); i++) {
    o << hits[i]->s_time << std::endl;
  }
  return o;
}
} // namespace std
class hitmgrtest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(hitmgrtest);
  CPPUNIT_TEST(initial_1);
  CPPUNIT_TEST(initial_2);

  CPPUNIT_TEST(sort_1);
  CPPUNIT_TEST(sort_2);

  CPPUNIT_TEST(merge_1);
  CPPUNIT_TEST(merge_2);
  CPPUNIT_TEST(merge_3);
  CPPUNIT_TEST(merge_4);

  CPPUNIT_TEST(add_1);
  CPPUNIT_TEST(add_2);
  CPPUNIT_TEST(add_3);
  CPPUNIT_TEST(add_4);

  CPPUNIT_TEST(havehit_1);
  CPPUNIT_TEST(havehit_2);
  CPPUNIT_TEST(havehit_3);

  CPPUNIT_TEST(nexthit_1);
  CPPUNIT_TEST_SUITE_END();

private:
  HitManager *m_pTestObject;
  DDASReadout::BufferArena *m_pArena;

public:
  /** @brief Create a HitManager with a 10s window and a buffer arena. */
  void setUp() {
    m_pTestObject = new HitManager(10 * 1.0e9);
    m_pArena = new DDASReadout::BufferArena;
  }

  /** @brief Cleanup managed objects. */
  void tearDown() {
    delete m_pTestObject;
    delete m_pArena;
  }

protected:
  void initial_1();
  void initial_2();

  void sort_1();
  void sort_2();

  void merge_1();
  void merge_2();
  void merge_3();
  void merge_4();

  void add_1();
  void add_2();
  void add_3();
  void add_4();

  void havehit_1();
  void havehit_2();
  void havehit_3();

  void nexthit_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(hitmgrtest);

/** @brief haveHit is initially false. */
void hitmgrtest::initial_1() { ASSERT(!m_pTestObject->haveHit()); }

/** @brief nextHit is initially nullptr. */
void hitmgrtest::initial_2() { ASSERT(!m_pTestObject->nextHit()); }

/** @brief Sort two hits in time order. */
void hitmgrtest::sort_1() {
  auto buf = m_pArena->allocate(1024);
  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;

  ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1111;

  std::deque<ZeroCopyHit *> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);

  m_pTestObject->sortHits(hits);

  // hits get timestamp ordered.

  EQ(double(1111), hits[0]->s_time);
  EQ(double(1234), hits[1]->s_time);
}

/** @brief Sort three hits in time order. */
void hitmgrtest::sort_2() {
  auto buf = m_pArena->allocate(1024);
  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;

  ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1111; // newer.

  ZeroCopyHit hit3(100, buf->s_pData, buf, m_pArena);
  hit3.s_time = 1222;

  std::deque<ZeroCopyHit *> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);
  hits.push_back(&hit3);

  m_pTestObject->sortHits(hits);

  EQ(double(1111), hits[0]->s_time);
  EQ(double(1222), hits[1]->s_time);
  EQ(double(1234), hits[2]->s_time);
}

/** @brief Assign hits to empty hit manager object. */
void hitmgrtest::merge_1() {
  auto buf = m_pArena->allocate(1024);
  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1111;

  ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1222;

  ZeroCopyHit hit3(100, buf->s_pData, buf, m_pArena);
  hit3.s_time = 1234;

  std::deque<ZeroCopyHit *> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);
  hits.push_back(&hit3);

  m_pTestObject->mergeHits(hits);
  EQ(hits, m_pTestObject->m_sortedHits);
}

/** @brief Append hits. */
void hitmgrtest::merge_2() {
  auto buf = m_pArena->allocate(1024);

  ZeroCopyHit hit0(100, buf->s_pData, buf, m_pArena);
  hit0.s_time = 0; // Everything's going to append to this.
  m_pTestObject->m_sortedHits.push_back(&hit0);

  std::deque<ZeroCopyHit *> hits;

  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;
  hits.push_back(&hit1);

  ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 6789;
  hits.push_back(&hit2);

  m_pTestObject->mergeHits(hits);
  EQ(double(0), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(1234), m_pTestObject->m_sortedHits[1]->s_time);
  EQ(double(6789), m_pTestObject->m_sortedHits[2]->s_time);
}

/** @brief Prepend hits. */
void hitmgrtest::merge_3() {
  auto buf = m_pArena->allocate(1024);

  ZeroCopyHit hit0(100, buf->s_pData, buf, m_pArena);
  hit0.s_time = 9999; // Everything's going to prepend to this.
  m_pTestObject->m_sortedHits.push_back(&hit0);

  std::deque<ZeroCopyHit *> hits;

  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;
  hits.push_back(&hit1);

  ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 6789;
  hits.push_back(&hit2);

  m_pTestObject->mergeHits(hits);

  EQ(double(1234), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(6789), m_pTestObject->m_sortedHits[1]->s_time);
  EQ(double(9999), m_pTestObject->m_sortedHits[2]->s_time);
}

/** @brief Merge with a tail gives sorted queue. */
void hitmgrtest::merge_4() {
  auto buf = m_pArena->allocate(1024);

  ZeroCopyHit hit0(100, buf->s_pData, buf, m_pArena);
  hit0.s_time = 0; // Everything's going to append to this.
  m_pTestObject->m_sortedHits.push_back(&hit0);

  ZeroCopyHit hit00(100, buf->s_pData, buf, m_pArena);
  hit00.s_time = 2222;
  m_pTestObject->m_sortedHits.push_back(&hit00);

  std::deque<ZeroCopyHit *> hits;

  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234; // Will be second.
  hits.push_back(&hit1);

  // The "tail" way later than the other timestamps:

  ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 9999; // Will be last.
  hits.push_back(&hit2);

  m_pTestObject->mergeHits(hits);

  EQ(double(0), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(1234), m_pTestObject->m_sortedHits[1]->s_time);
  EQ(double(2222), m_pTestObject->m_sortedHits[2]->s_time);
  EQ(double(9999), m_pTestObject->m_sortedHits[3]->s_time);
}

/** @brief Add hits. */
void hitmgrtest::add_1() {
  auto buf = m_pArena->allocate(1024);
  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;

  ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1111;

  std::deque<ZeroCopyHit *> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);

  m_pTestObject->addHits(hits);
  EQ(double(1111), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(1234), m_pTestObject->m_sortedHits[1]->s_time);
}

/** @brief Append add. */
void hitmgrtest::add_2() {
  auto buf = m_pArena->allocate(1024);
  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;

  ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1111;

  std::deque<ZeroCopyHit *> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);

  ZeroCopyHit hit0(100, buf->s_pData, buf, m_pArena);
  hit0.s_time = 0;
  m_pTestObject->m_sortedHits.push_back(&hit0);

  m_pTestObject->addHits(hits);

  EQ(double(0), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(1111), m_pTestObject->m_sortedHits[1]->s_time);
  EQ(double(1234), m_pTestObject->m_sortedHits[2]->s_time);
}

/** @brief Prepend add. */
void hitmgrtest::add_3() {
  auto buf = m_pArena->allocate(1024);
  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;

  ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1111;

  std::deque<ZeroCopyHit *> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);

  ZeroCopyHit hit0(100, buf->s_pData, buf, m_pArena);
  hit0.s_time = 9999;
  m_pTestObject->m_sortedHits.push_back(&hit0);

  m_pTestObject->addHits(hits);

  EQ(double(1111), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(1234), m_pTestObject->m_sortedHits[1]->s_time);
  EQ(double(9999), m_pTestObject->m_sortedHits[2]->s_time);
}

/** @brief Add with a tail (interleaved). */
void hitmgrtest::add_4() {
  auto buf = m_pArena->allocate(1024);
  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;

  ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1111; // newer.

  std::deque<ZeroCopyHit *> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);

  ZeroCopyHit hit0(100, buf->s_pData, buf, m_pArena);
  hit0.s_time = 0;
  m_pTestObject->m_sortedHits.push_back(&hit0);

  ZeroCopyHit hit00(100, buf->s_pData, buf, m_pArena);
  hit00.s_time = 9999;
  m_pTestObject->m_sortedHits.push_back(&hit00);

  m_pTestObject->addHits(hits);

  EQ(double(0), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(1111), m_pTestObject->m_sortedHits[1]->s_time);
  EQ(double(1234), m_pTestObject->m_sortedHits[2]->s_time);
  EQ(double(9999), m_pTestObject->m_sortedHits[3]->s_time);
}

/** @brief Single hit can't define a window. */
void hitmgrtest::havehit_1() {
  auto buf = m_pArena->allocate(1024);
  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 0;
  m_pTestObject->m_sortedHits.push_back(&hit1);
  ASSERT(!m_pTestObject->haveHit());
}

/** @brief Two hits but not outside the emission window. */
void hitmgrtest::havehit_2() {
  auto buf = m_pArena->allocate(1024);
  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 0;
  m_pTestObject->m_sortedHits.push_back(&hit1);

  ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1234;
  m_pTestObject->m_sortedHits.push_back(&hit2);

  ASSERT(!m_pTestObject->haveHit());
}

/** @brief Two hits outside the emission window. */
void hitmgrtest::havehit_3() // 2 hits outside of window.
{
  auto buf = m_pArena->allocate(1024);
  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 0;
  m_pTestObject->m_sortedHits.push_back(&hit1);

  ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = double(10) * 1.0e9 + double(1);
  m_pTestObject->m_sortedHits.push_back(&hit2);

  ASSERT(m_pTestObject->haveHit());
}

/** @brief Retrieve a single hit. */
void hitmgrtest::nexthit_1() {
  auto buf = m_pArena->allocate(1024);
  ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 0;
  m_pTestObject->m_sortedHits.push_back(&hit1);

  ZeroCopyHit *pHit = m_pTestObject->nextHit();
  EQ(&hit1, pHit);

  pHit = m_pTestObject->nextHit();
  ASSERT(!pHit);
}
