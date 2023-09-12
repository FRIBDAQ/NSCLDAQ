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

#ifndef __CMQDC32StackBuilder_H
#define __CMQDC32StackBuilder_h

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


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
    void addSoftReset(CVMUSBReadoutList& list);
    void addWriteAcquisitionState(CVMUSBReadoutList& list, bool on);
    void addReadAcquisitionState(CVMUSBReadoutList& list);

    void addResetReadout(CVMUSBReadoutList& list);

    void addDisableInterrupts(CVMUSBReadoutList& list);

    void addWriteIrqLevel(CVMUSBReadoutList& list, uint8_t level);
    void addReadIrqLevel(CVMUSBReadoutList& list);

    void addWriteIrqVector(CVMUSBReadoutList& list, uint8_t level);
    void addReadIrqVector(CVMUSBReadoutList& list);

    void addWriteIrqThreshold(CVMUSBReadoutList& list, uint16_t thresh);
    void addReadIrqThreshold(CVMUSBReadoutList& list);

    void addWriteWithdrawIrqOnEmpty(CVMUSBReadoutList& list, bool on);
    void addReadWithdrawIrqOnEmpty(CVMUSBReadoutList& list);

    void addWriteModuleID(CVMUSBReadoutList& list, uint16_t id);
    void addReadModuleID(CVMUSBReadoutList& list);

    // Thresholds
    void addWriteThreshold(CVMUSBReadoutList& list, unsigned int chan, 
                           int thresh);
    void addReadThreshold(CVMUSBReadoutList& list, unsigned int chan);
    void addWriteThresholds(CVMUSBReadoutList& list, 
                            std::vector<long int> thrs);
    void addWriteIgnoreThresholds(CVMUSBReadoutList& list, bool ignore);
    void addReadIgnoreThresholds(CVMUSBReadoutList& list);
                           


    void addWriteMarkerType(CVMUSBReadoutList& list, uint16_t type);
    void addReadMarkerType(CVMUSBReadoutList& list);

    void addWriteMemoryBankSeparation(CVMUSBReadoutList& list, uint16_t type);
    void addReadMemoryBankSeparation(CVMUSBReadoutList& list);

    void addWriteGateLimit0(CVMUSBReadoutList& list, uint8_t val);
    void addReadGateLimit0(CVMUSBReadoutList& list);
    void addWriteGateLimit1(CVMUSBReadoutList& list, uint8_t val);
    void addReadGateLimit1(CVMUSBReadoutList& list);
    void addWriteGateLimits(CVMUSBReadoutList& list, std::vector<long int> limits);

    void addWriteExpTrigDelay0(CVMUSBReadoutList& list, uint16_t val);
    void addReadExpTrigDelay0(CVMUSBReadoutList& list);
    void addWriteExpTrigDelay1(CVMUSBReadoutList& list, uint16_t val);
    void addReadExpTrigDelay1(CVMUSBReadoutList& list);
    void addWriteExpTrigDelays(CVMUSBReadoutList& list, std::vector<long int> values);

    void addWriteBankOffsets(CVMUSBReadoutList& list, std::vector<long int> values);
    void addReadBankOffset0(CVMUSBReadoutList& list);
    void addReadBankOffset1(CVMUSBReadoutList& list);

    void addWritePulserState(CVMUSBReadoutList& list, uint16_t state);
    void addReadPulserState(CVMUSBReadoutList& list);
    void addWritePulserAmplitude(CVMUSBReadoutList& list, uint8_t amp);
    void addReadPulserAmplitude(CVMUSBReadoutList& list);

    void addWriteTimeDivisor(CVMUSBReadoutList& list, uint16_t divisor);
    void addReadTimeDivisor(CVMUSBReadoutList& list);
    void addResetTimestamps(CVMUSBReadoutList& list);
    //

    void addWriteInputCoupling(CVMUSBReadoutList& list, uint16_t type);

    void addWriteECLTermination(CVMUSBReadoutList& list, uint16_t type);

    void addWriteECLGate1Input(CVMUSBReadoutList& list, uint16_t type);
    void addWriteECLFCInput(CVMUSBReadoutList& list, uint16_t type);

    void addWriteNIMGate1Input(CVMUSBReadoutList& list, uint16_t type);
    void addWriteNIMFCInput(CVMUSBReadoutList& list, uint16_t type);
    void addWriteNIMBusyOutput(CVMUSBReadoutList& list, uint16_t type);

    void addWriteTimeBaseSource(CVMUSBReadoutList& list, uint16_t val);
    void addWriteMultiEventMode(CVMUSBReadoutList& list, uint16_t val);
    void addWriteTransferCount(CVMUSBReadoutList& list, uint16_t val);

    void addInitializeFifo(CVMUSBReadoutList& list);

    void addWriteLowerMultLimits(CVMUSBReadoutList& list, std::vector<int> values);
    void addWriteUpperMultLimits(CVMUSBReadoutList& list, std::vector<int> values);

    void addWriteCounterReset(CVMUSBReadoutList& list, uint8_t mode);
    void addReadCounterReset(CVMUSBReadoutList& list);

    void addFifoRead(CVMUSBReadoutList& list, size_t transfers);
};

} // end of namespace

#endif
