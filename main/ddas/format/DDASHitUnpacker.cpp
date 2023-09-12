/**
 * @file DDASHitUnpacker.cpp
 * @brief Implementation of an unpacker for DDAS data recorded by 
 * NSCLDAQ/FRIBDAQ.
 */

#include "DDASHitUnpacker.h"
#include "DDASBitMasks.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
	
/**
 * @details
 * This expects data from a DDAS readout program. It will parse the entire 
 * body of the event in a manner that is consistent with the data present. 
 * In other words, it uses the sizes of the event encoded in the data to 
 * determine when the parsing is complete.
 *
 * While it parses, it stores the results into the data members of the object 
 * hit. Prior to parsing, all data members are reset to 0 using the Reset() 
 * method. 
 */
const uint32_t*
DAQ::DDAS::DDASHitUnpacker::unpack(
    const uint32_t* beg, const uint32_t* sentinel, DDASHit& hit
    )
{

    if (beg == sentinel) {
	std::stringstream errmsg;
	errmsg << "DDASHitUnpacker::unpack() ";
	errmsg << "Unable to parse empty data buffer.";
	throw std::runtime_error(errmsg.str());
    }

    const uint32_t* data = beg;

    data = parseBodySize(data, sentinel);
    data = parseModuleInfo(hit, data);
    data = parseHeaderWord0(hit, data);
    data = parseHeaderWords1And2(hit, data);
    data = parseHeaderWord3(hit, data);

    // finished upacking the minimum set of data
	    
    uint32_t channelheaderlength = hit.GetChannelLengthHeader();
    uint32_t channellength = hit.GetChannelLength();
    size_t tracelength = hit.GetTraceLength();
	    
    // more unpacking data
	    
    if(channellength != (channelheaderlength + tracelength/2)){
	std::stringstream errmsg;
	errmsg << "ERROR: Data corruption: ";
	errmsg << "Inconsistent data lengths found in header ";
	errmsg << "\nChannel length = " << std::setw(8) << channellength;
	errmsg << "\nHeader length  = " << std::setw(8) << channelheaderlength;
	errmsg << "\nTrace length   = " << std::setw(8) << tracelength;
	throw std::runtime_error(errmsg.str());
    }

    // Longwords per optional enabled data output:
    // External TS: 2
    // Energy sums: 4
    // QDC sums:    8
    // Trace:       ceil(0.5*L*f)
    //   where L = trace length in microseconds, f = module MSPS
     
    if(channelheaderlength == 6) {
	data = extractExternalTimestamp(data, hit);

    } else if(channelheaderlength == 8) {
	data = extractEnergySums(data, hit);

    } else if (channelheaderlength == 10) {
	data = extractEnergySums(data, hit);
	data = extractExternalTimestamp(data, hit);

    } else if(channelheaderlength == 12) {
	data = extractQDC(data, hit);

    } else if (channelheaderlength == 14) {
	data = extractQDC(data, hit);
	data = extractExternalTimestamp(data, hit);

    } else if(channelheaderlength == 16) {
	// If channel header length is 16 then the extra 12 words are
	// energy and QDC sums
	data = extractEnergySums(data, hit);
	data = extractQDC(data, hit);

    } else if(channelheaderlength == 18) {
	// If channel header length is 18 then the extra 12 words are
	// energy and QDC sums
	data = extractEnergySums(data, hit);
	data = extractQDC(data, hit);
	data = extractExternalTimestamp(data, hit);

    }

    // If trace length is non zero, retrieve the trace
    if(tracelength != 0) {
	data = parseTraceData(hit, data);
    }

    return data;
}

/**
 * @details
 * This expects data from a DDAS readout program. It will parse the entire 
 * body of the event in a manner that is consistent with the data present. 
 * In other words, it uses the sizes of the event encoded in the data to 
 * determine when the parsing is complete.
 *
 * While it parses, it stores the results into the data members of the object 
 * hit. Prior to parsing, all data members are reset to 0 using the Reset() 
 * method. 
 */
std::tuple<DAQ::DDAS::DDASHit, const uint32_t*>
DAQ::DDAS::DDASHitUnpacker::unpack(
    const uint32_t *beg, const uint32_t* sentinel
    )
{
    DDASHit hit;
    const uint32_t* data = unpack(beg, sentinel, hit);
    return std::make_tuple(hit, data);
}

/**
 * @details
 * The first word of the body passed to this function is the self-inclusive 
 * event size in 16-bit words. 
 */
const uint32_t*
DAQ::DDAS::DDASHitUnpacker::parseBodySize(
    const uint32_t* data, const uint32_t* sentinel
    )
{
    uint32_t nShorts = *data; 
    // Make sure there is enough data to parse
    if (
	(data + nShorts/sizeof(uint16_t) > sentinel) && (sentinel != nullptr)
	) {
	throw std::runtime_error(
	    "DDASHitUnpacker::unpack() Incomplete event data."
	    );
    }

    return (data+1);
}

/**
 * @details
 * The lower 16 bits encode the ADC frequency, the upper 16 bits encode the 
 * hardware revision and ADC resolution.
 */
const uint32_t*
DAQ::DDAS::DDASHitUnpacker::parseModuleInfo(
    DDASHit& hit, const uint32_t* data
    )
{
    uint32_t datum = *data++;
    hit.setADCFrequency(datum & LOWER16BITMASK);
    hit.setADCResolution((datum>>16) & 0xff);
    hit.setHardwareRevision((datum>>24) & 0xff);

    return data;
}

/**
 * @details
 * Word 0 contains:
 * - Crate/slot/channel information.
 * - The header and channel lengths in 32-bit words.
 * - The ADC overflow code.
 * - The module finish code (equals 1 if piled up).
 */
const uint32_t*
DAQ::DDAS::DDASHitUnpacker::parseHeaderWord0(
    DDASHit& hit, const uint32_t* data
    )
{
    uint32_t datum = *data++;
    hit.setChannel(datum & CHANNELIDMASK);
    hit.setSlot((datum & SLOTIDMASK) >> 4);
    hit.setCrate((datum & CRATEIDMASK) >> 8);
    hit.setChannelHeaderLength((datum & HEADERLENGTHMASK) >> 12 );
    hit.setChannelLength((datum & CHANNELLENGTHMASK) >> 17);
    hit.setOverflowCode((datum & OVERFLOWMASK) >> 30);
    hit.setFinishCode((datum & FINISHCODEMASK) >> 31 );
      
    return data;
}

/**
 * @details
 * Words 1 and 2 contain the timestamp and CFD information. The meaning of the 
 * CFD word depends on the module type. The unpacker abstracts this meaning 
 * away from the user. Note that we know the module type  if the module 
 * identifier word was unpacked before calling this function.
 *
 * Word 1 contains:
 * - The lower 32 bits of the 48-bit timestamp.
 * Word 2 contains:
 * - The upper 16 bits of the 48-bit timestamp.
 * - The CFD result.
 */
const uint32_t*
DAQ::DDAS::DDASHitUnpacker::parseHeaderWords1And2(
    DDASHit& hit, const uint32_t* data
    )
{
    uint32_t timelow      = *data++;
    uint32_t datum1       = *data++;
    uint32_t timehigh     = datum1&0xffff;
    uint32_t adcFrequency = hit.GetModMSPS();

    double   cfdCorrection;
    uint32_t cfdtrigsource, cfdfailbit, timecfd;
    uint64_t coarseTime;

    coarseTime = computeCoarseTime(adcFrequency, timelow, timehigh) ;
    cfdCorrection = parseAndComputeCFD(hit, datum1);

    hit.setTimeLow(timelow);
    hit.setTimeHigh(timehigh);
    hit.setCoarseTime( coarseTime ); 
    hit.setTime(static_cast<double>(coarseTime) + cfdCorrection);

    return data;
}

/**
 * @details
 * Word 3 contains:
 * - The ADC trace overflow flag.
 * - The trace length in samples (16-bit words).
 * - The hit energy.
 */
const uint32_t*
DAQ::DDAS::DDASHitUnpacker::parseHeaderWord3(
    DDASHit& hit, const uint32_t* data
    )
{
    hit.setTraceLength((*data >> 16) & 0x7fff);
    hit.setADCOverflowUnderflow(*data >> 31);
    hit.setEnergy(*data & LOWER16BITMASK);

    return (data+1);
}

/**
 * @details
 * The 16-bit trace data is stored two samples to one 32-bit word in 
 * little-endian. The data for sample i is stored in the lower 16 bits while 
 * the data for sample i+1 is stored in the upper 16 bits. For ADCs with less 
 * than 16-bit resolution, those bits are set to 0.
 */
const uint32_t*
DAQ::DDAS::DDASHitUnpacker::parseTraceData(
    DDASHit& hit, const uint32_t* data
    )
{
    std::vector<uint16_t>& trace = hit.GetTrace();
    size_t tracelength = hit.GetTraceLength();
    trace.reserve(tracelength);
    for(size_t i = 0; i < tracelength/2; i++){
	uint32_t datum = *data++;
	trace.push_back(datum & LOWER16BITMASK);
	trace.push_back((datum & UPPER16BITMASK)>>16);
    }

    return data;
}

/**
 * @details
 * The value of the CFD correction depends on the module. Because the module 
 * information is encoded in the data, this function should be called after 
 * parseModuleInfo().
 */
std::tuple<double, uint32_t, uint32_t, uint32_t>
DAQ::DDAS::DDASHitUnpacker::parseAndComputeCFD(uint32_t ModMSPS, uint32_t data)
{

    double correction;
    uint32_t cfdtrigsource, cfdfailbit, timecfd;

    // Check on the module MSPS and pick the correct CFD unpacking algorithm 
    if(ModMSPS == 100){
	// 100 MSPS modules don't have trigger source bits
	cfdfailbit    = ((data & BIT31MASK) >> 31) ; 
	cfdtrigsource = 0;
	timecfd       = ((data & BIT30to16MASK) >> 16);
	correction    = (timecfd/32768.0) * 10.0; // 32768 = 2^15
    }
    else if (ModMSPS == 250) {
	// CFD fail bit in bit 31
	cfdfailbit    = ((data & BIT31MASK) >> 31 );
	cfdtrigsource = ((data & BIT30MASK) >> 30 );
	timecfd       = ((data & BIT29to16MASK) >> 16);
	correction    = (timecfd/16384.0 - cfdtrigsource)*4.0; 
    }
    else if (ModMSPS == 500) {
	// no fail bit in 500 MSPS modules
	cfdtrigsource = ((data & BIT31to29MASK) >> 29 );
	timecfd       = ((data & BIT28to16MASK) >> 16);
	correction    = (timecfd/8192.0 + cfdtrigsource - 1)*2.0;
	cfdfailbit    = (cfdtrigsource == 7) ? 1 : 0;
    }

    return std::make_tuple(correction, timecfd, cfdtrigsource, cfdfailbit);

}

/**
 * @details
 * The value of the CFD correction depends on the module. Because the module 
 * information is encoded in the data, this function should be called after 
 * parseModuleInfo().
 */
double
DAQ::DDAS::DDASHitUnpacker::parseAndComputeCFD(DDASHit& hit, uint32_t data)
{

    double correction;
    uint32_t cfdtrigsource, cfdfailbit, timecfd;
    uint32_t ModMSPS = hit.GetModMSPS();

    // check on the module MSPS and pick the correct CFD unpacking algorithm 
    if(ModMSPS == 100){
	// 100 MSPS modules don't have trigger source bits
	cfdfailbit    = ((data & BIT31MASK) >> 31) ; 
	cfdtrigsource = 0;
	timecfd       = ((data & BIT30to16MASK) >> 16);
	correction    = (timecfd/32768.0) * 10.0; // 32768 = 2^15
    }
    else if (ModMSPS == 250) {
	// CFD fail bit in bit 31
	cfdfailbit    = ((data & BIT31MASK) >> 31 );
	cfdtrigsource = ((data & BIT30MASK) >> 30 );
	timecfd       = ((data & BIT29to16MASK) >> 16);
	correction    = (timecfd/16384.0 - cfdtrigsource)*4.0; 
    }
    else if (ModMSPS == 500) {
	// no fail bit in 500 MSPS modules
	cfdtrigsource = ((data & BIT31to29MASK) >> 29 );
	timecfd       = ((data & BIT28to16MASK) >> 16);
	correction    = (timecfd/8192.0 + cfdtrigsource - 1)*2.0;
	cfdfailbit    = (cfdtrigsource == 7) ? 1 : 0;
    }

    hit.setCFDFailBit(cfdfailbit);
    hit.setCFDTrigSourceBit(cfdtrigsource);
    hit.setRawCFDTime(timecfd);

    return correction;
}

/** 
 * @details
 * Form the timestamp from the low and high bits and convert it to a time in 
 * nanoseconds.
 *
 * The calculations for the various modules are as follows:
 *
 * For the 100 MSPS module:
 *
 * \f[\text{time} = 10\times((\text{timehigh} << 32) 
 * + \text{timelow})\f]
 *  
 * For the 250 MSPS module...
 *
 * \f[\text{time} = 8\times((\text{timehigh} << 32) 
 * + \text{timelow})\f]
 *
 * For the 500 MSPS module,
 *
 * \f[\text{time} = 10\times((\text{timehigh} << 32) 
 * + \text{timelow})\f]
 */
uint64_t
DAQ::DDAS::DDASHitUnpacker::computeCoarseTime(
    uint32_t adcFrequency, uint32_t timelow, uint32_t timehigh
    )
{
      
    uint64_t toNanoseconds = 1;

    if(adcFrequency == 100){
	toNanoseconds = 10;
    }
    else if (adcFrequency == 250) {
	toNanoseconds = 8;
    }
    else if (adcFrequency == 500) {
	toNanoseconds = 10;
    }

    uint64_t tstamp = timehigh;
    tstamp = tstamp << 32;
    tstamp |= timelow;

    return tstamp*toNanoseconds;
}

/**
 * @details
 * Energy sums consist of 4 32-bit words, which are, in order:
 * 0. The trailing (pre-gap ) sum.
 * 1. The gap sum.
 * 2. The leading (post-gap) sum.
 * 3. The 32-bit IEEE 754 floating point baseline value.
 *
 * If the hit is not reset between calls to this function, the energy sum 
 * data will be appended to the end of the exisiting energy sums.
 */
const uint32_t*
DAQ::DDAS::DDASHitUnpacker::extractEnergySums(
    const uint32_t* data, DDASHit& hit
    )
{
    std::vector<uint32_t>& energies = hit.GetEnergySums();
    energies.reserve(4);
    energies.insert(energies.end(), data, data+4);
    return data + 4;
}

/**
 * @details
 * QDC sums consist of 8 32-bit words. If the hit is not reset between calls 
 * to this function, the QDC sum data will be appended to the end of the 
 * exisiting QDC sums.
 */
const uint32_t*
DAQ::DDAS::DDASHitUnpacker::extractQDC(const uint32_t* data, DDASHit& hit)
{
    std::vector<uint32_t>& qdcVals = hit.GetQDCSums();
    qdcVals.reserve(8);
    qdcVals.insert(qdcVals.end(), data, data+8);
    return data + 8;
}

/**
 * @details
 * Unpack and set the 48-bit external timestamp. Unlike the internal timestamp 
 * where the conversion from clock tics to nanoseconds is known, for the 
 * external timestamp no unit conversion is applied. Converting the timestamp 
 * to proper units is left to the user.
 */
const uint32_t* 
DAQ::DDAS::DDASHitUnpacker::extractExternalTimestamp(
    const uint32_t* data, DDASHit& hit
    ) 
{
    uint64_t tstamp = 0;
    uint32_t temp = *data++;
    tstamp = *data++;
    tstamp = ((tstamp << 32) | temp); 
    hit.setExternalTimestamp(tstamp);
    return data;
}
