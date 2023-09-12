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
* @file     VX2750ModuleUnpacker.h
* @brief    Define event processor to unpack a VX27x0 module.
* @author   Ron Fox
*
*/
#ifndef VX2750MODULEUNPACKER_H
#define VX2750MODULEUNPACKER_H
#include <cstdint>
#include <string>
#include <vector>
#include <set>
class CTreeParameterArray;

namespace caen_spectcl {
    static const unsigned int VX2750_MAX_CHANNELS(64);
/**
 *   @class VX2750ModuleUpacker
 *     Unpacks data that comes from a single module of a VX2750
 *     This is intended to be registered with a VX2750EventProcessor
 *     which will iterate through the modules in the event, matching module names
 *     with unpackers and calling each unpacker.
 */
class VX2750ModuleUnpacker {
private:
    std::string                 m_moduleName;
    std::uint64_t               m_channelMask;
    CTreeParameterArray*        m_ns;               // Timestamp in nanoseconds.
    CTreeParameterArray*        m_rawTimestamp;     // Raw timestamps
    CTreeParameterArray*        m_fineTimestamp;    // Fine timestamp from CFD.
    CTreeParameterArray*        m_energy;           // DPP Energy results.
    std::uint16_t               m_lowPriorityFlags[VX2750_MAX_CHANNELS];
    std::uint16_t               m_highPriorityFlags[VX2750_MAX_CHANNELS];
    std::uint16_t               m_downSampleSelection[VX2750_MAX_CHANNELS];
    std::uint16_t               m_failFlags[VX2750_MAX_CHANNELS];
    std::uint16_t               m_analogProbe1Types[VX2750_MAX_CHANNELS];
    std::vector<std::uint32_t>  m_analogProbe1Samples[VX2750_MAX_CHANNELS];
    std::uint16_t               m_analogProbe2Types[VX2750_MAX_CHANNELS];
    std::vector<std::uint32_t>  m_analogProbe2Samples[VX2750_MAX_CHANNELS];
    std::uint32_t               m_digitalProbe1Types[VX2750_MAX_CHANNELS];
    std::vector<std::uint8_t>   m_digitalProbe1Samples[VX2750_MAX_CHANNELS];
    std::uint32_t               m_digitalProbe2Types[VX2750_MAX_CHANNELS];
    std::vector<std::uint8_t>   m_digitalProbe2Samples[VX2750_MAX_CHANNELS];
    std::uint32_t               m_digitalProbe3Types[VX2750_MAX_CHANNELS];
    std::vector<std::uint8_t>   m_digitalProbe3Samples[VX2750_MAX_CHANNELS];
    std::uint32_t               m_digitalProbe4Types[VX2750_MAX_CHANNELS];
    std::vector<std::uint8_t>   m_digitalProbe4Samples[VX2750_MAX_CHANNELS];

public:
    VX2750ModuleUnpacker(const char* moduleName, const char* paramBaseName);
    virtual ~VX2750ModuleUnpacker();
    
    void reset();                   // Data reset method.
    const void* unpackHit(const void* pData);
    
    // Selectors:
    
    std::uint64_t getChannelMask() const;
    std::set<unsigned> getChannelSet() const;
    std::uint16_t getLowPriorityFlags(unsigned channel) const;
    std::uint16_t getHighPriorityFlags(unsigned channel) const;
    std::uint16_t getDownSampleSelection(unsigned channel) const;
    std::uint16_t  getFailFlags(unsigned channel) const;
    
    std::uint16_t getAnalogProbe1Type(unsigned channel) const;
    const std::vector<std::uint32_t>& getAnalogProbe1Samples(unsigned channel) const;
    std::uint16_t getAnalogProbe2Type(unsigned channel) const;
    const std::vector<std::uint32_t>& getAnalogProbe2Samples(unsigned channel) const;
    
    std::uint16_t getDigitalProbe1Type(unsigned channel) const;
    const std::vector<std::uint8_t>&  getDigitalProbe1Samples(unsigned channel) const;
    std::uint16_t getDigitalProbe2Type(unsigned channel) const;
    const std::vector<std::uint8_t>&  getDigitalProbe2Samples(unsigned channel) const;
    std::uint16_t getDigitalProbe3Type(unsigned channel) const;
    const std::vector<std::uint8_t>&  getDigitalProbe3Samples(unsigned channel) const;
    std::uint16_t getDigitalProbe4Type(unsigned channel) const;
    const std::vector<std::uint8_t>&  getDigitalProbe4Samples(unsigned channel) const;
    
    
    // Utilities:
private:
    void checkChannel(unsigned channel) const;
    
    
    
};
}
#endif