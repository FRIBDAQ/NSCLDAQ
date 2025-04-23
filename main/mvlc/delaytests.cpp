#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "CDelay.h"
#include "CStack.h"
#include "CVMUSBReadoutList.h"
#include "MVLCConfigParser.h"
#include <TCLInterpreter.h>
#include <tcl.h>
#include "CReadoutModule.h"

#include <string>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <TCLException.h>
#include <tcl.h>


// The test class:

class DelayTests : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE(DelayTests);
    CPPUNIT_TEST(command_1);
    CPPUNIT_TEST(delay_1);
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();
    void delay_1();
public:
    void setUp() {
        // Make the test script file:

        char nameTemplate[100];
        strcpy(nameTemplate, "delay.tclXXXXXX");
        m_fd = mkstemp(nameTemplate);
        m_filename = nameTemplate;

        // Make the parser - two step creation.

        m_parser = new MVLCConfigParser(m_filename);
        m_parser->initialize();
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

CPPUNIT_TEST_SUITE_REGISTRATION(DelayTests);


/////////////////// Tests

void
DelayTests::command_1() {
    Tcl_Interp* interp = m_parser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(interp, "delay", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);
}

void
DelayTests::delay_1() {
    {
        {
            std::ofstream script(m_filename); 
            script << "delay create d -value 10\n";
            script << "stack create e -trigger nim1 -modules d\n";
        }   
        CPPUNIT_ASSERT_NO_THROW(
            (*m_parser)()
        );
        CStack* s = m_parser->getEventStack();
        CPPUNIT_ASSERT(s);
        CVMUSBReadoutList list;

        s->addReadoutList(list);
        auto ops = list.dumpForMvlc();

        CPPUNIT_ASSERT_EQUAL(size_t(1), ops.size());
        CPPUNIT_ASSERT_EQUAL(
            std::string("software_delay 10"),
            ops.at(0)
        );
    }
}