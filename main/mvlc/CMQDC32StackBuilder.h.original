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

#ifndef MVLC_CMQDC32StackBuilder_H
#define MVLC_CMQDC32StackBuilder_h

#include "CReadoutHardware.h"
#include <stdint.h>
#include <string>
#include <vector>

// Forward class definitions:

class CVMUSB;
class CVMUSBReadoutList;


namespace MQDC32 {

class CMQDC32StackBuilder 
{
  private:
    uint32_t m_base;

  public:
    CMQDC32StackBuilder() = default;
    CMQDC32StackBuilder(const CMQDC32StackBuilder& rhs) = default;
    ~CMQDC32StackBuilder() = default;

    void setBase(uint32_t base) { m_base = base; }
    uint32_t getBase(uint32_t base) const { return m_base; }

  public:
    // Interactive methods
    void resetAll(CVMUSB& ctlr);
    void doSoftReset(CVMUSB& ctlr);


    // Stack building methods
    void addSoftReset(CVMUSB& list);
    void addWriteAcquisitionState(CVMUSB& list, bool on);
    void addResetReadout(CVMUSB& list);

    void addDisableInterrupts(CVMUSB& list);
    void addWriteIrqLevel(CVMUSB& list, uint8_t level);
    void addWriteIrqVector(CVMUSB& list, uint8_t level);
    void addWriteIrqThreshold(CVMUSB& list, uint16_t thresh);
    
    void addWriteWithdrawIrqOnEmpty(CVMUSB& list, bool on);
  

    void addWriteModuleID(CVMUSB& list, uint16_t id);
    

    // Thresholds
    void addWriteThreshold(CVMUSB& list, unsigned int chan, 
                           int thresh);
    void addWriteThresholds(CVMUSB& list, 
                            std::vector<long int> thrs);
    void addWriteIgnoreThresholds(CVMUSB& list, bool ignore);
                           

    void addWriteMarkerType(CVMUSB& list, uint16_t type);
    void addReadMarkerType(CVMUSB& list);

    void addWriteMemoryBankSeparation(CVMUSB& list, uint16_t type);
    

    void addWriteGateLimit0(CVMUSB& list, uint8_t val);
    void addWriteGateLimit1(CVMUSB& list, uint8_t val);
    void addWriteGateLimits(CVMUSB& list, std::vector<long int> limits);

    void addWriteExpTrigDelay0(CVMUSB& list, uint16_t val);
    void addWriteExpTrigDelay1(CVMUSB& list, uint16_t val);
    void addWriteExpTrigDelays(CVMUSB& list, std::vector<long int> values);

    void addWriteBankOffsets(CVMUSB& list, std::vector<long int> values);
    
    void addWritePulserState(CVMUSB& list, uint16_t state);
    void addWritePulserAmplitude(CVMUSB& list, uint8_t amp);

    void addWriteTimeDivisor(CVMUSB& list, uint16_t divisor);
    void addResetTimestamps(CVMUSB& list);
    //

    void addWriteInputCoupling(CVMUSB& list, uint16_t type);

    void addWriteECLTermination(CVMUSB& list, uint16_t type);

    void addWriteECLGate1Input(CVMUSB& list, uint16_t type);
    void addWriteECLFCInput(CVMUSB& list, uint16_t type);

    void addWriteNIMGate1Input(CVMUSB& list, uint16_t type);
    void addWriteNIMFCInput(CVMUSB& list, uint16_t type);
    void addWriteNIMBusyOutput(CVMUSB& list, uint16_t type);

    void addWriteTimeBaseSource(CVMUSB& list, uint16_t val);
    void addWriteMultiEventMode(CVMUSB& list, uint16_t val);
    void addWriteTransferCount(CVMUSB& list, uint16_t val);

    void addInitializeFifo(CVMUSB& list);

    void addWriteLowerMultLimits(CVMUSB& list, std::vector<int> values);
    void addWriteUpperMultLimits(CVMUSB& list, std::vector<int> values);

    void addWriteCounterReset(CVMUSB& list, uint8_t mode);
    

    // Add Readoutlist methods:

    void addResetReadout(CVMUSBReadoutList& list);             // overload.
    void addFifoRead(CVMUSBReadoutList& list, size_t transfers);
};

} // end of namespace

#endif
