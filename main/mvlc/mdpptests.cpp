/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief Test the mdpp* commands and their classes.
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "MVLCConfigParser.h"
#include "CMDPP32SCP.h"
#include "CMDPP32QDC.h"
#include "CMDPP16QDC.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include "CReadoutModule.h"
#include <string>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <TCLException.h>


class MdppTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(MdppTests);
    CPPUNIT_TEST(command_1);   // mdpp32scp
    CPPUNIT_TEST(command_2);   // mdpp16qdc
    CPPUNIT_TEST(command_3);   // mdpp32qdc

    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(create_2);
    CPPUNIT_TEST(create_3);

    CPPUNIT_TEST(read_1);
    CPPUNIT_TEST(read_2);
    CPPUNIT_TEST(read_3);
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();
    void command_2();
    void command_3();

    void create_1();
    void create_2();
    void create_3();

    void read_1();
    void read_2();
    void read_3();

public:
    void setUp() {
        char nameTemplate[100];
        strcpy(nameTemplate, "mdpp.tclXXXXXX");
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

CPPUNIT_TEST_SUITE_REGISTRATION(MdppTests);

// Check that the commands were registered:


void
MdppTests::command_1() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(pInterp, "mdpp32scp", nullptr, TCL_GLOBAL_ONLY);

    CPPUNIT_ASSERT(token);
}
void
MdppTests::command_2() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(pInterp, "mdpp16qdc", nullptr, TCL_GLOBAL_ONLY);

    CPPUNIT_ASSERT(token);
}
void
MdppTests::command_3() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(pInterp, "mdpp32qdc", nullptr, TCL_GLOBAL_ONLY);

    CPPUNIT_ASSERT(token);
}

// CHeck that we can acutally make one of each object:

void MdppTests::create_1() {
    {
        std::ofstream script(m_filename);
        script << "mdpp32scp create d -base 0x12340000\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    auto pModule = m_parser->findDevice("d");
    CPPUNIT_ASSERT(pModule);

    auto pDriver= dynamic_cast<CMDPP32SCP*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDriver);
}

void MdppTests::create_2() {
    {
        std::ofstream script(m_filename);
        script << "mdpp16qdc create d -base 0x12340000\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    auto pModule = m_parser->findDevice("d");
    CPPUNIT_ASSERT(pModule);

    auto pDriver= dynamic_cast<CMDPP16QDC*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDriver);
}
void MdppTests::create_3() {
    {
        std::ofstream script(m_filename);
        script << "mdpp32qdc create d -base 0x12340000\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    auto pModule = m_parser->findDevice("d");
    CPPUNIT_ASSERT(pModule);

    auto pDriver= dynamic_cast<CMDPP32QDC*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDriver);
}

// Check the read operations.

// scp fifo read of 65535 items followed by a ReadoutReset write.

void
MdppTests::read_1() {
    {
        std::ofstream script(m_filename);
        script << "mdpp32scp create d -base 0x12340000\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    auto pModule = m_parser->findDevice("d");
    CPPUNIT_ASSERT(pModule);

    auto pDriver= dynamic_cast<CMDPP32SCP*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDriver);

    CVMUSBReadoutList list;
    pDriver->addReadoutList(list);
    auto ops = list.dumpForMvlc();


    CPPUNIT_ASSERT_EQUAL(size_t(2), ops.size());

    auto fiford = ops.at(0);
    auto reset  = ops.at(1);

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_block_read 0xb 65535 0x12340000"),
        fiford
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12346034 0x1"),
        reset
    );
}

void
MdppTests::read_2() {
    {
        std::ofstream script(m_filename);
        script << "mdpp16qdc create d -base 0x12340000\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    auto pModule = m_parser->findDevice("d");
    CPPUNIT_ASSERT(pModule);

    auto pDriver= dynamic_cast<CReadoutHardware*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDriver);

    CVMUSBReadoutList list;
    pDriver->addReadoutList(list);
    auto ops = list.dumpForMvlc();


    CPPUNIT_ASSERT_EQUAL(size_t(2), ops.size());

    auto fiford = ops.at(0);
    auto reset  = ops.at(1);

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_block_read 0xb 65535 0x12340000"),
        fiford
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12346034 0x1"),
        reset
    );
}

void
MdppTests::read_3() {
    {
        std::ofstream script(m_filename);
        script << "mdpp32qdc create d -base 0x12340000\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    auto pModule = m_parser->findDevice("d");
    CPPUNIT_ASSERT(pModule);

    auto pDriver= dynamic_cast<CReadoutHardware*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDriver);

    CVMUSBReadoutList list;
    pDriver->addReadoutList(list);
    auto ops = list.dumpForMvlc();


    CPPUNIT_ASSERT_EQUAL(size_t(2), ops.size());

    auto fiford = ops.at(0);
    auto reset  = ops.at(1);

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_block_read 0xb 65535 0x12340000"),
        fiford
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12346034 0x1"),
        reset
    );
}

