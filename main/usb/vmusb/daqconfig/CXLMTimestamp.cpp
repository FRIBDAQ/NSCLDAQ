/*
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


/**
 *  This file contains a template device driver for the CC-USB 
 * readout framework.  While these comments are no substitute
 * for the documentation available online at http://docs.nscl.msu.edu
 * The idea is that you will have to modify several sections of this code
 * Each section you need to modify is bracketed with comments of the form
 *  MODIFY ME HERE and
 *  END MODIFICATIONS
 *
 *  Comments above eac modification describe what you are doing and to some extent why.
 *
 * The final objective of this exercise is to produce a Tcl loadable package that you
 * can incorporate into the CC-USB readout framework via a load /some/path/to/yourpackage.so
 * command in your configuration file.
 *
 * As such each driver consists of two segments:
 * The main chunk is device driver code.  Device driver code 
 *
 * - Establishes configuration parameters and their constraints.
 * - Uses the values of those configuration parameters to initialize an instance of the device.
 * - Uses the values of the configuration parameters to contribute the CCUSB instructions required
 *   to read the device to the CC-USB readout list to which the module was assigned.
 *
 *  The second chunk is package initialization code. In that chunk, we need to make your driver
 *  known to the configuration subsystem, assigning it a Tcl command ensemble that can
 *  create, configure, and  query the configuration of instances of the driver.
 */


/*
 *  This template builds a driver class named XLMTimestamp  The first thing you should do
 *  is do a global search and replace fo XLMTimestamp to a name that matches the
 *  device you are trying to support.

 * MODIFY ME HERE
 *
 *  END MODIFICATIONS>
 */

#include <CReadoutHardware.h>
#include <stdint.h>
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <CXLMTimestamp.h>
#ifdef MVLC_GENERATOR
#include <XXUSBConfigurableObject.h>
#endif

static const uint32_t Interrupt              (0x000004); // Interrupt/reset register.
static const uint8_t  registerAmod           (CVMUSBReadoutList::a32UserData);
static const uint8_t  blockTransferAmod      (CVMUSBReadoutList::a32UserBlock);
static const uint8_t  privBlockTransferAmod  (CVMUSBReadoutList::a32PrivBlock);


// Import the XLM namesapce
using namespace XLM;


CXLMTimestamp::CXLMTimestamp()  : CXLM() {}
#ifndef MVLC_GENERATOR
CXLMTimestamp::CXLMTimestamp(const CXLMTimestamp& rhs)
   : CXLM(rhs)
{}
#endif

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
#ifdef MVLC_GENERATOR
CXLMTimestamp::onAttach(CConfigurableObject& configuration)
#else
CXLMTimestamp::onAttach(CReadoutModule& configuration)
#endif
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
    std::cout << "Loading firmware from file : " << firmware << std::endl;

    loadFirmware(controller, firmware);
#ifdef MVLC_GENERATOR
    controller.delay(100000);
#else    
    sleep(1);
#endif    
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
#ifndef MVLC_GENERATOR
/**
 * This method virtualizes copy construction by providing a virtual method that
 * invokes it.  Usually you don't have to modify this code.
 *
 * @return CXLMTimestamp*
 * @retval Pointer to a dynamically allocated driver instance created by copy construction
 *         from *this
 */
CReadoutHardware*
CXLMTimestamp::clone() const
{
  return new CXLMTimestamp(*this);
}

#endif

#ifdef MVLC_GENERATOR

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

#endif