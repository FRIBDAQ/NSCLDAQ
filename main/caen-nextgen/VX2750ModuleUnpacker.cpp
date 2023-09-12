/*
*-------------------------------------------------------------
 
 CAEN SpA 
 Via Vetraia, 11 - 55049 - Viareggio ITALY
 +390594388398 - www.caen.it

------------------------------------------------------------

**************************************************************************
* @note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
* software, documentation and results solely at his own risk.
*
* @file     VX2750ModuleUnpacker.cpp
* @brief    Implement unpacking of a single module.
* @author   Ron Fox
*
*/

#include "VX2750ModuleUnpacker.h"
#include <TreeParameter.h>
#include <sstream>
#include <string>
#include <stdexcept>
#include <iostream>
#include <string.h> 

namespace caen_spectcl {
/**
 * constructor
 *    Construct the tree parameters and reset the channel mask to prep for
 *    the first set of decodings
 * @param moduleName - must match the module name in the data to unpack.
 * @param paramBaseName - used for the parameter base names for the treeparameters.
 *                 We create 64 element tree parameter arrays::
 *                 -  basname.ns   - Timestamp in nanoseconds.
 *                 -  basename.rawTime - Raw coarse timestamps.
 *                 -  basename.cfdTime - CFD timestamp.
 *                 -  basename.energy  - DPP eneregies fished out of the waveforms.
 */
VX2750ModuleUnpacker::VX2750ModuleUnpacker(
    const char* moduleName, const char* paramBaseName
) :
    m_moduleName(moduleName),
    m_channelMask(0),
    m_ns(nullptr), m_rawTimestamp(nullptr), m_fineTimestamp(nullptr),
    m_energy(nullptr)
{
    {
        std::stringstream ns;
        ns << paramBaseName << ".ns";
        m_ns = new CTreeParameterArray(ns.str(), "ns", 64, 0);
    }
    {
        std::stringstream ts;
        ts << paramBaseName << ".rawTime";
        m_rawTimestamp = new CTreeParameterArray(ts.str(), "arb", 64, 0);
    }
    {
        std::stringstream finet;
        finet << paramBaseName << ".fineTime";
        m_fineTimestamp = new CTreeParameterArray(finet.str(), "arb", 64, 0);
    }
    {
        std::stringstream e;
        e << paramBaseName << ".energy";
        m_energy = new CTreeParameterArray(e.str(), "arb", 64, 0);
    }
    
}
/**
 * destructor
 *   Must delete the tree parameter arrays we created.  Everything else self-destructs
 */
VX2750ModuleUnpacker::~VX2750ModuleUnpacker()
{
    delete m_ns;
    delete m_rawTimestamp;
    delete m_fineTimestamp;
    delete m_energy;
}

/**
 * reset
 *   Should be called before decoding the hits that make up an event built event.
 *   Just clears the m_channelMask field so that we can properly keep track of the
 *   channels in an event.  Clear's the probe arrays as well.
 */
void
VX2750ModuleUnpacker::reset()
{
    m_channelMask = 0;
    for (int i = 0; i < VX2750_MAX_CHANNELS; i++) {
        m_analogProbe1Samples[i].clear();
        m_analogProbe2Samples[i].clear();
        m_digitalProbe1Samples[i].clear();
        m_digitalProbe2Samples[i].clear();
        m_digitalProbe3Samples[i].clear();
        m_digitalProbe4Samples[i].clear();
    }
}
/**
 * unpackHit
 *   Unpack a hit into the appropriate chunks of the tree parameter array and
 *   the internal data which can be fetched by event processors e.g.
 * @param pData - pointer to the module data.
 * @return const void* - Pointer to the byte just after the unpacked data.
 */
const void*
VX2750ModuleUnpacker::unpackHit(const void* pData)
{
    // This union allows us to access the data in the most natural way
    // for each data type:
    
    union pointer {
        const char*         c;
        const std::uint8_t* b;
        const std::uint16_t* w;
        const std::uint32_t* l;
        const std::uint64_t* q;
    } p;
    p.c = reinterpret_cast<const char*>(pData);
    
    p.l++;                          // Skip the size longword.
    
    // Check the mdoule name:
    
    std::string name(p.c);
    if (name != m_moduleName) {
        throw std::logic_error("Mismatch between data module name and unpacker module name!");
    }
    p.b += name.size() + 1;                      // +1 for null term.
    if (((name.size()+1) % 2) == 1 ) p.b++;        // Paded out to uint16_t.
    
    std::uint16_t ch = *p.w;
    std::uint64_t m  = 1;
    m = m << ch;                                // Bit in channel mask
    
    if (m & m_channelMask != 0) {
        std::cerr << "** Warning: duplicate channel " << ch <<
            " in module: " << name << " Second hit overwrites first" <<  std::endl;
    }
    m_channelMask |= m;
    
    p.w++;
    
    // Timestamp, coarse, fine, and energy...all the fixed size stuff:
    
    (*m_ns)[ch] = static_cast<double>(*(p.q));  p.q++;
    (*m_rawTimestamp)[ch] = static_cast<double>(*(p.q)); p.q++;
    (*m_fineTimestamp)[ch] = static_cast<double>(*(p.w)) ; p.w++;
    (*m_energy)[ch] = static_cast<double>(*(p.w)); p.w++;
    m_lowPriorityFlags[ch] = static_cast<double>(*(p.w)); p.w++;
    m_highPriorityFlags[ch] = static_cast<double>(*(p.w)); p.w++;
    m_downSampleSelection[ch] = static_cast<double>(*(p.w)); p.w++;
    m_failFlags[ch] = static_cast<double>(*(p.w)) ; p.w++;
    
    // Analog probe 1:
    
    m_analogProbe1Types[ch] = *(p.w); p.w++;
    size_t nSamples = *(p.l); p.l++;
    m_analogProbe1Samples[ch].resize(nSamples);
    memcpy(m_analogProbe1Samples[ch].data(), p.l, nSamples*sizeof(std::uint32_t));
    p.l += nSamples;
    
    // Analog probe 2
    
    m_analogProbe2Types[ch] = *(p.w); p.w++;
    nSamples = *(p.l); p.l++;
    m_analogProbe2Samples[ch].resize(nSamples);
    memcpy(m_analogProbe2Samples[ch].data(), p.l, nSamples*sizeof(std::uint32_t));
    p.l += nSamples;
    
    // Digital Probe 1:
    
    m_digitalProbe1Types[ch] =  *(p.w) ; p.w++;
    size_t nBytes = *(p.l); p.l++;
    m_digitalProbe1Samples[ch].resize(nBytes);
    memcpy(m_digitalProbe1Samples[ch].data(), p.b, nBytes);
    p.b += nBytes;
    
    // Digital Probe 2:
    
    m_digitalProbe2Types[ch] =  *(p.w) ; p.w++;
    nBytes = *(p.l); p.l++;
    m_digitalProbe2Samples[ch].resize(nBytes);
    memcpy(m_digitalProbe2Samples[ch].data(), p.b, nBytes);
    p.b += nBytes;
    
    // Digital Probe 1:
    
    m_digitalProbe3Types[ch] =  *(p.w) ; p.w++;
    nBytes = *(p.l); p.l++;
    m_digitalProbe3Samples[ch].resize(nBytes);
    memcpy(m_digitalProbe3Samples[ch].data(), p.b, nBytes);
    p.b += nBytes;
    // Digital Probe 1:
    
    m_digitalProbe4Types[ch] =  *(p.w) ; p.w++;
    nBytes = *(p.l); p.l++;
    m_digitalProbe4Samples[ch].resize(nBytes);
    memcpy(m_digitalProbe4Samples[ch].data(), p.b, nBytes);
    p.b += nBytes;
    
    const uint8_t* pBegin = reinterpret_cast<const uint8_t*>(pData);
    if (((p.b - pBegin) % 2) == 1) p.b++;  // Skip any padding.
    
    return p.b;              // Any field will do.
}
///////////////////////////////////////////////////////////////////////////////
// Getter for things that are not tree parameters... after all to get a
// tree parameter, just instantiate one with the same array and it'll bind to the
// same underlying data.
//

/**
 * getChannelMask
 *    @return uint64_t - mask of channels present in this event
 *    @note - in general, the unpackHit method will be potentially several times
 *            per event by our client.  Once the event has been fully unpacked,
 *            this can be called to get a mask of the channels that were unpacked.
 *            Bit 0 is channel zero, bit 1 channel 1 and so on.
 */
std::uint64_t
VX2750ModuleUnpacker::getChannelMask() const
{
    return m_channelMask;
}
/**
 * getChannelSet
 *    Same as above in case you want an std::set of the channels that have been
 *    hit.  This can be simpler to iterate over.
 * @return std::set<unsigned> set containing the channels that were hit.
 */
std::set<unsigned>
VX2750ModuleUnpacker::getChannelSet() const
{
    std::set<unsigned> result;
    for (int i =0; i < 64; i++) {
        if (m_channelMask & (1 << i)) {
            result.insert(i);
        }
    }
    return result;
}
/**
 * getLowPriorityFlags
 *    @param channel - Channel to fetch.
 *    @return std::uint16_t the flags for that channel
 *    @throw std::invalid_argument If the channel is invalid.  This is the
 *    case if:
 *    *   The channel number is too large (Bigger than VX2750_MAX_CHANNELS).
 *    *   The channel number's bit is not set in the hit channels mask.
 */
std::uint16_t
VX2750ModuleUnpacker::getLowPriorityFlags(unsigned channel) const
{
    checkChannel(channel);
    
    return m_lowPriorityFlags[channel];
}
/**
 * getHighPriorityFlags
 *    @param channel
 *    @return std::Uint16 _t the high priority flags for the selected channel.
 *    @throw std::invalid_argument - the channel is invalid, see getLowPriorityFLags
 */
std::uint16_t
VX2750ModuleUnpacker::getHighPriorityFlags(unsigned channel) const
{
    checkChannel(channel);
    return  m_highPriorityFlags[channel];
}
/**
 * getDownSampleSelection
 *     @param channel
 *     @return std::uint16_t - Code describing down sampling selection for the channel
 *     @throw std::invalid_argument -see getLowPriorityFlags.
 */
std::uint16_t
VX2750ModuleUnpacker::getDownSampleSelection(unsigned channel) const
{
    checkChannel(channel);
    return m_downSampleSelection[channel];
}
/**
 * getFailFlags
 *   @param channel - channel number
 *   @return std::uint16_t - fail flags for the channel
 *   @throw std::invalid_argument - if the channel is invalid.
 */
std::uint16_t
VX2750ModuleUnpacker::getFailFlags(unsigned channel) const
{
    checkChannel(channel);
    return m_failFlags[channel];
}
/**
 * getAnalogProbe1Type
 *    @param channel
 *    @return std::uint16_t - analog probe type code for the channel.
 *    @thow std::invalid_argument - the channel is not valid.
 */
std::uint16_t
VX2750ModuleUnpacker::getAnalogProbe1Type(unsigned channel) const
{
    checkChannel(channel);
    return m_analogProbe1Types[channel];
}
/**
 * getAnalogProbe1Samples
 *    @param channel - the channel to fetch for.
 *    @return const std::vector<uint32_t>& - reference to the samples for probe
 *                     for this channel
 *      @throw std::invalid_argument - if the channel is not valid.
 *      
 */
const std::vector<std::uint32_t>&
VX2750ModuleUnpacker::getAnalogProbe1Samples(unsigned channel) const
{
    checkChannel(channel);
    return m_analogProbe1Samples[channel];
}
/**
 * getAnalogProbe2Type
 *    @param channel
 *    @return std::uint16_t - analog probe type code for the channel.
 *    @thow std::invalid_argument - the channel is not valid.
 */
std::uint16_t
VX2750ModuleUnpacker::getAnalogProbe2Type(unsigned channel) const
{
    checkChannel(channel);
    return m_analogProbe2Types[channel];
}
/**
 * getAnalogProbe2Samples
 *    @param channel - the channel to fetch for.
 *    @return const std::vector<uint32_t>& - reference to the samples for probe
 *                     for this channel
 *      @throw std::invalid_argument - if the channel is not valid.
 *      
 */
const std::vector<std::uint32_t>&
VX2750ModuleUnpacker::getAnalogProbe2Samples(unsigned channel) const
{
    checkChannel(channel);
    return m_analogProbe2Samples[channel];
}

/**
 * getDigitalProbe1Type
 *   @param channel - a channel number with a hit.
 *   @return std::uint16_t - the probe type code.
 *   @throw std::invalid_argument - Channel is not a valid hit.
 */
std::uint16_t
VX2750ModuleUnpacker::getDigitalProbe1Type(unsigned channel) const
{
    checkChannel(channel);
    return m_digitalProbe1Types[channel];
}
/**
 * getDigitalProbe1Samples
 *    @param channel -valid hit channel number.
 *    @return const std::vector<std::uint8_t>& - references the hit samples.
 *    @throw std::invalid_argument - invalid channel.
 *    
 */
const std::vector<std::uint8_t>&
VX2750ModuleUnpacker::getDigitalProbe1Samples(unsigned channel) const
{
    checkChannel(channel);
    return m_digitalProbe1Samples[channel];
}

/**
 * getDigitalProbe2Type
 *   @param channel - a channel number with a hit.
 *   @return std::uint16_t - the probe type code.
 *   @throw std::invalid_argument - Channel is not a valid hit.
 */
std::uint16_t
VX2750ModuleUnpacker::getDigitalProbe2Type(unsigned channel) const
{
    checkChannel(channel);
    return m_digitalProbe2Types[channel];
}
/**
 * getDigitalProbe2Samples
 *    @param channel -valid hit channel number.
 *    @return const std::vector<std::uint8_t>& - references the hit samples.
 *    @throw std::invalid_argument - invalid channel.
 *    
 */
const std::vector<std::uint8_t>&
VX2750ModuleUnpacker::getDigitalProbe2Samples(unsigned channel) const
{
    checkChannel(channel);
    return m_digitalProbe2Samples[channel];
}

/**
 * getDigitalProbe3Type
 *   @param channel - a channel number with a hit.
 *   @return std::uint16_t - the probe type code.
 *   @throw std::invalid_argument - Channel is not a valid hit.
 */
std::uint16_t
VX2750ModuleUnpacker::getDigitalProbe3Type(unsigned channel) const
{
    checkChannel(channel);
    return m_digitalProbe3Types[channel];
}
/**
 * getDigitalProbe3Samples
 *    @param channel -valid hit channel number.
 *    @return const std::vector<std::uint8_t>& - references the hit samples.
 *    @throw std::invalid_argument - invalid channel.
 *    
 */
const std::vector<std::uint8_t>&
VX2750ModuleUnpacker::getDigitalProbe3Samples(unsigned channel) const
{
    checkChannel(channel);
    return m_digitalProbe3Samples[channel];
}

/**
 * getDigitalProbe4Type
 *   @param channel - a channel number with a hit.
 *   @return std::uint16_t - the probe type code.
 *   @throw std::invalid_argument - Channel is not a valid hit.
 */
std::uint16_t
VX2750ModuleUnpacker::getDigitalProbe4Type(unsigned channel) const
{
    checkChannel(channel);
    return m_digitalProbe4Types[channel];
}
/**
 * getDigitalProbe4Samples
 *    @param channel -valid hit channel number.
 *    @return const std::vector<std::uint8_t>& - references the hit samples.
 *    @throw std::invalid_argument - invalid channel.
 *    
 */
const std::vector<std::uint8_t>&
VX2750ModuleUnpacker::getDigitalProbe4Samples(unsigned channel) const
{
    checkChannel(channel);
    return m_digitalProbe4Samples[channel];
}
//////////////////////////////////////////////////////////////////////////////
// Private utilities.

/**
 * checkChannel
 *    Ensures a channel is valid.  A channel is valid if:
 *    -  It is in the range [0, VX2750_MAX_CHANNELS)
 *    -  It's corresponding bit is set in m_chnanelMask (a hit has been
 *       processed for it).
 *   @param channel - channel to check.
 *   @throw std::invalid_argument - if the channel is not valid.
 */
void
VX2750ModuleUnpacker::checkChannel(unsigned channel) const
{
    if (channel >= VX2750_MAX_CHANNELS) {
        throw std::invalid_argument("Channel number is out of range");
    }
    
    if ((m_channelMask & (1 << channel)) == 0) {
        throw std::invalid_argument("Channel was not hit");
    }
}

}                                             // caen_spectcl namespace.