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
#include "CMADC32.h"
#include "CMADCChain.h"
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

class MadcChainTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(MadcChainTests);
    CPPUNIT_TEST(command_1);
    CPPUNIT_TEST(chain_1);  // three valid modules.
    CPPUNIT_TEST(chain_2);  // three modules, one does not exist.
    CPPUNIT_TEST(chain_3);  // 3 modules but one is not an madc.
    CPPUNIT_TEST(read_1);
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();

    void chain_1();
    void chain_2();
    void chain_3();

    void read_1();

public:
    void setUp() {
        // Make a temp script file:

        char nameTemplate[100];
        strcpy(nameTemplate, "madcchain.tclXXXXXX");
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
CPPUNIT_TEST_SUITE_REGISTRATION(MadcChainTests);

/** "madcchain is a command. " */

void
MadcChainTests::command_1() {
    auto pInterp = m_parser->getInterpreter()->getInterpreter();  //Tcl_Interp*

    auto token = Tcl_FindCommand(pInterp, "madcchain", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);
}

/**  Good chain with thee modules: */

void
MadcChainTests::chain_1() {
    {
        std::ofstream script(m_filename);
        script << "madc create adc1 -base 0x10000000\n";
        script << "madc create adc2 -base 0x20000000\n";
        script << "madc create adc3 -base 0x30000000\n";
        script << "madcchain create chain -modules [list adc1 adc2 adc3] -cbltaddress 0xff000000 -mcastaddress 0xfe000000\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
}
/** Bad chain because one of the modules does not exist */
void
MadcChainTests::chain_2() {
    {
        std::ofstream script(m_filename);
        script << "madc create adc1 -base 0x10000000\n";
        // script << "madc create adc2 -base 0x20000000\n";
        script << "madc create adc3 -base 0x30000000\n";
        script << "madcchain create chain -modules [list adc1 adc2 adc3] -cbltaddress 0xff000000 -mcastaddress 0xfe000000\n";
    }
    CPPUNIT_ASSERT_THROW(
        (*m_parser)(),
        CTCLException
    );
}
/** Bad chain because one of the modules is not an madc. */

void
MadcChainTests::chain_3() {
    {
        std::ofstream script(m_filename);
        script << "madc create adc1 -base 0x10000000\n";
        script << "madc create adc2 -base 0x20000000\n";
        script << "stack create event\n";
        script << "madcchain create chain -modules [list adc1 adc2 event] -cbltaddress 0xff000000 -mcastaddress 0xfe000000\n";
    }
    CPPUNIT_ASSERT_THROW(
        (*m_parser)(),
        CTCLException
    );
}
/// read out list is correct:

void
MadcChainTests::read_1() {
    {
        std::ofstream script(m_filename);
        script << "madc create adc1 -base 0x10000000\n";
        script << "madc create adc2 -base 0x20000000\n";
        script << "madc create adc3 -base 0x30000000\n";
        script << "madcchain create chain -modules [list adc1 adc2 adc3] -cbltaddress 0xff000000 -mcastaddress 0xfe000000\n";
        script << "stack create event -trigger nim1 -modules chain\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CVMUSBReadoutList list;
    CStack* pEvent = m_parser->getEventStack();
    CPPUNIT_ASSERT(pEvent);
    pEvent->addReadoutList(list);

    auto ops = list.dumpForMvlc();

    CPPUNIT_ASSERT_EQUAL(size_t(2), ops.size());
    auto rd = ops.at(0);     // FIFO read
    auto reset = ops.at(1);  // Readout reset.

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_block_read 0xb 73728 0xff000000"), rd
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0xfe006034 0x1"),
        reset
    );



}