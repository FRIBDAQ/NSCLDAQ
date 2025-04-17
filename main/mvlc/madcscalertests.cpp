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

    Rudimentary tests for the madc scaler module
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "CVMUSB.h"
#include "CMADCScaler.h"
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

// Some useful constants snatched from CMADCScaler.cpp

static const uint8_t amod(CVMUSBReadoutList::a32UserData);

static const int daq_time_lo(0x60a0);
static const int daq_time_hi(0x60a2);

static const int time_0(0x60a8);
static const int time_1(0x60aa);

static const int time_reset(0x6090);



class MADCScalerTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(MADCScalerTests);
    CPPUNIT_TEST(command_1);    // THe command was registered.
    CPPUNIT_TEST(create_1);     // Can cretae one and add it to a stack.
    CPPUNIT_TEST(init_1);      // Initialization is correct (clear scaler).
    CPPUNIT_TEST(readout_1);
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();
    void create_1();
    void init_1();
    void readout_1();
public:
    void setUp() {
        // Make a temp scipt file:

        char nameTemplate[100];
        strcpy(nameTemplate, "madcscaler.tclXXXXXX");
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

CPPUNIT_TEST_SUITE_REGISTRATION(MADCScalerTests);


void
MADCScalerTests::command_1() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(pInterp, "madcscaler", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);
}

void 
MADCScalerTests::create_1() {
    {
        std::ofstream script(m_filename);
        script << "madcscaler create scaler -base 0x12000000\n";
        script << "stack create sc -trigger scaler -modules scaler\n";
    }

    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CStack* scaler = m_parser->getScalerStack();
    CPPUNIT_ASSERT(scaler);

    auto madcModule = m_parser->findDevice("scaler");
    CPPUNIT_ASSERT(madcModule);

    // The encapsulated driver must be a CMADCScaler.

    CMADCScaler* madc = dynamic_cast<CMADCScaler*>(madcModule->getDriver());
    CPPUNIT_ASSERT(madc);
}
void
MADCScalerTests::init_1() {
    {
        std::ofstream script(m_filename);
        script << "madcscaler create scaler -base 0x12000000\n";
        script << "stack create sc -trigger scaler -modules scaler\n";
    }

    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CStack* scaler = m_parser->getScalerStack();
    CPPUNIT_ASSERT(scaler);

    CVMUSB controller;
    scaler->Initialize(controller);

    auto ops = controller.getRecordedOperations();

    // One operation


    CPPUNIT_ASSERT_EQUAL(size_t(1), ops.size());
    auto init = ops.at(0);

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12006090 0x2"), init
    );
}

void
MADCScalerTests::readout_1() {
    {
        std::ofstream script(m_filename);
        script << "madcscaler create scaler -base 0x12000000\n";
        script << "stack create sc -trigger scaler -modules scaler\n";
    }

    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CStack* scaler = m_parser->getScalerStack();
    CPPUNIT_ASSERT(scaler);

    CVMUSBReadoutList list;
    scaler->addReadoutList(list);
    auto ops = list.dumpForMvlc();

    CPPUNIT_ASSERT_EQUAL(size_t(5), ops.size());

    // first reads the 16 bits of daq_time_lo last is a reset:

    auto rddaqlo = ops.at(0);
    auto reset   = ops.at(4);

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_read_mem 0x9 d16 0x120060a0"),
        rddaqlo
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12006090 0x2"), reset
    );
    
}