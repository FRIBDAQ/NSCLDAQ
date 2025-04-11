/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321

    @author Ron Fox <fox at frib dot  msu dot edu>
    @brief Tests for the v785 module.



*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include "MVLCConfigParser.h"
#include <stdlib.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <memory>
#include <TCLException.h>
#include <TCLInterpreter.h>
#include <tcl.h>
#include <sstream>
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include "CStack.h"


static const size_t adcinitcount(47);
static const size_t tdcadditional(2);
static const size_t qdcadditional(1);

#define Const(name) static const int name  = 
// register defs shamlelessly copied from C785.cpp - where they should be in a header.


Const(eventBuffer)      0;
Const(firmware)    0x1000;
Const(GEO)         0x1002;
Const(McastAddr)   0x1004;
Const(BSet1)       0x1006;
Const(BClear1)     0x1008;
Const(IPL)         0x100a;
Const(Vector)      0x100c;
Const(Status1)    0x100e;
Const(Control1)    0x1010;
Const(AderHigh)    0x1012;
Const(AderLow)     0x1014;
Const(RESET)       0x1016;	// Better to use Bset according to manual.
Const(McastCtl)    0x101a;
Const(EventTrig)   0x1020;
Const(Status2)     0x1022;
Const(EventCountL) 0x1024;
Const(EventCountH) 0x1026;
Const(IncrementEC) 0x1028;
Const(IncrementO)  0x102a;
Const(LoadTest)    0x102c;
Const(FastClear)   0x102e;
Const(BSet2)       0x1032;
Const(BClear2)     0x1034;
Const(LogicalCrate) 0x103c;
Const(ECountReset) 0x1040;
Const(FSRange)     0x1060;
Const(Iped)        0x1060;
Const(Thresholds)   0x1080;	// Continues through 10bf, 32 D16 words.

class C785Tests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(C785Tests);
    CPPUNIT_TEST(command_1);                // THe command was added.
    CPPUNIT_TEST(create_1);                 // Create the default one in a stack.
    CPPUNIT_TEST(init_1);                   // basic init of adc.
    CPPUNIT_TEST(init_2);                   // init of tdc.
    CPPUNIT_TEST(init_3);                   // init of qdc.
    // add tests that modify the number of ops e.g. suppression opotions.
    CPPUNIT_TEST(read_1);
    CPPUNIT_TEST_SUITE_END();
protected:
    void command_1();
    void create_1();
    void init_1();
    void init_2();
    void init_3();
    void read_1();

public:
    void setUp() {
        // Make a script file for the tests and a parser constructed on it.

        char nameTemplate[100];
        strcpy(nameTemplate, "adc.tclXXXXXX");
        m_fd = mkstemp(nameTemplate);
        m_filename = nameTemplate;

        m_parser = new MVLCConfigParser(m_filename);
        m_parser->initialize();
    }
    void tearDown() {
        delete m_parser;
        close(m_fd);
        unlink(m_filename.c_str());
    }
private:
    std::string writecmd(uint32_t offset, uint16_t value);    // Assumes base is 0x80000000

private:
    int m_fd;
    std::string m_filename;
    MVLCConfigParser* m_parser;
};

CPPUNIT_TEST_SUITE_REGISTRATION(C785Tests);


// private utils:

//  produce a write command:
// Assumes the write is 16 bits wide, the amod is a32UserData and the
// base is 0x80000000
std::string
C785Tests::writecmd(uint32_t offset, uint16_t value) {
    std::stringstream op;
    op << "vme_write 0x" << std::hex << unsigned(CVMUSBReadoutList::a32UserData) << " d16 0x"
         << offset + 0x80000000 << " 0x" << value;

    std::string result = op.str();


    return result;
}

///////////////////////////////////// command was registered ////////////////////////////////////

void
C785Tests::command_1() {
    Tcl_Interp* interp = m_parser->getInterpreter()->getInterpreter();  // Tcl_Interp*.

    auto token = Tcl_FindCommand(interp, "adc", nullptr, TCL_GLOBAL_ONLY);

    CPPUNIT_ASSERT(token);                         // FINdable.
}

////////////////////////////////// Create the device:

void
C785Tests::create_1() {
    {
        std::ofstream script(m_filename);
        script << "adc create d -base 0x80000000\n";
        script << "stack create event -trigger nim1 -modules d\n" ;         // For later tests.
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    
    CPPUNIT_ASSERT(m_parser->findDevice("d"));                     // got made.
}
///////////////////////// init code.

/** for an adc */

// For an adc initialization with only the -base set produces 47 ops I think.
void 
C785Tests::init_1() {
    {
        std::ofstream script(m_filename);
        script << "adc create d -base 0x80000000\n";
        script << "stack create event -trigger nim1 -modules d\n" ;      
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    auto estack = m_parser->getEventStack();
    CPPUNIT_ASSERT(estack);
    CVMUSB controller;
    estack->Initialize(controller);

    auto ops = controller.getRecordedOperations();

    CPPUNIT_ASSERT_EQUAL(adcinitcount, ops.size());

    // starts with a reset and ends with a write of 0x24 to control reg 1.

    auto reset1 = writecmd(BSet1, 0x80);            // toggling the rest bit.
    auto reset2 = writecmd(BClear1, 0x80);
    CPPUNIT_ASSERT_EQUAL(reset1, ops[0]);
    CPPUNIT_ASSERT_EQUAL(reset2, ops[1]);
    
    // last item turns on buserr on end:

    auto last = writecmd(Control1, 0x24);
    CPPUNIT_ASSERT_EQUAL(last, ops.back());
}   

//

/** for a tdc   adds tdcadditional more writes to */

void
C785Tests::init_2() {
    {
        std::ofstream script(m_filename);
        script << "adc create d -base 0x80000000 -type tdc\n";
        script << "stack create event -trigger nim1 -modules d\n" ;      
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    auto estack = m_parser->getEventStack();
    CPPUNIT_ASSERT(estack);
    CVMUSB controller;
    estack->Initialize(controller);

    auto ops = controller.getRecordedOperations();

    CPPUNIT_ASSERT_EQUAL(adcinitcount + tdcadditional, ops.size());

    // the next to the last operation sets the commonstart/stop
    // the one before it the full scale range.  
    
    // common stop is off:

    std::string setstartstop = ops.at(adcinitcount + tdcadditional - 2);
    CPPUNIT_ASSERT_EQUAL(
        writecmd(BClear2, 0x400), setstartstop
    );
    // default range is set before that.

    float  nsrange =  600.0;    //Default range in ns.
    float  rRange  = 36040.0/(nsrange + 1.3333);
    uint16_t writeVal = static_cast<uint16_t>(rRange + 0.5);
    auto setrange = ops.at(adcinitcount + tdcadditional -3);
    CPPUNIT_ASSERT_EQUAL(
        writecmd(FSRange, writeVal), setrange
    );

}

/** for a qdc. one more write than an adc to set the iped. */
void
C785Tests::init_3() {
    {
        std::ofstream script(m_filename);
        script << "adc create d -base 0x80000000 -type qdc\n";
        script << "stack create event -trigger nim1 -modules d\n" ;      
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    auto estack = m_parser->getEventStack();
    CPPUNIT_ASSERT(estack);
    CVMUSB controller;
    estack->Initialize(controller);

    auto ops = controller.getRecordedOperations();
    CPPUNIT_ASSERT_EQUAL(adcinitcount + qdcadditional, ops.size());

    // Next to the last is a write of the default value (180) for -iped  value.

    std::string setiped = ops.at(adcinitcount + qdcadditional - 2);
    CPPUNIT_ASSERT_EQUAL(
        writecmd(Iped, 180), setiped
    );
}

// read operations:

void C785Tests::read_1() {}