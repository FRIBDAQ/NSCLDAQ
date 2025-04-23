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

    Minimal tests fo the hytec 2530 driver
*/

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "CReadoutModule.h"
#include "CVMUSB.h"
#include "CMADC32.h"
#include "CNADC2530.h"
#include "MVLCConfigParser.h"
#include "CStack.h"
#include <TCLInterpreter.h>
#include <TCLException.h>
#include <tcl.h>
#include <string>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>

class HytecTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(HytecTests);
    CPPUNIT_TEST(command_1);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(init_1);
    CPPUNIT_TEST(readout_1);
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();     // The hytec command is registered.
    void create_1();     // Can make device via command.
    void init_1();        // 12 initialization operations.
    void readout_1();   // Readout list checks.
public:
    void setUp() {
        // Make a temp script file:

        char nameTemplate[100];
        strcpy(nameTemplate, ".tclXXXXXX");
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
CPPUNIT_TEST_SUITE_REGISTRATION(HytecTests);

// 'hytec' is a command.

void
HytecTests::command_1() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(pInterp, "hytec", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);
}

// 'hytec' can create devices.

void
HytecTests::create_1() {
    {
        std::ofstream script(m_filename);
        script << "hytec create nadc -csr 0x120000 -memory 0xff000000\n";   // a24 regs, a32 memory.
        script << "stack create evetn -trigger nim1 -modules nadc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );


    // SHould be able to find the module and its driver is a CNADC2530 instance:

    auto pModule = m_parser->findDevice("nadc");
    CPPUNIT_ASSERT(pModule);

    auto pDevice = pModule->getDriver();
    CPPUNIT_ASSERT(pDevice);
    CPPUNIT_ASSERT(dynamic_cast<CNADC2530*>(pDevice));
}
// initialization.

void
HytecTests::init_1(){
    {
        std::ofstream script(m_filename);
        script << "hytec create nadc -csr 0x120000 -memory 0xff000000\n";   // a24 regs, a32 memory.
        script << "stack create evetn -trigger nim1 -modules nadc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );


    CStack* pStack = m_parser->getEventStack();
    CPPUNIT_ASSERT(pStack);                      // We did make one.

    CVMUSB controller;
    pStack->Initialize(controller);

    auto ops = controller.getRecordedOperations();

    CPPUNIT_ASSERT_EQUAL(size_t(12), ops.size());
}

// Readout list.

void
HytecTests::readout_1() {
    {
        std::ofstream script(m_filename);
        script << "hytec create nadc -csr 0x120000 -memory 0xff000000\n";   // a24 regs, a32 memory.
        script << "stack create evetn -trigger nim1 -modules nadc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );


    CStack* pStack = m_parser->getEventStack();
    CPPUNIT_ASSERT(pStack);                      // We did make one.

    CVMUSBReadoutList list;
    pStack->addReadoutList(list);

    auto ops = list.dumpForMvlc();

    // LIst size:  Note masked count read is 2 list elements.

    CPPUNIT_ASSERT_EQUAL(size_t(9), ops.size());
}