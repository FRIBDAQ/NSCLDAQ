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
#include "C3820.h"
#include "CReadoutModule.h"
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <XXUSBConfigurableObject.h>

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <map>
#include <set>
using namespace std;

//////////////////////////////////////////////////////////////////////////
//////////////////////////  Constants ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// To reduce the development time, only the required consts are created.
//

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

// input modes note the special value "default" because the default value
// depends on the 

static const char* inputModes[] = {
  "default", "None", "LNEInhLNE", "LNEInhboth", "LNEInhCount", "Inh4s",
  "LNEHiscal", "LneInhClr",
  nullptr
};

static std::map<std::string, uint32_t> inputModeValues  = {
   {"default", 0xffffffff},                 // detectable funky value.
   {"None", acqInpNone},                    // mode 0
   {"LNEInhLNE", acqInpLNEInhLNE},          // mode 1
   {"LNEInhboth", acqInpLNEInhboth},        // mode 2
   {"LNEInhCount", acqInpLNEInhCount},      // mode 3
   {"Inh4s", acqInpLNEInh4s},               // mode 4
   {"LNEHiscal", acqInpLNEHiscal},          // mode 5
   {"LneInhClr", acqInpLneInhClr}           // mode 6
};

//  These input modes are acceptable in timestamp mode.
static std::set<uint32_t> goodTsInputModes = {
  acqInpLNEInhLNE, acqInpLNEInhboth, acqInpLNEInhCount, acqInpLNEHiscal,
  acqInpLneInhClr
};


// Output mode strings and their corresopnding acqOutMode values.
// The default matches the hardcoded 11.2-007 value that puts a 50MHz
// clock out on an output.

static const char* outputModeStrings[] = {
  "clock50Mhz", "LNEAndLed", "clock2x10Mhz", "clock1x10Mhz",
  nullptr
};

static std::map<std::string, uint32_t> outputModeValues = {
  {"clock50Mhz", acqOutMode50Mhz},
  {"LNEAndLed", acqOutModeled},
  {"clock2x10Mhz", acqOutMode2x10Mhz},
  {"clock1x10Mhz", acqoutMode1x10Mhz}
};

//////////////////////////////////////////////////////////////////////////
////////////////////// Constructors and canonicals ///////////////////////
//////////////////////////////////////////////////////////////////////////

/*!
   Construction is a no-op.
*/
C3820::C3820() {}

C3820::~C3820() {}




/////////////////////////////////////////////////////////////////////////
//////////////////////// Object operations //////////////////////////////
/////////////////////////////////////////////////////////////////////////

/*!
   Called when the object is attached to a configuration.  We
   establish our configuration parameter.
   \param configuration : CReadoutModule& 
      Reference to the module that holds our configuration.
*/
void
C3820::onAttach(XXUSB::CConfigurableObject& configuration)
{
  m_pConfiguration = &configuration;

  // Add the base parameter:

  m_pConfiguration->addParameter("-base",
				 XXUSB::CConfigurableObject::isInteger, 
				 NULL, "0");
  m_pConfiguration->addParameter("-timestamp",
				 XXUSB::CConfigurableObject::isBool, NULL, "off");
  
  m_pConfiguration->addEnumParameter("-inputmode", inputModes, "default");
  m_pConfiguration->addEnumParameter("-outputmode", outputModeStrings, "clock50Mhz");
}
/*!
   Called to setup the module for data taking. We'll make a small setup list
   and submit it for immediate execution.  The base we will fetch from the
   configuration.
   \param controller : CVMUSB&
      Reference to the controller.<
*/
void
C3820::Initialize(CVMUSB& controller)
{
  // First figure out our base address:

  uint32_t base = getBase();
  std::string inputModeString = m_pConfiguration->cget("-inputmode");
  uint32_t    inputMode       = inputModeValues[inputModeString];
  std::string outputModeStr   = m_pConfiguration->cget("-outputmode");
  uint32_t    outputMode      = outputModeValues[outputModeStr];
  
  

  // Ensure that this module is an SIS 3820 and if so reset it. 
  // these are done as single shot operations so that the reset is conditional
  // on the correct module being installed, and in case there is a delay required
  // between reset and next acces...hopefully the USB turnaround will take care of
  // it or, if not, we can insert a usleep as needed.
  //

  uint32_t id;
  
  controller.vmeWrite32(base+KeyReset, CVMUSBReadoutList::a32UserData, 
				static_cast<uint32_t>(0));
  


  //  Now set up the initialization list:

  

  //--ddc NOT clear,  for clearing set this to zero
  bool tsMode = m_pConfiguration->getBoolParameter("-timestamp");
  uint32_t clearing;
  uint32_t lne;
  uint32_t inpmode;

  // Figure out the input mode etc. part of the ACQ register note:
  // Defaults depend on timestamp mode on/off.
  // Modes that don't have LNE enabled output a warning in timestamp mode
  // as there's no clear way to latch the sacler.
  
  if(tsMode){
    clearing = acqNonClearing; //  Don't clear on latch.
    lne=acqLNEFP;
    if (inputMode == 0xffffffff) {
      inpmode=acqInpLNEInhLNE;
    } else {
      inpmode = inputMode;
    } 
    cout << "We are in timestamp mode, using external LNE(1) and CLEARING IS DISABLED" << endl;
    
    // See if we need to warn that the input mode doesn't have LNE.
    
    if (goodTsInputModes.count(inpmode) == 0) {
      cout << "***** WARNING **** you selected timestamp mode but your input mode has no external LNE *****\n";
      cout << "***** WARNING **** this means I don't understand where the timestamp latch comes from.\n";
      cout << "***** WARNING **** you'd better know what you're doing or choose a different input mode\n";
    }
  } else {
    lne=acqLNEVME;
    
    if (inputMode == 0xffffffff) {
      inpmode=acqInpLNEInh4s; //inhibit in 4 groups!!
    } else  {
      inpmode = inputMode;                     // There are no 'bad' input modes
    }                                          // if we're not a timestamp module.
    clearing = 0; //  clear on latch.
  }
    
  controller.vmeWrite32(base+AcqMode, CVMUSBReadoutList::a32UserData,
		      acq32Bit    | lne   | acqArmWithFP | acqSRAMMemory |
		       inpmode | outputMode | acqModeLatch | clearing);
  controller.vmeWrite32(base+KeyFifoReset, CVMUSBReadoutList::a32UserData, uint32_t(0));
  controller.vmeWrite32(base+KeyClearCounters, CVMUSBReadoutList::a32UserData, uint32_t(0));
  controller.vmeWrite32(base+KeyArm, CVMUSBReadoutList::a32UserData, (uint32_t)0);
  controller.vmeWrite32(base+KeyEnable, CVMUSBReadoutList::a32UserData, (uint32_t)0);


  
}
/*!
  Add the event read of this module to the list.  All we need to do is
  read 32 longwords from the sdram memory.
  \param list : CVMUSBReadoutList&
  The list to add to.
*/

void
C3820::addReadoutList(CVMUSBReadoutList& list)
{
  uint32_t base = getBase();
  bool tsMode = m_pConfiguration->getBoolParameter("-timestamp");

  
  if (tsMode) {            // Assumed to LNE externally

    // Timestamped mode, only read channel 1, 17 and the highbits register.
    // Note the SIS 3820 manual numbers channels from 1 hence the 0/16 below.
    
    list.addRead32(base+ShadowCounters+0*sizeof(uint32_t), CVMUSBReadoutList::a32UserData);
    list.addRead32(base+ShadowCounters+16*sizeof(uint32_t), CVMUSBReadoutList::a32UserData);
    list.addRead32(base+HighBits, CVMUSBReadoutList::a32UserData);
  } else {
    // SOftware load next event.
    list.addWrite32(base+KeyLNE, CVMUSBReadoutList::a32UserData, (uint32_t)0);
    // Non timestamp mode, read all scalers.
    list.addBlockRead32(base+ShadowCounters, CVMUSBReadoutList::a32UserBlock,
        (uint32_t)32);
  }


}


///////////////////////////////////////////////////////////////////////////
////////////////////////// Private utilities /////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint32_t
C3820::getBase() const
{
  string baseString = m_pConfiguration->cget("-base");
  uint32_t base     = strtoul(baseString.c_str(), NULL, 0); // must work!
  return base;
}

//////////////////////////////// The command to manage the device:

/** constructor
 *    Create a DeviceCommand, "sis3820" that can manage Struck 3820 scalers.
 */
C3820Command::C3820Command(CTCLInterpreter& interp, TCLConfigParser& parser) :
  DeviceCommand(interp, "sis3820", parser) {}

/** destructor */

C3820Command::~C3820Command() {}

/**
 * createDevice - make the device module wrapping a C3820 object:
 * 
 * @param name - ignored name of the module.
 */
CReadoutModule*
C3820Command::createDevice(std::string name) {
    CReadoutModule* result = new CReadoutModule;
    result->SetDriver(new C3820);

    return result;
}
