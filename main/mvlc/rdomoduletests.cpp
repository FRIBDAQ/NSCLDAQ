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
#include "CReadoutHardware.cpp"
#undef private
#include "CReadoutModule.h"
#include "DeviceCommand.h"
#include "TCLConfigParser.h"
#include <XXUSBConfigurableObject.h>
#include <string>


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


