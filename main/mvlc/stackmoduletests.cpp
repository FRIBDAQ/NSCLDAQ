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
@author Ron Fox <fox at frib dot msu dot edu>
@brief Test  CStack and CStackCommand in its interactions with !V

*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "TCLConfigParser.h"
#define private public
#include "CStack.h"
#undef private
#include "CReadoutModule.h"
#include "CReadoutHardware.h"
#include <TCLInterpreter.h>

#include <stdlib.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <memory>
#include <iostream>
#include <sstream>
#include <TCLException.h>
#include <stdexcept>
#include <XXUSBConfigurableObject.h>
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"

// Test classes live in a namespace to avoid collisions:

namespace stackmoduletests {
    /* A dummy module and its command that can be put in a stack */
    //  values for initialization, list and endrun.,
    //
    class Dummy : public CReadoutHardware {        // don't have to do anything.
        XXUSB::CConfigurableObject* m_pConfig;
        public:
            void onAttach(XXUSB::CConfigurableObject& config) {
                m_pConfig = &config;
                config.addIntegerParameter("-init");
                config.addIntegerParameter("-list");
                config.addIntegerParameter("-end");
            }
            void Initialize(CVMUSB& c) { // care about the value.
                c.vmeWrite32(0, 0, m_pConfig->getIntegerParameter("-init"));
            }
            void addReadoutList(CVMUSBReadoutList& list) {
                list.addMarker(m_pConfig->getIntegerParameter("-list"));
            }
            void onEndRun(CVMUSB& c) {  // carea bout the value.
                c.vmeWrite32(0, 0, m_pConfig->getIntegerParameter("-end"));
            }
    };
    class DummyCommand : public DeviceCommand {
    public:
        DummyCommand(CTCLInterpreter& interp, TCLConfigParser& parser) :
            DeviceCommand(interp, "dummy", parser) {}
    protected:
        CReadoutModule* createDevice(std::string name) {
            auto result = new CReadoutModule;
            result->SetDriver(new Dummy);
            return result;
        }
    };

    /* Derive from TCLConfigParser to add the 'stack ' command*/

    class MyParser : public TCLConfigParser {
    public:
        MyParser(const std::string& infile) : 
            TCLConfigParser(infile) {}
        ~MyParser() {}
    protected:
        virtual void addExtensions() {
            addExtension(new CStackCommand(*getInterpreter(), *this));
            addExtension(new DummyCommand(*getInterpreter(), *this));
        }
    };
}

/** The test: */

class StackCommandTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(StackCommandTests);
    CPPUNIT_TEST(none_1);    //Initially, there are no event/scaler stacks.;
    CPPUNIT_TEST(none_2);    // Making a stack interrupt triggted  -> warning to stderr.
    CPPUNIT_TEST(event_1);   // There's a good event stack.
    CPPUNIT_TEST(event_2);   // Attempted to make two event stacks -> logic_error.
    CPPUNIT_TEST(scaler_1);  // There's a good scaler stack.
    CPPUNIT_TEST(scaler_2);  // THere's 2 scaler stacks -> logic error.
    CPPUNIT_TEST(both_1);   // Registration of both a scaler and an event stack are ok:

    // Tests  of the -modules option for stack - note we us an event stack just
    // so it's easy to dredge out the stack again.

    CPPUNIT_TEST(modules_1);   // There are none.
    CPPUNIT_TEST(modules_2);   // THere is one
    CPPUNIT_TEST(modules_3);   // There are sseveral and they are ordered.
    CPPUNIT_TEST(modules_4);   // adding one that does not exist fails.

    // Stack elements contribute VME operations and lists:

    CPPUNIT_TEST(init_1);      // Single module.
    CPPUNIT_TEST(init_2);      // Severl modules..
    CPPUNIT_TEST(list_1);      // SIngle mod
    CPPUNIT_TEST(list_2);     // several...
    CPPUNIT_TEST(end_1);
    CPPUNIT_TEST(end_2);
    CPPUNIT_TEST_SUITE_END();
    // Tests
protected:
    void none_1();
    void none_2();

    void event_1();
    void event_2();

    void scaler_1();
    void scaler_2();

    void both_1();

    void modules_1();
    void modules_2();
    void modules_3();
    void modules_4();

    void init_1();
    void init_2();
    void list_1();
    void list_2();
    void end_1();
    void end_2();
private:
    int m_fd;
    std::string m_filename;
    TCLConfigParser* m_parser;

public:
    void setUp() {     // Run pre-test>
        // Make a temp file for configuration scripts.
        char nameTemplate[100];
        strcpy(nameTemplate, "config.tclXXXXXX");
        m_fd = mkstemp(nameTemplate);
        m_filename = nameTemplate;

        // Make a parser to test with:

        m_parser = new stackmoduletests::MyParser(m_filename);
        m_parser->initialize();                  // adds the commands.

    }
    void tearDown() {
        delete m_parser;

        close(m_fd);
        unlink(m_filename.c_str());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(StackCommandTests);

//////////////////////////// Tests that yield neither scaler nor event stack //////////////////////

/* Initially there are neither: */

void
StackCommandTests::none_1() {
    CPPUNIT_ASSERT(!m_parser->getEventStack());
    CPPUNIT_ASSERT(!m_parser->getScalerStack());
}

/* IF we add an interrupt stack then that makes a warning message
 *    We a capture the message in a string stream just to ensure 
*     one was emitted.
 *    analyzing the actual text is too fragile a test for us.
 *   We do check that there's still not an event/scaler stack.
 
 */
void
StackCommandTests::none_2() {
    
    // THis trick of redirecting stderr for C++ is from 
    // https://stackoverflow.com/questions/26579513/checking-printf-output-in-cppunit

    std::ostringstream err;
    std::streambuf* orig_buf(std::cerr.rdbuf(err.rdbuf()));

    // Make our script:

    {
        std::ofstream script(m_filename);
        script << "stack create bad -trigger interrupt\n";
    }
    try {
        CPPUNIT_ASSERT_NO_THROW((*m_parser)());  // no exception but an error msg:
        std::cerr.rdbuf(orig_buf);               // restore the stream.

    } catch(...) {
        std::cerr.rdbuf(orig_buf);    
        throw;
    }

    // Now we can do things in an ordinary manner:

    std::string msg = err.str();
    CPPUNIT_ASSERT(msg.size() != 0);                  // There is a message.

    CPPUNIT_ASSERT(!m_parser->getEventStack());
    CPPUNIT_ASSERT(!m_parser->getScalerStack());

    
}

///////////////////////////////////////// Tests event  stacks //////////////////////////////

// ALl good one stack:

void
StackCommandTests::event_1() {
    // Make the script:

    {
        std::ofstream script(m_filename);
        script << "stack create event -trigger nim1\n";
    }
    CPPUNIT_ASSERT_NO_THROW((*m_parser)());

    // There is a stack and it's "event".

    auto pStackDriver = m_parser->getEventStack();
    CPPUNIT_ASSERT(pStackDriver);
    auto stackModule = m_parser->findDevice("event");
    CPPUNIT_ASSERT_EQUAL(
        static_cast<CReadoutHardware*>(pStackDriver), stackModule->getDriver()
    );

    // THere is no scaler stack:

    CPPUNIT_ASSERT(!m_parser->getScalerStack());
}
/** A second event stack is a logic error but the first one remains: */

void
StackCommandTests::event_2() {
    {
        std::ofstream script(m_filename);
        script << "stack create event -trigger nim1\n";
        script << "stack create event2 -trigger nim1\n";    // illegal!!
    }
    CPPUNIT_ASSERT_THROW(
        (*m_parser)(),
        std::logic_error
    );

    // There is a stack and it's "event".

    auto pStackDriver = m_parser->getEventStack();
    CPPUNIT_ASSERT(pStackDriver);
    auto stackModule = m_parser->findDevice("event");
    CPPUNIT_ASSERT_EQUAL(
        static_cast<CReadoutHardware*>(pStackDriver), stackModule->getDriver()
    );

    // THere is no scaler stack:

    CPPUNIT_ASSERT(!m_parser->getScalerStack());
}
//////////////////////////////////// Tests for scaler stacks /////////////////////////

/*  only one periodic stack - no exception. */

void 
StackCommandTests::scaler_1() {
    {
        std::ofstream script(m_filename);
        script << "stack create scaler -trigger scaler\n";
    }
    CPPUNIT_ASSERT_NO_THROW((*m_parser)());

    // There is a stack and it's "event".

    auto pStackDriver = m_parser->getScalerStack();
    CPPUNIT_ASSERT(pStackDriver);
    auto stackModule = m_parser->findDevice("scaler");
    CPPUNIT_ASSERT_EQUAL(
        static_cast<CReadoutHardware*>(pStackDriver), stackModule->getDriver()
    );

    // THere is no scaler stack:

    CPPUNIT_ASSERT(!m_parser->getEventStack());
}
/*  Two scaler stacks -> logic error but the first one is still there: */

void
StackCommandTests::scaler_2() {
    {
        std::ofstream script(m_filename);
        script << "stack create scaler -trigger scaler\n";
        script << "stack create scaler2 -trigger scaler\n";
        
    }
    CPPUNIT_ASSERT_THROW(
        (*m_parser)(), std::logic_error
    );

    // There is a stack and it's "event".

    auto pStackDriver = m_parser->getScalerStack();
    CPPUNIT_ASSERT(pStackDriver);
    auto stackModule = m_parser->findDevice("scaler");
    CPPUNIT_ASSERT_EQUAL(
        static_cast<CReadoutHardware*>(pStackDriver), stackModule->getDriver()
    );

    // THere is no scaler stack:

    CPPUNIT_ASSERT(!m_parser->getEventStack());
}
///////////////////////////// Both a scaler and an event stack ////////////////////////////

void
StackCommandTests::both_1() {
    {
        std::ofstream script(m_filename);
        script << "stack create scaler -trigger scaler\n";
        script << "stack create event -trigger nim1\n";
        
    }
    CPPUNIT_ASSERT_NO_THROW((*m_parser)());

    // Should be an event and scaler and they are the right ones:

    CStack* event = m_parser->getEventStack();
    CPPUNIT_ASSERT(event);
    CReadoutModule* eventModule = m_parser->findDevice("event");
    CPPUNIT_ASSERT_EQUAL(
        static_cast<CReadoutHardware*>(event), eventModule->getDriver()
    );

    CStack* scaler = m_parser->getScalerStack();
    CPPUNIT_ASSERT(scaler);
    CReadoutModule* scalerModule = m_parser->findDevice("scaler");
    CPPUNIT_ASSERT_EQUAL(
        static_cast<CReadoutHardware*>(scaler), scalerModule->getDriver()
    );
}
///////////////////////////////////// modules test ////////////////////////////

/** initially there are no modules */
void
StackCommandTests::modules_1() {
    {
        std::ofstream script(m_filename);
        script << "stack create event -trigger nim1\n";
    }
    (*m_parser)();


    auto members = m_parser->getEventStack()->getStackElements();
    CPPUNIT_ASSERT_EQUAL(size_t(0), members.size());
}
/* I can put an element in th estack:*/
void
StackCommandTests::modules_2() {
    {
        std::ofstream script(m_filename);
        script << "dummy create segment\n";
        script << "stack create event -trigger nim1 -modules segment\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    // Verify the stack's module list:

    auto elements = m_parser->getEventStack()->getStackElements();
    CPPUNIT_ASSERT_EQUAL(size_t(1), elements.size());
    CPPUNIT_ASSERT_EQUAL(elements.front(), m_parser->findDevice("segment"));



}
/** Put afew modules in the list... shoulid have the correct order. */
void
StackCommandTests::modules_3() {
    {
        std::ofstream script(m_filename);
        script << "dummy create seg1\n";
        script << "dummy create seg2\n";
        script << "dummy create seg3\n";
        script << "dummy create seg4\n";
        script << "stack create event -trigger nim1 -modules [list seg1 seg2 seg3 seg4]\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );
    auto elements = m_parser->getEventStack()->getStackElements();
    CPPUNIT_ASSERT_EQUAL(size_t(4), elements.size());

    std::vector<std::string> ordering = {"seg1", "seg2", "seg3", "seg4"};
    auto p = elements.begin();
    for (auto name : ordering) {
        auto dname = (*p)->getConfiguration()->getName();
        CPPUNIT_ASSERT_EQUAL(name, dname);
        ++p;
    }
}
/** A parse error gets tossed (TCLException) if a module does not exist */
void
StackCommandTests::modules_4() {
    {
        std::ofstream script(m_filename);
        script << "stack create event -trigger nim -modules nosuch\n";
    };
    CPPUNIT_ASSERT_THROW(
        (*m_parser)(),
        CTCLException
    );
    
}
///////////////////////////// test initialize.

// one dummy.
void
StackCommandTests::init_1() {
    {
        std::ofstream script(m_filename);
        script << "dummy create seg1 -init 1\n";
        script << "stack create event -modules seg1 -trigger nim1\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CVMUSB controller;
    m_parser->getEventStack()->Initialize(controller);
    
   auto ops = controller.getRecordedOperations();
    CPPUNIT_ASSERT_EQUAL(size_t(1), ops.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x0 d32 0x0 0x1"),
        ops.at(0)
    );
}
// three dummies:

void 
StackCommandTests::init_2() {
    {
        std::ofstream script(m_filename);
        script << "dummy create seg1 -init 1\n";
        script << "dummy create seg2 -init 2\n";
        script << "dummy create seg3 -init 3\n";
        script << "stack create event -modules [list seg1 seg2 seg3] -trigger nim1\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CVMUSB controller;
    m_parser->getEventStack()->Initialize(controller);
    
    auto ops = controller.getRecordedOperations();
    CPPUNIT_ASSERT_EQUAL(size_t(3) , ops.size());

    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x0 d32 0x0 0x1"),
        ops.at(0)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x0 d32 0x0 0x2"),
        ops.at(1)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x0 d32 0x0 0x3"),
        ops.at(2)
    );
}
////////////////////////////// Test addReadoutList:

void
StackCommandTests::list_1() {
    {
        std::ofstream script(m_filename);
        script << "dummy create seg1 -list 0x10\n";
        script << "stack create event -modules seg1 -trigger nim1\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CVMUSBReadoutList list;
    m_parser->getEventStack()->addReadoutList(list);

    auto ops = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), ops.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("write_marker 0x10"),
        ops.at(0)
    );
}

void
StackCommandTests::list_2() {
    {
        std::ofstream script(m_filename);
        script << "dummy create seg1 -list 0x10\n";
        script << "dummy create seg2 -list 0x20\n";
        script << "dummy create seg3 -list 0x30\n";
        script << "stack create event -modules [list seg1 seg2 seg3] -trigger nim1\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CVMUSBReadoutList list;
    m_parser->getEventStack()->addReadoutList(list);

    auto ops = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(3), ops.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("write_marker 0x10"),
        ops.at(0)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("write_marker 0x20"),
        ops.at(1)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("write_marker 0x30"),
        ops.at(2)
    );
}
// test onEndRun:

void
StackCommandTests::end_1() {
    {
        std::ofstream script(m_filename);
        script << "dummy create seg1 -end 0x100\n";
        script << "stack create event -modules seg1 -trigger nim1\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CVMUSB c;
    m_parser->getEventStack()->onEndRun(c);

    auto ops = c.getRecordedOperations();
    CPPUNIT_ASSERT_EQUAL(size_t(1), ops.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x0 d32 0x0 0x100"),
        ops.at(0)
    );
}
void
StackCommandTests::end_2() {
    {
        std::ofstream script(m_filename);
        script << "dummy create seg1 -end 0x100\n";
        script << "dummy create seg2 -end 0x200\n";
        script << "dummy create seg3 -end 0x300\n";
        script << "stack create event -modules [list seg1 seg2 seg3] -trigger nim1\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CVMUSB c;
    m_parser->getEventStack()->onEndRun(c);

    auto ops = c.getRecordedOperations();
    CPPUNIT_ASSERT_EQUAL(size_t(3), ops.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x0 d32 0x0 0x100"),
        ops.at(0)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x0 d32 0x0 0x200"),
        ops.at(1)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x0 d32 0x0 0x300"),
        ops.at(2)
    );

}