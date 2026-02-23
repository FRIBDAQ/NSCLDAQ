// Template for a test suite.

#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>

#include <algorithm>
#include <cstdlib>
#include <random>

#include "Asserts.h"
#define private public
#include "BufferArena.h"
#include "ReferenceCountedBuffer.h"
#include "ZeroCopyHit.h"
#undef private
#include "testcommon.h"

class zcopyhittest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(zcopyhittest);

  CPPUNIT_TEST(refcheck_1);
  CPPUNIT_TEST(refcheck_2);
  CPPUNIT_TEST(refcheck_3);

  CPPUNIT_TEST(recycle_1);
  CPPUNIT_TEST(recycle_2);

  CPPUNIT_TEST(free_1);
  CPPUNIT_TEST(free_2);

  CPPUNIT_TEST_SUITE_END();

private:
  DDASReadout::BufferArena *m_pArena;

public:
  /** @brief Create a buffer arena for the tests. */
  void setUp() { m_pArena = new DDASReadout::BufferArena; }

  /**
   * @brief If there are buffers in the pool first set their reference
   * counts -> 0. Otherwise we'll get another throw when we delete the
   * buffers when the pool is cleaned up if there are refs.
   */
  void tearDown() {
    for (int i = 0; i < m_pArena->m_BufferPool.size(); i++) {
      m_pArena->m_BufferPool[i]->s_references = 0;
    }
    delete m_pArena;
  }

protected:
  void refcheck_1();
  void refcheck_2();
  void refcheck_3();

  void recycle_1();
  void recycle_2();

  void free_1();
  void free_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(zcopyhittest);

/**
 * @brief Creating the hit increments the buffer reference counter.
 * Deleting it decrements and returns it to the arena.
 */
void zcopyhittest::refcheck_1() {
  DDASReadout::ReferenceCountedBuffer *pBuffer =
      m_pArena->allocate(4 * sizeof(uint32_t)); // A single hit fits in this.
  makeHit((uint32_t *)(*pBuffer), 0, 1, 2, 3, 0x1234, 100);
  {
    DDASReadout::ZeroCopyHit zhit(4, (uint32_t *)(*pBuffer), pBuffer, m_pArena);
    ASSERT(pBuffer->isReferenced());
  }
  ASSERT(!pBuffer->isReferenced()); // No longer referenced.
  ASSERT(!m_pArena->m_BufferPool.empty());
  EQ(pBuffer, m_pArena->m_BufferPool.front()); // Back in the pool.
}

/**
 * @brief Ensure that there's not going to be a premature release of the
 * buffer if one of two hits is released.
 */
void zcopyhittest::refcheck_2() {
  // Ensure that there's not going to be a premature release of the
  // buffer if one of two hits is released.

  DDASReadout::ReferenceCountedBuffer *pBuffer =
      m_pArena->allocate(8 * sizeof(uint32_t)); // Two hits worth of data.

  uint32_t *p32 = *pBuffer; // Points to the data.
  makeHit(p32, 0, 1, 2, 3, 0x1234, 100);
  makeHit(p32 + 4, 0, 1, 2, 4, 0x123f, 200);
  {
    DDASReadout::ZeroCopyHit zHit1(4, p32, pBuffer, m_pArena);
    {
      DDASReadout::ZeroCopyHit zHit2(4, p32 + 4, pBuffer, m_pArena);
    }
    // only hit1 was deleted so:

    ASSERT(pBuffer->isReferenced());
    ASSERT(m_pArena->m_BufferPool.empty());
  }

  // Now the last one is released as is the buffer:

  ASSERT(!pBuffer->isReferenced());
  ASSERT(!m_pArena->m_BufferPool.empty());
  EQ(pBuffer, m_pArena->m_BufferPool.front());
}

/**
 * @brief Doesn't matter the order of release, the buffer doesn't go away until
 * the last hit is released.
 */
void zcopyhittest::refcheck_3() {
  DDASReadout::ReferenceCountedBuffer *pBuffer =
      m_pArena->allocate(4 * sizeof(uint32_t) * 100); // 100 hits.

  uint32_t *pHit = *pBuffer;
  std::vector<DDASReadout::ZeroCopyHit *> hits;
  for (int i = 0; i < 100; i++) {
    makeHit(pHit, 0, 1, 2, 3, i, i);
    hits.push_back(new DDASReadout::ZeroCopyHit(4, pHit, pBuffer, m_pArena));
    pHit += 4;
  }
  EQ(size_t(100), pBuffer->s_references); // Should be one ref per hit.

  // Shuffle the hits and release them one by one:

  std::random_device rd;
  std::default_random_engine rng(rd());
  std::shuffle(hits.begin(), hits.end(), rng);

  // Now delete all but one item.
  // - Buffer should have references.
  // - Buffer pool should still be empty.

  for (int i = 0; i < hits.size() - 1; i++) {
    delete hits[i];
    ASSERT(pBuffer->isReferenced());
    ASSERT(m_pArena->m_BufferPool.empty());
  }

  // The last one returns the buffer:

  delete hits[hits.size() - 1];

  ASSERT(!pBuffer->isReferenced());
  ASSERT(!m_pArena->m_BufferPool.empty());
  EQ(pBuffer, m_pArena->m_BufferPool.front());
}

/**
 * @brief If I recycle a hit in the same buffer arena/buffer, I still have
 * a reference count and the buffer was not returned.
 */
void zcopyhittest::recycle_1() {

  DDASReadout::ReferenceCountedBuffer *pBuf =
      m_pArena->allocate(sizeof(uint32_t) * 4 * 2); // Two hits.

  makeHit(*pBuf, 0, 1, 2, 3, 0, 0);
  makeHit((uint32_t *)(*pBuf) + 4, 0, 1, 2, 4, 1, 1);

  DDASReadout::ZeroCopyHit *pHit =
      new DDASReadout::ZeroCopyHit(4, (uint32_t *)(*pBuf), pBuf, m_pArena);

  pHit->setHit(4, (uint32_t *)(*pBuf) + 4, pBuf, m_pArena);

  // The buffer is still referenced and has not been returned to the pool:
  ASSERT(pBuf->isReferenced());
  ASSERT(m_pArena->m_BufferPool.empty());

  delete pHit;
}

/**
 * @brief We can recycle an empty hit. Recycling increments the buffer
 * reference counter. Destruction of the hit dereferences and releases it.
 */
void zcopyhittest::recycle_2() {
  DDASReadout::ReferenceCountedBuffer *pBuf =
      m_pArena->allocate(4 * sizeof(uint32_t));
  uint32_t *p = *pBuf;
  makeHit(p, 0, 1, 2, 3, 1234, 100);
  {
    DDASReadout::ZeroCopyHit hit; // Empty hit.
    hit.setHit(4, p, pBuf, m_pArena);

    EQ(size_t(1), pBuf->s_references);
    ASSERT(m_pArena->m_BufferPool.empty());
  }

  // Dereferenced and released:

  EQ(size_t(0), pBuf->s_references);
  ASSERT(!m_pArena->m_BufferPool.empty());
  EQ(pBuf, m_pArena->m_BufferPool.front());
}

/** @brief Freeing a hit releases its resources. */
void zcopyhittest::free_1() {
  DDASReadout::ReferenceCountedBuffer *pBuf =
      m_pArena->allocate(4 * sizeof(uint32_t));
  uint32_t *p = *pBuf;

  makeHit(p, 0, 1, 2, 3, 124, 100);
  DDASReadout::ZeroCopyHit hit(4, p, pBuf, m_pArena);

  hit.freeHit();
  ASSERT(!pBuf->isReferenced());
  ASSERT(!m_pArena->m_BufferPool.empty());
  EQ(pBuf, m_pArena->m_BufferPool.front());
}

/** @brief Free allows a new hit to be set properly. */
void zcopyhittest::free_2() {
  DDASReadout::ReferenceCountedBuffer &buf =
      *(m_pArena->allocate(3 * 4 * sizeof(uint32_t))); // 3 hits.

  uint32_t *p = buf;
  makeHit(p, 0, 1, 2, 3, 123, 100);
  makeHit(p + 4, 0, 1, 2, 4, 222, 123);
  makeHit(p + 8, 0, 1, 2, 6, 333, 321);

  DDASReadout::ZeroCopyHit h1(4, p, &buf, m_pArena);
  DDASReadout::ZeroCopyHit h2(4, p + 4, &buf, m_pArena);

  h1.freeHit(); // But the buffer is still referenced:

  ASSERT(buf.isReferenced());
  ASSERT(m_pArena->m_BufferPool.empty());

  h1.setHit(4, p + 8, &buf, m_pArena);
  EQ(size_t(2), buf.s_references);

  // This should free the buffer:

  h1.freeHit();
  h2.freeHit();

  ASSERT(!buf.isReferenced());
  ASSERT(!m_pArena->m_BufferPool.empty());
  EQ(&buf, m_pArena->m_BufferPool.front());
}
