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
        virtual void addDevice(std::string devname, CReadoutModule* driver) { // to add -test, -second opts
            driver->getConfiguration()->addParameter("-test", nullptr, nullptr);
            driver->getConfiguration()->addParameter("-second", nullptr, nullptr);
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
    CPPUNIT_TEST(config_1);    // good configuration operation.
    CPPUNIT_TEST(config_2);    // Multiple options good.
    CPPUNIT_TEST(config_3);    // partial config with bad option.
    CPPUNIT_TEST(config_4);    // No such device.
    CPPUNIT_TEST(config_5);    // bad parameter count.
    CPPUNIT_TEST(cget_1);      // Get single config param.
    CPPUNIT_TEST(cget_2);      // Get all config params.
    CPPUNIT_TEST(cget_3);      // Bad device name.
    CPPUNIT_TEST(cget_4);      // Bad config param.
    CPPUNIT_TEST(cget_5);      // try to get two option values....bad.
    CPPUNIT_TEST_SUITE_END();

protected:
    void construct_1();

    void create_1();
    void create_2();
    void create_3();
    void create_4();
    void create_5();

    void config_1();
    void config_2();
    void config_3();
    void config_4();
    void config_5();

    void cget_1();
    void cget_2();
    void cget_3();
    void cget_4();
    void cget_5();

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
 // Duplicate device fails but keeps the old one 

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
 //////////////////////////// Test the config subcommand /////////////////////////

 /*  A good configuration operation: */

 void
 DevCmdTests::config_1() {
    {
        std::ofstream script(m_scriptFile);
        script << "device create abcd\n";  // Make the device.
        script << "device config abcd -test testing\n";  // configure it
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_pParser)()
    );

    // the module exists and is configured:

    auto device = m_pParser->findDevice("abcd");
    CPPUNIT_ASSERT(device);
    std::string value = device->getConfiguration()->cget("-test");
    CPPUNIT_ASSERT_EQUAL(std::string("testing"), value);
 }
 /* Can provide multiple configs:  */
void
 DevCmdTests::config_2() {
    {
        std::ofstream script(m_scriptFile);
        script << "device create abcd\n";  // Make the device.
        script << "device config abcd -test testing -second 2\n";  // configure it
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_pParser)()
    );

    auto device = m_pParser->findDevice("abcd");
    CPPUNIT_ASSERT(device);
    std::string value = device->getConfiguration()->cget("-test");
    CPPUNIT_ASSERT_EQUAL(std::string("testing"), value);
    CPPUNIT_ASSERT_EQUAL(std::string("2"), device->getConfiguration()->cget("-second"));
 }

 /* If the second config option is invalid it's an error but the first one _is_ set. */

 void
 DevCmdTests::config_3() {
    {
        std::ofstream script(m_scriptFile);
        script << "device create abcd\n";  // Make the device.
        script << "device config abcd -test testing -sec 2\n";  // configure it -sec is not legal.
    }
    CPPUNIT_ASSERT_THROW(
        (*m_pParser)(),
        CTCLException
    );

    auto device = m_pParser->findDevice("abcd");
    CPPUNIT_ASSERT(device);
    std::string value = device->getConfiguration()->cget("-test");
    CPPUNIT_ASSERT_EQUAL(std::string("testing"), value);

 }
/* Configuring a nonexistent device: */
 void 
 DevCmdTests::config_4() {
    {
        std::ofstream script(m_scriptFile);
        script << "device config abcd -test testing\n"; // didn't make the device yet.
    }
    CPPUNIT_ASSERT_THROW(
        (*m_pParser)(),
        CTCLException
    );
 }
 /* Configure but invalid param count e.g. missing value: */
 void
 DevCmdTests::config_5() {
    {
        std::ofstream script(m_scriptFile);
        script << "device create abcd\n";  // Make the device.
        script << "device config abcd -test testing -second\n";  // missing -second value.

    }
    CPPUNIT_ASSERT_THROW(
        (*m_pParser)(),
        CTCLException
    );

    // I tihnk this is checked before attempting to configure so:

    auto device = m_pParser->findDevice("abcd");
    CPPUNIT_ASSERT(device);
    std::string value = device->getConfiguration()->cget("-test");
    CPPUNIT_ASSERT_EQUAL(std::string(""), value);
 }

 ///////////////////////////////// Test cget subcommand. ////////////////////////////

 /*  Cget of  one item:  */ 

 void
 DevCmdTests::cget_1() {
    { 
        std::ofstream script(m_scriptFile);
        script << "device create abcd -test testing -second two\n";
        script << "set config [device cget abcd -second]\n";  // in a var so we can get it from the interp.
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_pParser)()
    );

    const char* value = Tcl_GetVar(m_pParser->getInterpreter()->getInterpreter(), "config", TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(value);     // Was able to get it.
    std::string config(value);
    CPPUNIT_ASSERT_EQUAL(std::string("two"), config);
 }
 /* Cget all of the parameters: */

 void
 DevCmdTests::cget_2()
 {
    { 
        std::ofstream script(m_scriptFile);
        script << "device create abcd -test testing -second two\n";
        script << "set config [device cget abcd]";  // in a var so we can get it from the interp.
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_pParser)()
    );
    // Bit of white box here.  We know that the configuration parameters are in a map indexed by
    // option name so they will come out in alpha order:

    const char* value = Tcl_GetVar(m_pParser->getInterpreter()->getInterpreter(), "config", TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(value);     // Was able to get it.
    std::string config(value);
    CPPUNIT_ASSERT_EQUAL(std::string("{-second two} {-test testing}"), config);
 }
 /* no such device */

 void
 DevCmdTests::cget_3() {
    { 
        std::ofstream script(m_scriptFile);
        script << "device create abcd -test testing -second two\n";
        script << "set config [device cget abcdeee]";  
    }
    CPPUNIT_ASSERT_THROW(
        (*m_pParser)(),
        CTCLException
    );
 }
 /* no such config param*/ 
 void
 DevCmdTests::cget_4() {
    { 
        std::ofstream script(m_scriptFile);
        script << "device create abcd -test testing -second two\n";
        script << "set config [device cget abcd -no-such-option]";  
    }
    CPPUNIT_ASSERT_THROW(
        (*m_pParser)(),
        CTCLException
    );
 }
 /*  not allowed to specify more than one option: */

 void
 DevCmdTests::cget_5() {
    { 
        std::ofstream script(m_scriptFile);
        script << "device create abcd -test testing -second two\n";
        script << "set config [device cget abcd -test -second]";  
    }
    CPPUNIT_ASSERT_THROW(
        (*m_pParser)(),
        CTCLException
    );
 }
 