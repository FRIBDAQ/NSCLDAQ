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

    Minimal tests fo the madcchain driver.
*/

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "CVMUSB.h"
#include "CV977.h"
#include "CReadoutModule.h"
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

class V977Tests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(V977Tests);
    CPPUNIT_TEST(command_1);   // "v977" is registered.
    CPPUNIT_TEST(create_1);    // Can use "v977" to create  a module.
    CPPUNIT_TEST(init_1);     // number of initialization ops is correct.
    CPPUNIT_TEST(read_1);    // Readout list is correctd.
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();
    void create_1();
    void init_1();
    void read_1();

public:
    void setUp() {
        // Make a temp script file:

        char nameTemplate[100];
        strcpy(nameTemplate, "v977.tclXXXXXX");
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
CPPUNIT_TEST_SUITE_REGISTRATION(V977Tests);

void
V977Tests::command_1() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();
    auto token  = Tcl_FindCommand(pInterp, "v977", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);
}

void V977Tests::create_1() {
    {
        std::ofstream script(m_filename);
        script << "v977 create input -base 0x80000000\n";
        script << "stack create event -trigger nim1 -modules input\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    auto pModule = m_parser->findDevice("input");
    CPPUNIT_ASSERT(pModule);
    auto pDriver = pModule->getDriver();
    CPPUNIT_ASSERT(pDriver);
    CPPUNIT_ASSERT(dynamic_cast<CV977*>(pDriver));
}

void V977Tests::init_1() {
    {
        std::ofstream script(m_filename);
        script << "v977 create input -base 0x80000000\n";
        script << "stack create event -trigger nim1 -modules input\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    CStack* pEvent = m_parser->getEventStack();
    CVMUSB controller;
    pEvent->Initialize(controller);

    auto ops = controller.getRecordedOperations();
    CPPUNIT_ASSERT_EQUAL(size_t(6), ops.size());
}

// Note that by default it is singlehit read and clear.

void V977Tests::read_1() {
    {
        std::ofstream script(m_filename);
        script << "v977 create input -base 0x80000000\n";
        script << "stack create event -trigger nim1 -modules input\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    CStack* pEvent = m_parser->getEventStack();
    CVMUSBReadoutList list;
    pEvent->addReadoutList(list);

    auto ops = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), ops.size());

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_read_mem 0x9 d16 0x80000016"),
        ops.at(0)
    );
}