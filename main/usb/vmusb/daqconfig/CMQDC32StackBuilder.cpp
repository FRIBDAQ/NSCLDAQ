
#include <CMQDC32StackBuilder.h>
#include <MQDC32Registers.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <unistd.h>
#include <string>
#include <memory>

using namespace std;

namespace MQDC32 {


  void CMQDC32StackBuilder::resetAll(CVMUSB& ctlr) {

    doSoftReset(ctlr);

    unique_ptr<CVMUSBReadoutList> pList(ctlr.createReadoutList());
    addWriteAcquisitionState(*pList,0);
    addResetReadout(*pList);
    ctlr.executeList(*pList, 2);

  }

  void CMQDC32StackBuilder::doSoftReset(CVMUSB& ctlr) {
    unique_ptr<CVMUSBReadoutList> pList(ctlr.createReadoutList());
    addSoftReset(*pList);
    ctlr.executeList(*pList, 2);
    sleep(1);
  }

  void CMQDC32StackBuilder::addSoftReset(CVMUSBReadoutList& list) {
    list.addWrite16(m_base + Reg::Reset, initamod, 1);
  }

  void CMQDC32StackBuilder::addWriteAcquisitionState(CVMUSBReadoutList& list, bool state) 
  {
    list.addWrite16(m_base + Reg::StartAcq, initamod, static_cast<uint16_t>(state));
  }
  void CMQDC32StackBuilder::addReadAcquisitionState(CVMUSBReadoutList& list) 
  {
    list.addRead16(m_base + Reg::StartAcq, initamod);
  }

    // Actual value really should depend on the -multievent mode.
    // this is correct for single event
    
  void CMQDC32StackBuilder::addResetReadout(CVMUSBReadoutList& list) {
    list.addWrite16(m_base + Reg::ReadoutReset, initamod, 0);
  }

  void CMQDC32StackBuilder::addDisableInterrupts(CVMUSBReadoutList& list) 
  {
    list.addWrite16(m_base + Reg::Ipl, initamod, 0);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteIrqLevel(CVMUSBReadoutList& list, uint8_t level) 
  {
    list.addWrite16(m_base + Reg::Ipl, initamod, level);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadIrqLevel(CVMUSBReadoutList& list) 
  {
    list.addRead16(m_base + Reg::Ipl, initamod);
  }

  void CMQDC32StackBuilder::addWriteIrqVector(CVMUSBReadoutList& list, uint8_t level) 
  {
    list.addWrite16(m_base + Reg::Vector, initamod, level);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadIrqVector(CVMUSBReadoutList& list) 
  {
    list.addRead16(m_base + Reg::Vector, initamod);
  }

  void CMQDC32StackBuilder::addWriteIrqThreshold(CVMUSBReadoutList& list, uint16_t thresh) 
  {
    list.addWrite16(m_base + Reg::IrqThreshold, initamod, thresh);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadIrqThreshold(CVMUSBReadoutList& list) 
  {
    list.addRead16(m_base + Reg::IrqThreshold, initamod);
  }


  void CMQDC32StackBuilder::addWriteWithdrawIrqOnEmpty(CVMUSBReadoutList& list, bool on) 
  {
    list.addWrite16(m_base + Reg::WithdrawIrqOnEmpty, initamod, (uint16_t)on);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadWithdrawIrqOnEmpty(CVMUSBReadoutList& list) 
  {
    list.addRead16(m_base + Reg::WithdrawIrqOnEmpty, initamod);
  }

  void CMQDC32StackBuilder::addWriteModuleID(CVMUSBReadoutList& list, uint16_t id)
  {
    list.addWrite16(m_base + Reg::ModuleId, initamod, id); // Module id.
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadModuleID(CVMUSBReadoutList& list)
  {
    list.addRead16(m_base + Reg::ModuleId, initamod); // Module id.
  }


  void CMQDC32StackBuilder::addWriteThreshold(CVMUSBReadoutList& list, unsigned int chan, 
      int thresh)
  {
    uint32_t addr = m_base + Reg::Thresholds + chan*sizeof(uint16_t);
    list.addWrite16(addr, initamod, thresh);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadThreshold(CVMUSBReadoutList& list, unsigned int chan)
  {
    uint32_t addr = m_base + Reg::Thresholds + chan*sizeof(uint16_t);
    list.addRead16(addr, initamod);
  }

  void CMQDC32StackBuilder::addWriteThresholds(CVMUSBReadoutList& list,
      vector<long int> thrs)
  {
    for (size_t chan=0; chan<32; ++chan) {
      addWriteThreshold(list, chan, thrs.at(chan));
    }
  }

  void CMQDC32StackBuilder::addWriteIgnoreThresholds(CVMUSBReadoutList& list, bool off)
  {
    list.addWrite16(m_base+Reg::IgnoreThresholds, initamod, uint16_t(off));
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadIgnoreThresholds(CVMUSBReadoutList& list)
  {
    list.addRead16(m_base+Reg::IgnoreThresholds, initamod);
  }


  void CMQDC32StackBuilder::addWriteMarkerType(CVMUSBReadoutList& list, uint16_t type)
  {
    list.addWrite16(m_base + Reg::MarkType, initamod, type); 
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadMarkerType(CVMUSBReadoutList& list)
  {
    list.addRead16(m_base + Reg::MarkType, initamod); 
  }

  void CMQDC32StackBuilder::addWriteMemoryBankSeparation(CVMUSBReadoutList& list, 
      uint16_t type)
  {
    list.addWrite16(m_base + Reg::BankOperation, initamod, type);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadMemoryBankSeparation(CVMUSBReadoutList& list)
  {
    list.addRead16(m_base + Reg::BankOperation, initamod);
  }

  void CMQDC32StackBuilder::addWriteGateLimit0(CVMUSBReadoutList& list, uint8_t limit)
  {
    list.addWrite16(m_base + Reg::GateLimit0, initamod, limit);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadGateLimit0(CVMUSBReadoutList& list)
  {
    list.addRead16(m_base + Reg::GateLimit0, initamod);
  }


  void CMQDC32StackBuilder::addWriteGateLimit1(CVMUSBReadoutList& list, uint8_t limit)
  {
    list.addWrite16(m_base + Reg::GateLimit1, initamod, limit);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadGateLimit1(CVMUSBReadoutList& list)
  {
    list.addRead16(m_base + Reg::GateLimit1, initamod);
  }

  void CMQDC32StackBuilder::addWriteGateLimits(CVMUSBReadoutList& list, vector<long int> limits)
  {
    addWriteGateLimit0(list,limits.at(0));
    addWriteGateLimit1(list,limits.at(1));
  }

  void CMQDC32StackBuilder::addWriteExpTrigDelay0(CVMUSBReadoutList& list, uint16_t delay)
  {
    list.addWrite16(m_base + Reg::ExpTrigDelay0, initamod, delay);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadExpTrigDelay0(CVMUSBReadoutList& list)
  {
    list.addRead16(m_base + Reg::ExpTrigDelay0, initamod);
  }

  void CMQDC32StackBuilder::addWriteExpTrigDelay1(CVMUSBReadoutList& list, uint16_t delay)
  {
    list.addWrite16(m_base + Reg::ExpTrigDelay1, initamod, delay);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadExpTrigDelay1(CVMUSBReadoutList& list)
  {
    list.addRead16(m_base + Reg::ExpTrigDelay1, initamod);
  }


  void CMQDC32StackBuilder::addWriteExpTrigDelays(CVMUSBReadoutList& list, vector<long int> delays) 
  {
    addWriteExpTrigDelay0(list,delays.at(0));
    addWriteExpTrigDelay1(list,delays.at(1));
  }

  void CMQDC32StackBuilder::addWriteBankOffsets(CVMUSBReadoutList& list, vector<long int> values) 
  {
    list.addWrite16(m_base + Reg::BankOffset0, initamod, values.at(0));
    list.addDelay(MQDCDELAY);
    list.addWrite16(m_base + Reg::BankOffset1, initamod, values.at(1));
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadBankOffset0(CVMUSBReadoutList& list) 
  {
    list.addRead16(m_base + Reg::BankOffset0, initamod);
  }
  void CMQDC32StackBuilder::addReadBankOffset1(CVMUSBReadoutList& list) 
  {
    list.addRead16(m_base + Reg::BankOffset1, initamod);
  }

  void CMQDC32StackBuilder::addWritePulserState(CVMUSBReadoutList& list, uint16_t state)
  {
    list.addWrite16(m_base+Reg::TestPulser, initamod, state);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadPulserState(CVMUSBReadoutList& list)
  {
    list.addRead16(m_base+Reg::TestPulser, initamod);
  }

  void CMQDC32StackBuilder::addWritePulserAmplitude(CVMUSBReadoutList& list, uint8_t val)
  {
    list.addWrite16(m_base+Reg::PulserAmp, initamod, val);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadPulserAmplitude(CVMUSBReadoutList& list)
  {
    list.addRead16(m_base+Reg::PulserAmp, initamod);
  }

  void CMQDC32StackBuilder::addWriteTimeDivisor(CVMUSBReadoutList& list, uint16_t divisor)
  {
    list.addWrite16(m_base + Reg::TimingDivisor, initamod, divisor);
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addReadTimeDivisor(CVMUSBReadoutList& list)
  {
    list.addRead16(m_base + Reg::TimingDivisor, initamod);
  }


  void CMQDC32StackBuilder::addWriteInputCoupling(CVMUSBReadoutList& list, uint16_t type)
  {
    list.addWrite16(m_base+Reg::InputCoupling, initamod, type);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addResetTimestamps(CVMUSBReadoutList& list) 
  {
    list.addWrite16(m_base + Reg::TimestampReset, initamod, uint16_t(3)); // Reset both counters.
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteECLTermination(CVMUSBReadoutList& list, uint16_t type){
    list.addWrite16(m_base + Reg::ECLTermination, initamod, type);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteECLGate1Input(CVMUSBReadoutList& list, uint16_t type)
  {
    list.addWrite16(m_base + Reg::ECLGate1, initamod, type);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteECLFCInput(CVMUSBReadoutList& list, uint16_t type)
  {
    list.addWrite16(m_base + Reg::ECLFC, initamod, type);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteNIMGate1Input(CVMUSBReadoutList& list, uint16_t type){
    list.addWrite16(m_base + Reg::NIMGate1, initamod, type);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteNIMFCInput(CVMUSBReadoutList& list, uint16_t type)
  {
    list.addWrite16(m_base + Reg::NIMFC, initamod, type);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteNIMBusyOutput(CVMUSBReadoutList& list, uint16_t type) {
    list.addWrite16(m_base + Reg::NIMBusy, initamod, type);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteTimeBaseSource(CVMUSBReadoutList& list, uint16_t val){
    list.addWrite16(m_base + Reg::TimingSource, initamod, val);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteMultiEventMode(CVMUSBReadoutList& list, uint16_t val){
    list.addWrite16(m_base + Reg::MultiEvent, initamod, val);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteTransferCount(CVMUSBReadoutList& list, uint16_t val){
    list.addWrite16(m_base + Reg::MaxTransfer, initamod, val);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addInitializeFifo(CVMUSBReadoutList& list)
  {
    list.addWrite16(m_base + Reg::InitFifo, initamod, 1);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteLowerMultLimits(CVMUSBReadoutList& list, vector<int> values) {
    list.addWrite16(m_base + Reg::MultLimitLow0, initamod, values.at(0));
    list.addDelay(MQDCDELAY);
    list.addWrite16(m_base + Reg::MultLimitLow1, initamod, values.at(1));
    list.addDelay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addWriteUpperMultLimits(CVMUSBReadoutList& list, vector<int> values) {
    list.addWrite16(m_base + Reg::MultLimitHigh0, initamod, values.at(0));
    list.addDelay(MQDCDELAY);
    list.addWrite16(m_base + Reg::MultLimitHigh1, initamod, values.at(1));
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteCounterReset(CVMUSBReadoutList& list, uint8_t mode) {
    list.addWrite16(m_base + Reg::EventCounterReset, initamod, mode);
    list.addDelay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addReadCounterReset(CVMUSBReadoutList& list) {
    list.addRead16(m_base + Reg::EventCounterReset, initamod);
  }

  void CMQDC32StackBuilder::addFifoRead(CVMUSBReadoutList& list, size_t transfers) {
    list.addFifoRead32(m_base + Reg::eventBuffer, readamod, transfers);
  }
} // end of namespace
