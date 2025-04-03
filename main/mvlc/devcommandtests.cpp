/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief Tests for the DeviceCommand clasS
@note we also test its interactions with the TCLConfigParser.

*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include "TCLConfigParser.h"
#define private public
#include "DeviceCommand.h"
#include "CReadoutModule.h"
#undef private
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <memory>
#include <TCLInterpreter.h>
#include <TCLException.h>
#include <tcl.h>


/* These are dummy test classes in a namespace to avoid collision with other tests: */
namespace devcmdtest {
    // My dummmy test command:
    class TestDeviceCommand : public DeviceCommand {
    public:
        TestDeviceCommand(CTCLInterpreter& interp, TCLConfigParser& parser) :
            DeviceCommand(interp, "device", parser) {}

        virtual CReadoutModule* createDevice(std::string name) {
            return new CReadoutModule;   // Normally we'd create and setdDriver as well.
        }
    };
    // Test parser class will create/register devcmdtest:
    class TestTCLConfigParser : public TCLConfigParser {
    public:
        TestTCLConfigParser(const std::string& infile) :
            TCLConfigParser(infile) {}
        
    protected:
        void addExtensions() {
            auto testcmd = new TestDeviceCommand(*getInterpreter(), *this);
            addExtension(*testcmd);
        }
    };
}

class DevCmdTests : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE(DevCmdTests);\
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST_SUITE_END();

protected:
    void construct_1();
public:
    void setUp() {
        // Create a temp file...

        char nameTemplate[100];
        strcpy(nameTemplate, "configXXXXXX.tcl");
        m_fd = mkstemp(nameTemplate);
        m_scriptFile = nameTemplate;

        // And an initialized test parser on it (has the "device" command).
        m_pParser = new devcmdtest::TestTCLConfigParser(m_scriptFile);
        m_pParser->initialize();                       // Register commands.

    }
    void tearDown() {
        delete m_pParser;

        close(m_fd);
        unlink(m_scriptFile.c_str());
    }
private:
    std::string      m_scriptFile;   // Script filename.
    int              m_fd;           // fd open on it.
    TCLConfigParser* m_pParser;      // Test parser.
};

CPPUNIT_TEST_SUITE_REGISTRATION(DevCmdTests);

////////////////////////////// construction tests //////////////////////////////////

/** the parser interpreter shoulid have a "device" command */
void
DevCmdTests::construct_1() {
     Tcl_Interp* pInterp = m_pParser->getInterpreter()->getInterpreter();
     auto pCommand = Tcl_FindCommand(pInterp, "device", nullptr, TCL_GLOBAL_ONLY);

     CPPUNIT_ASSERT(pCommand);              // Null if not found.
}