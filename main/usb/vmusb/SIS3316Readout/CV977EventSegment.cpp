/**
 * @file CV977EventSegment.cpp
 * @brief Implement the event segment for the CAEN V977 eventsegment.
 * @author Ron Fox <fox at frib dot msu dot edu>
 *  This software is Copyright by the Board of Trustees of Michigan
 *  State University (c) Copyright 2025
*
*  You may use this software under the terms of the GNU public license
*   (GPL).  The terms of this license are described at:
*
*    http://www.gnu.org/licenses/gpl.txt
*
*    Author:
*            Ron Fox
*            Facility for Rare Isotop Beams
*            Michigan State University
*            East Lansing, MI 48824-1321
* 
 */
#include "CV977EventSegment.h"
#include <XXUSBConfigurableObject.h>
#include <Globals.h>

#includu <CVMUSB.h>
#incldue <CVMUSBReadoutList.h>

///////////////////////////////////   register offsets  ////////////////////////////////

#define Const(name) static const int name  = 
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

// Address modifier for all operations.

static const uint8_t amod(CVMUSBReadoutList::a32UserData); // for all accesses.

/////////////////////////////////// configuration constraints ///////////////////////////

static XXUSB::CConfigurableObject::limit Zero(0);
static XXUSB::CConfigurableObject::limit Uint16(0xffff);
static XXUSB::CConfigurableObject::limit Byte(0xff);
static XXUSB::CConfigurableObject::limit IPLHigh(7);

static XXUSB::CConfigurableObject::Limits Short(Zero, Uint16);
static XXUSB::CConfigurableObject::Limits VectorValues(Zero, Byte);
static XXUSB::CConfigurableObject::Limits IPLLimits(Zero, IPLHigh);

static const char* ReadModeValues[] = {
    "singlehit", "multihit", nullptr
};

//////////////////////// Method Implementations ///////////////////////////////////////////////

/**
 *  constructor
 *    - Initializes the VME interface to null as it might not exist yet
 *    - Save our name.
 *    - Create and stock the configuration.
 * 
 * @param name - the name of this object.
 */
CV977EventSegment::CV977EventSegment(const char* name) :
    m_name(name), m_pConfiguration(nullptr), m_pVME(nullptr)
{
    m_pConfiguration = new XXUSB::CConfigurableObject(m_name);
    setupConfiguration();
}
/** destructor: 
 * 
*/
CV977EventSegment::~CV977EventSegment() {
    delete m_pConfiguration;

}

/**
 * initialize
 *     Called after we've been configured successfully.  Setup the
 * module's registers properly... and clear it to prep for the first trigger.
 * This is lifted shamelessly from the VMUSB CV977 class.
 */

void
CV977EventSegment::initialize() {
    // Get the base address. We sort of need that to be able to 
  // do anything.  This module has no signature words to allow us
  // to determine if the module is what it's supposed to be so 
  // we'll charge ahead blind and do a software reset before building
  // the initialization list:
  
  uint32_t base  = m_pConfiguration->getIntegerParameter("-base");
  CVMUSB& controller = *Globals::pUSBController;
  controller.vmeWrite16(base + Reset, amod, (uint16_t)0xffff);   // Reset the module.

  // Get the rest of the parameters:

  uint16_t  inputMask  = m_pConfiguration->getUnsignedParameter("-inputmask");
  uint16_t  outputMask = m_pConfiguration->getIntegerParameter("-outputmask");

  bool      pattern    = m_pConfiguration->getBoolParameter("-pattern");
  bool      gate       = m_pConfiguration->getBoolParameter("-gate");
  bool      ormask     = m_pConfiguration->getBoolParameter("-ormask");

  // setup the module

  CVMUSBReadoutList   list;
  list.addWrite16(base + InputMask,  amod, inputMask);
  list.addWrite16(base + OutputMask, amod, outputMask);
  

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
  if (status < 0) {                           // List execution failed.
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
  // Clear single an multi hit by doing a read:

  uint32_t junk;
  controller.vmeRead32(base + SingleHitRdClear, amod, &junk);
  controller.vmeRead32(base + MultiHitRdClear, amod, &junk);
}

/**
 * read
 *   @param pBuffer - pointer to where to put the data.
 *   @param maxwords - Maximum number of 16 bit words that can fit  in pBuffer.
 * 
 */
size_t
CV977EventSegment::read(void* pBuffer, size_t maxwords) {
    // We just read one 16 bit word

    // Need some config info:

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

    Globals::pUSBController->vmeRead16(base + offset, amod, pBuffer);   // Read the register.

    return 1;
}

//////////////////////////// private utilities: //////////////////////////////////////////////////

/**
 * setupConfiguration
 *    Define the configuration and its constraints:
 */

 void
 CV977EventSegment::setupConfiguration() {
    configuration.addIntegerParameter("-base");
    configuration.addIntegerParameter("-inputmask", 0, 0xffff, 0);
    configuration.addEnumParameter("-readmode", ReadmodeValues, "singlehit");
    configuration.addIntegerParameter("-outputmask",0, 0xffff, 0);
    configuration.addBooleanParameter("-readandclear",true);
    configuration.addBooleanParameter("-pattern", false);
    configuration.addBooleanParameter("-gate", true);
    configuration.addBooleanParameter("-ormask", false);
 }