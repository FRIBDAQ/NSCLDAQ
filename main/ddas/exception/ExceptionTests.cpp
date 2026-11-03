/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
         Aaron Chester
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/

#include "Asserts.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "CXIAException.h"

class ExceptionTests : public CppUnit::TestFixture {

public:
  CPPUNIT_TEST_SUITE(ExceptionTests);
  CPPUNIT_TEST(xia_success);
  CPPUNIT_TEST(format_terminated);
  CPPUNIT_TEST(format_embedded_terminator);
  CPPUNIT_TEST(format_full_buffer);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp(){};
  void tearDown(){};

  void xia_success();
  void format_terminated();
  void format_embedded_terminator();
  void format_full_buffer();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ExceptionTests);

/**
 * @brief Check the return code and text for CXIAException.
 * @note (ASC 3/12/24): XIA exception tests assume that a return code of 0
 * will result in a context message 'success.'
 */
void ExceptionTests::xia_success() {
  CXIAException e("This is a test", "myFunction", 0);
  EQ(0, e.ReasonCode());
  std::string expected(
      "This is a test XIA API Error: myFunction returned 0 with "
      "reason text 'success'");
  // Checking with C-style strings is much more annoying...
  EQ(expected, std::string(e.ReasonText()));
}

/**
 * @brief A normal, short NUL-terminated buffer formats as expected.
 */
void ExceptionTests::format_terminated() {
  std::string reason("success"); // NUL-terminated by std::string.
  std::string expected(CXIAException::formatReason(
      "This is a test", "myFunction", 0, reason.c_str()));
  EQ(std::string("This is a test XIA API Error: myFunction returned 0 with "
                 "reason text 'success'"),
     expected);
}

/**
 * @brief A buffer with an embedded terminator is truncated there: trailing
 * zero-fill does not leak into the message.
 */
void ExceptionTests::format_embedded_terminator() {
  char buf[kXIABUFSIZE] = {};   // Zero-initialized, as in the constructor.
  std::memcpy(buf, "short", 5); // Remaining bytes stay NUL.
  std::string expected(
      CXIAException::formatReason("This is a test", "f", 1, buf));
  EQ(std::string(
         "This is a test XIA API Error: f returned 1 with reason text 'short'"),
     expected);
}

/**
 * @brief Worst case: the XIA API fills every writable byte without terminating.
 * @details
 * The constructor zero-initializes buf and passes kXIABUFSIZE - 1, so the final
 * byte always stays a terminator. Confirm the reason text is exactly size - 1
 * characters with no overrun.
 */
void ExceptionTests::format_full_buffer() {
  char buf[kXIABUFSIZE] = {};
  std::memset(buf, 'X', kXIABUFSIZE - 1); // buf[kXIABUFSIZE - 1] stays '\0'.
  std::string expected(
      "This is a test XIA API Error: f returned 2 with reason text '" +
      std::string(buf) + "'");
  EQ(expected, CXIAException::formatReason("This is a test", "f", 2, buf));
}
