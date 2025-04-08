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

#include "CMDPP32SCP.h"
#include "CReadoutModule.h"
#include <unistd.h>
#include <CVMUSB.h>
#include <bitset>
#include <iomanip>

using std::vector;
using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::setw;

/////////////////////////////////////////////////////////////////////////////////
// Arrays for ENUM parameters

static const char*    MarkTypeStrings[] = {"eventcount", "timestamp", "extended-timestamp"};
static const uint16_t MarkTypeValues[] = {0, 1, 3};

static const char*    TDCResolutionStrings[] = {"24ps", "49ps", "98ps", "195ps", "391ps", "781ps"};
static const uint16_t TDCResolutionValues[]  = {0, 1, 2, 3, 4, 5};

static const char*    IrqSourceStrings[] = {"event", "data"};
static const uint16_t IrqSourceValues[]  = {0, 1};

//////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and other 'canonical' methods

/**
 * Constructor
 */
CMDPP32SCP::CMDPP32SCP()
{
  m_pConfiguration = 0;
}

/**
 * Copy construction.  This cannot be virtual by the rules of C++ the clone()
 * method normally creates a new object from an existing template object.
 * 
 * @param rhs  - MDPP32SCP is being copied to create the new device.
 */
CMDPP32SCP::CMDPP32SCP(const CMDPP32SCP& rhs)
{
  m_pConfiguration = 0;
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}
/**
 * Destruction.  If your object creatd any dynamic data it must be freed here:
 */
CMDPP32SCP::~CMDPP32SCP()
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
CMDPP32SCP::onAttach(CReadoutModule& configuration)
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

  m_pConfiguration -> addIntegerParameter("-multievent",    0, 15, 0xb);
  m_pConfiguration -> addEnumParameter("-marktype", MarkTypeStrings, MarkTypeStrings[1]);

  m_pConfiguration -> addEnumParameter("-tdcresolution", TDCResolutionStrings, TDCResolutionStrings[0]);
  m_pConfiguration -> addIntegerParameter("-outputformat",  0,  24, 0);

  m_pConfiguration -> addIntegerParameter("-windowstart", 0, 0x7fff, 0x3fbe);
  m_pConfiguration -> addIntegerParameter("-windowwidth", 0, 0x3fff, 0x20);
  m_pConfiguration -> addBooleanParameter("-firsthit", true);
  m_pConfiguration -> addBooleanParameter("-testpulser", false);
  m_pConfiguration -> addIntegerParameter("-pulseramplitude",  0,  0xfff, 400);
  m_pConfiguration -> addIntegerParameter("-triggersource", 0, 0x400, 0x400);
  m_pConfiguration -> addIntegerParameter("-triggeroutput", 0, 0x400, 0x400);
	m_pConfiguration -> addBooleanParameter("-monitoron",     false);
	m_pConfiguration -> addIntegerParameter("-setmonitorch",  0,    31,     0);
	m_pConfiguration -> addIntegerParameter("-setwave",       0,     3,     0);
 
  m_pConfiguration -> addIntListParameter("-tfintdiff",        1, 0x007f,  8,  8,  8,     20);
  m_pConfiguration -> addIntListParameter("-pz",              64, 0xffff, 32, 32, 32, 0xffff);
  m_pConfiguration -> addIntListParameter("-gain",           100,  25000,  8,  8,  8,    200);
  m_pConfiguration -> addIntListParameter("-threshold",        0, 0xfa00, 32, 32, 32,   2000);
  m_pConfiguration -> addIntListParameter("-shapingtime",      8,   2000,  8,  8,  8,    200);
  m_pConfiguration -> addIntListParameter("-blr",              0, 0x0003,  8,  8,  8,      2);
  m_pConfiguration -> addIntListParameter("-signalrisetime",   0, 0x007f,  8,  8,  8,      0);
  m_pConfiguration -> addIntListParameter("-resettime",       16, 0x03ff,  8,  8,  8,     16);
  m_pConfiguration -> addBooleanParameter("-printregisters",  false);
  m_pConfiguration -> addIntListParameter("-trigtoirq",        0, 0xffff, 14, 14, 14,      0);
}
/**
 * This method is called when a driver instance is being asked to initialize the hardware
 * associated with it. Usually this involves querying the configuration of the device
 * and using VMUSB controller functions and possibily building and executing
 * CVMUSBReadoutList objects to initialize the device to the configuration requested.
 * 
 * @param controller - Refers to a VMUSB controller object connected to the VME crate
 *                     being managed by this framework.
 *
 */
void
CMDPP32SCP::Initialize(CVMUSB& controller)
{
  uint32_t base = m_pConfiguration -> getUnsignedParameter("-base");

  // Retreiving trigger information before the module reset
  uint16_t triggersource = m_pConfiguration -> getIntegerParameter("-triggersource");
  if (triggersource == 0x400) {
    controller.vmeRead16(base + TriggerSource, initamod, &triggersource);
  }

  uint16_t triggeroutput = m_pConfiguration -> getIntegerParameter("-triggeroutput");
  if (triggeroutput == 0x400) {
    controller.vmeRead16(base + TriggerOutput, initamod, &triggeroutput);
  }

  controller.vmeWrite16(base + Reset,        initamod, 1);
  sleep(1);
  controller.vmeWrite16(base + StartAcq,     initamod, 0);
  controller.vmeWrite16(base + InitFifo,     initamod, 1);
  controller.vmeWrite16(base + ReadoutReset, initamod, 1);

  CVMUSBReadoutList list;	// Initialization instructions will be added to this.

  // First disable the interrupts so that we can't get any spurious ones during init.
  list.addWrite16(base + Ipl, initamod, 0);
  list.addDelay(MDPPDELAY);

  // Now retrieve the configuration parameters:
  uint16_t       id                  = m_pConfiguration -> getIntegerParameter("-id");
  uint16_t       ipl                 = m_pConfiguration -> getIntegerParameter("-ipl");
  uint16_t       ivector             = m_pConfiguration -> getIntegerParameter("-vector");

  uint16_t       irqdatathreshold    = m_pConfiguration -> getIntegerParameter("-irqdatathreshold");
  uint16_t       maxtransfer         = m_pConfiguration -> getIntegerParameter("-maxtransfer");
  uint16_t       irqsource           = IrqSourceValues[m_pConfiguration -> getEnumParameter("-irqsource", IrqSourceStrings)];
  uint16_t       irqeventthreshold   = m_pConfiguration -> getIntegerParameter("-irqeventthreshold");

  uint16_t       multievent          = m_pConfiguration -> getIntegerParameter("-multievent");
  uint16_t       marktype            = MarkTypeValues[m_pConfiguration -> getEnumParameter("-marktype", MarkTypeStrings)];

  uint16_t       tdcresolution       = TDCResolutionValues[m_pConfiguration -> getEnumParameter("-tdcresolution", TDCResolutionStrings)];
  uint16_t       outputformat        = m_pConfiguration -> getIntegerParameter("-outputformat");

  uint16_t       windowstart         = m_pConfiguration -> getIntegerParameter("-windowstart");
  uint16_t       windowwidth         = m_pConfiguration -> getIntegerParameter("-windowwidth");
  bool           firsthit            = m_pConfiguration -> getBoolParameter("-firsthit");
  bool           testpulser          = m_pConfiguration -> getBoolParameter("-testpulser");
  uint16_t       pulseramplitude     = m_pConfiguration -> getIntegerParameter("-pulseramplitude");
	bool           monitoron           = m_pConfiguration -> getBoolParameter("-monitoron");
	uint16_t       monitorchannel      = m_pConfiguration -> getIntegerParameter("-setmonitorch");
	uint16_t       monitorwave         = m_pConfiguration -> getIntegerParameter("-setwave");

  auto           tfintdiff           = m_pConfiguration -> getIntegerList("-tfintdiff");
  auto           pz                  = m_pConfiguration -> getIntegerList("-pz");
  auto           gain                = m_pConfiguration -> getIntegerList("-gain");
  auto           threshold           = m_pConfiguration -> getIntegerList("-threshold");
  auto           shapingtime         = m_pConfiguration -> getIntegerList("-shapingtime");
  auto           blr                 = m_pConfiguration -> getIntegerList("-blr");
  auto           signalrisetime      = m_pConfiguration -> getIntegerList("-signalrisetime");
  auto           resettime           = m_pConfiguration -> getIntegerList("-resettime");
  bool           isPrintRegisters    = m_pConfiguration -> getBoolParameter("-printregisters");
  auto           trigtoirq           = m_pConfiguration -> getIntegerList("-trigtoirq");

  list.addWrite16(base + ModuleId,          initamod, id); // Module id.

  list.addWrite16(base + MultiEvent,        initamod, multievent);
  list.addWrite16(base + MarkType,          initamod, marktype);

  list.addWrite16(base + TDCResolution,     initamod, tdcresolution);
  if (!(outputformat == 0 || outputformat == 4 || outputformat == 8
        || outputformat == 16 || outputformat == 24)) {
    char msg[100];
    sprintf(msg, "outputformat %d is invalid value for outputformat!", outputformat);
    throw msg;
  }
  list.addWrite16(base + OutputFormat,      initamod, outputformat);

  list.addWrite16(base + WindowStart,       initamod, windowstart);
  list.addWrite16(base + WindowWidth,       initamod, windowwidth);
  list.addWrite16(base + FirstHit,          initamod, firsthit);
  list.addWrite16(base + TestPulser,        initamod, testpulser);
  list.addWrite16(base + PulserAmplitude,   initamod, pulseramplitude);
  list.addWrite16(base + TriggerSource,     initamod, triggersource&0x3ff);
  list.addWrite16(base + TriggerOutput,     initamod, triggeroutput&0x3ff);

  for (uint16_t iIncr = 0; iIncr < 14; iIncr++) {
    list.addWrite16(base + TrigToIRQ1L + 2*iIncr, initamod, (uint16_t)trigtoirq.at(iIncr));
  }

  for (uint16_t channelPair = 0; channelPair < 8; channelPair++) {
    list.addWrite16(base + ChannelSelection,    initamod, channelPair);
    list.addWrite16(base + TFIntDiff,           initamod, (uint16_t)tfintdiff.at(channelPair));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + PZ0,                 initamod, (uint16_t)pz.at(channelPair*4));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + PZ1,                 initamod, (uint16_t)pz.at(channelPair*4 + 1));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + PZ2,                 initamod, (uint16_t)pz.at(channelPair*4 + 2));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + PZ3,                 initamod, (uint16_t)pz.at(channelPair*4 + 3));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + Gain,                initamod, (uint16_t)gain.at(channelPair));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + Threshold0,          initamod, (uint16_t)threshold.at(channelPair*4));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + Threshold1,          initamod, (uint16_t)threshold.at(channelPair*4 + 1));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + Threshold2,          initamod, (uint16_t)threshold.at(channelPair*4 + 2));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + Threshold3,          initamod, (uint16_t)threshold.at(channelPair*4 + 3));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + ShapingTime,         initamod, (uint16_t)shapingtime.at(channelPair));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + BLR,                 initamod, (uint16_t)blr.at(channelPair));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + SignalRiseTime,      initamod, (uint16_t)signalrisetime.at(channelPair));
    list.addDelay(MDPPCHCONFIGDELAY);
    list.addWrite16(base + ResetTime,           initamod, (uint16_t)resettime.at(channelPair));
    list.addDelay(MDPPCHCONFIGDELAY);
  }

  // Finally clear the converter and set the IPL which enables interrupts if
  // the IPL is non-zero, and does no harm if it is zero.
  list.addWrite16(base + Ipl,               initamod, ipl);
  list.addWrite16(base + Vector,            initamod, ivector);
  list.addWrite16(base + IrqDataThreshold,  initamod, irqdatathreshold);
  list.addWrite16(base + MaxTransfer,       initamod, maxtransfer);
  list.addWrite16(base + IrqSource,         initamod, irqsource);
  list.addWrite16(base + IrqEventThreshold, initamod, irqeventthreshold);

  list.addWrite16(base + TimestampReset,    initamod, 1); // reset CTRA for 46 bit timestamp reset

  // Now reset again and start daq:
  list.addWrite16(base + InitFifo,          initamod, 1);

  list.addWrite16(base + StartAcq,          initamod, 1);
  list.addWrite16(base + ReadoutReset,      initamod, 1);

	list.addWrite16(base + MonSwitch,         initamod, monitoron);
	list.addWrite16(base + SetMonChannel,     initamod, monitorchannel);
	list.addWrite16(base + SetWave,           initamod, monitorwave);

  char readBuffer[100];		// really a dummy as these are all write...
  size_t bytesRead;
  int status = controller.executeList(list, readBuffer, sizeof(readBuffer), &bytesRead);
  if (status < 0) {
     throw string("List excecution to initialize an MDPP32SCP failed");
  }

  if (isPrintRegisters) {
    printRegisters(controller);
  }
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
CMDPP32SCP::addReadoutList(CVMUSBReadoutList& list)
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
CMDPP32SCP::onEndRun(CVMUSB& controller)
{
}

/**
 * This method virtualizes copy construction by providing a virtual method that
 * invokes it. Usually you don't have to modify this code.
 *
 * @return CMDPP32SCP*
 * @retval Pointer to a dynamically allocated driver instance created by copy construction
 *         from *this
 */
CReadoutHardware*
CMDPP32SCP::clone() const
{
  return new CMDPP32SCP(*this);
}
/**
 * setChainAddresses
 *
 * Called by the chain to insert this module into a CBLT readout with common
 * CBLT base address and MCST address.
 *
 * This method is not tested with MDPP32SCP.
 *
 *   @param controller - The controller object.
 *   @param position   - indicator of the position of the module in chain (begining, middle, end)
 *   @param cbltBase   - Base address for CBLT transfers.
 *   @param mcstBase   - Base address for multicast writes.
 */
void
CMDPP32SCP::setChainAddresses(CVMUSB& controller, CMesytecBase::ChainPosition position,
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
  cerr << "Setting mdpp32-scp chain address with " << std::hex << controlRegister << std::dec << endl;

  // program the registers, note that the address registers take only the top 8 bits.
  controller.vmeWrite16(base + CbltAddress,     initamod, (uint16_t)(cbltBase >> 24));
  controller.vmeWrite16(base + McstAddress,     initamod, (uint16_t)(mcastBase >> 24));
  controller.vmeWrite16(base + CbltMcstControl, initamod, (uint16_t)(controlRegister));    
}

/**
 *  initCBLTReadout
 *
 *  Initialize the readout for CBLT transfer (called by chain).
 *  This method is not tested with MDPP32SCP.
 *
 *    @param controller - the controller object.
 *    @param cbltAddress - the chain block/broadcast address.
 *    @param wordsPerModule - Maximum number of words that can be read by this mod
 */
void
CMDPP32SCP::initCBLTReadout(CVMUSB& controller,
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

/**
 * Printing all register values in MDPP-32 module with SCP firmware
 * read from the module, not the user-input values.
 *
 *  @param controller - a vmusb controller
 */

void
CMDPP32SCP::printRegisters(CVMUSB& controller)
{
  uint32_t base = m_pConfiguration -> getIntegerParameter("-base");

  uint16_t data = 0;
  int status = controller.vmeRead16(base + ModuleId, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "Module ID: " << (data&0xff) << endl;
  }

  status = controller.vmeRead16(base + FirmwareRev, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "Firmware Revision ID: " << "0x" << std::hex << (data&0xffff) << std::dec << endl;
  }

  status = controller.vmeRead16(base + Ipl, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "IRQ level: " << (data&0x7) << endl;
  }

  status = controller.vmeRead16(base + Vector, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "IRQ vector: " << (data&0xff) << endl;
  }

  status = controller.vmeRead16(base + IrqDataThreshold, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "IRQ data threshold: " << (data&0x7f) << " [# of 32 bit words]" << endl;
  }

  status = controller.vmeRead16(base + MaxTransfer, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "Maximum transfer data: " << (data&0x7f) << endl;
  }

  status = controller.vmeRead16(base + IrqSource, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    data = data&0x1;
    cout << setw(30) << "IRQ source: " << data << " ";
    if (data == 0)      cout << "(event threshold exceeded)";
    else if (data == 1) cout << "(data threshold exceeded)";
    cout << endl;
  }

  status = controller.vmeRead16(base + IrqEventThreshold, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "IRQ event threshold: " << (data&0x7f) << " [# of 32 bit words]" << endl;
  }

  status = controller.vmeRead16(base + MultiEvent, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "Multi event(bin): " << (std::bitset<4>(data&0xf)) << endl;
  }

  status = controller.vmeRead16(base + MarkType, initamod, &data); 
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "Marking type(bin): ";
    if (data == 0)      cout << std::bitset<2>(data) << " (event counter)";
    else if (data == 1) cout << std::bitset<2>(data) << " (time stamp)";
    else if (data == 3) cout << std::bitset<2>(data) << " (extended time stamp)";
    else                cout << std::bitset<2>(data) << " (error)";
    cout << endl;
  }

  status = controller.vmeRead16(base + TDCResolution, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    data = data&0x7;
    cout << setw(30) << "TDC resolution: " << data << " (25ns/" << (1 << 10 - data) << "=" << int(25./(1 << 10 - data)*1000) << "ps)" << endl;
  }

  status = controller.vmeRead16(base + OutputFormat, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "Output Format: " << data << " ";
    if (data == 0)       cout << "(standard: time and amplitude)";
    else if (data == 4)  cout << "(compact streaming readout mode)";
    else if (data == 8)  cout << "(standard streaming readout mode)";
    else if (data == 16) cout << "(standard with samples mode)";
    else if (data == 24) cout << "(standard streaming readout with samples mode)";
    else                 cout << "(error)";
    cout << endl;
  }

  status = controller.vmeRead16(base + WindowStart, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    data = data&0x7fff;
    cout << setw(30) << "Window Start: " << data << " (16384 - " << data << ") (*1.56 [ns]) = " << (16384 - data)*1.56 << " [ns]" << endl;
  }

  status = controller.vmeRead16(base + WindowWidth, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    data = data&0x3fff;
    cout << setw(30) << "Window Width: " << data << " (*1.56 [ns]) = " << data*1.56 << " [ns]" << endl;
  }

  status = controller.vmeRead16(base + FirstHit, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "First Hit: " << data << endl;
  }

  status = controller.vmeRead16(base + TestPulser, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "Internal test pulser: " << (data ? "On" : "Off") << endl;
  }

  status = controller.vmeRead16(base + PulserAmplitude, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    data = data&0xfff;
    cout << setw(30) << "Pulser amplitude: " << data << " (0x" << std::hex << data << std::dec << ")" << endl;
  }

  status = controller.vmeRead16(base + TriggerSource, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "Trigger Source: " << data << " (0x" << std::hex << data << std::dec << ")" << endl;
  }

  status = controller.vmeRead16(base + TriggerOutput, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "Trigger Output: " << data << " (0x" << std::hex << data << std::dec << ")" << endl;
  }

  status = controller.vmeRead16(base + MonSwitch, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "Monitor On: " << data << endl;
  }

  status = controller.vmeRead16(base + SetMonChannel, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "Monitor Channel: " << data << endl;
  }

  status = controller.vmeRead16(base + SetWave, initamod, &data);
  if (status < 0) {
    cerr << "Error in reading register" << endl;
  } else {
    cout << setw(30) << "Monitor Wave: " << data << endl;
  }
  
  cout << endl;

  for (uint16_t channelPair = 0; channelPair < 8; channelPair++) {
    controller.vmeWrite16(base + ChannelSelection, initamod, channelPair);

    usleep(21);
    cout << setw(30) << "Channels: " << channelPair*4 << "-" << (channelPair + 1)*4 - 1 << endl;

    status = controller.vmeRead16(base + TFIntDiff, initamod, &data);
    if (status < 0) {
      cerr << "Error in reading register" << endl;
    } else {
      cout << setw(30) << "TF integration/differentiation Time: " << (data&0x7f) << " (*12.5 [ns], " << (data&0x7f)*12.5 << " ns)" << endl;
    }

    status = controller.vmeRead16(base + PZ0, initamod, &data);
    if (status < 0) {
      cerr << "Error in reading register" << endl;
    } else {
      char channelNumber[100] = "";
      sprintf(channelNumber, "Ch %d PZ: ", channelPair*4);
      cout << setw(30) << channelNumber << (data&0xffff);
      if (data == 0xffff) {
          cout << "(Infinity)" << endl;
      } else {
          cout << " (* 12.5 [ns], " << (data&0xffff)*12.5 << " ns)" << endl;
      }
    }

    status = controller.vmeRead16(base + PZ1, initamod, &data);
    if (status < 0) {
        cerr << "Error in reading register" << endl;
    } else {
        char channelNumber[100] = "";
        sprintf(channelNumber, "Ch %d PZ: ", channelPair*4 + 1);
        cout << setw(30) << channelNumber << (data&0xffff);
        if (data == 0xffff) {
            cout << "(Infinity)" << endl;
        } else {
            cout << " (* 12.5 [ns], " << (data&0xffff)*12.5 << " ns)" << endl;
        }
    }

    status = controller.vmeRead16(base + PZ2, initamod, &data);
    if (status < 0) {
        cerr << "Error in reading register" << endl;
    } else {
        char channelNumber[100] = "";
        sprintf(channelNumber, "Ch %d PZ: ", channelPair*4 + 2);
        cout << setw(30) << channelNumber << (data&0xffff);
        if (data == 0xffff) {
            cout << "(Infinity)" << endl;
        } else {
            cout << " (* 12.5 [ns], " << (data&0xffff)*12.5 << " ns)" << endl;
        }
    }

    status = controller.vmeRead16(base + PZ3, initamod, &data);
    if (status < 0) {
        cerr << "Error in reading register" << endl;
    } else {
        char channelNumber[100] = "";
        sprintf(channelNumber, "Ch %d PZ: ", channelPair*4 + 3);
        cout << setw(30) << channelNumber << (data&0xffff);
        if (data == 0xffff) {
            cout << "(Infinity)" << endl;
        } else {
            cout << " (* 12.5 [ns], " << (data&0xffff)*12.5 << " ns)" << endl;
        }
    }

    status = controller.vmeRead16(base + Threshold0, initamod, &data);
    if (status < 0) {
      cerr << "Error in reading register" << endl;
    } else {
      char channelNumber[100] = "";
      sprintf(channelNumber, "Ch %d Threshold: ", channelPair*4);
      data = data&0xffff;
      cout << setw(30) << channelNumber << data << " (0x" << std::hex << data << std::dec << ")" << endl;
    }

    status = controller.vmeRead16(base + Threshold1, initamod, &data);
    if (status < 0) {
        cerr << "Error in reading register" << endl;
    } else {
        char channelNumber[100] = "";
        sprintf(channelNumber, "Ch %d Threshold: ", channelPair*4 + 1);
        data = data&0xffff;
        cout << setw(30) << channelNumber << data << " (0x" << std::hex << data << std::dec << ")" << endl;
    }

    status = controller.vmeRead16(base + Threshold2, initamod, &data);
    if (status < 0) {
        cerr << "Error in reading register" << endl;
    } else {
        char channelNumber[100] = "";
        sprintf(channelNumber, "Ch %d Threshold: ", channelPair*4 + 2);
        data = data&0xffff;
        cout << setw(30) << channelNumber << data << " (0x" << std::hex << data << std::dec << ")" << endl;
    }

    status = controller.vmeRead16(base + Threshold3, initamod, &data);
    if (status < 0) {
        cerr << "Error in reading register" << endl;
    } else {
        char channelNumber[100] = "";
        sprintf(channelNumber, "Ch %d Threshold: ", channelPair*4 + 3);
        data = data&0xffff;
        cout << setw(30) << channelNumber << data << " (0x" << std::hex << data << std::dec << ")" << endl;
    }

    status = controller.vmeRead16(base + ShapingTime, initamod, &data);
    if (status < 0) {
        cerr << "Error in reading register" << endl;
    } else {
        cout << setw(30) << "Shaping time: " << (data&0x7ff) << " (*12.5 [ns], " << (data&0x7ff)*12.5 << " ns)" << endl;
    }

    status = controller.vmeRead16(base + BLR, initamod, &data);
    if (status < 0) {
        cerr << "Error in reading register" << endl;
    } else {
        cout << setw(30) << "Base line restorer: " << data;
        switch (data) {
            case 0: cout << " (Off)" << endl; break;
            case 1: cout << " (Int. time = 4 Shaping time)" << endl; break;
            case 2: cout << " (Int. time = 8 Shaping time)" << endl; break;
            default: cout << " (error)" << endl; break;
        }
    }

    status = controller.vmeRead16(base + SignalRiseTime, initamod, &data);
    if (status < 0) {
        cerr << "Error in reading register" << endl;
    } else {
        cout << setw(30) << "Signal rise time: " << (data&0x7f) << " (*12.5 [ns], " << (data&0x7f)*12.5 << " ns)" << endl;
    }

    status = controller.vmeRead16(base + ResetTime, initamod, &data);
    if (status < 0) {
      cerr << "Error in reading register" << endl;
    } else {
      cout << setw(30) << "Reset time: " << (data&0x3ff) << " (*12.5 [ns])" << endl;
    }

    cout << endl;
  }
}
//////////////////////////////////////////////////////////////////////////////////////////
