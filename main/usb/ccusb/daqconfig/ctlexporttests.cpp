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

/** @file:  
 *  @brief: 
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CConfiguration.h"
#include <tcl.h>
#include "tclUtil.h"
#include "CMockCCUSB.h"
#include <TCLInterpreter.h>

class aTestSuite : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(aTestSuite);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
    }
    void tearDown() {
        
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(aTestSuite);

void aTestSuite::test_1()
{
    CMockCCUSB controller;
    CConfiguration config;
    config.exportController(&controller);
    
    // Get the expected swith name:
    
    std::string sb = tclUtil::swigPointer(&controller, "CCCUSB");
    
    // Get the actual -- they should match:
    
    Tcl_Interp* pRaw = config.getInterpreter()->getInterpreter();
    
    const char* actual = Tcl_GetVar(pRaw, "::Globals::aController", 0);
    ASSERT(actual);
    EQ(sb, std::string(actual));
}