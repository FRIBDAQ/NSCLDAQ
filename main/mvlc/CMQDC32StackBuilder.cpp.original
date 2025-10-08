
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

    auto pList = &ctlr;                         // Trick to minimize port effort.
    addWriteAcquisitionState(*pList,0);
    addResetReadout(*pList);

  }

  void CMQDC32StackBuilder::doSoftReset(CVMUSB& ctlr) {
    auto pList = &ctlr;
    
    addSoftReset(*pList);
    ctlr.delay(1000000);
  
  }

  void CMQDC32StackBuilder::addSoftReset(CVMUSB& list) {
    list.vmeWrite16(m_base + Reg::Reset, initamod, 1);
  }

  void CMQDC32StackBuilder::addWriteAcquisitionState(CVMUSB& list, bool state) 
  {
    list.vmeWrite16(m_base + Reg::StartAcq, initamod, static_cast<uint16_t>(state));
  }
  

    // Actual value really should depend on the -multievent mode.
    // this is correct for single event
    
  void CMQDC32StackBuilder::addResetReadout(CVMUSB& list) {
    list.vmeWrite16(m_base + Reg::ReadoutReset, initamod, 0);
  }

  void CMQDC32StackBuilder::addDisableInterrupts(CVMUSB& list) 
  {
    list.vmeWrite16(m_base + Reg::Ipl, initamod, 0);
    list.delay(1);
  }

  void CMQDC32StackBuilder::addWriteIrqLevel(CVMUSB& list, uint8_t level) 
  {
    list.vmeWrite16(m_base + Reg::Ipl, initamod, level);
    list.delay(MQDCDELAY);
  }
  

  void CMQDC32StackBuilder::addWriteIrqVector(CVMUSB& list, uint8_t level) 
  {
    list.vmeWrite16(m_base + Reg::Vector, initamod, level);
    list.delay(MQDCDELAY);
  }
  

  void CMQDC32StackBuilder::addWriteIrqThreshold(CVMUSB& list, uint16_t thresh) 
  {
    list.vmeWrite16(m_base + Reg::IrqThreshold, initamod, thresh);
    list.delay(MQDCDELAY);
  }
  


  void CMQDC32StackBuilder::addWriteWithdrawIrqOnEmpty(CVMUSB& list, bool on) 
  {
    list.vmeWrite16(m_base + Reg::WithdrawIrqOnEmpty, initamod, (uint16_t)on);
    list.delay(MQDCDELAY);
  }
  

  void CMQDC32StackBuilder::addWriteModuleID(CVMUSB& list, uint16_t id)
  {
    list.vmeWrite16(m_base + Reg::ModuleId, initamod, id); // Module id.
    list.delay(MQDCDELAY);
  }
  

  void CMQDC32StackBuilder::addWriteThreshold(CVMUSB& list, unsigned int chan, 
      int thresh)
  {
    uint32_t addr = m_base + Reg::Thresholds + chan*sizeof(uint16_t);
    list.vmeWrite16(addr, initamod, thresh);
    list.delay(MQDCDELAY);
  }
 

  void CMQDC32StackBuilder::addWriteThresholds(CVMUSB& list,
      vector<long int> thrs)
  {
    for (size_t chan=0; chan<32; ++chan) {
      addWriteThreshold(list, chan, thrs.at(chan));
    }
  }

  void CMQDC32StackBuilder::addWriteIgnoreThresholds(CVMUSB& list, bool off)
  {
    list.vmeWrite16(m_base+Reg::IgnoreThresholds, initamod, uint16_t(off));
    list.delay(MQDCDELAY);
  }
  


  void CMQDC32StackBuilder::addWriteMarkerType(CVMUSB& list, uint16_t type)
  {
    list.vmeWrite16(m_base + Reg::MarkType, initamod, type); 
    list.delay(MQDCDELAY);
  }
  

  void CMQDC32StackBuilder::addWriteMemoryBankSeparation(CVMUSB& list, 
      uint16_t type)
  {
    list.vmeWrite16(m_base + Reg::BankOperation, initamod, type);
    list.delay(MQDCDELAY);
  }
 

  void CMQDC32StackBuilder::addWriteGateLimit0(CVMUSB& list, uint8_t limit)
  {
    list.vmeWrite16(m_base + Reg::GateLimit0, initamod, limit);
    list.delay(MQDCDELAY);
  }
  


  void CMQDC32StackBuilder::addWriteGateLimit1(CVMUSB& list, uint8_t limit)
  {
    list.vmeWrite16(m_base + Reg::GateLimit1, initamod, limit);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteGateLimits(CVMUSB& list, vector<long int> limits)
  {
    addWriteGateLimit0(list,limits.at(0));
    addWriteGateLimit1(list,limits.at(1));
  }

  void CMQDC32StackBuilder::addWriteExpTrigDelay0(CVMUSB& list, uint16_t delay)
  {
    list.vmeWrite16(m_base + Reg::ExpTrigDelay0, initamod, delay);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteExpTrigDelay1(CVMUSB& list, uint16_t delay)
  {
    list.vmeWrite16(m_base + Reg::ExpTrigDelay1, initamod, delay);
    list.delay(MQDCDELAY);
  }
  
  void CMQDC32StackBuilder::addWriteExpTrigDelays(CVMUSB& list, vector<long int> delays) 
  {
    addWriteExpTrigDelay0(list,delays.at(0));
    addWriteExpTrigDelay1(list,delays.at(1));
  }

  void CMQDC32StackBuilder::addWriteBankOffsets(CVMUSB& list, vector<long int> values) 
  {
    list.vmeWrite16(m_base + Reg::BankOffset0, initamod, values.at(0));
    list.delay(MQDCDELAY);
    list.vmeWrite16(m_base + Reg::BankOffset1, initamod, values.at(1));
    list.delay(MQDCDELAY);
  }
  

  void CMQDC32StackBuilder::addWritePulserState(CVMUSB& list, uint16_t state)
  {
    list.vmeWrite16(m_base+Reg::TestPulser, initamod, state);
    list.delay(MQDCDELAY);
  }
 

  void CMQDC32StackBuilder::addWritePulserAmplitude(CVMUSB& list, uint8_t val)
  {
    list.vmeWrite16(m_base+Reg::PulserAmp, initamod, val);
    list.delay(MQDCDELAY);
  }


  void CMQDC32StackBuilder::addWriteTimeDivisor(CVMUSB& list, uint16_t divisor)
  {
    list.vmeWrite16(m_base + Reg::TimingDivisor, initamod, divisor);
    list.delay(MQDCDELAY);
  }


  void CMQDC32StackBuilder::addWriteInputCoupling(CVMUSB& list, uint16_t type)
  {
    list.vmeWrite16(m_base+Reg::InputCoupling, initamod, type);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addResetTimestamps(CVMUSB& list) 
  {
    list.vmeWrite16(m_base + Reg::TimestampReset, initamod, uint16_t(3)); // Reset both counters.
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteECLTermination(CVMUSB& list, uint16_t type){
    list.vmeWrite16(m_base + Reg::ECLTermination, initamod, type);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteECLGate1Input(CVMUSB& list, uint16_t type)
  {
    list.vmeWrite16(m_base + Reg::ECLGate1, initamod, type);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteECLFCInput(CVMUSB& list, uint16_t type)
  {
    list.vmeWrite16(m_base + Reg::ECLFC, initamod, type);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteNIMGate1Input(CVMUSB& list, uint16_t type){
    list.vmeWrite16(m_base + Reg::NIMGate1, initamod, type);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteNIMFCInput(CVMUSB& list, uint16_t type)
  {
    list.vmeWrite16(m_base + Reg::NIMFC, initamod, type);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteNIMBusyOutput(CVMUSB& list, uint16_t type) {
    list.vmeWrite16(m_base + Reg::NIMBusy, initamod, type);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteTimeBaseSource(CVMUSB& list, uint16_t val){
    list.vmeWrite16(m_base + Reg::TimingSource, initamod, val);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteMultiEventMode(CVMUSB& list, uint16_t val){
    list.vmeWrite16(m_base + Reg::MultiEvent, initamod, val);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteTransferCount(CVMUSB& list, uint16_t val){
    list.vmeWrite16(m_base + Reg::MaxTransfer, initamod, val);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addInitializeFifo(CVMUSB& list)
  {
    list.vmeWrite16(m_base + Reg::InitFifo, initamod, 1);
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteLowerMultLimits(CVMUSB& list, vector<int> values) {
    list.vmeWrite16(m_base + Reg::MultLimitLow0, initamod, values.at(0));
    list.delay(MQDCDELAY);
    list.vmeWrite16(m_base + Reg::MultLimitLow1, initamod, values.at(1));
    list.delay(MQDCDELAY);
  }
  void CMQDC32StackBuilder::addWriteUpperMultLimits(CVMUSB& list, vector<int> values) {
    list.vmeWrite16(m_base + Reg::MultLimitHigh0, initamod, values.at(0));
    list.delay(MQDCDELAY);
    list.vmeWrite16(m_base + Reg::MultLimitHigh1, initamod, values.at(1));
    list.delay(MQDCDELAY);
  }

  void CMQDC32StackBuilder::addWriteCounterReset(CVMUSB& list, uint8_t mode) {
    list.vmeWrite16(m_base + Reg::EventCounterReset, initamod, mode);
    list.delay(MQDCDELAY);
  }

  
  /// addReadoutList methods.

  void CMQDC32StackBuilder::addResetReadout(CVMUSBReadoutList& list) {
    list.addWrite16(m_base + Reg::ReadoutReset, initamod, 0);
  }

  void CMQDC32StackBuilder::addFifoRead(CVMUSBReadoutList& list, size_t transfers) {
    list.addFifoRead32(m_base + Reg::eventBuffer, readamod, transfers);
  }
} // end of namespace
