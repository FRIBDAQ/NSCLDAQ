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
#include "C830.h"
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
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();
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