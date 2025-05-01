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

    Rudimentary tests for the madc32 driver.
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "CMADC32.h"
#include "MVLCConfigParser.h"
#include "CStack.h"
#include <TCLInterpreter.h>
#include <tcl.h>
#include <string>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>



class MADC32Tests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(MADC32Tests);
    CPPUNIT_TEST(command_1);
    CPPUNIT_TEST(read_1);
    CPPUNIT_TEST_SUITE_END();

protected:              // Tests.
    void command_1();    // commmand is registerd.
    void read_1();       // read operation is corect.

public:
    void setUp() {
     // Make a temp scipt file:

     char nameTemplate[100];
     strcpy(nameTemplate, "madc32.tclXXXXXX");
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

void
MADC32Tests::command_1() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(pInterp, "madc", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);

}

void
MADC32Tests::read_1() {
    {
        std::ofstream script(m_filename);
        script << "madc create adc -base 0x12340000\n";
        script << "stack create e -trigger nim1 -modules adc\n";
    }

    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CStack* p = m_parser->getEventStack();
    CPPUNIT_ASSERT(p);

    CVMUSBReadoutList list;
    p->addReadoutList(list);

    auto ops = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(2), ops.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_block_read 0xb 45 0x12340000"),
        ops.at(0)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12346034 0x1"),
        ops.at(1)
    );

}

CPPUNIT_TEST_SUITE_REGISTRATION(MADC32Tests);