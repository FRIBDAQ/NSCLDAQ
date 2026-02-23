// Template for a test suite.
#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Asserts.h"
#include "ReferenceCountedBuffer.h"

class RefCBTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RefCBTests);
  CPPUNIT_TEST(construction);
  CPPUNIT_TEST(ref_1);
  CPPUNIT_TEST(deref_1);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}
  void tearDown() {}

protected:
  void construction();
  void ref_1();
  void deref_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RefCBTests);

void RefCBTests::construction() {
  DDASReadout::ReferenceCountedBuffer b(100);
  EQ(size_t(100), b.s_size);
  EQ(size_t(0), b.s_references);
  ASSERT(b.s_pData);
  ASSERT(!b.isReferenced());
}

void RefCBTests::ref_1() {
  DDASReadout::ReferenceCountedBuffer b(100);
  b.reference();
  EQ(size_t(1), b.s_references);
  ASSERT(b.isReferenced());

  b.reference();
  EQ(size_t(2), b.s_references);
  ASSERT(b.isReferenced());

  b.s_references = 0; // Else throws.
}

void RefCBTests::deref_1() {
  DDASReadout::ReferenceCountedBuffer b(100);
  b.reference();
  b.dereference();
  ASSERT(!b.isReferenced());

  b.reference();
  b.reference();
  b.dereference();
  b.reference();
  b.dereference();
  b.dereference();

  ASSERT(!b.isReferenced());
}