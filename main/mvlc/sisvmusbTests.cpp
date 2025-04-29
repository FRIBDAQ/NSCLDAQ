/**
 *  @file sisvmusbTests.cpp
 *  @brief tests for sis_vmusb_interface.
 *  @author Ron Fox<rfoxkendo@gmail.com>
 * 
 *  
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
 */

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "CVMUSB.h"
#define private public    /* See into the private members of the class */
#include "sis_vmusb_interface.h"    
#undef privates

class sisvmusbTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(sisvmusbTests);
    CPPUNIT_TEST(mustopen_1);
    CPPUNIT_TEST(mustopen_2);
    CPPUNIT_TEST_SUITE_END();

protected:
    void mustopen_1();
    void mustopen_2();

    // I don't have setup/teardown requirements.
public:
    void setUp() {}
    void tearDown() {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(sisvmusbTests);

void
sisvmusbTests::mustopen_1() {
    // If not open, get_vmeopen_messages fails.

    sis_vmusb_interface sis;
    unsigned int found; 
    char msg[100];
    int status = sis.get_vmeopen_messages(msg, &found);
    CPPUNIT_ASSERT_EQUAL(-1, status);
    CPPUNIT_ASSERT_EQUAL(unsigned(0), found);
}

void
sisvmusbTests::mustopen_2() {
    // Actual operations won't work if not open:

    sis_vmusb_interface sis;
    int status = sis.vme_A32D32_write(0, 0);
    CPPUNIT_ASSERT_EQUAL(-1, status);
}