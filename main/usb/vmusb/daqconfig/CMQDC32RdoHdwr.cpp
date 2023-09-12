/*
    This software is Copyright by the Board of Trustees of Michigan 
    State University (c) Copyright 2015


    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
     Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <config.h>
#include "CMQDC32RdoHdwr.h"
#include "CReadoutModule.h"
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <set>
#include <memory>
#include <stdexcept>

#include <iostream>
#include "MQDC32Registers.h"


using namespace std;

//////////////////////////////////////////////////////////////////////
// Local constants.


/////////////////////////////////////////////////////////////////////////////////
// Data that drives parameter validity checks.
static const char* GateModeValues[] = {"common", "separate",0};
static const char* TimingSourceValues[] = {"vme", "external",0};
static const char* InputCouplingValues[] = {"AC","DC",0};
static const char* NIMBusyModes[] = {"busy", "rcbus", "full", "overthreshold",0};
static const char* SyncModeValues[] = {"never","begin_run","extern_oneshot",0};
static const char* MultiEventModeValues[] = {"off","on","limited",0};
// Legal values for the resolution...note in this case the default is explicitly defined as 8k

////////////////////////////////////////////////////////////////////////////////
// Constructors and implemented canonical operations:

/* These are largely trivial in nature: */

CMQDC32RdoHdwr::CMQDC32RdoHdwr() :
  CMesytecBase(),
  m_logic(),
  m_pConfig(0) 
{}


/*! Copy construction involves a deep copy */

CMQDC32RdoHdwr::CMQDC32RdoHdwr(const CMQDC32RdoHdwr& rhs) :
  CMesytecBase(rhs),
  m_pConfig(0)
{
  if (rhs.m_pConfig) {
    m_pConfig = new CReadoutModule(*(rhs.m_pConfig));
  }
}

CMQDC32RdoHdwr::~CMQDC32RdoHdwr() {}

CMQDC32RdoHdwr&
CMQDC32RdoHdwr::operator=(const CMQDC32RdoHdwr& rhs)
{
  CMesytecBase::operator=(rhs);
  return *this;
}
/////////////////////////////////////////////////////////////////////////////////
// Object operations:
//

/*!
   Attach the module to its configuration.
   This is called when the object has been created by the configuration software.
   we will register our configuration parameters, validators and limits.
   A pointer to the configuration object is retained so that The module configuration
   can be gotten when we need it.

   \param configuration - The Readout module that will hold our configuration.

*/
void
CMQDC32RdoHdwr::onAttach(CReadoutModule& configuration)
{

  m_pConfig = &configuration;

  // Create the configuration parameters.
  m_pConfig->addIntegerParameter("-base", 0);
  m_pConfig->addIntegerParameter("-id",  0, 0xff, 0);

  m_pConfig->addBooleanParameter("-timestamp", false);

  m_pConfig->addBooleanParameter("-usethresholds", true);
  m_pConfig->addIntListParameter("-thresholds",
                                  0, MQDC32::Thresholds::Max, 
                                  32, 32, 32, 
                                  0);  // def value


  // IRQ related details
  m_pConfig->addIntegerParameter("-ipl", 0, 7, 0);
  m_pConfig->addIntegerParameter("-vector", 0, 0xff, 0);
  m_pConfig->addIntegerParameter("-irqthreshold", 0, 0xffff, 1);
  m_pConfig->addEnumParameter("-multievent", MultiEventModeValues, 
				MultiEventModeValues[0]);
  m_pConfig->addIntegerParameter("-maxtransfers", 1, 0xffff, 1);
  m_pConfig->addBooleanParameter("-countevents", false);
  m_pConfig->addBooleanParameter("-skipberr", false);

  m_pConfig->addIntListParameter("-bankoffsets", 
                                  0, MQDC32::BankOffsets::Max,
                                  2, 2, 2, 128);
  m_pConfig->addEnumParameter("-gatemode", GateModeValues, GateModeValues[0]);

  // the hold delays and widths have the same list constraints.
  // just different default values.
  m_pConfig->addIntListParameter("-gatelimits", 
                                  0, MQDC32::GateLimit::Max, // min val, max val 
                                  2, 2, 2, // min size, max size, default size
                                  MQDC32::GateLimit::Max);  // no limit 
  m_pConfig->addIntListParameter("-exptrigdelays",
                                  0, MQDC32::ExpTrigDelay::Max, // min val, max val 
                                  2, 2, 2, // min size, max size, default size
                                  0);  // def value

  // input coupling
  m_pConfig->addEnumParameter("-inputcoupling0", 
                               InputCouplingValues, 
                               InputCouplingValues[0]);
  m_pConfig->addEnumParameter("-inputcoupling1", 
                               InputCouplingValues, 
                               InputCouplingValues[0]);

  // test pulser
  m_pConfig->addBooleanParameter("-pulser", false);
  m_pConfig->addIntegerParameter("-pulseramp", 0, 0xff, 32);

  // output configuration
  m_pConfig->addBooleanParameter("-ecltermination", true);
  m_pConfig->addBooleanParameter("-ecltiming", false);
  m_pConfig->addBooleanParameter("-nimtiming", false);
  m_pConfig->addEnumParameter("-nimbusy", NIMBusyModes, NIMBusyModes[0]);

  // timing 
  m_pConfig->addEnumParameter("-timingsource", 
                                     TimingSourceValues, 
                                     TimingSourceValues[0]);
  m_pConfig->addIntegerParameter("-timingdivisor", 0, 0xffff, 1);
  m_pConfig->addEnumParameter("-resetlogic",SyncModeValues,SyncModeValues[1]);

  m_pConfig->addIntegerParameter("-multlowerlimit0",
                                   0, 32, 0);
  m_pConfig->addIntegerParameter("-multlowerlimit1",
                                   0, 16, 0);

  m_pConfig->addIntegerParameter("-multupperlimit0",
                                   0, 32, 32);
  m_pConfig->addIntegerParameter("-multupperlimit1",
                                   0, 16, 16);

}

/*!
   Initialize the module prior to data taking.  We will get the initialization
   data from the configuration.  Unfortunately, there's no way to verify the
   base address we were given actually points to a module.

   \param CVMUSB&controller   References a VMSUB controller that will be used
          to initilize the module (the module is in a VME crate connected to that
          VMUSB object.
*/
void
CMQDC32RdoHdwr::Initialize(CVMUSB& controller)
{
  // Locate the module and reset it and the fifo.
  // These operations are the only individual operations and are done
  // in case time is needed between reset and the next operations on the module.
  // The remaining operations on the module will be done in 
  // a list so that they don't take so much time.


  CVMUSB& ctlr = controller;
  m_logic.setBase(getBase());

  // this is a workaround for a bug in the VMUSB,
  // addWriteAcquisitionState is used because it does not add a 
  // delay to the stack after the write.
  unique_ptr<CVMUSBReadoutList> pList(controller.createReadoutList());
  m_logic.addWriteAcquisitionState(*pList,0);
  auto result = ctlr.executeList(*pList, 2);
  if (result.size() == 0) {
    throw std::runtime_error("Failure while disabling MQDC32 acquisition mode.");
  }

  pList.reset(controller.createReadoutList());
  // First disable the interrupts so that we can't get any spurious ones during init.
  m_logic.addDisableInterrupts(*pList);

  configureModuleID(*pList);

  // Configure the IRQ and buffering mode
  configureIrq(*pList);
  configureMultiEventMode(*pList);

  // configure conversion / acquisition parameters
  configureThresholds(*pList);
  configureGates(*pList);
  configureBankOffsets(*pList);
 
  configureMemoryBankSeparation(*pList);
  
  // configure inputs/outputs
  configureECLInputs(*pList);
  configureNIMInputs(*pList);
  configureNIMBusy(*pList);
  configureInputCoupling(*pList);
  configureECLTermination(*pList);
  //  ... timestamp
  configureTimeBaseSource(*pList);
  configureTimeDivisor(*pList);
  configureMarkerType(*pList);
  configureCounterReset(*pList);
  
  // multiplicity thresholds
  configureMultiplicity(*pList);

  configureTestPulser(*pList);
  // see page 29 of MQDC manual for starting the readout.
  // 1. Fifo reset
  // 2. Readout reset
  // 3. start acquisition
  m_logic.addInitializeFifo(*pList);

  result = ctlr.executeList(*pList, 2);
  if (result.size()==0) {
    throw std::runtime_error("Failure while executing list.");
  }

  // allow time for that configuration to set in before accepting gates
  // because some of the config will effect the digitized values.
  sleep(1);

  // begin accepting gates.
  pList->clear();
  m_logic.addWriteAcquisitionState(*pList,true);
  m_logic.addResetReadout(*pList);
  result = ctlr.executeList(*pList, 2);
  if (result.size()==0) {
    throw std::runtime_error("Failure while executing list.");
  }

}

/*! \brief Utility func for retrieving the base address
 *
 */
uint32_t CMQDC32RdoHdwr::getBase() {
  return m_pConfig->getUnsignedParameter("-base");
}

/*! \brief  Add operations to the stack to set module id
 *
 * \param list a readout list
 */
void CMQDC32RdoHdwr::configureModuleID(CVMUSBReadoutList& list) {
  uint16_t id = m_pConfig->getIntegerParameter("-id");
  m_logic.addWriteModuleID(list,id);
}

/*! \brief Add operations to stack to set up the thresholds
 *
 * \param list a readout list
 */
void CMQDC32RdoHdwr::configureThresholds(CVMUSBReadoutList& list) {
  if (m_pConfig->getBoolParameter("-usethresholds")) {

    // make sure that we honor thresholds that we are given
    m_logic.addWriteIgnoreThresholds(list,false);

    // pass the user's thresholds to the stack
    auto thresholds  = m_pConfig->getIntegerList("-thresholds");
    m_logic.addWriteThresholds(list,thresholds);
  } else {
    // set all thresholds to 0 (disables them)
    m_logic.addWriteThresholds(list,vector<long int>(32,0));
    // ignore thresholds. period.
    m_logic.addWriteIgnoreThresholds(list,true);
  }
}


/*! \brief Add operations to stack to use event count or timestamp
 *
 *  If the timestamp is to be used, the extended tstamp is always used. otherwise
 *  the device is configured to use the event count.
 *
 * \param list a readout list
 */
void CMQDC32RdoHdwr::configureMarkerType(CVMUSBReadoutList& list) {
  using namespace MQDC32::MarkerType;
  if (m_pConfig->getBoolParameter("-timestamp")) {
    m_logic.addWriteMarkerType(list, Timestamp46Bit);
  } else {
    m_logic.addWriteMarkerType(list, EventCount);
  }
}

/*! \brief Set up the gate mode to use single or separate 
 *
 * \todo, give proper documentation for this.
 *
 * \param list  a readout list
 */
void CMQDC32RdoHdwr::configureMemoryBankSeparation(CVMUSBReadoutList& list) {
  int modeIndex = m_pConfig->getEnumParameter("-gatemode",GateModeValues);
  switch(modeIndex) {
    case 0:
      m_logic.addWriteMemoryBankSeparation(list,0);
      break;
    case 1:
      m_logic.addWriteMemoryBankSeparation(list,1);
      break;
  }									
}

/*! \brief Set up the internal gate parameters 
 *
 * The exptrigdelays are the delay between when the actual exp gate arrives
 * and when the integration gate is applied to the signal. the gate limit is 
 * the length of the integration time used. It adjusts the actual gate width
 * if it is shorter.
 *
 * \param list  a readout list
 */
void CMQDC32RdoHdwr::configureGates(CVMUSBReadoutList& list) {

  auto exptrigdelays = m_pConfig->getIntegerList("-exptrigdelays");
  auto gatelimits = m_pConfig->getIntegerList("-gatelimits");

  m_logic.addWriteExpTrigDelays(list, exptrigdelays);
  m_logic.addWriteGateLimits(list, gatelimits);
}

/*! \brief Set up the internal gate parameters 
 *
 * The exptrigdelays are the delay between when the actual exp gate arrives
 * and when the integration gate is applied to the signal. the gate limit is 
 * the length of the integration time used. It adjusts the actual gate width
 * if it is shorter.
 *
 * \param list  a readout list
 */
void CMQDC32RdoHdwr::configureBankOffsets(CVMUSBReadoutList& list) {
  auto offsets = m_pConfig->getIntegerList("-bankoffsets"); 
  m_logic.addWriteBankOffsets(list, offsets);
}

/*! Set up the test pulser
 *
 * The pulser is really only effective in user amplitude mode.In that case,
 * we need to set the user's amplitude as well. We do so here.
 *  
 * \param list  a readout list
 *
 */
void CMQDC32RdoHdwr::configureTestPulser(CVMUSBReadoutList& list) {
  using namespace MQDC32::Pulser;

  int  amplitude = m_pConfig->getUnsignedParameter("-pulseramp");
  if (m_pConfig->getBoolParameter("-pulser")) {
    // user defined amplitude is the only supported
    m_logic.addWritePulserState(list,UserAmplitude);
    m_logic.addWritePulserAmplitude(list, amplitude);
  } else {
    // off
    m_logic.addWritePulserState(list, Off);
  }

}

/*! \brief Configure the input coupling for the signals
 *
 * The two banks of inputs are configured using a single register so this
 * handles the configuration of both. 
 *
 * \param list  a readout list
 *
 */
void CMQDC32RdoHdwr::configureInputCoupling(CVMUSBReadoutList& list) {
  uint16_t coupling0 = m_pConfig->getEnumParameter("-inputcoupling0",InputCouplingValues); 
  uint16_t coupling1 = m_pConfig->getEnumParameter("-inputcoupling1",InputCouplingValues); 

  uint16_t coupling = (coupling0|(coupling1<<1));
  m_logic.addWriteInputCoupling(list, coupling);
}

/*! \brief Add commands to adjust the time divisor
 *
 * This effectively sets a prescaler for the timestamp.
 *
 * \param list  a readout list
 *
 */
void CMQDC32RdoHdwr::configureTimeDivisor(CVMUSBReadoutList& list) {
  uint16_t timedivisor = m_pConfig->getUnsignedParameter("-timingdivisor");

  m_logic.addWriteTimeDivisor(list, timedivisor);
  // the reset of the timestamp is left to the user...
//  m_logic.addResetTimestamps(list); 
}

/*! \brief Add commands to enable or disable termination for the ECL inputs
 *
 * If termination is enabled, everything is terminated. Gate0, Gate1, and 
 * Reset are terminated for the common inputs. The individual gate inputs 
 * are also terminated for both banks. Otherwise, nothing is terminated.
 *
 * \param list  a readout list
 */
void CMQDC32RdoHdwr::configureECLTermination(CVMUSBReadoutList& list) {
  if (m_pConfig->getBoolParameter("-ecltermination")) {
    // terminate all!
    m_logic.addWriteECLTermination(list, 0x1f);
  }
  else {
    // terminate nothing
    m_logic.addWriteECLTermination(list, 0);
  }
}

/*! \brief Set up the ECL inputs to be consistent with the mode of the module
 *
 *  If the user is sending an external oscillator for a timestamp, they should
 *  set the ECLinputs to deal with timing. One of them is the oscillator inputs
 *  and the other clears the timestamp. Othewise, they just behave as normal. One
 *  is the Gate1 input and the other is the Fast clear.
 *
 *  \param list   a readout list
 */
void CMQDC32RdoHdwr::configureECLInputs(CVMUSBReadoutList& list) {
  
  using namespace MQDC32;

  if (m_pConfig->getBoolParameter("-ecltiming")) {
    m_logic.addWriteECLGate1Input(list, ECLGate1::Oscillator);
    m_logic.addWriteECLFCInput(list,  ECLFC::ResetTstamp);
  } else {
    m_logic.addWriteECLGate1Input(list, ECLGate1::Gate);
    m_logic.addWriteECLFCInput(list,  ECLFC::FastClear);
  }
}

/*! Set up the ECL inputs to be consistent with the mode of the module
 *
 *  If the user is sending an external oscillator for a timestamp, they should
 *  set the ECLinputs to deal with timing. One of them is the oscillator inputs
 *  and the other clears the timestamp. Othewise, they just behave as normal. One
 *  is the Gate1 input and the other is the Fast clear.
 *
 *  \param list   a readout list
 */
void CMQDC32RdoHdwr::configureNIMInputs(CVMUSBReadoutList& list) {
  
  using namespace MQDC32;
  if (m_pConfig->getBoolParameter("-nimtiming")) {
    m_logic.addWriteNIMGate1Input(list, NIMGate1::Oscillator); 
    m_logic.addWriteNIMFCInput(list, NIMFC::ResetTstamp);
  }
  else {
    m_logic.addWriteNIMGate1Input(list, NIMGate1::Gate);
    m_logic.addWriteNIMFCInput(list, NIMFC::ResetTstamp);
  }
}

/*! Configures the signal that is outputted from the NIM busy.
 *
 *  \param list a readout list
 */
void CMQDC32RdoHdwr::configureNIMBusy(CVMUSBReadoutList& list) {
  int mode = m_pConfig->getEnumParameter("-nimbusy",NIMBusyModes);

  using namespace MQDC32::NIMBusy;

  switch (mode) {
    case 0:
      m_logic.addWriteNIMBusyOutput(list, Busy);
      break;
    case 1:
      m_logic.addWriteNIMBusyOutput(list, RCBus);
      break;
    case 2:
      m_logic.addWriteNIMBusyOutput(list, Full);
      break;
    case 3:
      m_logic.addWriteNIMBusyOutput(list, OverThreshold);
      break;
    default:
      // by default we just set it to busy
      m_logic.addWriteNIMBusyOutput(list, Busy);
      break;
  }
}


/*! \brief Adds commands to set timestamp source 
 *
 * Sets the time source as either the vme backplane 16MHz clock or an external
 * source.
 *
 * \param list    a readout list
 *
 */
void CMQDC32RdoHdwr::configureTimeBaseSource(CVMUSBReadoutList& list) {
  uint16_t id = m_pConfig->getEnumParameter("-timingsource",TimingSourceValues);
  m_logic.addWriteTimeBaseSource(list,id);
}

/*! \brief Adds commands for configuring the interrupt configuration
 *
 * The irq level is set in addition to the irq vector that the device responds to.
 * furthermore, the irq is withdrawn when the fifo is empty.
 *
 * \param list    a readout list
 */
void CMQDC32RdoHdwr::configureIrq(CVMUSBReadoutList& list) {
  uint8_t     ipl         = m_pConfig->getIntegerParameter("-ipl");
  uint8_t     ivector     = m_pConfig->getIntegerParameter("-vector");
  m_logic.addWriteIrqVector(list, ivector);
  m_logic.addWriteIrqLevel(list, ipl);
  m_logic.addWriteWithdrawIrqOnEmpty(list,true);
}

/*! \brief Configure the system for single or multi event mode
 *
 *  In the end, both of these are really just multi event mode. The
 *  difference is how many transfer words are allowed and what the irq
 *  threshold is.
 *
 * \param list    a readout list
 */
void CMQDC32RdoHdwr::configureMultiEventMode(CVMUSBReadoutList& list) {
  using namespace MQDC32::TransferMode;

  int modeIndex = m_pConfig->getEnumParameter("-multievent", MultiEventModeValues);
  std::string mode = MultiEventModeValues[modeIndex];

  int multiEventValue = 0;

  if (mode == "off") {
    multiEventValue = Single;
  } else if (mode == "on") {
    multiEventValue = Unlimited;
  } else if (mode == "limited") {
    multiEventValue = Limited;
  } else { 
    string msg("CMQDC32RdoHdwr::configureMultiEventMode() invalid mode provided.");
    throw std::runtime_error(msg);
  }
  
  if (m_pConfig->getBoolParameter("-skipberr")) {
    multiEventValue |= EmitEOB;
  }
  if (m_pConfig->getBoolParameter("-countevents")) {
    multiEventValue |= CountEvents;
  }
  m_logic.addWriteMultiEventMode(list, multiEventValue);

  uint16_t nUnits = m_pConfig->getUnsignedParameter("-irqthreshold");
  m_logic.addWriteIrqThreshold(list, nUnits);

  uint16_t nTransfers = m_pConfig->getUnsignedParameter("-maxtransfers");
  m_logic.addWriteTransferCount(list, nTransfers);
  
}

/*! \brief Set up the multiplicity limits
 *
 * The lower and upper limits for the two banks are set up.
 *
 * \param list  a readout list
 */
void CMQDC32RdoHdwr::configureMultiplicity(CVMUSBReadoutList& list) {
  int lower0 = m_pConfig->getIntegerParameter("-multlowerlimit0");
  int lower1 = m_pConfig->getIntegerParameter("-multlowerlimit1");
  int upper0 = m_pConfig->getIntegerParameter("-multupperlimit0");
  int upper1 = m_pConfig->getIntegerParameter("-multupperlimit1");

  m_logic.addWriteLowerMultLimits(list, {lower0,lower1});
  m_logic.addWriteUpperMultLimits(list, {upper0,upper1});

//  m_logic.addWriteLowerMultLimits(list, std::vector<int>()={lower0,lower1});
//  m_logic.addWriteUpperMultLimits(list, std::vector<int>()={upper0,upper1});
}

/*! \brief Set up the scheme for dealing with the counter reset (-resetlogic)
 *
 * \param list  a readout list
 */
void
CMQDC32RdoHdwr::configureCounterReset(CVMUSBReadoutList& list) 
{
  using namespace MQDC32::CounterReset;
  int modeIndex = m_pConfig->getEnumParameter("-resetlogic", SyncModeValues);
  switch (modeIndex) {
    case 0:
     m_logic.addWriteCounterReset(list, Never); 
     break; 
    case 1:
     m_logic.addWriteCounterReset(list, (CTRA|CTRB)); 
     break; 
    case 2:
     m_logic.addWriteCounterReset(list, External); 
     break; 
    default:
     // do nothing.
     break; 
  }
}

/*!
  Add instructions to read out the QDC for a event. Since we're working with 
  either single or multi event mode, we will read at least the minimum transfers
  for a complete readout of an extra cycle plus the headers, berr et.

  \param list  - The VMUSB read9out list that's being built for this stack.
*/
void
CMQDC32RdoHdwr::addReadoutList(CVMUSBReadoutList& list)
{
  uint32_t base = getBase();
  m_logic.setBase(base);
  int index = m_pConfig->getEnumParameter("-multievent", MultiEventModeValues);
  string mode = MultiEventModeValues[index];

  if (mode == "limited") {
    uint32_t maxTransfers = m_pConfig->getUnsignedParameter("-maxtransfers");
    m_logic.addFifoRead(list,maxTransfers+40);
  } else if (mode == "off") {
    m_logic.addFifoRead(list,40);
  } else {
    m_logic.addFifoRead(list, 1024);
  }
  m_logic.addResetReadout(list);
  //  list.addDelay(5);


}

/*! Operations to do after a run has ended. This is intended to turn of 
 * the acceptance of gates.
 *
 * \param ctlr  a VMUSB device
 */
void
CMQDC32RdoHdwr::onEndRun(CVMUSB& ctlr) {
  unique_ptr<CVMUSBReadoutList> pList(ctlr.createReadoutList());

  m_logic.addWriteAcquisitionState(*pList,false);
  m_logic.addResetReadout(*pList);

  ctlr.executeList(*pList,2);
}

// Cloning supports a virtual copy constructor.

CReadoutHardware*
CMQDC32RdoHdwr::clone() const
{
  return new CMQDC32RdoHdwr(*this);
}
//////////////////////////////////////////////////////////////////////////////////
//
// Code here provides support for the madcchain pseudo module that use these 
// devices in CBLT mode.
//

/*!
   Set up the chain/mcast addresses.
   @param controller - Handle to VM_USB controller object.
   @param position   - A value from the position enumerator CMesytecBase::ChainPosition
   @param cbltBase   - Base address for CBLT transfers.
   @param mcastBase  - Base address for multicast transfers.

   Note that both mcast and cblt are enabled for now.
*/
void
CMQDC32RdoHdwr::setChainAddresses(CVMUSB&                ctlr,
                                  CMesytecBase::ChainPosition position,
                                  uint32_t               cbltBase,
                                  uint32_t               mcastBase)
{

  uint32_t base = getBase();

  cerr << "Position: " << position << endl;
  cerr << "base: " << hex << base << dec << endl;

  // Compute the value of the control register..though we will first program
  // the addresses then the control register:
  using namespace MQDC32;

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
  cerr << "Setting chain address with " << hex << controlRegister << dec << endl;

  // program the registers, note that the address registers take only the top 8 bits.

  ctlr.vmeWrite16(base + Reg::CbltAddress, initamod, (uint16_t)(cbltBase >> 24));
  ctlr.vmeWrite16(base + Reg::McstAddress, initamod, (uint16_t)(mcastBase >> 24));
  ctlr.vmeWrite16(base + Reg::CbltMcstControl, initamod, (uint16_t)(controlRegister));

}

/*!
   Set up data taking for CBLT readout with the timestamp parameters we are using and
   the mcast address for the chain
   @param ctlr - CVMUSB ctlr reference.
   @param mcast  - Multicast address used to program the chain.
   @param rdoSize - Words per module.
*/
void
CMQDC32RdoHdwr::initCBLTReadout(CVMUSB& ctlr, uint32_t mcast, int rdoSize)
{
  // We need our timing source
  // IRQThreshold
  // VECTOR
  // IPL
  // Timestamp on/off

  // Assumptions:  Internal oscillator reset if using timestamp
  //               ..else no reset.
  //               most modulep arameters are already set up.


  int irqThreshold   = m_pConfig->getIntegerParameter("-irqthreshold");
  int vector         = m_pConfig->getIntegerParameter("-vector");
  int ipl            = m_pConfig->getIntegerParameter("-ipl");
  bool timestamping  = m_pConfig->getBoolParameter("-timestamp");
  
  using namespace MQDC32;
  // Stop acquistiion
  // ..and clear buffer memory:
  ctlr.vmeWrite16(mcast + Reg::StartAcq, initamod, (uint16_t)0);
  ctlr.vmeWrite16(mcast + Reg::InitFifo, initamod, (uint16_t)0);

  // Set stamping

  if(timestamping) {
    // Oscillator sources are assumed to already be set.
    // Reset the timer:

//    ctlr.vmeWrite16(mcast + Reg::MarkType,       initamod, (uint16_t)1); // Show timestamp, not event count.
    ctlr.vmeWrite16(mcast + Reg::TimestampReset, initamod, (uint16_t)3); // reset all counter.
  }
  else {
 //   ctlr.vmeWrite16(mcast + Reg::MarkType,       initamod, (uint16_t)0); // Use Eventcounter.
    ctlr.vmeWrite16(mcast + Reg::EventCounterReset, initamod, (uint16_t)0); // Reset al event counters.
  }
  // Set multievent mode
  
//  ctlr.vmeWrite16(mcast + Reg::MultiEvent, initamod, (uint16_t)3);      // Multi event mode 3.
  ctlr.vmeWrite16(mcast + Reg::IrqThreshold, initamod, (uint16_t)irqThreshold);
  ctlr.vmeWrite16(mcast + Reg::MaxTransfer, initamod,  (uint16_t)rdoSize);

  // Set the IRQ

  ctlr.vmeWrite16(mcast + Reg::Vector, initamod, (uint16_t)vector);
  ctlr.vmeWrite16(mcast + Reg::Ipl,    initamod, (uint16_t)ipl);
  ctlr.vmeWrite16(mcast + Reg::IrqThreshold, initamod, (uint16_t)irqThreshold);

  // Init the buffer and start data taking.

  ctlr.vmeWrite16(mcast + Reg::InitFifo, initamod, (uint16_t)0);
  ctlr.vmeWrite16(mcast + Reg::ReadoutReset, initamod, (uint16_t)0);
  ctlr.vmeWrite16(mcast + Reg::StartAcq , initamod, (uint16_t)1);
}

