/*
@file CXLMTimestamp.cpp
@brief implement the XXUSB Timestamp reader.
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/







#include "CXLMTimestamp.h"

#include <stdint.h>
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include "CReadoutModule.h"
#include <XXUSBConfigurableObject.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using XXUSB::CConfigurableObject;

static const uint32_t Interrupt              (0x000004); // Interrupt/reset register.
static const uint8_t  registerAmod           (CVMUSBReadoutList::a32UserData);
static const uint8_t  blockTransferAmod      (CVMUSBReadoutList::a32UserBlock);
static const uint8_t  privBlockTransferAmod  (CVMUSBReadoutList::a32PrivBlock);


// Import the XLM namesapce
using namespace XLM;


CXLMTimestamp::CXLMTimestamp()  : CXLM() {}

/**
 * Destruction.  If your object creatd any dynamic data it must be freed here:
 */
CXLMTimestamp::~CXLMTimestamp() 
{}

///////////////////////////////////////////////////////////////////////////////////////
// Interfaces the driver provides to the framework.

/**
 * This function is called when an instance of the driver has been associated with
 * its configuration database.  The template code stores that in m_pConfiguration
 * The configuration is a CReadoutModule which in turn is derived from
 * XXUSB::CConfigurableObject which encapsulates the configuration database.
 *
 *  You need to invoke methods from XXUSB::CConfigurableObject to create configuration parameters.
 *  by convention a configuration parameter starts with a -.  To illustrate this,
 *  template code will create a -base parameter that captures the base address of the module.
 *  In addition we'll create an -id parameter which will be the value of a marker that will
 *  be put in the event.  The marker value will be constrainted to be 16 bits wide.
 *
 * @parm configuration - Reference to the configuration object for this instance of the driver.
 */
void
CXLMTimestamp::onAttach(CConfigurableObject& configuration)
{
  // Call the base class's onAttach
  // This stores the m_pConfiguration pointer for later use
  CXLM::onAttach(configuration);
}

/**
 * The initialization routine sets loads the firmware if users want it to be loaded.
 * It then clears the scaler value.
 * 
 * @param controller - Refers to a CVUSB controller object connected to the VME crate
 *                     being managed by this framework.
 *
 */
void
CXLMTimestamp::Initialize(CVMUSB& controller)
{

  // Load the firmware
  if (m_pConfiguration->getBoolParameter("-loadfirmware")) {
    std::string firmware = m_pConfiguration->cget("-firmware");
    if (firmware == "") {
      throw std::string("CXLMTimestamp - -loadfirmware requires that -firmware be configured");
    }

    loadFirmware(controller, firmware);
    // Can't sleep.  Have to add a delay to the stack being buitl:

    // sleep(1);
    controller.delay(1000);
  }

  // Clear the scaler
  accessBus(controller, CXLM::REQ_X);
  controller.vmeWrite32( FPGA(), registerAmod, static_cast<uint32_t>(1)); 
  controller.vmeWrite32( FPGA(), registerAmod, static_cast<uint32_t>(0)); 
  accessBus(controller, 0);
  
}

/**
 * This method is called to ask a driver instance to contribute to the readout list (stack)
 * in which the module has been placed.  Normally you'll need to get some of the configuration
 * parameters and use them to add elements to the readout list using CCUSBReadoutList methods.
 *
 * @param list - A CCUSBReadoutList reference to the list that will be loaded into the
 *               CCUSB.
 */
void
CXLMTimestamp::addReadoutList(CVMUSBReadoutList& list)
{

  // acquire the bus
  list.addWrite32(Interface() + 0x0000c, registerAmod, static_cast<uint32_t>(1));
  addBusAccess(list, CXLM::REQ_X, static_cast<uint32_t>(0));


  // read the two scaler values
  list.addRead32(FPGA() + 1*sizeof(uint32_t), registerAmod);
  list.addRead32(FPGA() + 2*sizeof(uint32_t), registerAmod);

  // release the bus
  addBusAccess(list, 0, static_cast<uint32_t>(0));
  list.addWrite32(Interface() + 0x0000c, registerAmod, static_cast<uint32_t>(0));
}

////////////////////  Implement the creator command:
/**
 * construtor:
 */
XLMTSCommand::XLMTSCommand(CTCLInterpreter& interp, TCLConfigParser& parser) :
  DeviceCommand(interp, "XLMTimestamp", parser) {}

XLMTSCommand::~XLMTSCommand() {}

/**
 * createDevice
 *     Create the device instance:
 */
CReadoutModule*
XLMTSCommand::createDevice(std::string name) {
  auto dev =  new CXLMTimestamp;
  auto result = new CReadoutModule;
  result->SetDriver(dev);

  return result;
}
