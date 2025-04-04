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
@brief Test Interaction with CReadoutHardware objects.
*/

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "CVMUSBReadoutList.h"
#include "CVMUSB.h"
#define private  public
#include "CReadoutHardware.h"
#undef private
#include "CReadoutModule.h"
#include "DeviceCommand.h"
#include "TCLConfigParser.h"
#include <XXUSBConfigurableObject.h>
#include <string>

#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <TCLInterpreter.h>
#include <tcl.h>

// these are test classes supplying concrete classes for abstract base classes.
// they are in a namespace to avoid collision with any similar classes in other tests:

namespace rdomoduletest {
    // Readout hardware for our tests that actuall sort of does something:
    // will be a marker class with a non-empty initialize...
    class TestReadoutHardware : public CReadoutHardware {
    public:
        std::string m_name;  // So we can verify that we found the module.
    public:
        TestReadoutHardware(std::string name) : m_name(name) {}
        virtual ~TestReadoutHardware() {}

        // add a -value option that is an integer:

        void onAttach(XXUSB::CConfigurableObject& config) {
            CReadoutHardware::onAttach(config);            // store it.
            config.addIntegerParameter("-value");         // marker value.
            config.addIntegerParameter("-base");          // base address for the heck of it.
        }
        void Initialize(CVMUSB& controller) {
            auto config = getConfiguration();
            controller.vmeWrite32(config->getIntegerParameter("-base"), CVMUSBReadoutList::a32UserData, 0x1234);
        }
        void addReadoutList(CVMUSBReadoutList& list) {
            auto config = getConfiguration();
            list.addMarker(config->getIntegerParameter("-value")); // Readout is a marker.
        }
        void onEndRun(CVMUSB& interface) {
            auto config = getConfiguration();
            interface.vmeWrite32(config->getIntegerParameter("-base"), CVMUSBReadoutList::a32UserData, 0);
        }
    };
    // We need a command to create//config our 'device'.

    class TestReadoutCmd : public DeviceCommand {
    public: 
        TestReadoutCmd(CTCLInterpreter& interp, TCLConfigParser& parser) :
            DeviceCommand(interp, "marky", parser) {}

        virtual CReadoutModule* createDevice(std::string name) {
            
            auto result = new CReadoutModule();
            result->SetDriver(new TestReadoutHardware(name));
            return result;
        }
    };
    // Specialised parser that has our TestReadoutCommand registered:

    class TestParser : public TCLConfigParser {
    public:
        TestParser(const std::string& infile) : TCLConfigParser(infile) {}
        virtual ~TestParser() {}

        virtual void addExtensions() {
            addExtension(*(new TestReadoutCmd(*getInterpreter(), *this)));
        }
    };

}

// Finally the test class.

class ReadoutModuleTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ReadoutModuleTests);
    CPPUNIT_TEST(setup_1);
    CPPUNIT_TEST_SUITE_END();

protected:                       // test declarations.
    void setup_1();
public:
    void setUp() {
        char nameTemplate[100];
        strcpy(nameTemplate, "config.tclXXXXXX");
        m_fd = mkstemp(nameTemplate);
        m_filename = nameTemplate;

        m_pParser = new rdomoduletest::TestParser(m_filename);
        m_pParser->initialize();

    }
    void tearDown() {
        close(m_fd);
        unlink(m_filename.c_str());
        delete m_pParser;
    }
private:                          // Data managed by setUp and tearDown:

    int              m_fd;
    std::string      m_filename;
    TCLConfigParser* m_pParser;

};
CPPUNIT_TEST_SUITE_REGISTRATION(ReadoutModuleTests);


///////////////// setup tests ///////////////////////////////////////

void ReadoutModuleTests::setup_1() {
    // m_pParser should have the command "marky"

    Tcl_Interp* pInterp = m_pParser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(pInterp, "marky", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);
}
