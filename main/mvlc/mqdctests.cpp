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

    Rudimentary tests for the mtdc module
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "CVMUSB.h"
#include "CMQDC32RdoHdwr.h"
#include "CReadoutModule.h"
#include "MVLCConfigParser.h"
#include "CStack.h"
#include "CMTDC32.h"
#include <TCLInterpreter.h>
#include <TCLException.h>
#include <tcl.h>
#include <string>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>

class MqdcTests :  public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(MqdcTests);
    CPPUNIT_TEST(command_1);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(read_1);   // By default multievent mode is off so a fifo reado fo 40 items. then a reset
    CPPUNIT_TEST(read_2);   //limied mode, maxtransfers + 40
    CPPUNIT_TEST(read_3);   // read 1k longs.
    CPPUNIT_TEST(end_1);    // It has an end of run operation set.
    CPPUNIT_TEST_SUITE_END();
protected:
    void command_1();
    void create_1();
    void read_1();
    void read_2();
    void read_3();
    void end_1();
public:
    void setUp() {
        char nameTemplate[100];
         strcpy(nameTemplate, "mqdc.tclXXXXXX");
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

CPPUNIT_TEST_SUITE_REGISTRATION(MqdcTests);


// The command should be registered:

void
MqdcTests::command_1() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();

    auto token = Tcl_FindCommand(pInterp, "mqdc", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);
}
// We can actually create a module:

void 
MqdcTests::create_1() {
    {
        std::ofstream script(m_filename);
        script << "mqdc create qdc -base 0x12000000\n";
        script << "stack create event -trigger nim1 -modules qdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    // SHould be able to find the module and it should be wrapping the correct type:

    auto pModule = m_parser->findDevice("qdc");
    CPPUNIT_ASSERT(pModule);

    CMQDC32RdoHdwr* pDriver = dynamic_cast<CMQDC32RdoHdwr*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDriver);
}
// Read with a default module. (-multieventoff)

void MqdcTests::read_1() {
    {
        std::ofstream script(m_filename);
        script << "mqdc create qdc -base 0x12000000 -multievent off -maxtransfers 100\n";
        script << "stack create event -trigger nim1 -modules qdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    

    auto pModule = m_parser->findDevice("qdc");
    CPPUNIT_ASSERT(pModule);

    CMQDC32RdoHdwr* pDriver = dynamic_cast<CMQDC32RdoHdwr*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDriver);

    CVMUSB controller;
    pDriver->Initialize(controller);

    CVMUSBReadoutList list;
    pDriver->addReadoutList(list);

    auto ops = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(2), ops.size());

    auto rd = ops.at(0);
    auto reset = ops.at(1);
    // A fifo read with 40 words - max single event I guess.

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_block_read 0xb 40 0x12000000"),
        rd
    );
    // A reset readout:

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12006034 0x0"),
        reset
    );
}

void MqdcTests::read_2() {
    {
        std::ofstream script(m_filename);
        script << "mqdc create qdc -base 0x12000000 -multievent limited -maxtransfers 100\n";
        script << "stack create event -trigger nim1 -modules qdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    

    auto pModule = m_parser->findDevice("qdc");
    CPPUNIT_ASSERT(pModule);

    CMQDC32RdoHdwr* pDriver = dynamic_cast<CMQDC32RdoHdwr*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDriver);

    // Logic's base gets set on initialization:

    CVMUSB controller;
    pDriver->Initialize(controller);

    CVMUSBReadoutList list;
    pDriver->addReadoutList(list);

    auto ops = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(2), ops.size());

    auto rd = ops.at(0);
    auto reset = ops.at(1);
    // A fifo read with 40 words - max single event I guess.

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_block_read 0xb 140 0x12000000"),
        rd
    );
    // A reset readout:

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12006034 0x0"),
        reset
    );
}
void MqdcTests::read_3() {
    {
        std::ofstream script(m_filename);
        script << "mqdc create qdc -base 0x12000000 -multievent on -maxtransfers 100\n";
        script << "stack create event -trigger nim1 -modules qdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    

    auto pModule = m_parser->findDevice("qdc");
    CPPUNIT_ASSERT(pModule);

    CMQDC32RdoHdwr* pDriver = dynamic_cast<CMQDC32RdoHdwr*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDriver);

    CVMUSB controller;
    pDriver->Initialize(controller);

    CVMUSBReadoutList list;
    pDriver->addReadoutList(list);

    auto ops = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(2), ops.size());

    auto rd = ops.at(0);
    auto reset = ops.at(1);
    // A fifo read with 40 words - max single event I guess.

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_block_read 0xb 1024 0x12000000"),
        rd
    );
    // A reset readout:

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12006034 0x0"),
        reset
    );
}

void MqdcTests::end_1() {
    {
        std::ofstream script(m_filename);
        script << "mqdc create qdc -base 0x12000000 -multievent on -maxtransfers 100\n";
        script << "stack create event -trigger nim1 -modules qdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    

    auto pModule = m_parser->findDevice("qdc");
    CPPUNIT_ASSERT(pModule);

    CMQDC32RdoHdwr* pDriver = dynamic_cast<CMQDC32RdoHdwr*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDriver);

    
    CVMUSB controller;
    pDriver->Initialize(controller);
    controller.clearRecordedOperations();  // only want the onend ops.
    pDriver->onEndRun(controller);

    auto ops = controller.getRecordedOperations();

    CPPUNIT_ASSERT_EQUAL(size_t(2), ops.size());

    auto acqoff = ops.at(0);
    auto reset = ops.at(1);

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x1200603a 0x0"),
        acqoff
    );

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d16 0x12006034 0x0"),
        reset
    );
}