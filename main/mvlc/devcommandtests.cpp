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
#include <XXUSBConfigurableObject.h>
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
        
    public:
        // Normally the call to attach  callse  the driver's attach which configures
        // the options, _but_ in these tests, we don't actually have a driver so we cheat.
        virtual void addDevice(std::string devname, CReadoutModule* driver) { // to add -test option.
            driver->getConfiguration()->addParameter("-test", nullptr, nullptr);
            TCLConfigParser::addDevice(devname, driver);
        }    
    protected:
        void addExtensions() {
            auto testcmd = new TestDeviceCommand(*getInterpreter(), *this);
            addExtension(*testcmd);
        }
    };
}

class DevCmdTests : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE(DevCmdTests);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(create_1);    // Test create subcommand.
    CPPUNIT_TEST(create_2);    // Test create command with configuration.
    CPPUNIT_TEST(create_3);    // Test create command with bad configuration.
    CPPUNIT_TEST(create_4);    // Test create command with bad parameter count.
    CPPUNIT_TEST(create_5);    // DUplicate error.
    CPPUNIT_TEST_SUITE_END();

protected:
    void construct_1();

    void create_1();
    void create_2();
    void create_3();
    void create_4();
    void create_5();
public:
    void setUp() {
        // Create a temp file...

        char nameTemplate[100];
        strcpy(nameTemplate, "config.tclXXXXXX");
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
 ///////////////////////////////////// Create tests ///////////////////////////////////////


 // Just make one.
 void
 DevCmdTests::create_1() {
    // Make the script and execute it:
    {
        std::ofstream script(m_scriptFile);
        script << "device create abcd\n";
    }
    CPPUNIT_ASSERT_NO_THROW((*m_pParser)());

    // There should be a device named "abcd":

    CPPUNIT_ASSERT(m_pParser->findDevice("abcd"));
 }
 // Make one and configure -test to be something.

 void
 DevCmdTests::create_2() {
    {
        std::ofstream script(m_scriptFile);
        script << "device create abcd -test testing\n";
    }
    CPPUNIT_ASSERT_NO_THROW((*m_pParser)());

    // THe configuration should have -test set to "testing"
    auto device = m_pParser->findDevice("abcd");
    auto config = device->getConfiguration();
    std::string value;
    CPPUNIT_ASSERT_NO_THROW(
        value = config->cget("-test")
    );

    CPPUNIT_ASSERT_EQUAL(std::string("testing"), value);
 }

 // Configure with bad option:

 void
 DevCmdTests::create_3() {
    {
        std::ofstream script(m_scriptFile);
        script << "device create abcd -testing test\n"; // bad option namne.
    }
    CPPUNIT_ASSERT_THROW(
        (*m_pParser)(),
        CTCLException
    );
 }

 // invalid parameter count:

 void
 DevCmdTests::create_4() {
    {
        std::ofstream script(m_scriptFile);
        script << "device create abcd -test\n";   // missing value for option.
    }
    CPPUNIT_ASSERT_THROW(
        (*m_pParser)(),
        CTCLException
    );

 }
 // Duplicate device fails but keeps the old one ok

 void
 DevCmdTests::create_5() {
    {
        std::ofstream script(m_scriptFile);
        script << "device create abcd -test testing\n";   
        script << "device create abcd -test {will not be set}";  // Duplicate device:
    }
    CPPUNIT_ASSERT_THROW(
        (*m_pParser)(),
        CTCLException
    );

    auto device = m_pParser->findDevice("abcd");
    auto config = device->getConfiguration();
    std::string value;
    CPPUNIT_ASSERT_NO_THROW(
        value = config->cget("-test")
    );

    CPPUNIT_ASSERT_EQUAL(std::string("testing"), value);
 }