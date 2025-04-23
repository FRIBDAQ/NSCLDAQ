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
#include <config.h>
#include "CV1495sc.h"
#include <XXUSBConfigurableObject.h>
#include "CReadoutModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include <stdint.h>


/**
 * Register definitions for the module:
 * Names uswed are from the table in section 3 of the CAEN V1495Scaler reference manual.
 *
 */

static const uint32_t MEB(0);	           // Multievent buffer

static const uint32_t ACQ_CTRL(0x1000);    // Acquisition control
static const uint32_t ACQ_CTRL_SET(0x1004); // Bit set
static const uint32_t ACQ_CTRL_CLR(0x1008); // bit clear

// Bit defintions:

static const uint32_t EN_EXTTRG(0x01);
static const uint32_t EN_INTTRG(0x02);
static const uint32_t G_PORT_TYPE(0x04);
static const uint32_t AUTO_RESET(0x08);
static const uint32_t G1_MODE_INHIBIT(0x00); // g1_mode bits
static const uint32_t G1_MODE_RESET(0x10);
static const uint32_t G1_MODE_TEST(0x20);
static const uint32_t EN_TIMETAG(0x40);

static const uint32_t FWREV(0x100c); // Firmware revision

static const uint32_t STATUS(0x1010);	 // Status register:
static const uint32_t STATUS_MEBFULL(1); // no more room left in MEB.

static const uint32_t COMMANDS(0x1014); // Command register:
static const uint32_t COMMANDS_SWTRG(0x01);
static const uint32_t COMMANDS_SWCNTRES(0x02);
static const uint32_t COMMANDS_SWCLR(0x04);
static const uint32_t COMMANDS_BDRESET(0x10);

static const uint32_t CHEN_A(0x1020);
static const uint32_t CHEN_B(0x1024);
static const uint32_t CHEN_D(0x1028);
static const uint32_t CHEN_E(0x102c);

static const uint32_t DWELL_TIME(0x1030);

/* constants that have to do with the configuration values */

// Legal values for the -inputlevel parameter:

static  const char* inputLevels[] = {
  "nim", "ttl", 0
};

static const uint32_t inputLevelRegisterValues[] = {
  0, G_PORT_TYPE
};

// Legal values for the g1 mode and their register values:

static  const char*  g1Modes[] = {
  "inhibit", "reset", "test", 0
};
static const uint32_t g1ModesRegisterValues[] = {
  G1_MODE_INHIBIT, G1_MODE_RESET, G1_MODE_TEST
};


// Address modifiers we're going to use:

static const uint8_t initAmod(CVMUSBReadoutList::a32PrivData);
static const uint8_t rdoAmod(CVMUSBReadoutList::a32PrivBlock);


// We're not interested in a live read of the counters so those registers are omitted.


/*----------------------------------------------------------------------------------------------
**
** Canonical methods:
*/

/**
 * Constructor for the V1495 Scaler module.  This is pretty much a no-op.
 */
CV1495sc::CV1495sc() :
  m_pConfiguration(0)
{}

/**
 * Destructor ... also a no-op.  We are not the owner of storage that belongs to 
 * m_pConfiguration.
 */
CV1495sc::~CV1495sc() 
{}


/*------------------------------------------------------------------------------------------
**
** Operations that are required of module implementations (pure virtual in the base class).
**
*/

/**
 * onAttach - saves the configuration object and sets it up to accept the set of 
 * configuration options we support.  See the class header for a description of these
 * options.
 * @param configuration - Reference to the module's configuration.
 */
void
CV1495sc::onAttach(XXUSB::CConfigurableObject& configuration)
{
  m_pConfiguration = &configuration;

  m_pConfiguration->addParameter("-base", XXUSB::CConfigurableObject::isInteger, NULL, "0");
  m_pConfiguration->addParameter("-exttrig", XXUSB::CConfigurableObject::isBool, NULL, "false");
  m_pConfiguration->addParameter("-inttrig", XXUSB::CConfigurableObject::isBool, NULL, "false");
  m_pConfiguration->addParameter("-vmetrigger", XXUSB::CConfigurableObject::isBool, NULL, "true");
  m_pConfiguration->addEnumParameter("-inputlevel", inputLevels,
				     "nim");
  m_pConfiguration->addParameter("-autoreset", XXUSB::CConfigurableObject::isBool, NULL, "true");
  m_pConfiguration->addEnumParameter("-g1_mode",
				     g1Modes, "inhibit");
  m_pConfiguration->addParameter("-includetime", XXUSB::CConfigurableObject::isBool, NULL, "false");
  m_pConfiguration->addParameter("-bank1enables", XXUSB::CConfigurableObject::isInteger, NULL, "0xffffffff");
  m_pConfiguration->addParameter("-bank2enables", XXUSB::CConfigurableObject::isInteger, NULL, "0xffffffff");
  m_pConfiguration->addParameter("-bank3enables", XXUSB::CConfigurableObject::isInteger, NULL, "0xffffffff");
  m_pConfiguration->addParameter("-bank4enables", XXUSB::CConfigurableObject::isInteger, NULL, "0xffffffff");

  m_pConfiguration->addParameter("-dwelltime", XXUSB::CConfigurableObject::isInteger, NULL, "0x2000000");
  
}
/**
 * Initialize is called as daq is starting.  We are expected to process the current 
 * state of the configuration and load the device registers appropriately.
 *
 * @param controller - reference to the object that is connected with the VM-USB controller.
 *
 */
void
CV1495sc::Initialize(CVMUSB& controller)
{
  // First get the base address and reset the module.  This is done is 
  // single shot mode as it may take the modules some time to recover from the
  // reset:

  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");
  controller.vmeWrite32(base + COMMANDS, initAmod, COMMANDS_BDRESET | COMMANDS_SWCLR | COMMANDS_SWCNTRES);

  // Get the rest of the parameters:

  bool exttrig = m_pConfiguration->getBoolParameter("-exttrig");
  bool inttrig = m_pConfiguration->getBoolParameter("-inttrig");
  bool vmetrig = m_pConfiguration->getBoolParameter("-vmetrigger");

  std::string inputLevelString = m_pConfiguration->cget("-inputlevel");
  uint32_t  inputLevelValue         = mapEnum(inputLevelString, inputLevels, inputLevelRegisterValues);

  bool autoreset = m_pConfiguration->getBoolParameter("-autoreset");
  
  std::string g1ModeString = m_pConfiguration->cget("-g1_mode");
  uint32_t g1ModeValue = mapEnum(g1ModeString, g1Modes, g1ModesRegisterValues);

  bool includeTime = m_pConfiguration->getBoolParameter("-includetime");
  uint32_t dwelltime = m_pConfiguration->getUnsignedParameter("-dwelltime");
  uint32_t b1enables = m_pConfiguration->getUnsignedParameter("-bank1enables");
  uint32_t b2enables = m_pConfiguration->getUnsignedParameter("-bank2enables");
  uint32_t b3enables = m_pConfiguration->getUnsignedParameter("-bank3enables");
  uint32_t b4enables = m_pConfiguration->getUnsignedParameter("-bank4enables");


  // The remainder of the setup is done in a listwith the
  // last thing done to clear the module counters and MEB:
  // list are not supported in mvlc init translations.

  CVMUSB& list(controller);
  
  // Compute and set the value of the ACQ_CTRL register (well put the set in the list):

  uint32_t acqctrlValue = g1ModeValue;
  if (exttrig)     acqctrlValue |= EN_EXTTRG;
  if (inttrig)     acqctrlValue  |= EN_INTTRG;
  acqctrlValue |= inputLevelValue;
  if (autoreset)   acqctrlValue |= AUTO_RESET;
  if (includeTime) acqctrlValue |= EN_TIMETAG;
  list.vmeWrite32(base + ACQ_CTRL, initAmod, acqctrlValue);

  // Add dwell time register to the list:

  list.vmeWrite32(base + DWELL_TIME, initAmod, dwelltime);
 
  // The channel enable registers:

  list.vmeWrite32(base + CHEN_A, initAmod, b1enables);
  list.vmeWrite32(base + CHEN_B, initAmod, b2enables);
  list.vmeWrite32(base + CHEN_D, initAmod, b3enables);
  list.vmeWrite32(base + CHEN_E, initAmod, b4enables);

  // finally clear the mdoule MEB and counters:

  list.vmeWrite32(base+COMMANDS, initAmod, COMMANDS_SWCNTRES | COMMANDS_SWCLR);
  
}
/**
 * addReadoutList is called to produce the segment of a VMUSBReadoutList that
 * reads this device.  We're going to build a block mode read, however the number of longs
 * will depend on :
 * - If time readout is enabled (it adds one longword).
 * - The channel enables (only enabled channels appear in the data stream).
 *
 * @param list - Reference to a VMUSBReadoutList that we are going to add our entries to.
 *
 */
void
CV1495sc::addReadoutList(CVMUSBReadoutList& list)
{
  // Get the parameters we need to figure out the size of the block read...and just 
  // as important...where to direct it.

  uint32_t base          = m_pConfiguration->getUnsignedParameter("-base");
  bool     haveTimestamp = m_pConfiguration->getBoolParameter("-includetime");
  uint32_t b1enables     = m_pConfiguration->getUnsignedParameter("-bank1enables");
  uint32_t b2enables     = m_pConfiguration->getUnsignedParameter("-bank2enables");
  uint32_t b3enables     = m_pConfiguration->getUnsignedParameter("-bank3enables");
  uint32_t b4enables     = m_pConfiguration->getUnsignedParameter("-bank4enables");

  // compute the size..there's always a header, there may be a timestamp
  // there's a long for each bit set in the various enabls masks.

  size_t transfers = 1;	// Control register.
  if(haveTimestamp) transfers++; // timestamp.
  transfers += countBits(b1enables);
  transfers += countBits(b2enables);
  transfers += countBits(b3enables);
  transfers += countBits(b4enables);


  // Now all of this has built up to the following anticlimax:

  list.addWrite32(base + COMMANDS, initAmod, COMMANDS_SWTRG);
  list.addDelay(1);
  list.addFifoRead32(base + MEB, rdoAmod, transfers);
}

/*-----------------------------------------------------------------------------------*/

// Private utilities:

/**
 * Map an enumerated value to a 32 bit integer via a look up table:
 *
 * @param value - the value of the enum to map.
 * @param stringArray - null terminated array of pointers to legal values for 'value'
 * @param valueArray - corresponding values e.g. stringArray[i] => valueArray[i]
 * 
 * @return uint32_t
 * @retval the looked up value from valueArray.
 * @throw std::string  if the value is not in stringArray
 */
uint32_t
CV1495sc::mapEnum(std::string value, 
		  const char** stringArray, 
		  const uint32_t* valueArray)
{
  while(*stringArray) {
    if (value == *stringArray) {
      return *valueArray;
    }
    stringArray++;
    valueArray++;
  }
  throw std::string("mapEnum had an illegal value for the string array it was given to search");
}

/**
 * Countbits:  Counts the bits that are set in a 32 bit word.  This uses
 * the method shown in the AMD optimization guide at 
 * http://suport.amd.com/us/Embedded_TechDocs/25112.PDF on pages
 *  following pg 179.
 *
 * @param mask - 32 bit unsigned mask.
 *
 * @return size_t
 * @retval number of bits set in mask
 *
 */
size_t
CV1495sc::countBits(uint32_t mask)
{
  uint32_t v = mask - ((mask >> 1) & 0x55555555);       
  v = (v & 0x33333333) + ((v >> 2) & 0x33333333);  
  size_t result =  ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count

  return result;
}
