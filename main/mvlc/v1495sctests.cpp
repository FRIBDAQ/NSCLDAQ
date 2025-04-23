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

    Rudimentary tests for the v1495 logic module with scaler firmware.
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "CVMUSB.h"
#include "CV1495sc.h"
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

class V1495scTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(V1495scTests);
    CPPUNIT_TEST(command_1);   // the 'v1495sc' command is registered.
    CPPUNIT_TEST(create_1);    // Can script create an object.
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();
    void create_1();

public:
    void setUp() {
        char nameTemplate[100];
         strcpy(nameTemplate, "v1495sc.tclXXXXXX");
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
    int              m_fd;
    std::string      m_filename;
    TCLConfigParser* m_parser;

};
CPPUNIT_TEST_SUITE_REGISTRATION(V1495scTests);

void V1495scTests::command_1() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(pInterp, "v1495sc", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);
}

void V1495scTests::create_1() {
    {
        std::ofstream script(m_filename);
        script << "v1495sc create scaler -base 0x12340000\n";
        script << "stack create scalers -trigger scaler -modules scaler\n";
    }

    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    auto pModule = m_parser->findDevice("scaler");
    CPPUNIT_ASSERT(pModule);
    auto pDriver = pModule->getDriver();
    CPPUNIT_ASSERT(pDriver);
    CPPUNIT_ASSERT(dynamic_cast<CV1495sc*>(pDriver));
}