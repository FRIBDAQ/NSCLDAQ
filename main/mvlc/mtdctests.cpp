/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321

    Rudimentary tests for the mtdc module
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "CVMUSB.h"
#include "CMADCScaler.h"
#include "CReadoutModule.h"
#include "MVLCConfigParser.h"
#include "CStack.h"
#include "CMTDC32.h"
#include <TCLInterpreter.h>
#include <TCLException.h>
#include <tcl.h>
#include <string>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>

class MtdcTests: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(MtdcTests);
    CPPUNIT_TEST(command_1);
    CPPUNIT_TEST(stack_1);
    CPPUNIT_TEST(read_1);   // readout list...init is too complex.
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();
    void stack_1();
    void read_1();
public:
    void setUp() {
         // Make a temp scipt file:

         char nameTemplate[100];
         strcpy(nameTemplate, "mtdc.tclXXXXXX");
         m_fd = mkstemp(nameTemplate);
         m_filename = nameTemplate;
 
         m_parser = new MVLCConfigParser(m_filename);
         m_parser->initialize();                   // Register extensions.
    }
    void tearDown() {
        delete m_parser;
        close(m_fd);
        unlink(m_filename.c_str());
    }
private:
    int m_fd;
    std::string m_filename;
    TCLConfigParser* m_parser;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MtdcTests);

// mtdc is registered:

void
MtdcTests::command_1() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();

    auto token = Tcl_FindCommand(pInterp, "mtdc", nullptr, TCL_GLOBAL_ONLY);

    CPPUNIT_ASSERT(token);
}

// We can create a TDC and add it to a stack via script.

void 
MtdcTests::stack_1() {
    {
        std::ofstream script(m_filename);
        script << "mtdc create tdc -base 0x12000000\n";
        script << "stack create event -trigger nim1 -modules tdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    // Can find the module -- assume the stack worked because it's been testsed.

    CReadoutModule* pModule = m_parser->findDevice("tdc");
    CPPUNIT_ASSERT(pModule);

    CMTDC32* pTdc = reinterpret_cast<CMTDC32*>(pModule->getDriver());
    CPPUNIT_ASSERT(pTdc);
}


// addReadoutList does the right stuff.
void
MtdcTests::read_1() {
    {
        std::ofstream script(m_filename);
        script << "mtdc create tdc -base 0x12000000\n";
        script << "stack create event -trigger nim1 -modules tdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    CStack *pStack = m_parser->getEventStack();
    CPPUNIT_ASSERT(pStack);

    CVMUSBReadoutList list;
    pStack->addReadoutList(list);

    auto  ops = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(2), ops.size());

    auto rd = ops.at(0);
    auto clear = ops.at(1);

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_block_read 0xb 1024 0x12000000"), rd
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12006034 0x1"), clear
    );
}

