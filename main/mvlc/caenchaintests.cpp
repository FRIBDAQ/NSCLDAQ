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
#include "CCAENChain.h"
#include "C785.h"     // Don't actually know if I'll need this.

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


class CAENChainTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CAENChainTests);
    CPPUNIT_TEST(command_1);     // caenchain was registered.
    
    CPPUNIT_TEST(chain_1);       // Two modules both ok.
    CPPUNIT_TEST(chain_2);       // three modules all ok.
    CPPUNIT_TEST(chain_3);       // THree modules one nonexistent.
    CPPUNIT_TEST(chain_4);       // THre modules one, not a C785.

    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();
    void chain_1();
    void chain_2();
    void chain_3();
    void chain_4();

public:
    // The usual setup/teardown for this sort of test
    void setUp() {
            // Make a temp scipt file:

        char nameTemplate[100];
        strcpy(nameTemplate, "caenchain.tclXXXXXX");
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
CPPUNIT_TEST_SUITE_REGISTRATION(CAENChainTests);

///////////////////// Command registration:

void
CAENChainTests::command_1() {
    auto interp = m_parser->getInterpreter()->getInterpreter();  // Tcl_Interp*.
    auto token = Tcl_FindCommand(interp, "caenchain", nullptr, TCL_GLOBAL_ONLY);

    CPPUNIT_ASSERT(token);
}
////////////////////// tests for validating chains.

void
CAENChainTests::chain_1() {
    // Make the script:

    {
        std::ofstream script(m_filename);
        script << "adc create one -base 0x10000000 -geo 2\n";
        script << "adc create two -base 0x20000000 -geo 3\n";
        script << "adc create three -base 0x40000000 -geo 4\n";
        script << "caenchain create chain -base 0xff000000 -modules [list one two]\n"; // minimal chain.
        script << "stack create event -trigger nim1 -modules chain\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

}

void
CAENChainTests::chain_2() {
    {
        std::ofstream script(m_filename);
        script << "adc create one -base 0x10000000 -geo 2\n";
        script << "adc create two -base 0x20000000 -geo 3\n";
        script << "adc create three -base 0x40000000 -geo 4\n";
        script << "caenchain create chain -base 0xff000000 -modules [list one two three]\n"; // middle module.
        script << "stack create event -trigger nim1 -modules chain\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
}
void
CAENChainTests::chain_3() {
    {
        std::ofstream script(m_filename);
        script << "adc create one -base 0x10000000 -geo 2\n";
        script << "adc create two -base 0x20000000 -geo 3\n";
        script << "adc create three -base 0x40000000 -geo 4\n";
        script << "caenchain create chain -base 0xff000000 -modules [list one nosuch]\n"; // minimal chain.
        script << "stack create event -trigger nim1 -modules chain\n";
    }
    CPPUNIT_ASSERT_THROW(
        (*m_parser)(), CTCLException
    );
}
void
CAENChainTests::chain_4() {
    {
        std::ofstream script(m_filename);
        script << "adc create one -base 0x10000000 -geo 2\n";
        script << "adc create two -base 0x20000000 -geo 3\n";
        script << "madc create three -base 0x40000000 -geo 4\n";
        script << "caenchain create chain -base 0xff000000 -modules [list one two three]\n"; // three isn't a valid module.
        script << "stack create event -trigger nim1 -modules chain\n";
    }
    CPPUNIT_ASSERT_THROW(
        (*m_parser)(), CTCLException
    );
}