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
/**
 * @file C3804.cpp
 * @brief implementation for supporting SIS 3804 scaler in MVLC an VMUSB
 * @note when compiling for mvlcgenerate, MVLC_GENERATOR is defined.
 */

#include <config.h>
#include "C3804.h"

#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef MVLC_GENERATOR
#include <XXUSBConfigurableObject.h>
#endif

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////


/*
  Local constants:
*/

// Register offsets:

static const uint32_t CSR         = 0x000;
static const uint32_t IDIRQ       = 0x004;
static const uint32_t DISABLES    = 0x00c;
static const uint32_t CLEARALL    = 0x020;
static const uint32_t LATCH       = 0x024;
static const uint32_t ENABLE      = 0x028;
static const uint32_t DISABLE     = 0x02c;
static const uint32_t BCSTCLR     = 0x030;
static const uint32_t BCSTLATCH   = 0x034;
static const uint32_t BCSTENABLE  = 0x038;
static const uint32_t BCSTDISABLE = 0x03c;
static const uint32_t CLEARGROUP  = 0x040;
static const uint32_t REFPULSERON = 0x050;
static const uint32_t REFPULSEROFF= 0x054;
static const uint32_t RESET       = 0x060;
static const uint32_t ONEPULSE    = 0x068;
static const uint32_t INDIVCLEAR  = 0x100; // array of clears for counter N and assoc. overflow.
static const uint32_t INDIVOVFCLR = 0x180; // array of overflow clears for counter N.
static const uint32_t SHADOW      = 0x200; // Array of shadow registers (clock first)
static const uint32_t COUNTER     = 0x280; // Primary counters {also clocks shadow}
static const uint32_t RDCLEAR     = 0x300; // Read shadow and clear primary counters.
static const uint32_t OVERFLOWS   = 0x380; // overflow flags.

// Control register bits:

static const uint32_t CR_LEDON         = 0x00000001;
static const uint32_t CR_SETIRQTEST    = 0x00000002;
static const uint32_t CR_SETINMODE0    = 0x00000004;
static const uint32_t CR_SETINMODE1    = 0x00000008;

static const uint32_t CR_ENABLE25MHZ   = 0x00000010; // 25mhz pulses in all channels.
static const uint32_t CR_ENABLEINTST   = 0x00000020; // enable input test mode.
static const uint32_t CR_ENABLEBCST    = 0x00000040; // enable response to broadcast addressing.
static const uint32_t CR_ENABLEHNDSHK  = 0x00000080; // enable as handshake controller for bcst mode.

static const uint32_t CR_LEDOFF        = 0x00000100; // Disable LED.
static const uint32_t CR_CLRIRQTEST    = 0x00000200; // Disable irq test mode.
static const uint32_t CR_CLRINMODE0    = 0x00000400; // Clear input mode bit 0.
static const uint32_t CR_CLRINMODE1    = 0x00000800; // Clear input mode bit 1.

static const uint32_t CR_DISABLE25MHZ  = 0x00001000; // Disable 25 mhz pulses in all channels.
static const uint32_t CR_DISABLEINTST  = 0x00002000; // Disable input test mode.
static const uint32_t CR_DISABLEBCST   = 0x00004000; // Disable broadcast test mode.
static const uint32_t CR_DISABLEHNDSHK = 0x00008000; // Disable broadcast hanshake controller.

static const uint32_t CR_ENABLEIRQ0    = 0x00100000;
static const uint32_t CR_ENABLEIRQ1    = 0x00200000;
static const uint32_t CR_ENABLEIRQ2    = 0x00400000;

static const uint32_t CR_DISABLEIRQ0   = 0x10000000;
static const uint32_t CR_DISABLEIRQ1   = 0x20000000;
static const uint32_t CR_DISABLEIRQ2   = 0x40000000;


//  Access methods:

static const uint8_t CONTROLAMOD    = CVMUSBReadoutList::a32UserData;
static const uint8_t READAMOD       = CVMUSBReadoutList::a32UserBlock;

/////////////////////////////////////////////////////////////////////////////////////

// Canonical methods:

// Construction is a noop... 
// configuration is done with onAttach:

C3804::C3804()                 {}

C3804::~C3804() {}

#ifndef MVLC_GENERATOR
C3804::C3804(const C3804& rhs) {}
C3804&
C3804::operator=(const C3804& rhs) {
  return *this;
}
#endif

///////////////////////////////////////////////////////////////////////////////////

// Public entries:


/*!
   Called when the object is attached to its configuration.
   We establish our configuration parameters, and validators,
   as well as saving a pointer to the configuration so that we can query
   it later.

   \param configuration - Reference to our configuration.
*/
void
#ifdef MVLC_GENERATOR
C3804::onAttach(XXUSB::CConfigurableObject& configuration) 
#else
C3804::onAttach(CReadoutModule& configuration) 
#endif
{
  m_pConfiguration = &configuration;

  // Define configuraton parameters we recognize:

  m_pConfiguration->addParameter("-base",
				 XXUSB::CConfigurableObject::isInteger,
				 NULL, "0");
  m_pConfiguration->addParameter("-refpulser",
				 XXUSB::CConfigurableObject::isBool,
				 NULL, "false");
  m_pConfiguration->addParameter("-disables",
				 XXUSB::CConfigurableObject::isInteger,
				 NULL, "0");
  m_pConfiguration->addParameter("-autoclear",
				 XXUSB::CConfigurableObject::isBool,
				 NULL, "true");
}

/*!
    Initialize the module to the appropriate mode.
    - Ensure the module is really a 3804 by reading the Module ID register.
    - RESET the module.
    - Program the control register... input mode 1, interrupts off BCST Mode disabled,
    - Depending on the state of the -disables,  program the count disable register.
    - If desired, turn on the reference pulser if not disable.
    - Clear all counters and overflows.

    \param controller - USB Controller object reference.

*/
void
C3804::Initialize(CVMUSB& controller) 
{
  // Get the configuration paramters we need:

  uint32_t base     = getIntegerParameter("-base");
  uint32_t disables = getIntegerParameter("-disables");
  bool     refpulser= getBoolParameter("-refpulser");


  // Check we have a 3804  only for VMUSB:
#ifndef MVLC_GENERATOR
  uint32_t id;
  checkRead(controller, base + IDIRQ, id);
  if ((id & 0xffff0000) != 0x38040000) {
    char message[1024];
    sprintf(message, "Module at 0x%x is not a 3804, id register: 0x%x",
	    base, id);
    throw string(message);

  }
  #endif
  // We've got a 3804, so now we can program the beast.. user led is off until
  // the beast is programmed, then turned back on.
  //

  checkWrite(controller, base + RESET, 0); // reset the module.
  checkWrite(controller, base + CSR, CR_LEDOFF |
	     CR_SETINMODE1 | CR_CLRINMODE1 | CR_DISABLEBCST | CR_DISABLEHNDSHK |
	     CR_DISABLEIRQ0 | CR_DISABLEIRQ1 | CR_DISABLEIRQ2);
  checkWrite(controller, base + DISABLE, disables);
  if (refpulser) {
    checkWrite(controller, base + REFPULSERON, 0);
  }
  else {
    checkWrite(controller, base + REFPULSEROFF, 0);
  }
  checkWrite(controller, base + ENABLE, 0); // Allow counting.

  // Ready to roll:

  checkWrite(controller, base + CSR, CR_LEDON);

}

/*!
   Add our readout instructions to the readout stack we live in.
   How we operate depends on the state of the -autoclear
   configuration parameter.
   - -autoclear true - We just do a block read from the RDCLEAR block.
     that clocks the shadow registers, reads and clears.
   - -autoclear false - we hit the LATCH regiseter and do a read of the SHADOW
      block.


    \note There are only 8 scalers to read.
*/
void
C3804::addReadoutList(CVMUSBReadoutList& list)
{
  bool     clear = getBoolParameter("-autoclear");
  uint32_t base  = getIntegerParameter("-base");

  
  if (clear) {
    list.addRead32(base+RDCLEAR, CONTROLAMOD);
  }
  else {
    list.addRead32(base+SHADOW, CONTROLAMOD);
  }
  for (int i =1; i < 8; i++) {
    list.addRead32(base+SHADOW+i*sizeof(uint32_t), CONTROLAMOD);
  }

}
#ifndef MVLC_GENERATOR
/*!
   Clone is essentially a virtual copy constructor.  We return a new *this.
*/
CReadoutHardware* 
C3804::clone() const
{
  return new C3804(*this);
}
#endif

///////////////////////////////////////////////////////////////////////////////////

// Utility functions to get configuration parameters.

// Return the value of an integer parameter.
// Parameters:
//    std::string name - name of the parameter.
// Returns:
//    value
// Throws a string exception (from cget) if there is no such parameter.
// caller is responsible for ensuring the parameter is an int.
//
uint32_t
C3804::getIntegerParameter(string name) const
{
  string sValue =  m_pConfiguration->cget(name);
  uint32_t    value  = strtoul(sValue.c_str(), NULL, 0);

  return value;
}
//  Return the value of a bool parameter.
// Parameters:
//    std::string name - name of the parameter.
// Returns:
//   true if the value is one of: true, yes, 1, on, enabled.
bool
C3804::getBoolParameter(string name) const
{
  string sValue = m_pConfiguration->cget(name);
  set<string> trueValues;
  trueValues.insert("true");
  trueValues.insert("yes");
  trueValues.insert("yes");
  trueValues.insert("1");
  trueValues.insert("on");
  trueValues.insert("enabled");


  return (trueValues.count(sValue) != 0);
}

// Utility functions to do I/O that throws exceptions on errors:
// Address modifiers are always CONTROLAMOD.
// Parameters:
//     controller - VMUSB controller handle.
//     address    - Location to write/read.
//     value      - data to read/write (ref on read, value on write).
//

void
C3804::checkRead(CVMUSB& controller, uint32_t address, uint32_t& value) 
{
#ifdef MVLC_GENERATOR
  throw string("Cannot do checkRead in C3804 in mvlcgenerator <BUG>");
#else
  int status = controller.vmeRead32(address, CONTROLAMOD, &value);
  if (status != 0) {
    throw string("Read from 3804 failed!!");
  }
#endif
}
void
C3804::checkWrite(CVMUSB& controller, uint32_t address, uint32_t value)
{
  int status = controller.vmeWrite32(address, CONTROLAMOD, value);
#ifndef MVLC_GENERATOR         // VMUSB Writes can fail. MVLCgenerator can't.
  if (status != 0) {
    throw string("Write to 3804 failed!");
  }
#endif
}

#ifdef MVLC_GENERATOR

//////////////////// Implement SIS3804Command

/** constructor
 * 
 */
SIS3804Command::SIS3804Command(CTCLInterpreter& interp, TCLConfigParser& parser) :
  DeviceCommand(interp, "sis3804", parser) {}

SIS3804Command::~SIS3804Command() {}

  /** 
   * createDevice 
  */
CReadoutModule* SIS3804Command::createDevice(std::string name) {
    auto result = new CReadoutModule;
    result->SetDriver(new C3804);

    return result;
}

#endif
