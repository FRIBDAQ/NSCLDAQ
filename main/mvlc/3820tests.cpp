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
@brief Test (minimally) The C3820 class and command generator.
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "C3820.h"
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

static const uint32_t KeyLNE           = 0x00000410;
static const uint32_t ShadowCounters = 0x00000800;
static const uint32_t HighBits   = 0x00000210;
class SIS3820Tests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(SIS3820Tests);
    CPPUNIT_TEST(command_1);   // sis3820 command was made.
    CPPUNIT_TEST(read_1);      // read not in timestamp mode.
    CPPUNIT_TEST(read_2);      // Read in timestamp mode.
    CPPUNIT_TEST_SUITE_END();
protected:
    void command_1();
    void read_1();
    void read_2();
public:
    void setUp() {
        // Make a temp scipt file:

        char nameTemplate[100];
        strcpy(nameTemplate, "sis3820.tclXXXXXX");
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
    std::string readvme(uint32_t offset);    // Construct the vme read string.

private:
    int m_fd;
    std::string m_filename;
    TCLConfigParser* m_parser;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SIS3820Tests);

/////////////////////////// utilties

// We make module bases 0.

std::string
SIS3820Tests::readvme(uint32_t offset) {
    std::stringstream o;
    o << "vme_read_mem 0x" << std::hex << 
        unsigned(CVMUSBReadoutList::a32UserData) << " d32 0x" << offset;

    std::string result = o.str();
    return result;
}

/////////////////////////// tests

//  The command should be registered.
void
SIS3820Tests::command_1() {
    auto token = Tcl_FindCommand(
        m_parser->getInterpreter()->getInterpreter(), "sis3820", nullptr, TCL_GLOBAL_ONLY
    );
    CPPUNIT_ASSERT(token);
}
// we use the two types  of reads - timestamp reader and non timesstamp reader.

// timestamp on.
void
SIS3820Tests::read_1() {
    {
        std::ofstream script(m_filename);
        script << "sis3820 create scaler -timestamp on\n";
        script << "stack create event -trigger nim1 -modules scaler\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    CStack* event = m_parser->getEventStack();
    CPPUNIT_ASSERT(event);

    CVMUSBReadoutList list;
    event->addReadoutList(list);

    // base is 0, should have added 3 reads the two shadow counters for scaler 0 and 16 and the
    // high bites register:

    auto ops = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(3), ops.size());
    CPPUNIT_ASSERT_EQUAL(readvme(ShadowCounters), ops.at(0));    // lower bits of ch 0.
    CPPUNIT_ASSERT_EQUAL(readvme(ShadowCounters + 16 * sizeof(uint32_t)), ops.at(1)); // lower bits of ch 16
    CPPUNIT_ASSERT_EQUAL(readvme(HighBits), ops.at(2));          // High bits of both.


}
// non timestamp just does a 32 bit write to the latch key register and a block read of 32 counters.
void
SIS3820Tests::read_2() {
    {
        std::ofstream script(m_filename);
        script << "sis3820 create scaler -timestamp off\n";
        script << "stack create event -trigger nim1 -modules scaler\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    CStack* event = m_parser->getEventStack();
    CPPUNIT_ASSERT(event);

    CVMUSBReadoutList list;
    event->addReadoutList(list);

    auto ops = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(2), ops.size());

    std::stringstream write;
    write << "vme_write 0x" << std::hex << unsigned(CVMUSBReadoutList::a32UserData) 
        << " d32 0x" << KeyLNE << " 0x" << 0;
    std::string writeOp = write.str();

    CPPUNIT_ASSERT_EQUAL(writeOp, ops.at(0));

    std::stringstream blkread;
    blkread << "vme_block_read_mem 0x" << std::hex << unsigned(CVMUSBReadoutList::a32UserBlock)
       << " " << std::dec << 32 << std::hex << " 0x" << ShadowCounters;
    std::string readop = blkread.str();

    CPPUNIT_ASSERT_EQUAL(readop, ops.at(1));
        
}