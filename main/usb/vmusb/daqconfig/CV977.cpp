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
*/

// Implementation of the C785 class VM-USB support for the CAEN V785.


#include <config.h>
#include <CV977.h>

#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <set>

#include <iostream>
#ifdef MVLC_GENERATOR
#include <XXUSBConfigurableObject.h>
#endif

using namespace std;

/*
  Local constants
*/

#define Const(name) static const int name  = 


// Address modifiers:

static const uint8_t amod(CVMUSBReadoutList::a32UserData); // for all accesses.

// Register map (relative to the base address:

Const(InputSet)           0x0000;
Const(InputMask)          0x0002;
Const(InputRead)          0x0004;
Const(SingleHitRead)      0x0006;
Const(MultiHitRead)       0x0008;
Const(OutputSet)          0x000a;
Const(OutputMask)         0x000c;
Const(InterruptMask)      0x000e;
Const(ClearOutput)        0x0010;
Const(SingleHitRdClear)   0x0016;
Const(MultiHitRdClear)    0x0018;
Const(TestControl)        0x001a;
Const(Ipl)                0x0020;
Const(Vector)             0x0022;
Const(SerialNumber)       0x0024;
Const(FirmwareRev)        0x0026;
Const(Control)            0x0028;
Const(Reset)              0x002e;

// Control register bits:

Const(CSR_Pattern)        0x0001;
Const(CSR_GateMask)       0x0002;
Const(CSR_OrMask)         0x0004;



//  Parameters for validity checking.

static XXUSB::CConfigurableObject::limit Zero(0);
static XXUSB::CConfigurableObject::limit Uint16(0xffff);
static XXUSB::CConfigurableObject::limit Byte(0xff);
static XXUSB::CConfigurableObject::limit IPLHigh(7);

static XXUSB::CConfigurableObject::Limits Short(Zero, Uint16);
static XXUSB::CConfigurableObject::Limits VectorValues(Zero, Byte);
static XXUSB::CConfigurableObject::Limits IPLLimits(Zero, IPLHigh);


static XXUSB::CConfigurableObject::isEnumParameter readModeValues;

// The trick below initializes the readModeValues set once and forever:

class CV977InitReadModeValues
{
public:
  CV977InitReadModeValues() {
    readModeValues.insert("singlehit");
    readModeValues.insert("multihit");
  }
};

static CV977InitReadModeValues initReadModeValues; // Forces construction.



/*
 *  Implementation of canonical functions.
 */



/*!
   Construction of the module. .Most of the real work is done at
   onAttach and initialization time, as well as by the configurator.
*/
CV977::CV977() :
  m_pConfiguration(0)
{

}
#ifndef MVLC_GENERATOR
/*!
   In copy construction, we want to copy the configuration if it already exists,
   so that we have a faithful copy of the rhs object .. including configuration.
   This can create a small memory leak as the destructor does not delete this.
*/
CV977::CV977(const CV977& rhs) :
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}
#endif
/*!
   Destructor...see above.
*/
CV977::~CV977()
{}
#ifdef MVLC_GENERATOR
/*!
  Assignment returns self else copying the configuration can lead to an
  infinite recursion loop.
*/
CV977&
CV977::operator=(const CV977& rhs)
{
  return *this;
}
#endif

/*!
   onAttach is called when the configuration entity is attached to this
   device.  The configuration entry must be populated with the options
   recognized by the driver for this device specifically:

   See the header for the recognized configuration parameters, their types
   limits and default values.

   @param configuration : CReadoutModule&
    The configuration that will be attached to this module.
*/
void 
#ifdef MVLC_GENERATOR
CV977::onAttach(XXUSB::CConfigurableObject& configuration)
#else
CV977::onAttach(CReadoutModule& configuration)
#endif
{
  m_pConfiguration = &configuration;

  configuration.addParameter("-base", XXUSB::CConfigurableObject::isInteger,
			     NULL, "0");
  configuration.addParameter("-inputmask", XXUSB::CConfigurableObject::isInteger,
			     &Short, "0");
  configuration.addParameter("-readmode", XXUSB::CConfigurableObject::isEnum,
			     &readModeValues, "singlehit");
  configuration.addParameter("-outputmask", XXUSB::CConfigurableObject::isInteger,
			     &Short, "0");
  configuration.addParameter("-interruptmask", XXUSB::CConfigurableObject::isInteger,
			     &Short, "0");
  configuration.addParameter("-readandclear", XXUSB::CConfigurableObject::isBool,
			     NULL, "true");
  configuration.addParameter("-ipl", XXUSB::CConfigurableObject::isInteger,
			     &IPLLimits, "0");
  configuration.addParameter("-vector", XXUSB::CConfigurableObject::isInteger,
			     &VectorValues, "0");
  configuration.addParameter("-pattern", XXUSB::CConfigurableObject::isBool,
			     NULL, "false");
  configuration.addParameter("-gate", XXUSB::CConfigurableObject::isBool,
			     NULL, "true");
  configuration.addParameter("-ormask", XXUSB::CConfigurableObject::isBool,
			     NULL, "false");





}
/*!
  Initialize the module, making it ready for data taking.
  This measn getting the value of all the parameters and programming 
  them into the appropriate registers.

  @param controller - Handle to the object that is able to perform
                      VM-USB operations for us.
*/
void
CV977::Initialize(CVMUSB& controller)
{

  // Get the base address. We sort of need that to be able to 
  // do anything.  This module has no signature words to allow us
  // to determine if the module is what it's supposed to be so 
  // we'll charge ahead blind and do a software reset before building
  // the initialization list:
  
  uint32_t base  = m_pConfiguration->getIntegerParameter("-base");
  controller.vmeWrite16(base + Reset, amod, (uint16_t)0xffff);

  // Get the rest of the parameters:

  uint16_t  inputMask  = m_pConfiguration->getUnsignedParameter("-inputmask");
  uint16_t  outputMask = m_pConfiguration->getIntegerParameter("-outputmask");
  uint16_t  ipl        = m_pConfiguration->getIntegerParameter("-ipl");
  uint16_t  vector     = m_pConfiguration->getIntegerParameter("-vector");

  bool      pattern    = m_pConfiguration->getBoolParameter("-pattern");
  bool      gate       = m_pConfiguration->getBoolParameter("-gate");
  bool      ormask     = m_pConfiguration->getBoolParameter("-ormask");

  // Build the list to initialize the module and fill it with instructions to
  // program all the registers but the control register.

  CVMUSBReadoutList   list;
  list.addWrite16(base + InputMask,  amod, inputMask);
  list.addWrite16(base + OutputMask, amod, outputMask);
  list.addWrite16(base + Ipl,        amod, ipl);
  list.addWrite16(base + Vector,     amod, vector);

  // compute the control register value and add instructions to set it:

  uint16_t csrValue = 0;
  if (pattern) csrValue |= CSR_Pattern;
  if (!gate)   csrValue |= CSR_GateMask;
  if (ormask)  csrValue |= CSR_OrMask;

  list.addWrite16(base + Control, amod, csrValue);

  // Now execute the initialization list.. don't have any read list but
  // we'll make a dummy:

  uint32_t inBuffer;
  size_t   bytesRead;
  
  int status = controller.executeList(list, &inBuffer, sizeof(inBuffer),
				      &bytesRead);
  if (status < 0) {
    int errorValue = errno;
    string message = "CV977::Initialization list execution failed on: ";
    if (status == -1) {
      message += " usb_bulk_write ";
    }
    if (status == -1) {
      message += " usb_bulk_read ";
    }
    message += strerror(errorValue);
    throw message;
  }
}
/*!
   Add instructions tothe readout list that describe how to read this module out.
   There's only one 16 bit register to read, but which one is read
   dependson the value of -readmode and -readandclear.

   @param list - The CVMEReadoutList that is being built up to load in to the
                 VM-USB as the readout stack.
*/
void
CV977::addReadoutList(CVMUSBReadoutList& list)
{
  // Need a few parameters to figure out the exact register to read:

  uint32_t base     = m_pConfiguration->getUnsignedParameter("-base");
  string   mode     = m_pConfiguration->cget("-readmode");
  bool     rdclear  = m_pConfiguration->getBoolParameter("-readandclear");

  // figure out the offset:

  uint32_t offset;
  if (mode == "singlehit") {
    if(rdclear) {
      offset = SingleHitRdClear;
    }
    else {
      offset = SingleHitRead;
    }
  }
  else {
    if (rdclear) {
      offset = MultiHitRdClear;
    }
    else {
      offset = MultiHitRead;
    }

  }
  list.addRead16(base + offset, amod);

}
#ifndef MVLC_GENERATOR
/*!
   Virtual copy constructor
*/
CReadoutHardware* 
CV977::clone() const
{
  return new CV977(*this);
}

#endif

#ifdef MVLC_GENERATOR
//////////////////////////////////////  V977Command implementation:

/**
 * constructor - nothing spectacular here.
 */
V977Command::V977Command(CTCLInterpreter& interp, TCLConfigParser& parser) :
  DeviceCommand(interp, "v977", parser) {}

/** 
 * destructor
 */
V977Command::~V977Command() {}

/**
 *  creatDevice 
 *    Encapsulate a new CV977 in a Readout module
 */
CReadoutModule* 
V977Command::createDevice(std::string name) {
  CReadoutModule* result = new CReadoutModule;
  result->SetDriver(new CV977);

  return result;
}
#endif
