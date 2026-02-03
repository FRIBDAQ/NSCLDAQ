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
 * @file CVMUSBControl.cpp
 * @brief Implementation of the VMUSB module for the readout framework.
 * @author Ron Fox
 */

#include "CVMUSBControl.h"
#include "CReadoutModule.h"
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <CErrnoException.h>
#include <Globals.h>


/*------------------------------------------------------------------------------
 * Static data (used to define e.g. enumerators.
 */

// -nimo1 possible values:

static const char* nimo1Enum[] = {
  "busy", "trigger", "busrequest", "eventdatatobuffer", "dgga", "endevent", "usbtrigger", 0
};
// -nimo2 possible value:

static const char* nimo2Enum[] = {
  "usbtrigger", "vmecommand", "vmeas", "eventdatatobuffer", "dgga", "dggb", "endevent", 0
};
// -topyellow values:

static const char* topYellowEnum[] = {
  "ofifonotempty" , "infifonotempty", "scaler", "infifofull", "dtack", 
  "berr", "vmebr", "vmebg", 0
};

// -red values:

static const char* redEnum[] = {
  "trigger", "nimi1", "nimi2", "busy", "dtack", "berr", "vmebr", "vmebg", 0
};

// -green values:

static const char* greenEnum[] = {
  "acquire", "stacknotempty", "eventready", "trigger", "dtack", "berr", "vemebr", "vmebg", 0
};

// -bottomyellow values:

static const char* bottomYellowEnum[] = {
  "notslot1", "usbtrigger", "usbreset", "berr", "dtack", "vmebr", "vmebg", 0
};

// What makes scaler A count:

static const char* scalerATrigEnum[] = {
  "dgga", "nimi1", "nimi2", "event", 0
};
// What makes scaler B count:

static const char* scalerBTrigEnum[] = {
  "carry", "nimi1", "nimi2", "event", 0
};
// What starts DGG A:

static const char* dggAStartEnum[] = {
  "off", "nimi1", "nimi2", "trigger", "endofevent", "usbtrigger", "pulser", 0
};
// What starts DGG B:

static const char* dggBStartEnum[] = {
  "off", "nimi1", "nimi2", "trigger", "endofevent", "usbtrigger", "pulser", 0
};

static const char* bufferLengthEnum[] = {
  "13k","8k","4k","2k","1k","512","256","128","64", "evtcount",0
};

// Maximum value for 16 bit word:

static const int MaxInt12(0x0fff);
static const int MaxInt16(0xffff);

/*-------------------------------------------------------------------------------------
 * Canonicals (at least those that are legal).
 */

/**
 * Constructor:
 *   This just nulls out the pointers and sets the default device and LED source
 *   selectors.
 */
CVMUSBControl::CVMUSBControl() :
  m_pConfiguration(0),
  m_nDeviceSourceSelector(  CVMUSB::DeviceSourceRegister::nimO1Busy       |
			    CVMUSB::DeviceSourceRegister::nimO2VMEAS      |
			    CVMUSB::DeviceSourceRegister::dggADisabled    |
			    CVMUSB::DeviceSourceRegister::dggBDisabled
			    ),
  m_nLedSourceSelector(
		       CVMUSB::LedSourceRegister::topYellowOutFifoNotEmpty |
		       CVMUSB::LedSourceRegister::redEventTrigger          |
		       CVMUSB::LedSourceRegister::greenAcquire             |
		       CVMUSB::LedSourceRegister::bottomYellowNotArbiter
                       )
{}


/**
 * Destructor does nothing but, being virtual chains to the parent:
 */
CVMUSBControl::~CVMUSBControl() 
{}

/*---------------------------------------------------------------------------------------
 *
 * Virtual methods that implement the API for CReadoutHardware objects.
 */


/**
 * onAttach
 *
 *  Called once a configuration object has been associated with this object.
 *  *  Save the configuration object in m_pConfiguration
 *  *  Define the configuration options and their constraints as described in 
 *     CVMUSBControl.h and its class level comments.
 *
 * @param configuration - reference to the configuration object being attached
 *                        to this object.
 */
void
CVMUSBControl::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  // NIM01 configuration options:

  m_pConfiguration->addEnumParameter( "-nimo1", nimo1Enum, "busy");
  m_pConfiguration->addBooleanParameter("-latcho1", false);
  m_pConfiguration->addBooleanParameter("-inverto1", false);

  // NIMi2 configuration options:

  m_pConfiguration->addEnumParameter("-nimo2", nimo2Enum, "vmeas");
  m_pConfiguration->addBooleanParameter("-latcho2", false);
  m_pConfiguration->addBooleanParameter("-inverto2", false);

  // top yellow LED configuration:

  m_pConfiguration->addEnumParameter("-topyellow", topYellowEnum, "ofifonotempty");
  m_pConfiguration->addBooleanParameter("-latchtopyellow", false);
  m_pConfiguration->addBooleanParameter("-inverttopyellow", false);

  // red  LED configuration:

  m_pConfiguration->addEnumParameter("-red", redEnum, "busy");
  m_pConfiguration->addBooleanParameter("-latchred", false);
  m_pConfiguration->addBooleanParameter("-invertred", false);

  // green LED configuration

  m_pConfiguration->addEnumParameter("-green", greenEnum, "acquire");
  m_pConfiguration->addBooleanParameter("-latchgreen", false);
  m_pConfiguration->addBooleanParameter("-invertgreen", false);

  // Bottom yellow LED configuration:

  m_pConfiguration->addEnumParameter("-bottomyellow", bottomYellowEnum, "dtack");
  m_pConfiguration->addBooleanParameter("-latchbottomyellow", false);
  m_pConfiguration->addBooleanParameter("-invertbottomyellow", false);

  // Scaler confiugration -- including whether or not the scalers should be read and
  // how.

  m_pConfiguration->addBooleanParameter("-readscalers", false);
  m_pConfiguration->addBooleanParameter("-incremental", true);
  m_pConfiguration->addEnumParameter("-scalera", scalerATrigEnum, "nimi2");
  m_pConfiguration->addEnumParameter("-scalerb", scalerBTrigEnum, "carry");
  
  // Gate and delay generators:

  m_pConfiguration->addEnumParameter("-dgga", dggAStartEnum, "pulser");
  m_pConfiguration->addEnumParameter("-dggb", dggBStartEnum, "nimi2");

  m_pConfiguration->addIntegerParameter("-widtha", 0, MaxInt16, 1);
  m_pConfiguration->addIntegerParameter("-widthb", 0, MaxInt16, 1);

  m_pConfiguration->addIntegerParameter("-delaya", 0);
  m_pConfiguration->addIntegerParameter("-delayb", 0);

  m_pConfiguration->addBooleanParameter("-mixedbuffers",false);
  m_pConfiguration->addBooleanParameter("-spanbuffers",false);
  m_pConfiguration->addBooleanParameter("-forcescalerdump",false);
  m_pConfiguration->addIntegerParameter("-busreqlevel",0,7,4);
  m_pConfiguration->addBooleanParameter("-optionalheader",false);

  m_pConfiguration->addEnumParameter("-bufferlength", bufferLengthEnum, "13k"); 
  m_pConfiguration->addIntegerParameter("-eventsperbuffer", 1, MaxInt12, 1); 
}
/**
 * Initialize
 *   Initialize the bits of the VM-USB we control.  These include:
 *   *  LED Source select register.
 *   *  Device source select register.
 *   *  DGGA/DGGB gate and fine delay register.
 *   *  DGGA/DGGB coarse delay register.
 *
 * @param controller - CVMUSB reference we can use to access the VM-USB controller.
 */
void
CVMUSBControl::Initialize(CVMUSB& controller)
{
  // Compute and set the LED source register.

  uint32_t topYellow = m_pConfiguration->getEnumParameter("-topyellow", topYellowEnum) << 
    CVMUSB::LedSourceRegister::topYellowShift;
  uint32_t red       = m_pConfiguration->getEnumParameter("-red", redEnum) 
    << CVMUSB::LedSourceRegister::redShift;
  uint32_t green     = m_pConfiguration->getEnumParameter("-green", greenEnum) 
    << CVMUSB::LedSourceRegister::greenShift;
  uint32_t bottomYellow = m_pConfiguration->getEnumParameter("-bottomyellow", bottomYellowEnum) <<
    CVMUSB::LedSourceRegister::bottomYellowShift;

  uint32_t ledSourceSelect = topYellow | red | green | bottomYellow; // base values.
  if (m_pConfiguration->getBoolParameter("-latchtopyellow")) {
    ledSourceSelect |= CVMUSB::LedSourceRegister::topYellowLatch;
  }
  if (m_pConfiguration->getBoolParameter("-inverttopyellow")) {
    ledSourceSelect |= CVMUSB::LedSourceRegister::topYellowInvert;
  }
  
  if (m_pConfiguration->getBoolParameter("-latchred")) {
    ledSourceSelect |= CVMUSB::LedSourceRegister::redLatch;
  }
  if (m_pConfiguration->getBoolParameter("-invertred")) {
    ledSourceSelect |= CVMUSB::LedSourceRegister::redInvert;
  }

  if (m_pConfiguration->getBoolParameter("-latchgreen")) {
    ledSourceSelect |= CVMUSB::LedSourceRegister::greenLatch;
  }
  if (m_pConfiguration->getBoolParameter("-invertgreen")) {
    ledSourceSelect |= CVMUSB::LedSourceRegister::greenInvert;
  }

  if (m_pConfiguration->getBoolParameter("-latchbottomyellow")) {
    ledSourceSelect |= CVMUSB::LedSourceRegister::bottomYellowLatch;
  }
  if (m_pConfiguration->getBoolParameter("-invertbottomyellow")) {
    ledSourceSelect |= CVMUSB::LedSourceRegister::bottomYellowInvert;
  }

  controller.writeLEDSource(ledSourceSelect);

  // Compute and set the device source register.

  uint32_t nimO1 = m_pConfiguration->getEnumParameter("-nimo1", nimo1Enum)
    << CVMUSB::DeviceSourceRegister::nimO1Shift;
  uint32_t nimO2 = m_pConfiguration->getEnumParameter("-nimo2", nimo2Enum)
    << CVMUSB::DeviceSourceRegister::nimO2Shift;
  uint32_t scalera = m_pConfiguration->getEnumParameter("-scalera", scalerATrigEnum)
    << CVMUSB::DeviceSourceRegister::scalerAShift;
  uint32_t scalerb = m_pConfiguration->getEnumParameter("-scalerb", scalerBTrigEnum)
    << CVMUSB::DeviceSourceRegister::scalerBShift;
  uint32_t dgga = m_pConfiguration->getEnumParameter("-dgga", dggAStartEnum)
    << CVMUSB::DeviceSourceRegister::dggAShift;
  uint32_t dggb = m_pConfiguration->getEnumParameter("-dggb", dggBStartEnum)
    << CVMUSB::DeviceSourceRegister::dggBShift;
  uint32_t devSource = nimO1 | nimO2 | scalera | scalerb | dgga | dggb;

  if (m_pConfiguration->getBoolParameter("-latcho1")) {
    devSource |= CVMUSB::DeviceSourceRegister::nimO1Latch;
  }
  if (m_pConfiguration->getBoolParameter("-inverto1")) {
    devSource |= CVMUSB::DeviceSourceRegister::nimO1Invert;
  }
  if (m_pConfiguration->getBoolParameter("-latcho2")) {
    devSource |= CVMUSB::DeviceSourceRegister::nimO2Latch;
  }
  if (m_pConfiguration->getBoolParameter("-inverto2")) {
    devSource |= CVMUSB::DeviceSourceRegister::nimO2Invert;
  }


  // Enable scalers;

  devSource |= CVMUSB::DeviceSourceRegister::scalerAEnable 
    | CVMUSB::DeviceSourceRegister::scalerBEnable;


  // Setup and clear:
#ifdef VMUSB_IDLE_BUSY_WORKAROUND  

  Globals::deviceSelectorValue = devSource;  // Issue #422 save the value for later.
  
  #else 
    controller.writeDeviceSource(devSource 
    | CVMUSB::DeviceSourceRegister::scalerAReset   // Write the dev soure register.
    | CVMUSB::DeviceSourceRegister::scalerBReset);

  // Reassert setup without the clear:

  controller.writeDeviceSource(devSource);  
  #endif

  // Compute and set the DGGA width/fine delay register.
  
  uint32_t width = m_pConfiguration->getIntegerParameter("-widtha");
  uint32_t delaya = m_pConfiguration->getIntegerParameter("-delaya");
  
  controller.writeDGG_A((width << 16) | (delaya & 0xffff));
  
  // Compute and set the DGGB width/fine delay register.

  width = m_pConfiguration->getIntegerParameter("-widthb");
  uint32_t delayb = m_pConfiguration->getIntegerParameter("-delayb");

  controller.writeDGG_B((width << 16) | (delayb & 0xffff));

  // Compute and set the coarse delay register.

  delaya = delaya >> 16;
  delayb = delayb & 0xffff0000;
  controller.writeDGG_Extended(delayb | delaya);

  // Save the LED and device source select registers for
  // use in the readout list manipulation:

  m_nDeviceSourceSelector = devSource;
  m_nLedSourceSelector    = ledSourceSelect;

  configureGlobalMode(controller);

  configureEventsPerBuffer(controller);
}
/**
 * addReadoutList
 *
 *  Adds our stuff to the readout list.
 *  The important configuration parameters are:
 *  *  -readscalers - unless true nothing is done.
 *  *  -incremental - Determines if the scalers are cleared
 *     after the read.
 *
 * @param list - reference to a CVMUSBReadoutList that
 *               is potentially augmented by us.
 */
void
CVMUSBControl::addReadoutList(CVMUSBReadoutList& list)
{
  if (m_pConfiguration->getBoolParameter("-readscalers")) {

    // Scalers are disabled when read.

    uint32_t disable = m_nDeviceSourceSelector &
      (~(CVMUSB::DeviceSourceRegister::scalerAEnable |
	 CVMUSB::DeviceSourceRegister::scalerBEnable));

//   list.addRegisterWrite(CVMUSB::RegisterOffsets::DEVSrcRegister,
//			  disable);

    // Read the scalers.

    list.addRegisterRead(CVMUSB::RegisterOffsets::ScalerA);
    list.addRegisterRead(CVMUSB::RegisterOffsets::ScalerB);


    // If necessary, clear the scalers

    if (m_pConfiguration->getBoolParameter("-incremental")) {
      disable |= CVMUSB::DeviceSourceRegister::scalerAReset
	| CVMUSB::DeviceSourceRegister::scalerBReset;
      list.addRegisterWrite(CVMUSB::RegisterOffsets::DEVSrcRegister,
			    disable);
    }

    
  }
}
/**
 * onEndRun
 *     Called just before the run ends. 
 * 
 * @param controller - the VMUSB contoller object by refernce
 * 
 */
void
CVMUSBControl::onEndRun(CVMUSB& controller) {

}


void CVMUSBControl::configureGlobalMode(CVMUSB& controller)
{
  using namespace std;

  uint16_t glbl_mode = controller.readGlobalMode();
  
  if (m_pConfiguration->getBoolParameter("-spanbuffers")) {
     glbl_mode |= CVMUSB::GlobalModeRegister::spanBuffers;
  } else {
     glbl_mode &= (~CVMUSB::GlobalModeRegister::spanBuffers);
  }

  if (m_pConfiguration->getBoolParameter("-mixedbuffers")) {
     glbl_mode |= CVMUSB::GlobalModeRegister::mixedBuffers;
  } else {
     glbl_mode &= (~CVMUSB::GlobalModeRegister::mixedBuffers);
  }

  if (m_pConfiguration->getBoolParameter("-forcescalerdump")) {
     glbl_mode |= CVMUSB::GlobalModeRegister::flushScalers;
  } else {
     glbl_mode &= (~CVMUSB::GlobalModeRegister::flushScalers);
  }

  uint16_t breq = m_pConfiguration->getUnsignedParameter("-busreqlevel");
  glbl_mode &= (~CVMUSB::GlobalModeRegister::busReqLevelMask);
  glbl_mode |= (CVMUSB::GlobalModeRegister::busReqLevelMask 
                  & (breq << CVMUSB::GlobalModeRegister::busReqLevelShift));

  uint16_t buflen = m_pConfiguration->getEnumParameter("-bufferlength",bufferLengthEnum);
  glbl_mode &= (~CVMUSB::GlobalModeRegister::bufferLenMask);
  glbl_mode |= (CVMUSB::GlobalModeRegister::bufferLenMask 
                 & (buflen << CVMUSB::GlobalModeRegister::bufferLenShift));

  if (m_pConfiguration->getBoolParameter("-optionalheader")) {
     glbl_mode |= CVMUSB::GlobalModeRegister::doubleHeader;
  } else {
     glbl_mode &= (~CVMUSB::GlobalModeRegister::doubleHeader);
  }

  controller.writeGlobalMode(glbl_mode);
 
  // Read back our value and check to see that it is what we set.
  uint16_t new_glbl_mode = controller.readGlobalMode();

  if (glbl_mode != new_glbl_mode) {
    stringstream msg;
    msg << "FAILURE when setting global mode register to 0x" 
        << hex << setfill('0') << setw(4) << glbl_mode;
    msg << " , 0x" << hex << setfill('0') << setw(4) << new_glbl_mode 
        << " was set instead";

    throw msg.str();
  }

}

void CVMUSBControl::configureEventsPerBuffer(CVMUSB& controller)
{
  uint32_t evtperbuf = m_pConfiguration->getUnsignedParameter("-eventsperbuffer");
  controller.writeEventsPerBuffer(evtperbuf); 

  uint32_t newval = controller.readEventsPerBuffer();

  // Check that it was set to what we specified 
  if (newval != (evtperbuf&0xfff)) {
    using namespace std;
    stringstream msg;
    msg << "FAILURE when setting events per buffer to 0x" << hex << setfill('0') << setw(4) << evtperbuf; 
    msg << " , 0x" << hex << setfill('0') << setw(4) << newval << " was set instead";

    throw msg.str();
  }
}

/**
 * clone
 *
 *  Virtual constructor no-op since copy construction is now
 *  unimplemented...returns a null pointer.
 */
CReadoutHardware*
CVMUSBControl::clone() const
{
  CVMUSBControl* pClone = new CVMUSBControl();
  if (m_pConfiguration) pClone->m_pConfiguration = m_pConfiguration;
  return pClone;
}
