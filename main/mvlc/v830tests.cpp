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

    Minimal tests fo the v830 driver.
*/

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "CVMUSB.h"
#include "CStack.h"
#include "C830.h"
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


class V830Tests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(V830Tests);
    CPPUNIT_TEST(command_1);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(init_1);
    CPPUNIT_TEST(readout_1);
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();
    void create_1();
    void init_1();
    void readout_1();
public:
    void setUp() {
        // Make a temp script file:

        char nameTemplate[100];
        strcpy(nameTemplate, "v830.tclXXXXXX");
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

CPPUNIT_TEST_SUITE_REGISTRATION(V830Tests);


// ensure command was registered:

void V830Tests::command_1() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(pInterp, "v830", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);
}
// scripts can create one.

void V830Tests::create_1() {
    {
        std::ofstream script(m_filename);
        script << "v830 create scaler -base 0x12340000\n";
        script << "stack create scalers -modules scaler\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    auto module = m_parser->findDevice("scaler");
    CPPUNIT_ASSERT(module);
    
    auto driver = dynamic_cast<C830*>(module->getDriver());
    CPPUNIT_ASSERT(driver);
    
}

// initialization looks reasonable

void V830Tests::init_1() {
    {
        std::ofstream script(m_filename);
        script << "v830 create scaler -base 0x12340000\n";
        script << "stack create scalers  -trigger scaler -modules scaler\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CStack* s = m_parser->getScalerStack();
    CPPUNIT_ASSERT(s);

    CVMUSB controller;
    s->Initialize(controller);

    auto ops = controller.getRecordedOperations();

    CPPUNIT_ASSERT_EQUAL(size_t(7), ops.size());

    // first one is a reset write:

    auto reset = ops.front();
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12341120 0x0"),
        reset
    );
    // last one sets the ipl to 0 (default value).

    auto setipl = ops.back();
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12341112 0x0"),
        setipl
    );
}
// readout list :

void V830Tests::readout_1() {
    {
        std::ofstream script(m_filename);
        script << "v830 create scaler -base 0x12340000\n";
        script << "stack create scalers  -trigger scaler -modules scaler\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CStack* s = m_parser->getScalerStack();
    CPPUNIT_ASSERT(s);

    CVMUSBReadoutList list;
    s->addReadoutList(list);

    auto ops = list.dumpForMvlc();

    // List is latch operation + 32 reads + clear op 34 operations.

    CPPUNIT_ASSERT_EQUAL(size_t(34), ops.size());

    auto latch = ops.front();

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12341124 0x0"),
        latch
    );

    auto clear = ops.back();
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12341122 0x0"),
        clear
    );
}

