/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Aaron Chester 
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file DDASDataSimulator.cpp
 * @brief Implements the class for simulating DDAS-style data recored by 
 * NSCLDAQ readout programs.
 */

#include "DDASDataSimulator.h"

#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <memory>
#include <chrono>
#include <cmath>
#include <sstream>

#include <Exception.h>
#include <CErrnoException.h>

#include <NSCLDAQFormatFactorySelector.h>
#include <RingItemFactoryBase.h>
#include <DataFormat.h>
#include <CRingItem.h>
#include <CDataFormatItem.h>
#include <CRingStateChangeItem.h>
#include <CPhysicsEventItem.h>

#include <DDASBitMasks.h>
#include <DDASHit.h>

const uint64_t LOWER_TS_BIT_MASK = 0x00000000FFFFFFFF; //!< Mask lower 32 bits.
const uint64_t UPPER_TS_BIT_MASK = 0x0000FFFF00000000; //!< Mask upper 16 bits.
const uint32_t PIXIE_MAX_ENERGY = 65535; //!< Max allowed energy value.
const uint16_t CFD_100_MSPS_MASK = 0x7FFF; //!< CFD mask for 100 MSPS modules.
const uint16_t CFD_250_MSPS_MASK = 0x3FFF; //!< CFD mask for 250 MSPS modules.
const uint16_t CFD_500_MSPS_MASK = 0x1FFF; //!< CFD mask for 500 MSPS modules.

using namespace ufmt;
using namespace ddasfmt;

/**
 * @brief Get the supported version from the integer specifier.
 * @param version NSCLDAQ major version as an integer.
 * @throws std::invalid_argument If the version specifier does not correspond 
 *   to a known verson.
 * @return The mapped version which can be used by e.g., ufmt factories.
 */
static FormatSelector::SupportedVersions
mapVersion(int version)
{
    switch (version) {
    case 12:
	return FormatSelector::v12;
    case 11:
	return FormatSelector::v11;
    case 10:
	return FormatSelector::v10;
    default:
	throw std::invalid_argument("Invalid DAQ format version specifier");
    }
}

/**
 * @details
 * The format factory owns the concrete factory subclass. Any exceptions when 
 * mapping version specifiers are thrown to the caller.
 */
DAQ::DDAS::DDASDataSimulator::DDASDataSimulator(std::string fname, int version)
    : m_fname(fname), m_fd(-1), m_start(0), m_stop(0)
{    
    auto mappedVersion = mapVersion(version);
    m_pFactory = &FormatSelector::selectFactory(mappedVersion);
}

/**
 * @details
 * On begin:
 *   - Open an output data file for writing.
 *   - Save the run start time.
 *   - Write a format item to the output file.
 *   - Write the begin run item to the output file.
 */
void
DAQ::DDAS::DDASDataSimulator::beginRun(int sourceID)
{
    m_fd = open(m_fname.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);    
    if (m_fd == -1) {
	std::string msg(
	    "DDASDataSimulator::beginRun() failed to open output file "
	    );
	msg += m_fname;
	throw CErrnoException(msg);
    }

    const auto now = std::chrono::system_clock::now();
    m_start = std::chrono::system_clock::to_time_t(now);
    char title[100];
    sprintf(title, "BEGIN simulated data run");
    
    std::unique_ptr<CDataFormatItem> pFormatItem(
	m_pFactory->makeDataFormatItem()
	);
    m_pFactory->putRingItem(pFormatItem.get(), m_fd);
    
    std::unique_ptr<CRingStateChangeItem> pBeginItem(
	m_pFactory->makeStateChangeItem(BEGIN_RUN, 0, 0, m_start, title)
	);
    pBeginItem->setBodyHeader(UINT64_MAX, sourceID, 1); // BEGIN barrier = 1
    m_pFactory->putRingItem(pBeginItem.get(), m_fd);
}

/**
 * @details
 * On end:
 *  - Set the run stop time.
 *  - Calculates an elapsed run time based on the start time set during 
 *    `beginRun()`. The elapsed run time will be nonsense if the start time 
 *    is not set by calling `beginRun()` prior to this function.
 *  - Writes an end run item to the output file.
 *  - Closes the output file.
 */
void
DAQ::DDAS::DDASDataSimulator::endRun(int sourceID)
{
    const auto now = std::chrono::system_clock::now();
    m_stop = std::chrono::system_clock::to_time_t(now);
    time_t elapsed = m_stop - m_start;
    char title[100];
    sprintf(title, "END simulated data run");
    
    std::unique_ptr<CRingStateChangeItem> pEndItem(
	m_pFactory->makeStateChangeItem(END_RUN, 0, elapsed, m_stop, title)
	);
    pEndItem->setBodyHeader(UINT64_MAX, sourceID, 2); // END barrier = 2
    m_pFactory->putRingItem(pEndItem.get(), m_fd);

    if (fsync(m_fd) == -1) {
	std::string msg(
	    "DDASDataSimulator::endRun() failed to sync data to output file "
	    );
	msg += m_fname;
	throw CErrnoException(msg);
    }
    
    if (close(m_fd) == -1) {
	std::string msg(
	    "DDASDataSimulator::endRun() failed to close output file "
	    );
	msg += m_fname;
	throw CErrnoException(msg);
    }
}

/**
 * @details
 * Sets the internal data buffer using `setBuffer()` and creates a 
 * PHYSICS_EVENT ring item from it. For events with an external timestamp, the 
 * user must provide a clock calibration in nanoseconds per clock tick. 
 * Otherwise the calibraton is determined from the module type. All exceptions 
 * thrown when parsing the hit are raised to the caller.
 */
void
DAQ::DDAS::DDASDataSimulator::putHit(
    const DDASHit& hit, int sourceID, bool useExtTS, double cal
    )
{
    // If useExtTS is not set:
    //   - must provide a valid time
    //   - must not provide an external timestamp
    // If useExtTS is set:
    //   - time is ignored, it can be set or not
    //   - must provide a valid external timestamp
    if (!useExtTS && (hit.getTime() <= 0)) {
	std::stringstream msg;
	msg << "Invalid hit time: " << hit.getTime(); 
	throw std::runtime_error(msg.str());
    }
    if (!useExtTS && hit.getExternalTimestamp()) {
	std::stringstream msg;
	msg << "Not using external timestamp but hit has extTS = "
	    << hit.getExternalTimestamp();
	throw std::runtime_error(msg.str());
    }
    if (useExtTS && !hit.getExternalTimestamp()) {
	std::stringstream msg;
	msg << "Using external timestamp with invalid time: "
	    << hit.getExternalTimestamp();
	throw std::runtime_error(msg.str());
    }
    
    // Pack the hit into a data buffer:
    
    setBuffer(hit);

    // Make the ring item out of the buffer and write it:

    uint64_t timestamp;
    if (useExtTS) {
	double clockCal = cal;
	timestamp = hit.getExternalTimestamp() * clockCal;
	if (clockCal <= 0.0) {
	    std::stringstream msg;
	    msg << "Invalid clock calibration " << clockCal
		<< " for external timestamp!!";
	    throw std::runtime_error(msg.str());
	}
    } else {
	double clockCal = static_cast<double>(getClockPeriod(hit));
	timestamp = getCoarseTimestamp(hit) * clockCal;
    }
    
    uint32_t bodySize  = (m_evtBuf.size() + 2)*sizeof(uint32_t)
	+ sizeof(BodyHeader) + sizeof(RingItemHeader) + 100;    
    std::unique_ptr<CPhysicsEventItem> pPhysicsItem(
	m_pFactory->makePhysicsEventItem(timestamp, sourceID, 0, bodySize)
	); // Already has a correct body header.
    auto pBody = reinterpret_cast<uint32_t*>(pPhysicsItem->getBodyPointer());
    
    // Make the DDASReadout-style hit. Note that the self-inclusive size and
    // module identification word are added when we set the event buffer:

    std::copy(m_evtBuf.begin(), m_evtBuf.end(), pBody);
    pBody += m_evtBuf.size();
    pPhysicsItem->setBodyCursor(pBody);
    pPhysicsItem->updateSize();
    
    m_pFactory->putRingItem(pPhysicsItem.get(), m_fd);
}

/**
 * @details
 * Sets the data buffer based on the size of the hit passed in. Assumes that 
 * the input hit data is valid.
 */
void
DAQ::DDAS::DDASDataSimulator::setBuffer(const DDASHit& hit)
{
    m_evtBuf.clear(); // Clear buffer before adding data.
    
    uint32_t hdrLen = getHeaderLength(hit);
    uint32_t chanLen = hdrLen + std::ceil(hit.getTrace().size()/2);
    uint32_t inclusiveSize = (chanLen + 2)*sizeof(uint32_t)/sizeof(uint16_t);
    
    // Add the first two data words:
    
    m_evtBuf.push_back(inclusiveSize);
    m_evtBuf.push_back(getModInfoWord(hit));
   
    setWord0(hit);    
    setWords1And2(hit);
    setWord3(hit);

    // Parse the optional event data and write it. There are a number of
    // cases to handle depending on what data is or isn't present:

    uint32_t extraWords = hdrLen - SIZE_OF_RAW_EVENT;
    if (extraWords == SIZE_OF_EXT_TS) {
	setExternalTS(hit);
    } else if (extraWords == SIZE_OF_ENE_SUMS) {
	setEnergySums(hit);
    } else if (extraWords == (SIZE_OF_ENE_SUMS + SIZE_OF_EXT_TS)) {
	setEnergySums(hit);
	setExternalTS(hit);
    } else if (extraWords == SIZE_OF_QDC_SUMS) {
	setQDCSums(hit);
    } else if (extraWords == (SIZE_OF_QDC_SUMS + SIZE_OF_EXT_TS)) {
	setQDCSums(hit);
	setExternalTS(hit);
    } else if (extraWords == (SIZE_OF_ENE_SUMS + SIZE_OF_QDC_SUMS)) {
	setEnergySums(hit);
	setQDCSums(hit);
    } else if (
	extraWords == (SIZE_OF_ENE_SUMS + SIZE_OF_QDC_SUMS + SIZE_OF_EXT_TS)
	) {
	setEnergySums(hit);
	setQDCSums(hit);
	setExternalTS(hit);
    }

    // Last but not least, the trace:

    setTraceData(hit);
}

void
DAQ::DDAS::DDASDataSimulator::dumpBuffer()
{
    for (int i = 0; i < m_evtBuf.size(); i++) {
	char number[32];
	sprintf(number, "%08x ", m_evtBuf[i]);
	std::cout << number;
	if (((i+1)%4 == 0)) {
	    std::cout << std::endl;
	}
    }
    std::cout << std::endl;
}

///
// Private functions:
//

/**
 * @warning Ignores input channel length. The channel length of the hit is 
 * calculated from its input data.
 */
void
DAQ::DDAS::DDASDataSimulator::setWord0(const DDASHit& hit)
{    
    uint32_t finishCode = hit.getFinishCode(); // Unless otherwise set, == 0.
    uint32_t hdrLen     = getHeaderLength(hit);
    uint32_t chanLen    = hdrLen + std::ceil(hit.getTrace().size()/2);
    uint32_t crate      = hit.getCrateID();
    uint32_t slot       = hit.getSlotID();
    uint32_t chan       = hit.getChannelID();
    
    uint32_t word = 0x0;
    word |= (finishCode << FINISH_CODE_SHIFT);
    word |= (chanLen << CHANNEL_LENGTH_SHIFT);
    word |= (hdrLen << HEADER_LENGTH_SHIFT);
    word |= (crate << CRATE_ID_SHIFT);
    word |= (slot << SLOT_ID_SHIFT);
    word |= chan; // No shift.
    
    m_evtBuf.push_back(word);
}

/**
 * @details
 * Note that it is possible for the values of words 1 and 2 to be zeroes in 
 * the event that an external timestamp is specified. 
 */
void
DAQ::DDAS::DDASDataSimulator::setWords1And2(const DDASHit& hit)
{    
    double time = hit.getTime();
    uint64_t coarseTime = getCoarseTimestamp(hit); // In clock ticks.    
    double coarseTimeCal = coarseTime * getClockPeriod(hit); // In ns.
    double corr = time - coarseTimeCal;

    // Lower 32 bits of coarse TS, word 1:
    
    uint32_t word = 0x0;
    word = (coarseTime & LOWER_TS_BIT_MASK);
    m_evtBuf.push_back(word);

    // Upper 16 bits of coarse timestamp, word 2, lower 16 bits:
    
    word = 0x0;
    word |= (coarseTime & UPPER_TS_BIT_MASK) >> 32; // 16????

    // Formatted CFD result, word 2, upper 16 bits:
    
    uint32_t cfdResult = getPackedCFDResult(hit, corr);
    word |= (cfdResult & LOWER_16_BIT_MASK) << 16;
    m_evtBuf.push_back(word);
}

/**
 * @warning It's the user's responsibility to input valid trace and overflow 
 * data for their simulated module type.
 */
void
DAQ::DDAS::DDASDataSimulator::setWord3(const DDASHit& hit)
{
    uint32_t ene = hit.getEnergy();
    auto trace = hit.getTrace();
    uint32_t len = trace.size();
    uint32_t ovfl = hit.getADCOverflowUnderflow();
    if (ene > PIXIE_MAX_ENERGY) {
	std::stringstream msg;
	msg << "Warning!!! Hit energy " << ene
	    << " > Pixie list-mode energy max! Saving only the "
	    << "lower 16 bits!";
	std::cerr << msg.str() << std::endl;
    }
    uint32_t word = 0x0;
    word |= (ene & LOWER_16_BIT_MASK); // Heed the warning!
    word |= (len << 16);
    word |= (ovfl << 31);

    m_evtBuf.push_back(word);
}

void
DAQ::DDAS::DDASDataSimulator::setExternalTS(const DDASHit& hit)
{
    uint64_t ts = hit.getExternalTimestamp();
    uint32_t word = (ts & LOWER_TS_BIT_MASK);
    
    m_evtBuf.push_back(word); // Add the lower 32 bits...
    
    word = (ts & UPPER_TS_BIT_MASK) >> 32;
    
    m_evtBuf.push_back(word); // ... and the upper 16 bits.
}

/**
 * @details
 * Assumes the energy sums are the correct size. Note that 
 * ddasfmt::DDASHit.setEnergySums(std::vector<uint32_t>) enforces the size 
 * requirement.
 */ 
void
DAQ::DDAS::DDASDataSimulator::setEnergySums(const DDASHit& hit)
{
    auto esums = hit.getEnergySums();
    for (const auto& e : esums) {
	m_evtBuf.push_back(e);
    }
}

/**
 * @details
 * Assumes the QDC sums are the correct size. Note that 
 * ddasfmt::DDASHit.setQDCSums(std::vector<uint32_t>) enforces the size 
 * requirement.
 */
void
DAQ::DDAS::DDASDataSimulator::setQDCSums(const DDASHit& hit)
{
    auto qdc = hit.getQDCSums();
    for (const auto& q : qdc) {
	m_evtBuf.push_back(q);
    }
}

/**
 * @details
 * Packs two consecutive uint16_t trace sample data into one uint32_t word.
 */
void
DAQ::DDAS::DDASDataSimulator::setTraceData(const DDASHit& hit)
{    
    auto trace = hit.getTrace();
    int len = trace.size();
    for (int i = 0; i < len; i += 2) {
	uint32_t word = 0x0;
	word |= trace[i];
	word |= trace[i+1] << 16;	
	m_evtBuf.push_back(word);
    }
}

/**
 * @details
 * If the hit has a header length, use it. Responsibility for getting this 
 * correct is on the user. FRIBDAQ DDAS unpackers which use the event size 
 * contained in the data will fail if this is set incorrectly.
 *
 * Otherwise, determine the header length by inspecting the data contained in 
 * the hit. It is generally safer to take this approach.
 */
uint32_t
DAQ::DDAS::DDASDataSimulator::getHeaderLength(const DDASHit& hit)
{
    uint32_t hdrLen = hit.getChannelHeaderLength();
    if (hdrLen) {
	return hdrLen;
    }

    // Figure it out from the data:

    hdrLen = SIZE_OF_RAW_EVENT; // We have at least 4 words.
    if (hit.getExternalTimestamp()) {
	hdrLen += SIZE_OF_EXT_TS;
    }
    if (hit.getEnergySums().size()) {
	hdrLen += SIZE_OF_ENE_SUMS;   
    }
    if (hit.getQDCSums().size()) {
	hdrLen += SIZE_OF_QDC_SUMS;
    }

    return hdrLen;
}


uint32_t 
DAQ::DDAS::DDASDataSimulator::getModInfoWord(const DDASHit& hit)
{
    int rev  = hit.getHardwareRevision();
    int bits = hit.getADCResolution();
    int msps = hit.getModMSPS();
    
    uint32_t word = 0x0;
    word |= (rev << HW_REVISION_SHIFT);
    word |= (bits << ADC_RESOLUTION_SHIFT);
    word |= msps; // No shift.
  
    return word;
}

/**
 * @details
 * Based on the module MSPS and size of the CFD correction, we need to latch 
 * the coarse timestamp to the correct FPGA clock cycle. The FPGAs for the 250 
 * and 500 MSPS modules process multiple ADC samples per 125 MHz or 100 MHz 
 * clock cycle, respectively. We use the size of the correction from the 
 * previous clock cycle time to determine where to latch the coarse timestamp. 
 * 
 * The process is a little bit magic-number-y, but the coarse timestamp is 
 * latched to the next clock cycle time (implies CFD correction < 0) if the 
 * ZCP occurs between the set of samples currently being processed and the 
 * previous set:
 *  - 500 MSPS: CFD correction from previous clock cycle > 8 ns.
 *  - 250 MSPS: CFD correction from the previous clock cycle > 4 ns.
 */
uint64_t
DAQ::DDAS::DDASDataSimulator::getCoarseTimestamp(const DDASHit& hit)
{
    double time = hit.getTime();
    int samplePeriod = getSamplePeriod(hit);
    int clockPeriod = getClockPeriod(hit);

    // Largest correction for one sample group, equal to 0 for 100 MSPS:
    
    int corrGroupMax = clockPeriod - samplePeriod;

    // Timestamp corresponding to clock cycle prior to the correction:
    
    uint64_t coarseTime = static_cast<uint64_t>(time);
    coarseTime -= coarseTime % clockPeriod;

    // Re-latch if needed:
    
    auto corr = time - coarseTime; // The CFD correction.
    if (corrGroupMax && corr > corrGroupMax) {
	coarseTime += clockPeriod;
    }    

    return coarseTime/clockPeriod;
}

int
DAQ::DDAS::DDASDataSimulator::getClockPeriod(const DDASHit& hit)
{
    int clockPeriod;
    int msps = hit.getModMSPS();
    if (msps == 100) {
	clockPeriod = 10;
    } else if (msps == 250) {
	clockPeriod = 8;
    } else if (msps == 500) {
	clockPeriod = 10;
    } else {
	std::string msg("Cannot determine clock period for module MSPS: ");
	msg += std::to_string(msps);
	throw std::runtime_error(msg.c_str());
    }

    return clockPeriod;
}

int
DAQ::DDAS::DDASDataSimulator::getSamplePeriod(const DDASHit& hit)
{
    int samplePeriod;
    int msps = hit.getModMSPS();
    if (msps == 100) {
	samplePeriod = 10;
    } else if (msps == 250) {
	samplePeriod = 4;
    } else if (msps == 500) {
	samplePeriod = 2;
    } else {
	std::string msg("Cannot determine sampling period for module MSPS: ");
	msg += std::to_string(msps);
	throw std::runtime_error(msg.c_str());
    }

    return samplePeriod;
}

/**
 * @details
 * The CFD is always assumed to succeed, even if no correction exists. 
 * For 250 and 500 MSPS modules the CFD trigger source is identified based 
 * on the sign and magnitude of the ZCP.
 * 
 * @note (ASC 11/6/24): There may be rare cases when the nanosecond time is 
 * exactly halfway between two adjacent clock ticks. This causes issues because
 * the calculated zcp == 1 when the "real" zcp in the modules is in [0, 1). 
 * So in this case, we set the integer raw CFD value to its allowed maximum.
 */
uint32_t
DAQ::DDAS::DDASDataSimulator::getPackedCFDResult(
    const DDASHit& hit, double corr
    )
{
    uint32_t result = 0x0;
    double rawCFD;
    int failBit = 0; // Always succeed.
    int src; // Trigger source determined from the correction.
    int msps = hit.getModMSPS();
    double zcp = corr/static_cast<double>(getSamplePeriod(hit));
    
    if (msps == 100) {
	rawCFD = 32768.0*zcp;
	result |= static_cast<uint32_t>(std::floor(rawCFD));
	if (result == 32768) result = 32767; // Set to max allowed
	result = result & CFD_100_MSPS_MASK;
	result |= failBit << 15;
    } else if (msps == 250) {
	src = 1; // zcp < 0 case.
	if (zcp >= 0) {
	    src = 0;
	}
	rawCFD = 16384.0*(zcp + src);
	result |= static_cast<uint32_t>(std::floor(rawCFD));
	if (result == 16384) result = 16383; // Set to max allowed
	result = result & CFD_250_MSPS_MASK;
	result |= (src << 14);
	result |= (failBit << 15);
    } else if (msps == 500) {
	src = 0; // zcp < 0 case.
	if (zcp >= 0 && zcp < 1) {
	    src = 1;
	} else if (zcp >= 1 && zcp < 2) {
	    src = 2;
	} else if (zcp >= 2 && zcp < 3) {
	    src = 3;
	} else if (zcp >= 3 && zcp < 4) {
	    src = 4;
	}
	rawCFD = 8192.0*(zcp - src + 1);
	result |= static_cast<uint32_t>(std::floor(rawCFD));
	if (result == 8192) result = 8191; // Set to max allowed
	result = result &  CFD_500_MSPS_MASK;
	// For 500 MSPS, src == 7 indicates forced CFD. We always succeed, so:
	result |= (src << 13);
    }

    return result;
}
