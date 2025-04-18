/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2024.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Genie Jhang
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include "CMDPP32QDC.h"
#include "CReadoutModule.h"
#include <XXUSBConfigurableObject.h>
#include <unistd.h>
#include <CVMUSB.h>
#include <bitset>
#include <iomanip>
#include <iostream>

using std::vector;
using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::setw;

/////////////////////////////////////////////////////////////////////////////////
// Arrays for ENUM parameters

static const char*    DataLengthFormatStrings[] = {"8bit", "16bit", "32bit", "64bit", "numevents"};
static const uint16_t DataLengthFormatValues[] = {0, 1, 2, 3, 4};

static const char*    MarkTypeStrings[] = {"eventcount", "timestamp", "extended-timestamp"};
static const uint16_t MarkTypeValues[] = {0, 1, 3};

static const char*    TDCResolutionStrings[] = {"24ps", "49ps", "98ps", "195ps", "391ps", "781ps"};
static const uint16_t TDCResolutionValues[]  = {0, 1, 2, 3, 4, 5};

static const char*    ADCResolutionStrings[] = {"64k", "32k", "16k", "8k", "4k"};
static const uint16_t ADCResolutionValues[]  = {0, 1, 2, 3, 4};

static const char*         GainCorrectionStrings[] = {"div4", "mult4", "none"};
static CMDPP32QDC::EnumMap GainCorrectionMap(CMDPP32QDC::gainCorrectionMap());

static const char*    IrqSourceStrings[] = {"event", "data"};
static const uint16_t IrqSourceValues[]  = {0, 1};

//////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and other 'canonical' methods

/**
 * Constructor
 */
CMDPP32QDC::CMDPP32QDC() 
{
  m_pConfiguration = 0;
}


/**
 * Destruction.  If your object creatd any dynamic data it must be freed here:
 */
CMDPP32QDC::~CMDPP32QDC() 
{
}
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
CMDPP32QDC::onAttach(XXUSB::CConfigurableObject& configuration)
{
  m_pConfiguration = &configuration; 

  m_pConfiguration -> addParameter("-base", XXUSB::CConfigurableObject::isInteger, NULL, "0");
  m_pConfiguration -> addIntegerParameter("-id",                0, 255, 0);
  m_pConfiguration -> addIntegerParameter("-ipl",               0,   7, 0);
  m_pConfiguration -> addIntegerParameter("-vector",            0, 255, 0);

  m_pConfiguration -> addIntegerParameter("-irqdatathreshold",  0, 32256, 1);
  m_pConfiguration -> addIntegerParameter("-maxtransfer",       0, 32256, 1);
  m_pConfiguration -> addEnumParameter("-irqsource", IrqSourceStrings, IrqSourceStrings[0]);
  m_pConfiguration -> addIntegerParameter("-irqeventthreshold", 0, 32256, 3);

  m_pConfiguration -> addEnumParameter("-datalenformat", DataLengthFormatStrings, DataLengthFormatStrings[2]);
  m_pConfiguration -> addIntegerParameter("-multievent",    0, 15, 0xb);
  m_pConfiguration -> addEnumParameter("-marktype", MarkTypeStrings, MarkTypeStrings[1]);

  m_pConfiguration -> addEnumParameter("-tdcresolution", TDCResolutionStrings, TDCResolutionStrings[0]);
  m_pConfiguration -> addIntegerParameter("-outputformat",  0,  3, 3);
  m_pConfiguration -> addEnumParameter("-adcresolution", ADCResolutionStrings, ADCResolutionStrings[0]);

  m_pConfiguration -> addIntegerParameter("-windowstart", 0, 0x7fff, 0x3fbe);
  m_pConfiguration -> addIntegerParameter("-windowwidth", 0, 0x3fff, 0x80);
  m_pConfiguration -> addBooleanParameter("-firsthit", true);
  m_pConfiguration -> addBooleanParameter("-testpulser", false);
  m_pConfiguration -> addIntegerParameter("-pulseramplitude",  0,  0xfff, 400);
  m_pConfiguration -> addIntegerParameter("-triggersource", 0, 0x400, 0x400);
  m_pConfiguration -> addIntegerParameter("-triggeroutput", 0, 0x400, 0x400);
	m_pConfiguration -> addBooleanParameter("-monitoron",     false);
	m_pConfiguration -> addIntegerParameter("-setmonitorch",  0,    31,     0);
	m_pConfiguration -> addIntegerParameter("-setwave",       0,     3,     0);
 
  m_pConfiguration -> addIntListParameter("-signalwidth",    0, 0x03ff,  8,  8,  8,    30);
  m_pConfiguration -> addIntListParameter("-inputamplitude", 0, 0xffff,  8,  8,  8,  1000);
  m_pConfiguration -> addIntListParameter("-jumperrange",    0, 0xffff,  8,  8,  8,  2000);
  m_pConfiguration -> addBoolListParameter("-qdcjumper", 8, false);
  m_pConfiguration -> addIntListParameter("-intlong",        2,    506,  8,  8,  8,    16);
  m_pConfiguration -> addIntListParameter("-intshort",       1,    127,  8,  8,  8,     2);
  m_pConfiguration -> addIntListParameter("-threshold",      1, 0xffff, 32, 32, 32, 0x4ff);
  m_pConfiguration -> addIntListParameter("-resettime",      0, 0x03ff,  8,  8,  8,    32);
  m_pConfiguration -> addStringListParameter("-gaincorrectionlong",  8, GainCorrectionStrings[2]);
  m_pConfiguration -> addStringListParameter("-gaincorrectionshort", 8, GainCorrectionStrings[2]);
  // m_pConfiguration -> addBooleanParameter("-printregisters", false);
  m_pConfiguration -> addIntListParameter("-trigtoirq",      0, 0xffff, 14, 14, 14,     0);
}
/**
 * This method is called when a driver instance is being asked to initialize the hardware
 * associated with it. Usually this involves querying the configuration of the device
 * and using VMUSB controller functions and possibily building and executing
 * CVMUSBReadoutList objects to initialize the device to the configuration requested.
 * 
 * @param controller - Refers to a CCUSB controller object connected to the CAMAC crate
 *                     being managed by this framework.
 *
 */
void
CMDPP32QDC::Initialize(CVMUSB& controller)
{
  uint32_t base = m_pConfiguration -> getUnsignedParameter("-base");

  // Retreiving trigger information before the module reset
  uint16_t triggersource = m_pConfiguration -> getIntegerParameter("-triggersource");
  uint16_t triggeroutput = m_pConfiguration -> getIntegerParameter("-triggeroutput");
  

  controller.vmeWrite16(base + Reset,        initamod, 1);
  sleep(1);
  controller.vmeWrite16(base + StartAcq,     initamod, 0);
  controller.vmeWrite16(base + InitFifo,     initamod, 1);
  controller.vmeWrite16(base + ReadoutReset, initamod, 1);

  CVMUSB& list(controller);	// No list initialization...controller only

  // First disable the interrupts so that we can't get any spurious ones during init.
  list.vmeWrite16(base + Ipl, initamod, 0);
  list.delay(MDPPDELAY);

  // Now retrieve the configuration parameters:
  uint16_t       id                  = m_pConfiguration -> getIntegerParameter("-id");
  uint16_t       ipl                 = m_pConfiguration -> getIntegerParameter("-ipl");
  uint16_t       ivector             = m_pConfiguration -> getIntegerParameter("-vector");

  uint16_t       irqdatathreshold    = m_pConfiguration -> getIntegerParameter("-irqdatathreshold");
  uint16_t       maxtransfer         = m_pConfiguration -> getIntegerParameter("-maxtransfer");
  uint16_t       irqsource           = IrqSourceValues[m_pConfiguration -> getEnumParameter("-irqsource", IrqSourceStrings)];
  uint16_t       irqeventthreshold   = m_pConfiguration -> getIntegerParameter("-irqeventthreshold");

  uint16_t       datalenformat       = DataLengthFormatValues[m_pConfiguration -> getEnumParameter("-datalenformat", DataLengthFormatStrings)];
  uint16_t       multievent          = m_pConfiguration -> getIntegerParameter("-multievent");
  uint16_t       marktype            = MarkTypeValues[m_pConfiguration -> getEnumParameter("-marktype", MarkTypeStrings)];

	uint16_t       tdcresolution       = TDCResolutionValues[m_pConfiguration -> getEnumParameter("-tdcresolution", TDCResolutionStrings)];
  uint16_t       outputformat        = m_pConfiguration -> getIntegerParameter("-outputformat");
	uint16_t       adcresolution       = ADCResolutionValues[m_pConfiguration -> getEnumParameter("-adcresolution", ADCResolutionStrings)];

  uint16_t       windowstart         = m_pConfiguration -> getIntegerParameter("-windowstart");
  uint16_t       windowwidth         = m_pConfiguration -> getIntegerParameter("-windowwidth");
  bool           firsthit            = m_pConfiguration -> getBoolParameter("-firsthit");
  bool           testpulser          = m_pConfiguration -> getBoolParameter("-testpulser");
  uint16_t       pulseramplitude     = m_pConfiguration -> getIntegerParameter("-pulseramplitude");
	bool           monitoron           = m_pConfiguration -> getBoolParameter("-monitoron");
	uint16_t       monitorchannel      = m_pConfiguration -> getIntegerParameter("-setmonitorch");
	uint16_t       monitorwave         = m_pConfiguration -> getIntegerParameter("-setwave");

  auto           signalwidths        = m_pConfiguration -> getIntegerList("-signalwidth");
  auto           inputamplitude      = m_pConfiguration -> getIntegerList("-inputamplitude");
  auto           jumperrange         = m_pConfiguration -> getIntegerList("-jumperrange");
  auto           qdcjumper           = m_pConfiguration -> getIntegerList("-qdcjumper");
  auto           intlong             = m_pConfiguration -> getIntegerList("-intlong");
  auto           intshort            = m_pConfiguration -> getIntegerList("-intshort");
  auto           threshold           = m_pConfiguration -> getIntegerList("-threshold");
  auto           resettime           = m_pConfiguration -> getIntegerList("-resettime");
  vector<string> gaincorrectionlong  = m_pConfiguration -> getList("-gaincorrectionlong");
  vector<string> gaincorrectionshort = m_pConfiguration -> getList("-gaincorrectionshort");
  // bool           isPrintRegisters    = m_pConfiguration -> getBoolParameter("-printregisters");
  auto           trigtoirq           = m_pConfiguration -> getIntegerList("-trigtoirq");

  list.vmeWrite16(base + ModuleId,          initamod, id); // Module id.

  list.vmeWrite16(base + DataFormat,        initamod, datalenformat);
  list.vmeWrite16(base + MultiEvent,        initamod, multievent);
  list.vmeWrite16(base + MarkType,          initamod, marktype);

  list.vmeWrite16(base + TDCResolution,     initamod, tdcresolution);
  list.vmeWrite16(base + OutputFormat,      initamod, outputformat);
  list.vmeWrite16(base + ADCResolution,     initamod, adcresolution);

  list.vmeWrite16(base + WindowStart,       initamod, windowstart);
  list.vmeWrite16(base + WindowWidth,       initamod, windowwidth);
  list.vmeWrite16(base + FirstHit,          initamod, firsthit);
  list.vmeWrite16(base + TestPulser,        initamod, testpulser);
  list.vmeWrite16(base + PulserAmplitude,   initamod, pulseramplitude);
  list.vmeWrite16(base + TriggerSource,     initamod, triggersource&0x3ff);
  list.vmeWrite16(base + TriggerOutput,     initamod, triggeroutput&0x3ff);

  for (uint16_t iIncr = 0; iIncr < 14; iIncr++) {
    list.vmeWrite16(base + TrigToIRQ1L + 2*iIncr, initamod, (uint16_t)trigtoirq.at(iIncr));
  }

  for (uint16_t channelPair = 0; channelPair < 8; channelPair++) {
    list.vmeWrite16(base + ChannelSelection,    initamod, channelPair);
    list.vmeWrite16(base + SignalWidth,         initamod, (uint16_t)signalwidths.at(channelPair));
    list.delay(MDPPCHCONFIGDELAY);
    list.vmeWrite16(base + InputAmplitude,      initamod, (uint16_t)inputamplitude.at(channelPair));
    list.delay(MDPPCHCONFIGDELAY);
    list.vmeWrite16(base + JumperRange,         initamod, (uint16_t)jumperrange.at(channelPair));
    list.delay(MDPPCHCONFIGDELAY);
    list.vmeWrite16(base + QDCJumper,           initamod, (uint16_t)qdcjumper.at(channelPair));
    list.delay(MDPPCHCONFIGDELAY);
    list.vmeWrite16(base + IntegrationLong,     initamod, (uint16_t)intlong.at(channelPair));
    list.delay(MDPPCHCONFIGDELAY);
    list.vmeWrite16(base + IntegrationShort,    initamod, (uint16_t)intshort.at(channelPair));
    list.delay(MDPPCHCONFIGDELAY);
    list.vmeWrite16(base + Threshold0,          initamod, (uint16_t)threshold.at(channelPair*4));
    list.delay(MDPPCHCONFIGDELAY);
    list.vmeWrite16(base + Threshold1,          initamod, (uint16_t)threshold.at(channelPair*4 + 1));
    list.delay(MDPPCHCONFIGDELAY);
    list.vmeWrite16(base + Threshold2,          initamod, (uint16_t)threshold.at(channelPair*4 + 2));
    list.delay(MDPPCHCONFIGDELAY);
    list.vmeWrite16(base + Threshold3,          initamod, (uint16_t)threshold.at(channelPair*4 + 3));
    list.delay(MDPPCHCONFIGDELAY);
    list.vmeWrite16(base + ResetTime,           initamod, (uint16_t)resettime.at(channelPair));
    list.delay(MDPPCHCONFIGDELAY);
    list.vmeWrite16(base + LongGainCorrection,  initamod, (uint16_t)GainCorrectionMap[gaincorrectionlong.at(channelPair)]);
    list.delay(MDPPCHCONFIGDELAY);
    list.vmeWrite16(base + ShortGainCorrection, initamod, (uint16_t)GainCorrectionMap[gaincorrectionshort.at(channelPair)]);
    list.delay(MDPPCHCONFIGDELAY);
  }

  // Finally clear the converter and set the IPL which enables interrupts if
  // the IPL is non-zero, and does no harm if it is zero.
  list.vmeWrite16(base + Ipl,               initamod, ipl);
  list.vmeWrite16(base + Vector,            initamod, ivector);
  list.vmeWrite16(base + IrqDataThreshold,  initamod, irqdatathreshold);
  list.vmeWrite16(base + MaxTransfer,       initamod, maxtransfer);
  list.vmeWrite16(base + IrqSource,         initamod, irqsource);
  list.vmeWrite16(base + IrqEventThreshold, initamod, irqeventthreshold);

  // Now reset again and start daq:
  list.vmeWrite16(base + InitFifo,          initamod, 1);

  list.vmeWrite16(base + StartAcq,          initamod, 1);
  list.vmeWrite16(base + ReadoutReset,      initamod, 1);

	list.vmeWrite16(base + MonSwitch,         initamod, monitoron);
	list.vmeWrite16(base + SetMonChannel,     initamod, monitorchannel);
	list.vmeWrite16(base + SetWave,           initamod, monitorwave);
}

/**
 * This method is called to ask a driver instance to contribute to the readout list (stack)
 * in which the module has been placed.  Normally you'll need to get some of the configuration
 * parameters and use them to add elements to the readout list using VMUSBReadoutList methods.
 *
 * @param list - A CVMUSBReadoutList reference to the list that will be loaded into the
 *               VMUSB.
 */
void
CMDPP32QDC::addReadoutList(CVMUSBReadoutList& list)
{
  uint32_t base  = m_pConfiguration -> getUnsignedParameter("-base"); // Get the value of -slot.

  list.addFifoRead32(base + eventBuffer, readamod, (size_t)65535); 
  list.addWrite16(base + ReadoutReset, initamod, (uint16_t)1);
}


/** \brief On end procedures
 *
 *  This method is called after the VMUSB has been taken out of acquisition mode. It is a hook
 *  for the user to disable their device during times when not acquiring data. The default 
 *  implementation of this in the base class is a no-op.
 *
 *  @param controller - a vmusb controller
 */
void
CMDPP32QDC::onEndRun(CVMUSB& controller)
{
}


/*
  Creates a map from the value of -gaincorrectionlong and -gaincorrectionshort
  to the values that can be programmed into the system.
*/
CMDPP32QDC::EnumMap
CMDPP32QDC::gainCorrectionMap()
{
  EnumMap result;
  
  result["div4"]  = 0x0100;
  result["mult4"] = 0x1000; 
  result["none"]  = 0x0400;

  return result;
}

/**
 * setChainAddresses
 *
 * Called by the chain to insert this module into a CBLT readout with common
 * CBLT base address and MCST address.
 *
 * This method is not tested with MDPP32QDC.
 *
 *   @param controller - The controller object.
 *   @param position   - indicator of the position of the module in chain (begining, middle, end)
 *   @param cbltBase   - Base address for CBLT transfers.
 *   @param mcstBase   - Base address for multicast writes.
 */
void
CMDPP32QDC::setChainAddresses(CVMUSB& controller, CMesytecBase::ChainPosition position,
                              uint32_t cbltBase, uint32_t mcastBase)
{                                                                 
  uint32_t base = m_pConfiguration -> getIntegerParameter("-base");

  cerr << "Position: " << position << endl;
  cerr << std::hex << base << std::dec << endl;
  // Compute the value of the control register..though we will first program
  // the addresses then the control register:

  uint16_t controlRegister = MCSTENB | CBLTENB; // This much is invariant.
  switch (position) {
  case first:
    controlRegister |= FIRSTENB | LASTDIS;
    cerr << "First\n";
    break;
  case middle:
    controlRegister |= FIRSTDIS | LASTDIS;
    cerr << "Middle\n";
    break;
  case last:
    controlRegister |= FIRSTDIS | LASTENB;
    cerr << "Last\n";
    break;
  }
  cerr << "Setting mdpp32-qdc chain address with " << std::hex << controlRegister << std::dec << endl;

  // program the registers, note that the address registers take only the top 8 bits.
  controller.vmeWrite16(base + CbltAddress,     initamod, (uint16_t)(cbltBase >> 24));
  controller.vmeWrite16(base + McstAddress,     initamod, (uint16_t)(mcastBase >> 24));
  controller.vmeWrite16(base + CbltMcstControl, initamod, (uint16_t)(controlRegister));    
}

/**
 *  initCBLTReadout
 *
 *  Initialize the readout for CBLT transfer (called by chain).
 *  This method is not tested with MDPP32QDC.
 *
 *    @param controller - the controller object.
 *    @param cbltAddress - the chain block/broadcast address.
 *    @param wordsPerModule - Maximum number of words that can be read by this mod
 */
void
CMDPP32QDC::initCBLTReadout(CVMUSB& controller,
                            uint32_t cbltAddress,
                            int wordsPermodule)
{
  // We need our timing source
  // IRQThreshold
  // VECTOR
  // IPL
  // Timestamp on/off

  // Assumptions:  Internal oscillator reset if using timestamp
  //               ..else no reset.
  //               most modulep arameters are already set up.


  uint16_t irqDataThreshold  = m_pConfiguration -> getIntegerParameter("-irqdatathreshold");
  uint16_t irqEventThreshold = m_pConfiguration -> getIntegerParameter("-irqeventthreshold");
  uint16_t irqSource         = IrqSourceValues[m_pConfiguration -> getEnumParameter("-irqsource", IrqSourceStrings)];
  uint16_t vector            = m_pConfiguration -> getIntegerParameter("-vector");
  uint16_t ipl               = m_pConfiguration -> getIntegerParameter("-ipl");
  string   markType          = m_pConfiguration -> cget("-marktype");
  bool     timestamping      = (markType == "timestamp") || (markType == "extended-timestamp");
  
  // Stop acquistiion
  // ..and clear buffer memory:
  controller.vmeWrite16(cbltAddress + StartAcq, initamod, 0);
  controller.vmeWrite16(cbltAddress + InitFifo, initamod, 0);

  if(timestamping) {
    controller.vmeWrite16(cbltAddress + TimestampReset,    initamod, 3); // reset all counter.
  }
  else {
    controller.vmeWrite16(cbltAddress + EventCounterReset, initamod, 0); // Reset all event counters.
  }

  // Set the IRQ
  controller.vmeWrite16(cbltAddress + Ipl,    initamod, ipl);
  controller.vmeWrite16(cbltAddress + Vector, initamod, vector);

  controller.vmeWrite16(cbltAddress + MaxTransfer, initamod,  (uint16_t)wordsPermodule);

  if (irqSource == 0) {
    controller.vmeWrite16(cbltAddress + IrqSource,         initamod, irqSource);
    controller.vmeWrite16(cbltAddress + IrqEventThreshold, initamod, irqEventThreshold);
  } else {
    controller.vmeWrite16(cbltAddress + IrqSource,         initamod, irqSource);
    controller.vmeWrite16(cbltAddress + IrqDataThreshold,  initamod, irqDataThreshold);
  }

  // Init the buffer and start data taking.
  controller.vmeWrite16(cbltAddress + InitFifo,     initamod, 0);
  controller.vmeWrite16(cbltAddress + ReadoutReset, initamod, 0);
  controller.vmeWrite16(cbltAddress + StartAcq,     initamod, 1);
}
//////////////////////////////////////////////////////////////////////////////////////////
