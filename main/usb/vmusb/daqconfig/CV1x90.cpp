/*
    This software  Copyright by the Board of Trustees of Michigan
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
#include "CV1x90.h"

#include "CReadoutModule.h"
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <os.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string>
#include <set>

#include <iostream>

#include <CCAENV1x90Opcodes.h>
#include <CCAENV1x90Registers.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////

// constants that are not in the module header files:

#define Const(name) static const int name  = 


static const uint8_t initamod(CVMUSBReadoutList::a32UserData);
static const uint8_t readamod(CVMUSBReadoutList::a32UserBlock);

///////////////////////////////////////////////////////////////////////////////////////////

// Constraints to set on configuration parameters:


static XXUSB::CConfigurableObject::limit zero(0);	// A useful lower limit.
static XXUSB::CConfigurableObject::limit noLimit;     // No limit.

// The virtual slot number is constrained to be 0..31

static XXUSB::CConfigurableObject::limit  vsnMax(31);
static XXUSB::CConfigurableObject::Limits vsnLimits(zero, vsnMax);

// The IPL is constrained to be  0..7

static XXUSB::CConfigurableObject::limit iplMax(7);
static XXUSB::CConfigurableObject::Limits iplLimits(zero, iplMax);

// The vector is constrained to be 0..255:

static XXUSB::CConfigurableObject::limit   vectorMax(255);
static XXUSB::CConfigurableObject::Limits  vectorLimits(zero, vectorMax);

//  Termination is an enumeration:

static const char* terminationStrings[] = {"none", "switch", "on", NULL};
static XXUSB::CConfigurableObject::isEnumParameter 
                 terminationValues(XXUSB::CConfigurableObject::makeEnumSet(terminationStrings));

// The interrupt high water mark is constrained to be 0...65535:

static XXUSB::CConfigurableObject::limit   highWaterMax(65535);
static XXUSB::CConfigurableObject::Limits  highWaterLimits(zero, highWaterMax);

// The ECL output pin is an enum:

static const char* eclOutputStrings[] = {"ready", "full", "almostfull", "error", NULL};
static XXUSB::CConfigurableObject::isEnumParameter
                eclOutputValues(XXUSB::CConfigurableObject::makeEnumSet(eclOutputStrings));


// The trigger matching window is in the range 0..4095:

static XXUSB::CConfigurableObject::limit   windowMax(4095);
static XXUSB::CConfigurableObject::Limits  windowLimits(zero, windowMax);


// The window offset parameter runs -2048..40:

static XXUSB::CConfigurableObject::limit   offsetLow(-2048);
static XXUSB::CConfigurableObject::limit   offsetMax(40);
static XXUSB::CConfigurableObject::Limits  offsetLimits(offsetLow, offsetMax);

// The extra search margin is in the range 0..4095, as is the reject margin:

static XXUSB::CConfigurableObject::limit  marginMax(4095);
static XXUSB::CConfigurableObject::Limits marginLimits(zero, marginMax);

// Edge detect parameters are an enum:

static const char* edgeDetectStrings[] = {"pair", "leading", "trailing", "both", NULL};
static XXUSB::CConfigurableObject::isEnumParameter
               edgeDetectValues(XXUSB::CConfigurableObject::makeEnumSet(edgeDetectStrings));

// The edge detection resolution is an enum:

static const char* edgeResolutionStrings[] = {"800ps", "200ps", "100ps", "25ps", NULL};
static XXUSB::CConfigurableObject::isEnumParameter
     edgeResolutionValues(XXUSB::CConfigurableObject::makeEnumSet(edgeResolutionStrings));

// The leading edge resolution is also an enumeration:

static const char* leResolutionStrings[] = {"100ps", "200ps", "400ps", "800ps", "1.6ns", "3.12ns",
				     "6.25ns", "12.5ns", NULL};
XXUSB::CConfigurableObject::isEnumParameter
     leResolutionValues(XXUSB::CConfigurableObject::makeEnumSet(leResolutionStrings));

// pair width resolution is also an enum... along one:

static const char* pairResolutionStrings[] = {"100ps", "200ps", "400ps", "800ps", "1.6ns", 
				       "3.2ns", "6.25ns", "12.5ns", "25ns", "50ns",
				       "100ns", "200ns", "400ns", "800ns", NULL};
static XXUSB::CConfigurableObject::isEnumParameter
     pairResolutionValues(XXUSB::CConfigurableObject::makeEnumSet(pairResolutionStrings));

// Dead time resolution:

static const char* deadtimeStrings[] = {"5ns", "10ns", "30ns", "100ns", NULL};
static XXUSB::CConfigurableObject::isEnumParameter
           deadtimeValues(XXUSB::CConfigurableObject::makeEnumSet(deadtimeStrings));

// The maximum number of hits is actually an enum too:

static const char* maxHitStrings[] = {"0", "1", "2", "4", "8", "16", "32", "64", "128", 
			       "infinite", NULL};
static XXUSB::CConfigurableObject::isEnumParameter
     maxHitValues(XXUSB::CConfigurableObject::makeEnumSet(maxHitStrings));

// The global offset  must be two integers:

static const XXUSB::CConfigurableObject::limit listSize(2);
static  XXUSB::CConfigurableObject::ListSizeConstraint globalListSize = {listSize, listSize};



// Static member data:

// These are maps that convert an enumerated string to a parameter value to write
// to the micro.

CV1x90::EnumMap  CV1x90::m_edgeMap(CV1x90::edgeMap()); 
CV1x90::EnumMap  CV1x90::m_edgeResolutionMap(CV1x90::edgeResolutionMap());
CV1x90::EnumMap  CV1x90::m_leResolutionMap(CV1x90::leResolutionMap());
CV1x90::EnumMap  CV1x90::m_widthResolutionMap(CV1x90::widthResolutionMap());
CV1x90::EnumMap  CV1x90::m_deadtimeMap(CV1x90::deadtimeMap());
CV1x90::EnumMap  CV1x90::m_maxhitsMap(CV1x90::maxhitsMap());


////////////////////////////////////////////////////////////////////////////////////


/*!
  Constructor does not do much but initialize the member values to some nonsense
  values:
*/
CV1x90::CV1x90() :
  m_pConfiguration(0),
  m_Model(0),
  m_Suffix('\0'),
  m_nChipCount(0),
  m_nChannelCount(0)
{
  
}
/*!
   Copy construction:
*/
CV1x90::CV1x90(const CV1x90& rhs) :
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
  m_Model         = rhs.m_Model;
  m_Suffix        = rhs.m_Suffix;
  m_nChipCount    = rhs.m_nChipCount;
  m_nChannelCount = rhs.m_nChannelCount;
}

/*! null copy construction */

CV1x90::~CV1x90() {}


CV1x90&
CV1x90::operator=(const CV1x90& rhs)
{
  if (this != &rhs) {
    if (m_pConfiguration) {
      delete m_pConfiguration;
    }
    if (rhs.m_pConfiguration) {
      m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
    }
    else {
      m_pConfiguration = 0;
    }
    m_Model         = rhs.m_Model;
    m_Suffix        = rhs.m_Suffix;
    m_nChipCount    = rhs.m_nChipCount;
    m_nChannelCount = rhs.m_nChannelCount;

  }
  return *this;
}



////////////////////////////////////////////////////////////////////////////////////////


/*!
   Attach the object to a new configuration.  
   Save the configuration object and configure it with the configuration parameters
   we support.
   \param configuration - configuration object associated with this object.
*/
void
CV1x90::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;
  createConfiguration();
}


/*!
  Initialize the module.  This is going to be a long, drawn out process.
  It will also probably require a good deal of clock time as the
  module requires some interaction with its micro control register to 
  operate.

  TODO:  Model specific validations (e.g. only 1290 can specify resolution of 25ps.
*/
void
CV1x90::Initialize(CVMUSB& controller)
{
  // Get all the configuration parameters:

  uint32_t base            = m_pConfiguration->getUnsignedParameter("-base");
  int      vsn             = m_pConfiguration->getIntegerParameter("-vsn");
  int      ipl             = m_pConfiguration->getIntegerParameter("-ipl");
  int      ivector         = m_pConfiguration->getIntegerParameter("-vector");
  string   termination     = m_pConfiguration->cget("-termination");
  bool     tagTime         = m_pConfiguration->getBoolParameter("-tagtime");
  uint16_t highWater       = m_pConfiguration->getIntegerParameter("-highwatermark");
  string   eclOutput       = m_pConfiguration->cget("-ecloutput");
  uint16_t windowWidth     = m_pConfiguration->getIntegerParameter("-window");
  uint16_t windowOffset    = m_pConfiguration->getIntegerParameter("-offset");
  uint16_t extraSearch     = m_pConfiguration->getIntegerParameter("-extramargin");
  uint16_t rejectMargin    = m_pConfiguration->getIntegerParameter("-rejectmargin");
  bool     subtractTrigger = m_pConfiguration->getBoolParameter("-triggerrelative");
  string   edgeDetect      = m_pConfiguration->cget("-edgedetect");
  string   edgeResolution  = m_pConfiguration->cget("-edgeresolution");
  string   leResolution    = m_pConfiguration->cget("-leresolution");
  string   widthResolution = m_pConfiguration->cget("-widthresolution");
  string   deadtime        = m_pConfiguration->cget("-deadtime");
  bool     encapsulateChip = m_pConfiguration->getBoolParameter("-encapsulatechip");
  string   maxHits         = m_pConfiguration->cget("-maxhits");
  bool     markErrors      = m_pConfiguration->getBoolParameter("-errormark");
  bool     bypassOnError   = m_pConfiguration->getBoolParameter("-errorbypass");
  vector<long int> offset       = m_pConfiguration->getIntegerList("-globaloffset");


  vector<pair<int, int> > adjusts = getChannelAdjustments();


  // Reset the module, 
  validateModule(controller, base);

  controller.vmeWrite16(base + CCAENV1x90Registers::WReset, initamod, (uint16_t)0);
  WaitMicro(controller, base);
  Os::usleep(1000);			// Wait another msec just in case.
  WaitMicro(controller,base);	// ..and wait for the micro to be ready.

  // Figure out m_Model, m_Suffix, m_nChipCount, and m_nChannelCount

  moduleType(controller, base);
  computeCounts();

  // A bunch of the setup can be done by writing registers (not touching the micro)
  // for all of these we can construct a VMUSB list and pop that off in one shot...
  // which is quicker than doing a bunch of one-shot operations to the VM-USB
  // For the micro... we'll have no choice. 

  CVMUSBReadoutList registerWrites;

  // The control register will be set as follows:
  // BERREN        - enable bus error on end of transfer.
  // TERM/TERM_SW  - According to the value of the termination configuration parameter.
  // EMPTY_EVENT   - Enabled to make unpacking easier.
  // ALIGN64       - Disabled.
  // COMPENSATION  - Enabled.
  // TESTFIFO      - Disabled.
  // Compensaton RAM read - Disabled
  // Event FIFO enabled: - Disabled.
  // Trigger time enabled - According to the tagTime configuration parameter:

  uint16_t controlRegister = CCAENV1x90Registers::ControlRegister::BERREN  
                           | CCAENV1x90Registers::ControlRegister::COMPENSATION_ENABLE;
  if (termination == string("none")) {
    controlRegister |= CCAENV1x90Registers::ControlRegister::TERM_SW ; // computer controlled, but off.
  }
  else if (termination == string("switch")) {
    // TERM_SW = 0, means switch controlled, and TERM is irrelevant.
  }
  else {			// Validator ensures this is 'on' case:
    controlRegister |= CCAENV1x90Registers::ControlRegister::TERM_SW 
                    |  CCAENV1x90Registers::ControlRegister::TERM; // Software controlled, termination on.
  }


  if (tagTime) {
    controlRegister |= CCAENV1x90Registers::ControlRegister::TRIGGER_TAG_ENABLE;
  }

  registerWrites.addWrite16(base + CCAENV1x90Registers::WControlRegister, 
                initamod, controlRegister);

  // Interrupt level, interrupt vector, virtual slot, high water mark, and the
  // ecl output control are from the configuration:

  registerWrites.addWrite16(base + CCAENV1x90Registers::WInterruptLevel , 
			    initamod, (uint16_t)ipl);
  registerWrites.addWrite16(base + CCAENV1x90Registers::WInterruptVector,
			    initamod, (uint16_t)ivector);
  registerWrites.addWrite16(base + CCAENV1x90Registers::WVirtualSlot,
			    initamod, (uint16_t)vsn);
  registerWrites.addWrite16(base + CCAENV1x90Registers::WAlmostFullLevel,
			    initamod, (uint16_t)highWater);

  // figure out the correct settings for the OUT_PROG control register:

  uint16_t out_prog;

  if (eclOutput == string("ready")) {
    out_prog = CCAENV1x90Registers::OutputControl::DATA_READY;
  }
  else if (eclOutput == string("full")) {
    out_prog = CCAENV1x90Registers::OutputControl::FULL;
  }
  else if (eclOutput == string("almostfull")) {
    out_prog = CCAENV1x90Registers::OutputControl::ALM_FULL;
  }
  else {			// Validators ensure this is 'error'
    out_prog = CCAENV1x90Registers::OutputControl::ERROR;
  }
  registerWrites.addWrite16(base + CCAENV1x90Registers::WOutputControl, 
                initamod, out_prog);

  // Do that list:

  size_t dummy;
  int status = controller.executeList(registerWrites,
             &dummy,
             sizeof(dummy),
             &dummy);
  if (status < 0) {
    throw std::string("CCAENV1x90 initialization register writ list failed");
  }
  // Before setting anything else up, we should put the device in
  // trigger matching mode:

  uint16_t data[128];		// I think that's the biggest data buffer we'll need.
  WriteMicro(controller,
         base,
         CCAENV1x90Opcodes::TRG_MATCH,
         data, 0);

  // Set the trigger window width,  offset, extra search margin, and reject margin:

  WriteMicro(controller,
	     base,
	      CCAENV1x90Opcodes::SET_WIN_WIDTH,
	     &windowWidth,
	     1);
  WriteMicro(controller,
	     base,
	     CCAENV1x90Opcodes::SET_WIN_OFFS,
	     &windowOffset,
	     1);
  WriteMicro(controller,
	     base,
	     CCAENV1x90Opcodes::SET_SW_MARGIN,
	     &extraSearch,
	     1);
  WriteMicro(controller,
	     base,
	     CCAENV1x90Opcodes::SET_REJ_MARGIN,
	     &rejectMargin,
	     1);

  // The subtractTrigger bool controls whether or not we enable trigger time subtraction:

  unsigned short  triggerTime;
  if (subtractTrigger) {
    triggerTime =  CCAENV1x90Opcodes::EN_SUB_TRG;
  }
  else {
    triggerTime =  CCAENV1x90Opcodes::DIS_SUB_TRG;
  }
  WriteMicro(controller,
	     base,
	     triggerTime,
	     data,
	     0);

  // Set edge detection:

  data[0] = m_edgeMap[edgeDetect];
  WriteMicro(controller,
	     base,
	     CCAENV1x90Opcodes::SET_DETECTION,
	     data,
	     1);

  // Set edge resolution:

  data[0] = m_edgeResolutionMap[edgeResolution];
  WriteMicro(controller,
	     base,
	     CCAENV1x90Opcodes::SET_TR_LEAD_LSB,
	     data,
	     1);

  // The next one is for the pair mode resolution and sets the
  // leading time and width resolution:

  data [0] = m_leResolutionMap[leResolution] | m_widthResolutionMap[widthResolution];
  WriteMicro(controller,
	     base,
	     CCAENV1x90Opcodes::SET_PAIR_RES,
	     data,
	     1);

  // deadtime:

  data[0] = m_deadtimeMap[deadtime];
  WriteMicro(controller,
	     base,
	     CCAENV1x90Opcodes::SET_DEAD_TIME,
	     data,
	     1);

  //  Depending on the state of the encapsulateChip  configuration parameter,
  // the chip data will or won't be wrapped in headers/trailers:

  unsigned short  wrap;
  if (encapsulateChip) {
    wrap =  CCAENV1x90Opcodes::EN_HEADER_TRAILER;
  }
  else {
    wrap =  CCAENV1x90Opcodes::DIS_HEADER_TRAILER;
  }
  WriteMicro(controller,
	     base,
	     wrap,
	     data,
	     0);

  // Set the maximum number of hits a chip can deliver.

  data[0] = m_maxhitsMap[maxHits];
  WriteMicro(controller,
	     base,
	      CCAENV1x90Opcodes::SET_EVENT_SIZE,
	     data,
	     1);

  // Enable or disable error marks in the event stream:

  WriteMicro(controller,
	     base,
	     markErrors ?   CCAENV1x90Opcodes::EN_ERROR_MARK :  
	                    CCAENV1x90Opcodes::DIS_ERROR_MARK,
	     data, 0);

  // Enable/disable chip bypass on error:

  WriteMicro(controller,
	     base,
	     bypassOnError ?  CCAENV1x90Opcodes::EN_ERROR_BYPASS :  
	                      CCAENV1x90Opcodes::DIS_ERROR_BYPASS,
	     data, 0);

  // Enable all channels of the system:

  WriteMicro(controller,
	     base, 
	     CCAENV1x90Opcodes::EN_ALL_CH, 
	     data, 0);

  // Set the global offset and vernier.

  data[0] = offset[0];
  data[1] = offset[1];
  WriteMicro(controller,
	     base,
	     CCAENV1x90Opcodes::SET_GLOB_OFFS,
	     data,2);

  // Set the channel adjust values.  
  // These are in a vector of channel number adjust pairs.

  for (int i=0; i < adjusts.size(); i++) {
    int      channel = adjusts[i].first;
    uint16_t value   = adjusts[i].second;

    WriteMicro(controller,
	       base,
	       CCAENV1x90Opcodes::SET_ADJUST_CH | channel,
	       &value, 1);
  }
  controller.vmeWrite16(base + CCAENV1x90Registers::WClear, initamod, (uint16_t)0); // Clear pending events.

}
/*!
   Adds the readout for this module to the list.  We have to assume some large transfer size
   and let the BERR stop us.    The output buffer is only a window
   that is 4Kbytes.  We'll assume that we can't read more data than that (1Klongs).


   \param list - The readout list we are populating.
*/
void
CV1x90::addReadoutList(CVMUSBReadoutList& list)
{
  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");

  list.addBlockRead32(base, readamod, (size_t)1024);
  list.addWrite16(base + CCAENV1x90Registers::WClear, initamod, (uint16_t)0);
}
/*!
   Clone oursevles.. well this is really just a virtual copy constructor:

*/
CReadoutHardware*
CV1x90::clone() const
{
  return new CV1x90(*this);
}
///////////////////////////////////////////////////////////////////////////////////
// Utilities and factorizations.
//

/*
  Defines the configuration parameters and sets their constraints.
*/
void
CV1x90::createConfiguration()
{
  m_pConfiguration->addParameter("-base", XXUSB::CConfigurableObject::isInteger, NULL, "0");
  m_pConfiguration->addParameter("-vsn",  XXUSB::CConfigurableObject::isInteger, &vsnLimits, "0");
  m_pConfiguration->addParameter("-ipl",  XXUSB::CConfigurableObject::isInteger, &iplLimits, "0");
  m_pConfiguration->addParameter("-vector", XXUSB::CConfigurableObject::isInteger, &vectorLimits, "0");
  m_pConfiguration->addParameter("-termination",
				 XXUSB::CConfigurableObject::isEnum, &terminationValues, "on");
  m_pConfiguration->addParameter("-tagtime", XXUSB::CConfigurableObject::isBool, NULL, "on");
  m_pConfiguration->addParameter("-highwatermark",
				 XXUSB::CConfigurableObject::isInteger, &highWaterLimits, "1");
  m_pConfiguration->addParameter("-ecloutput", XXUSB::CConfigurableObject::isEnum, &eclOutputValues, "ready");
  m_pConfiguration->addParameter("-window", XXUSB::CConfigurableObject::isInteger, &windowLimits, "40");
  m_pConfiguration->addParameter("-offset", XXUSB::CConfigurableObject::isInteger, &offsetLimits, "-40");
  m_pConfiguration->addParameter("-extramargin", XXUSB::CConfigurableObject::isInteger, &marginLimits, "8");
  m_pConfiguration->addParameter("-rejectmargin", XXUSB::CConfigurableObject::isInteger, &marginLimits, "4");
  m_pConfiguration->addParameter("-triggerrelative", XXUSB::CConfigurableObject::isBool, NULL, "true");
  m_pConfiguration->addParameter("-edgedetect", XXUSB::CConfigurableObject::isEnum, &edgeDetectValues, 
				 "leading");
  m_pConfiguration->addParameter("-edgeresolution",
				 XXUSB::CConfigurableObject::isEnum, &edgeResolutionValues, "100ps");
  m_pConfiguration->addParameter("-leresolution",
				 XXUSB::CConfigurableObject::isEnum, &leResolutionValues, "100ps");
  m_pConfiguration->addParameter("-widthresolution", 
				 XXUSB::CConfigurableObject::isEnum, &pairResolutionValues, "100ps");
  m_pConfiguration->addParameter("-deadtime", XXUSB::CConfigurableObject::isEnum,
				 &deadtimeValues, "5ns");
  m_pConfiguration->addParameter("-encapsulatechip",
				 XXUSB::CConfigurableObject::isBool, NULL, "on");
  m_pConfiguration->addParameter("-maxhits",
				 XXUSB::CConfigurableObject::isEnum, &maxHitValues, "infinite");
  m_pConfiguration->addParameter("-errormark",
				 XXUSB::CConfigurableObject::isBool, NULL, "on");
  m_pConfiguration->addParameter("-errorbypass",
				 XXUSB::CConfigurableObject::isBool, NULL, "on");
  m_pConfiguration->addParameter("-globaloffset", 
				 XXUSB::CConfigurableObject::isIntList, &globalListSize, "0 0");
  m_pConfiguration->addParameter("-channeladjusts", 
				 validateChannelOffsets, NULL, "");
    

  m_pConfiguration->addParameter("-refchannel", 
				 XXUSB::CConfigurableObject::isInteger, NULL, "0");
  m_pConfiguration->addParameter("-depth", 
				 XXUSB::CConfigurableObject::isInteger, NULL, "16");
  m_pConfiguration->addParameter("-channelcount",
				 XXUSB::CConfigurableObject::isInteger, NULL,
				 "");

}
/*
**  Interact with a module to get the modle number. We are going to fill in the
**  m_Model with one of 1290, or 1190, and the suffix with 'A', 'B' or 'N'.
**
** Paramters:
**    controller - Reference to the VM-USB controller object we use to talk to the VME crate.
**    base       - Base address of the module.
*/
void
CV1x90::moduleType(CVMUSB& controller, uint32_t base)
{
  uint16_t modelLow, modelMiddle, modelHigh;
  uint32_t model;
  uint16_t version;
  
  // Get the model number:

  

  controller.vmeRead16(base + CCAENV1x90Registers::ConfigRom
		            + CCAENV1x90Registers::WModelNumber0, 
		       initamod, &modelLow);
  controller.vmeRead16(base + CCAENV1x90Registers::ConfigRom
		            + CCAENV1x90Registers::WModelNumber1, 
		       initamod, &modelMiddle);
  controller.vmeRead16(base + CCAENV1x90Registers::ConfigRom
		            + CCAENV1x90Registers::WModelNumber2, 
		       initamod, &modelHigh);

  // I've had bad experience on some modles with trash in the upper bits.

  modelLow    &= 0xff;
  modelMiddle &= 0xff;
  modelHigh   &= 0xff;

  model       = modelHigh;
  model       = (model << 8) | modelMiddle;
  model       = (model << 8) | modelLow;


  m_Model     = model;

  // Now figure out the subtype:

  uint16_t subType;
  controller.vmeRead16(base + CCAENV1x90Registers::ConfigRom
                            + CCAENV1x90Registers::WBoardVersion, 
		       initamod, &version);
  version &= 0xff;

  switch (version) {
  case 0:
    m_Suffix = 'A';
    break;
  case 1:
    m_Suffix = 'B';
    break;
  case 2:
    m_Suffix = 'N';
    break;
  }
}

/*
** Given the model and suffix, we need to compute the number of channels
** and number of chips.  These are stored in m_nChannelCount, and m_nChipCount
** respectively.
*/
void
CV1x90::computeCounts()
{
  // start with the 1190 variants:
  
  if (m_Model == 1190) {
    if (m_Suffix == 'A') {
      m_nChipCount    = 4;
      m_nChannelCount = 128;
    } 
    else {
      m_nChipCount    = 2;
      m_nChannelCount = 64;
    }
  }
  if (m_Model == 1290) {
    if (m_Suffix = 'A') {
      m_nChannelCount = 32;
      m_nChipCount    = 4;	// Hi res mode uses several channels in each chip per box channel
    }
    else {
      m_nChannelCount = 16;
      m_nChipCount   = 2;
    }
  }
}
/*
** Most board settings are done by communicating with a micro processor.
** The micro has a communication register and a status register on board
** (WMicroData and WMicroHandshake respectively).
**
** This function waits for the micro controller status register to indicate
** that the module is ready to be written to.  I would expect the latency of this
** operation to be quite high.
**
** Parameters:
**    controller - VMUSB controller object reference.
**    base       - The module base address.
*/
void
CV1x90::WaitMicro(CVMUSB& controller, uint32_t base)
{
  uint32_t address = base + CCAENV1x90Registers::WMicroHandshake;

  while (1) {
    uint16_t status;
    int ok = controller.vmeRead16(address, initamod, &status);
    if (ok != 0) {
      throw string("VME Read of micro handshake register failed");
    }
    if (status & CCAENV1x90Registers::MicroHandshake::WRITE_OK) return;
    
  }

}
/*
**  Many transactions with the module involve writing an operation code and
**  then a set of words to the micro register.   This function does just that.
** Expect this to be slow, because each word requires a wait for the micro to be ready.
**
** Parameters:
**    controller    - VME controller reference.
**    base          - Base address of the  board.
**    opcode        - Identifies the function to peform.
**    data          - POinter to additional data needed by the opcode.
**    words         - Number of words of additiona data (could be zero).
*/
void
CV1x90::WriteMicro(CVMUSB&  controller,
		   uint32_t  base,
		   uint16_t  opcode,
		   uint16_t* data,
		   size_t    words)
{
  uint32_t address = base + CCAENV1x90Registers::WMicroData;

  // The opcode:

  WaitMicro(controller, base);
  int status = controller.vmeWrite16(address, initamod, opcode);
  if (status != 0) {
    throw string("Write of opcode to micro register failed");
  }

  // Now the data words:

  while (words > 0) {
    WaitMicro(controller, base);
    status = controller.vmeWrite16(address, initamod, *data);
    if (status != 0) {
      throw string("Write of data to micro register failed");
    }
    words--; data++;
  }

}
/*
**  Get the channel adjustments from the configuration and return them in 
**  a more useful form for the software.  The configuration item is a list
**  of two element lists. Each element of the list is a 
**  channel number/value pair.
**
** Returns:
**   std::vector<pair<int, int> > where the first int of each element
**        is a channel number and the second, the adjust.
**
** NOTE: 
**   No real error checking is required as validateChannelOffsets have already done the 
**   heavy listing there.
*/
vector<pair<int, int> >
CV1x90::getChannelAdjustments()
{
  vector<pair<int, int> > result;

  string offsets = m_pConfiguration->cget("-channeladjusts");
  
  // Split this into it's fundamental list of components:

  const char** offsetList;
  int offsetListSize = 0;
  int status = Tcl_SplitList(NULL,
			     offsets.c_str(), &offsetListSize, &offsetList);

  // Now iterate over the list pulling out the pairs:
  
  if (status == TCL_OK) {
  
    for (int i=0; i < offsetListSize; i++) {
      const char** subList;
      int    subListSize;	// Only because we need it:

      Tcl_SplitList(NULL, offsetList[i], &subListSize, &subList);

      int channel, value;
      sscanf(subList[0], "%d", &channel);
      sscanf(subList[1], "%d", &value);

      result.push_back(pair<int, int>(channel, value));

      Tcl_Free((char*)subList);
    }
    
    // Free the list storage.
    
    Tcl_Free((char*)offsetList);

  }
  return result;
  
}
/*
** Validates a proposed value for the -channeladjusts.
** - Ensure the proposed value is a Tcl list.
** - Ensure that each element of the list is a 2 element sublist.
** - Ensure that each element of the sublist is an integer.
**
** Parameters:
**   name   - Name of the parameter being validated (should be "-channeladjusts").
**   value  - Proposed new value, this is what we are checking.
**   arg    - Additional argument (not used here).
** Returns:
**   true   - value is valid.
**   false  - value is not valid.
**
*/
bool
CV1x90::validateChannelOffsets(string name, string value, void* arg)
{
  // value must split into a Tcl list:

  int     listSize;
  const char**  listContents;

  int status = Tcl_SplitList(NULL, value.c_str(), &listSize, &listContents);
  if (status != TCL_OK) {
    return false;		// Failed.. not a list.
  }

  // Each element must be a 2 element sublist that is a pair of integers:
  // For now we don't put any limit on the channel nor on the value.
  // The channel - because we really don't have a good way of figuring out the
  //               model at this stage in the game and hence the channel count
  //               could be 32, 64, or 128.
  // The value  - well, because I'm a bit lazy.
  //

  for (int i =0; i < listSize; i++) {
    int argc = 0;
    const char** argv;

    status = Tcl_SplitList(NULL, listContents[i],  &argc, &argv);


    // Check for bad list or incorrect list size:
    
    if (status != TCL_OK) goto bad;
    if (argc != 2)        goto bad;

    // Ensure both elements of the list are integers.

    char* chanEnd;
    char* valEnd;
    unsigned long value =  strtoul(argv[0], &chanEnd, 0);
    value = strtoul(argv[1], &valEnd, 0);

    bool ok = (*chanEnd == '\0') && (*valEnd == '\0');
    Tcl_Free((char*)argv);  
    if (!ok) goto bad;

  }

  // If we got this far everyting is valid:

  Tcl_Free((char*)listContents);

  return true;

  // Bad but need to free the list elements:
 bad:  
  Tcl_Free((char*)listContents);
  return false;
}


/*!
 * \brief CV1x90::validateModule
 *
 * \param controller    the vmusb controller
 * \param base          base address of the device
 *
 * Reads back the constant value registers in the config prom to ensure that they
 * are what is expected. If the values are different than expected, then this
 * throws an exception.
 *
 * \throws std::string when data returned from read operation does not match our expectations.
 *
 */
void CV1x90::validateModule(CVMUSB &controller, uint32_t base)
{
    // check that the values of the config prom are what we expect based on the manual.
    // the manual does not quite give us all the information... the module does not return
    // exactly what is described. Rather, we need to mask off the upper byte of the
    // 16-bits returned by each read.

    using namespace CCAENV1x90Registers;
    uint32_t configBase = base + ConfigRom;
    uint16_t value;
    int status = controller.vmeRead16(configBase + WConstant2, 0x09, &value);
    if (status < 0) {
        throw std::string("CV1x90::validateModule() failed while reading config prom constant 2");
    } else if ((value&0xff) != 0x83) {
        throw std::string("CV1x90::validateModule() failed to identify device as a V1x90");
    }

    status = controller.vmeRead16(configBase + WConstant1, 0x09, &value);
    if (status < 0) {
        throw std::string("CV1x90::validateModule() failed while reading config prom constant 1");
    } else if ((value&0xff) != 0x84) {
        throw std::string("CV1x90::validateModule() failed to identify device as a V1x90");
    }

    status = controller.vmeRead16(configBase + WConstant0, 0x09, &value);
    if (status < 0) {
        throw std::string("CV1x90::validateModule() failed while reading config prom constant 0");
    } else if ((value&0xff) != 0x01) {
        throw std::string("CV1x90::validateModule() failed to identify device as a V1x90");
    }

}


////////////////////////////////////////////////////////////////////////////
//  Map maker map maker make me a map
//    - Sorry couldn't resist that allusion.
// This set of functions creates maps of enumerated strings to 
// corresponding programming values...allowing Initialize to 
// easily turn configuration enums into programming instructions
// to the micro.
//

/*
** Creates a map of the values of the -edgedetect values
** to valid values to give the micro when the edge detection is programmed.
** these are unfortunately magic numbers at this point. 
*/

CV1x90::EnumMap
CV1x90::edgeMap()
{
  EnumMap   result;
  result["pair"]     = 0;
  result["leading"]  = 2;
  result["trailing"] = 1;
  result["both"]     = 3;

  return result;
}
/*
** Creates a map from the values of -edgresolution to values that can be programmed
** into the micro:
*/
CV1x90::EnumMap
CV1x90::edgeResolutionMap()
{
  EnumMap result;

  result["800ps"]     = 0;
  result["200ps"]     = 1;
  result["100ps"]     = 2;
  result["25ps"]      = 3; 

  return result;
}
/*
** Creates a map from the values of -leresolution to the values that can be programmed
** into the hardware.
*/

CV1x90::EnumMap
CV1x90::leResolutionMap()
{
  EnumMap  result;

  result["100ps"]    = 0;
  result["200ps"]    = 1;
  result["400ps"]    = 2;
  result["800ps"]    = 3;
  result["1.6ns"]    = 4;
  result["3.12ns"]   = 5;
  result["6.25ns"]   = 6;
  result["12.5ns"]   = 7;

  return result;
}
/*
** Creates a map from the values of -widthresolution to the values that can be programmed into
** the hardware.
*/

CV1x90::EnumMap
CV1x90::widthResolutionMap()
{
  EnumMap result;

  result["100ps"]    = 0;
  result["200ps"]    = 1;
  result["400ps"]    = 2;
  result["800ps"]    = 3;
  result["1.6ns"]    = 4;
  result["3.2ns"]    = 5;
  result["6.25ns"]   = 6;
  result["12.5ns"]   = 7;
  result["25ns"]     = 8;
  result["50ns"]     = 9;
  result["100ns"]    = 10;
  result["200ns"]    = 11;
  result["400ns"]    = 12;
  result["800ns"]    = 13;
  

  return result;
}
/*
** Creates a map from the valid values of the -deadtime parameter to the values that
** can be programmed into the hardware:
*/
CV1x90::EnumMap
CV1x90::deadtimeMap()
{
  EnumMap result;

  result["5ns"]    = 0;
  result["10ns"]   = 1;
  result["30ns"]   = 2;
  result["100ns"]  = 3;

  return result;
}
/*
  Creates a map from the value values of -maxhits to the values that can be programmed
  into the system.
*/
CV1x90::EnumMap
CV1x90::maxhitsMap()
{
  EnumMap result;

  result["0"]   = 0;
  result["1"]   = 1;
  result["2"]   = 2;
  result["4"]   = 3;
  result["8"]   = 4;
  result["16"]  = 5;
  result["32"]  = 6;
  result["64"]  = 7;
  result["128"] = 8;
  result["infinite"] = 9;

  return result;
  
}
