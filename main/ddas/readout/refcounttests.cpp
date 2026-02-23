#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Asserts.h"
#define private public
#include "ReferenceCountedBuffer.h"
#undef private

class refcountedTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(refcountedTest);
  CPPUNIT_TEST(initial_1);
  CPPUNIT_TEST(refcount_1);
  CPPUNIT_TEST(refcount_2);
  CPPUNIT_TEST_SUITE_END();

private:
  DDASReadout::ReferenceCountedBuffer *m_pTestObj;

public:
  /** @brief Create a reference-counted buffer object with some storage. */
  void setUp() { m_pTestObj = new DDASReadout::ReferenceCountedBuffer(100); }

  /**
   * @brief Its an error to delete with reference counts, so set to 0 first.
   */
  void tearDown() {
    m_pTestObj->s_references = 0;
    delete m_pTestObj;
  }

protected:
  void initial_1();
  void refcount_1();
  void refcount_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(refcountedTest);

/** @brief Good initial state. */
void refcountedTest::initial_1() {
  // Initial state matches:

  // White box test:

  EQ(size_t(100), m_pTestObj->s_size);
  EQ(size_t(0), m_pTestObj->s_references);
  ASSERT(m_pTestObj->s_pData);

  // Black box testing:

  ASSERT(!m_pTestObj->isReferenced());
  uint8_t *p8 = *m_pTestObj;
  uint16_t *p16 = *m_pTestObj;
  uint32_t *p32 = *m_pTestObj;
  EQ((uint8_t *)(m_pTestObj->s_pData), p8);
  EQ((uint16_t *)(m_pTestObj->s_pData), p16);
  EQ((uint32_t *)(m_pTestObj->s_pData), p32);
}

/**
 * @brief Incrementing the reference count will:
 * - Increment s_references,
 * - Make the item referenced.
 */
void refcountedTest::refcount_1() {
  m_pTestObj->reference();
  EQ(size_t(1), m_pTestObj->s_references);
  ASSERT(m_pTestObj->isReferenced());
}

/** @brief Referencing and de-referening leads to unreferenced object. */
void refcountedTest::refcount_2() {
  m_pTestObj->reference();
  m_pTestObj->dereference();
  EQ(size_t(0), m_pTestObj->s_references);
  ASSERT(!m_pTestObj->isReferenced());
}
