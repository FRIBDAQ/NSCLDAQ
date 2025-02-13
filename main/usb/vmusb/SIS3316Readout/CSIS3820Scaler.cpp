/**
 * @file CSIS3820Scaler.cpp
 * @brief Implementation for the SIS3820 scaler used as a scaler in a scaler bank.
 * 
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
#include "CSIS3820Scaler.h"
#include <Globals.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <iostream>

// Register offsets of the SIS 3820 scaler module; These are byte offsets.

static const uint32_t CSR        = 0x00000000;     // Control status register.
static const uint32_t ModuleID   = 0x00000004;     // Firmware and ident.

static const uint32_t AcqMode    = 0x00000100; // Acquisition mode register.

// bits 32-47 of channel 1/17 48 bit channels:

static const uint32_t HighBits   = 0x00000210;


static const uint32_t KeyReset         = 0x00000400;     // Writing here resets.
static const uint32_t KeyFifoReset     = 0x00000404;   // Clear memory fifo logic.
static const uint32_t KeyClearCounters = 0x00000040c; // Clear scaler counters
static const uint32_t KeyLNE           = 0x00000410;
static const uint32_t KeyArm           = 0x00000414;
static const uint32_t KeyEnable        = 0x00000418;


static const uint32_t ShadowCounters = 0x00000800;

static const uint32_t SDRAM      = 0x00800000;

// Register bits:
//
static const uint32_t idMask     = 0xffff0000;
static const uint32_t idValue    = 0x38200000;



// Acquisition mode bits/fields and other values:

static const uint32_t acqNonClearing   = 0x00000001; //  Don't clear on latch.
static const uint32_t acq32Bit         = 0x00000000; // Data format is 32bits.
static const uint32_t acq24bit         = 0x00000004;
static const uint32_t acq16bit         = 0x00000008;
static const uint32_t acq8bit          = 0x0000000c; 
static const uint32_t acqLNEVME        = 0x00000000;
static const uint32_t acqLNEFP         = 0x00000010;
static const uint32_t acqLNE10Mhz      = 0x00000020;
static const uint32_t acqLNEChanN      = 0x00000030;
static const uint32_t acqLNEPresetN    = 0x00000040;
static const uint32_t acqArmWithFP     = 0x00000000;
static const uint32_t acqArmWithChN    = 0x00000100;
static const uint32_t acqSRAMMemory    = 0x00001000;
static const uint32_t acqAddMode       = 0x00002000;
static const uint32_t acqInpNone       = 0x00000000;
static const uint32_t acqInpLNEInhLNE  = 0x00010000;
static const uint32_t acqInpLNEInhboth = 0x00020000;
static const uint32_t acqInpLNEInhCount= 0x00030000;
static const uint32_t acqInpLNEInh4s   = 0x00040000;
static const uint32_t acqInpLNEHiscal  = 0x00050000;
static const uint32_t acqInpLneInhClr  = 0x00060000;
static const uint32_t acqInpInvert     = 0x00080000;
static const uint32_t acqOutModeled    = 0x00000000;
static const uint32_t acqOutMode50Mhz  = 0x00100000;
static const uint32_t acqOutMode2x10Mhz= 0x00200000;
static const uint32_t acqoutMode1x10Mhz= 0x00300000;
static const uint32_t acqOutInvert     = 0x00800000;
static const uint32_t acqModeLatch     = 0x00000000;
static const uint32_t acqModeMCS       = 0x20000000;
static const uint32_t acqModeHisto     = 0x30000000;
static const uint32_t acqModeTest      = 0x70000000;

//////////////////////////////// Implementation ////////////////////////////////////////////

/**
 * constructor
 *   @param pName - name given to the module for the configurable bank
 *   @param base  - Module VME base address.
 */
CSIS3820Scaler::CSIS3820Scaler(const char* pName, uint32_t base) :
    m_name(pName), m_base(base) {}

/** Destructor
 * 
 */
CSIS3820Scaler::~CSIS3820Scaler() {}

////////////////////////////// Selectors ?/////////////////////////////////

/**
 * base 
 *   @return uint32_t - module base address.
 */
uint32_t
CSIS3820Scaler::base() const {
    return m_base;
}

/**
 * name
 *   @return std::string - name of the module
 */
std::string
CSIS3820Scaler::name() const {
    return m_name;
}

/**
 * initialize
 *    - Output mode will be the 50MHz clock
 *    - Input mode will be inhiobit banks of four counters.
 */
void
CIS3820Scaler::initialize() {
    // the controller better have been built by now:

    CVMUSB& controller = *Globals::pUSBController;

    // shamelessly lifted and modifed from daqconfig/C3820.cpp:

  uint32_t base = m_base;
  
  uint32_t    inputMode       = acqInpLNEInh4s;
  uint32_t    outputMode      = acqOutMode50Mhz;
  
  

  // Ensure that this module is an SIS 3820 and if so reset it. 
  // these are done as single shot operations so that the reset is conditional
  // on the correct module being installed, and in case there is a delay required
  // between reset and next acces...hopefully the USB turnaround will take care of
  // it or, if not, we can insert a usleep as needed.
  //

  uint32_t id;
  int status = controller.vmeRead32(base+ModuleID,  CVMUSBReadoutList::a32UserData,
				    &id);
  if (status) {
    throw string("C3820::Initialize Single shot vme to read id register failed");
  }
  if ((id & idMask) != idValue) {
    char msg[1000];
    sprintf(msg, "C3820::Initialized, module @ 0x%08x is not an SIS3820 scaler",
	    base);
    throw string(msg);
  }
  status = controller.vmeWrite32(base+KeyReset, CVMUSBReadoutList::a32UserData, 
				static_cast<uint32_t>(0));
  if(status) {
    throw string("C3820::Initialize single shot write to key-reset faileed");
  }


  //  Now set up the initialization list:

  CVMUSBReadoutList initList;

  //--ddc NOT clear,  for clearing set this to zero
  bool tsMode = false;
  uint32_t clearing;
  uint32_t lne;
  uint32_t inpmode;

  
  
    lne=acqLNEVME;

    if (inputMode == 0xffffffff) {
        inpmode=acqInpLNEInh4s; //inhibit in 4 groups!!
    } else  {
        inpmode = inputMode;                     // There are no 'bad' input modes
    }                                          // if we're not a timestamp module.
    clearing = 0; //  clear on latch.

    
  initList.addWrite32(base+AcqMode, CVMUSBReadoutList::a32UserData,
		      acq32Bit    | lne   | acqArmWithFP | acqSRAMMemory |
		       inpmode | outputMode | acqModeLatch | clearing);
  initList.addWrite32(base+KeyFifoReset, CVMUSBReadoutList::a32UserData, uint32_t(0));
  initList.addWrite32(base+KeyClearCounters, CVMUSBReadoutList::a32UserData, uint32_t(0));
  initList.addWrite32(base+KeyArm, CVMUSBReadoutList::a32UserData, (uint32_t)0);
  initList.addWrite32(base+KeyEnable, CVMUSBReadoutList::a32UserData, (uint32_t)0);


  uint32_t inBuffer[100];
  size_t   bytesRead;
  status = controller.executeList(initList,
				      &inBuffer, sizeof(inBuffer), &bytesRead);
  if (status < 0) {
    throw string("C3820::Could not initialize via executeList.");
  }
}
/**
 *  read
 *     Read the 32 channels of scaler data.
 * 
 * @return std::vector<uint32_t>
 */
std::vector<uint32_t>
CSIS3820Scaler::read() {
    std::vector<uint32_t> result;
    result.resize(32);        // # scalers that can be read.
    uint32_t base = m_base;
    CVMUSB& controller = *Globals::pUsbController;
    size_t junk;

    controller.vmeWrite32(base+KeyLNE, CVMUSBReadoutList::a32UserData, (uint32_t)0); // latch the scalers.
    controller.vmeBlockRead(base+ShadowCounters, CVMUSBReadoutList::a32UserBlock,
                result.data(), (size_t)32, &junk);

    return result;
}