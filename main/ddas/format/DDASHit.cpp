/** 
 * @file DDASHit.cpp
 * @brief Implement DDASHit class used to encapsulate DDAS events.
 */

#include "DDASHit.h"

#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <iomanip>

#include "DDASBitMasks.h"

/** 
 * @details
 * All member data are zero-initialized.
 */
DAQ::DDAS::DDASHit::DDASHit() :
    time(0),
    coarsetime(0),
    energy(0),
    timehigh(0),
    timelow(0),
    timecfd(0),
    finishcode(0),
    channellength(0),
    channelheaderlength(0),
    overflowcode(0),
    chanid(0),
    slotid(0),
    crateid(0),
    cfdtrigsourcebit(0),
    cfdfailbit(0),
    tracelength(0),
    ModMSPS(0),
    energySums(),
    qdcSums(),
    trace(),
    externalTimestamp(0),
    m_adcResolution(0),
    m_hdwrRevision(0),
    m_adcOverflowUnderflow(false)
{}

/** 
 * @details
 * For primitive types, this sets the values to 0. For vector 
 * data (i.e. trace), the vector is cleared and resized to 0.
 */
void
DAQ::DDAS::DDASHit::Reset() {
    time = 0;
    coarsetime = 0;
    energy = 0;
    timehigh = 0;
    timelow = 0;
    timecfd = 0;
    finishcode = 0;
    channellength = 0;
    channelheaderlength = 0;
    overflowcode = 0;
    chanid = 0;
    slotid = 0;
    crateid = 0;
    cfdtrigsourcebit = 0;
    cfdfailbit = 0;
    tracelength = 0;
    ModMSPS = 0;

    energySums.clear();
    qdcSums.clear();
    trace.clear();
      
    externalTimestamp = 0;
    m_adcResolution = 0;
    m_hdwrRevision = 0;
    m_adcOverflowUnderflow = false;
}
	
/** 
 * @brief Destructor.
 */
DAQ::DDAS::DDASHit::~DDASHit()
{}

void
DAQ::DDAS::DDASHit::setChannel(uint32_t channel)
{
    chanid = channel;
}

void
DAQ::DDAS::DDASHit::setSlot(uint32_t slot)
{
    slotid = slot;
}

void
DAQ::DDAS::DDASHit::setCrate(uint32_t crate)
{
    crateid = crate;
}

void
DAQ::DDAS::DDASHit::setChannelHeaderLength(uint32_t channelHeaderLength)
{
    channelheaderlength = channelHeaderLength;
}

void
DAQ::DDAS::DDASHit::setChannelLength(uint32_t channelLength)
{
    channellength = channelLength;
}

void
DAQ::DDAS::DDASHit::setOverflowCode(uint32_t overflowBit)
{
    overflowcode = overflowBit;
}

void
DAQ::DDAS::DDASHit::setFinishCode(bool finishCode)
{
    finishcode = finishCode;
}

/**
 * @details
 * The coarse timestamp is the leading-edge time in nanoseconds, 
 * without the CFD correction applied.
 */
void
DAQ::DDAS::DDASHit::setCoarseTime(uint64_t time)
{
    coarsetime = time;
}

void
DAQ::DDAS::DDASHit::setRawCFDTime(uint32_t data)
{
    timecfd = data;
}

/**
 * @details
 * The 250 MSPS and 500 MSPS modules de-serialize data into an FPGA 
 * which operates at some fraction of the ADC sampling rate. The CFD 
 * trigger source bit specifies which fractional time offset from the 
 * FPGA clock tick the CFD zero-crossing occured. For 100 MSPS modules,
 * the source bit is always equal to 0 (FPGA captures data also at
 * 100 MSPS).
 */
void
DAQ::DDAS::DDASHit::setCFDTrigSourceBit(uint32_t bit)
{
    cfdtrigsourcebit = bit;
}

/**
 * @details
 * The CFD fail bit == 1 if the CFD algorithm fails. The CFD can fail 
 * if the threshold value is too high or the CFD algorithm fails to 
 * find a zero-crossing point within 32 samples of the leading-edge 
 * trigger point.
 */
void
DAQ::DDAS::DDASHit::setCFDFailBit(uint32_t bit)
{
    cfdfailbit = bit;
}

void
DAQ::DDAS::DDASHit::setTimeLow(uint32_t datum)
{
    timelow = datum;
}

void
DAQ::DDAS::DDASHit::setTimeHigh(uint32_t datum)
{
    timehigh = datum & 0xffff;
}

void
DAQ::DDAS::DDASHit::setTime(double compTime)
{
    time = compTime;
}

void
DAQ::DDAS::DDASHit::setEnergy(uint32_t value)
{
    energy = value;
}
	
void
DAQ::DDAS::DDASHit::setTraceLength(uint32_t length)
{
    tracelength = length;
}

void
DAQ::DDAS::DDASHit::setADCFrequency(uint32_t value)
{
    ModMSPS = value;
}

void
DAQ::DDAS::DDASHit::setADCResolution(int value)
{
    m_adcResolution = value;
}

void
DAQ::DDAS::DDASHit::setHardwareRevision(int value)
{
    m_hdwrRevision = value;
}

void
DAQ::DDAS::DDASHit::appendEnergySum(uint32_t value)
{
    energySums.push_back(value);
}

void
DAQ::DDAS::DDASHit::appendQDCSum(uint32_t value)
{
    qdcSums.push_back(value);
}

void
DAQ::DDAS::DDASHit::appendTraceSample(uint16_t value)
{
    trace.push_back(value);
}

void
DAQ::DDAS::DDASHit::setExternalTimestamp(uint64_t value)
{
    externalTimestamp = value;
}

void
DAQ::DDAS::DDASHit::setADCOverflowUnderflow(bool state)
{
    m_adcOverflowUnderflow = state;
}

///
// Private methods
//

void
DAQ::DDAS::DDASHit::copyIn(const DDASHit& rhs) {
    time = rhs.time;
    coarsetime = rhs.coarsetime;
    energy= rhs.energy;
    timehigh = rhs.timehigh;
    timelow = rhs.timelow;
    timecfd = rhs.timecfd;
    finishcode = rhs.finishcode;
    channellength = rhs.channellength;
    channelheaderlength = rhs.channelheaderlength;
    overflowcode = rhs.overflowcode;
    chanid = rhs.chanid;
    slotid= rhs.slotid;
    crateid = rhs.crateid;
    cfdtrigsourcebit = rhs.cfdtrigsourcebit;
    cfdfailbit = rhs.cfdfailbit;
    tracelength =rhs.tracelength;
    ModMSPS = rhs.ModMSPS;
    energySums = rhs.energySums;
    qdcSums =rhs.qdcSums;
    trace = rhs.trace;
    externalTimestamp= rhs.externalTimestamp;
    m_hdwrRevision = rhs.m_hdwrRevision;
    m_adcResolution= rhs.m_adcResolution;
    m_adcOverflowUnderflow = rhs.m_adcOverflowUnderflow;      
}
