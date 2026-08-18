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

#include <cppunit/extensions/HelperMacros.h>

#include "CXIAException.h"

class ExceptionTests : public CppUnit::TestFixture {

public:
  CPPUNIT_TEST_SUITE(ExceptionTests);
  CPPUNIT_TEST(xia_success);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {};
  void tearDown() {};

  void xia_success();
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
  std::string msg("This is a test XIA API Error: myFunction returned 0 with "
                  "reason text 'success'");
  // Checking with C-style strings is much more annoying...
  EQ(msg, std::string(e.ReasonText()));
}
