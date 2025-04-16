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

    Rudimentary tests for the caenchain driver.
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "CVMUSB.h"
#include "CMADCScaler.h"

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
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();
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