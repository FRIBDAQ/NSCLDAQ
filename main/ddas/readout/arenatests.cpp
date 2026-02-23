#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Asserts.h"
#define private public
#include "BufferArena.h"
#include "ReferenceCountedBuffer.h"
#undef private

class arenaTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(arenaTest);
  CPPUNIT_TEST(initial_1);
  CPPUNIT_TEST(alloc_1);
  CPPUNIT_TEST(alloc_2);
  CPPUNIT_TEST(alloc_3);
  CPPUNIT_TEST(alloc_4);
  CPPUNIT_TEST(alloc_5);
  CPPUNIT_TEST(alloc_6);
  CPPUNIT_TEST_SUITE_END();

private:
  DDASReadout::BufferArena *m_pTestObj;

public:
  void setUp() { m_pTestObj = new DDASReadout::BufferArena; }

  void tearDown() { delete m_pTestObj; }

protected:
  void initial_1();
  void alloc_1();
  void alloc_2();
  void alloc_3();
  void alloc_4();
  void alloc_5();
  void alloc_6();
};

CPPUNIT_TEST_SUITE_REGISTRATION(arenaTest);

/** @brief We start with an empty buffer pool. */
void arenaTest::initial_1() { ASSERT(m_pTestObj->m_BufferPool.empty()); }

/**
 * @brief Allocation doesn't add to the buffer pool.  We get a buffer of
 * exactly the requested size and no reference counts.
 */
void arenaTest::alloc_1() {
  DDASReadout::ReferenceCountedBuffer *pBuffer = m_pTestObj->allocate(100);
  EQ(size_t(100), pBuffer->s_size);
  ASSERT(!pBuffer->isReferenced());
  ASSERT(pBuffer->s_pData);

  // Should just be able to delete it:

  CPPUNIT_ASSERT_NO_THROW(delete pBuffer);
}

/**
 * @brief Allocation and freeing is ok -- if I never referenced. Freeing puts
 * the buffer into the free buffer queue.
 */
void arenaTest::alloc_2() {
  DDASReadout::ReferenceCountedBuffer *pBuffer = m_pTestObj->allocate(100);
  CPPUNIT_ASSERT_NO_THROW(m_pTestObj->free(pBuffer));

  EQ(size_t(1), m_pTestObj->m_BufferPool.size());
  EQ(pBuffer, m_pTestObj->m_BufferPool.front());
}

/** @brief Freeing a referenced buffer is a std::logic_error. */
void arenaTest::alloc_3() {
  DDASReadout::ReferenceCountedBuffer *pBuffer = m_pTestObj->allocate(100);
  pBuffer->reference();
  CPPUNIT_ASSERT_THROW(m_pTestObj->free(pBuffer), std::logic_error);

  pBuffer->dereference(); // Now I can free it.
  CPPUNIT_ASSERT_NO_THROW(m_pTestObj->free(pBuffer));
}

/** @brief Buffers get re-used if they are big enough. */
void arenaTest::alloc_4() {
  DDASReadout::ReferenceCountedBuffer *pBuffer = m_pTestObj->allocate(100);
  m_pTestObj->free(pBuffer);

  DDASReadout::ReferenceCountedBuffer *pReused = m_pTestObj->allocate(100);
  EQ(pBuffer, pReused);
  m_pTestObj->free(pReused);
}

/**
 * @brief Re-use happens as long as the buffer is _at_least_ as big as
 * the request.
 */
void arenaTest::alloc_5() {
  DDASReadout::ReferenceCountedBuffer *pBuffer = m_pTestObj->allocate(100);
  m_pTestObj->free(pBuffer);

  DDASReadout::ReferenceCountedBuffer *pReused = m_pTestObj->allocate(50);
  EQ(pBuffer, pReused);
  m_pTestObj->free(pReused);
}

/**
 * @brief Reallocation of the front is used if needed to get the right size.
 * This test will force the reallocation.
 */
void arenaTest::alloc_6() {
  DDASReadout::ReferenceCountedBuffer *pBuf1 = m_pTestObj->allocate(100);
  DDASReadout::ReferenceCountedBuffer *pBuf2 = m_pTestObj->allocate(500);
  DDASReadout::ReferenceCountedBuffer *pBuf3 = m_pTestObj->allocate(100);

  // We'll make the 500 be bracked in the free list by the two 100s':

  m_pTestObj->free(pBuf1);
  m_pTestObj->free(pBuf2);
  m_pTestObj->free(pBuf3);

  DDASReadout::ReferenceCountedBuffer *pReused = m_pTestObj->allocate(300);
  EQ(pBuf1, pReused);
}
