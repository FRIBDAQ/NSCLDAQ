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
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();     // The hytec command is registered.
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