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

    Minimal tests fo the CAEN V1729 driver
*/

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "CVMUSB.h"
#include "CReadoutModule.h"
#include "CV1729.h"

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



class v1729Tests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(v1729Tests);
    CPPUNIT_TEST(command_1);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(init_1);
    CPPUNIT_TEST(read_1);
    CPPUNIT_TEST(read_2);
    CPPUNIT_TEST_SUITE_END();
protected:
    void command_1();     // Command got registered.
    void create_1();      // Can create one.
    void init_1();        // Last init op turns on DAQ.
    void read_1();        // 0 delay case does not insert a wait.
    void read_2();        // a nonzero delay adds a delay to the front of the list.
public:
    void setUp() {
        // Make a temp script file:

        char nameTemplate[100];
        strcpy(nameTemplate, "v1729.tclXXXXXX");
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
CPPUNIT_TEST_SUITE_REGISTRATION(v1729Tests);

void v1729Tests::command_1() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(pInterp, "v1729a", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);
}

void v1729Tests::create_1() {
    {
        std::ofstream script(m_filename);
        script << "v1729a create wf -base 0x12340000\n";
        script << "stack create event -trigger nim1 -modules wf\n";  // We'll want htis in later tests.
    }

    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    auto pModule = m_parser->findDevice("wf");
    CPPUNIT_ASSERT(pModule);
    auto pDriver = pModule->getDriver();
    CPPUNIT_ASSERT(pDriver);
    CPPUNIT_ASSERT(dynamic_cast<CV1729*>(pDriver));   // driver is a v1729 driver.
}

void v1729Tests::init_1() {
    {
        std::ofstream script(m_filename);
        script << "v1729a create wf -base 0x12340000\n";
        script << "stack create event -trigger nim1 -modules wf\n";  // We'll want htis in later tests.
    }

    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    CStack* pEvents = m_parser->getEventStack();
    CPPUNIT_ASSERT(pEvents);

    CVMUSB controller;
    pEvents->Initialize(controller);

    auto ops = controller.getRecordedOperations();

    auto enable = ops.back();

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12341700 0x0"), enable
    );
}

void v1729Tests::read_1() {
    {
        std::ofstream script(m_filename);
        script << "v1729a create wf -base 0x12340000\n";
        script << "stack create event -trigger nim1 -modules wf\n";  // We'll want htis in later tests.
    }

    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    CStack* pEvents = m_parser->getEventStack();
    CPPUNIT_ASSERT(pEvents);

    CVMUSBReadoutList list;
    pEvents->addReadoutList(list);

    auto ops = list.dumpForMvlc();   // with no delay ony 5 operations.

    CPPUNIT_ASSERT_EQUAL(size_t(5), ops.size());

    // The last one restarts the DAQ:

    auto enable = ops.back();

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12341700 0x1"), enable
    );
}
void v1729Tests::read_2() {
    {
        std::ofstream script(m_filename);
        script << "v1729a create wf -base 0x12340000 -delay 1\n";
        script << "stack create event -trigger nim1 -modules wf\n";  // We'll want htis in later tests.
    }

    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    CStack* pEvents = m_parser->getEventStack();
    CPPUNIT_ASSERT(pEvents);

    CVMUSBReadoutList list;
    pEvents->addReadoutList(list);

    auto ops = list.dumpForMvlc();   // with delay 6 operations.

    CPPUNIT_ASSERT_EQUAL(size_t(6), ops.size());

    // The last one restarts the DAQ:

    auto enable = ops.back();

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12341700 0x1"), enable
    );

    // First list item is delay of 1usec - 17 ticks due to tyhe +1.


    auto  delay = ops.at(0);
    CPPUNIT_ASSERT_EQUAL(
        std::string("wait 17"), delay
    );
}