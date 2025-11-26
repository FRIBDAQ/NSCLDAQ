/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief Header for S800 trigger module register file parser..
*/
#ifndef S800TRIGGERREGISTERS_H
#define S800TRIGGERREGISTERS_H

#include <cstdint>   // Let's go all in and do this the modern way.
#include <string>

class CVMUSB;
class CVMUSBReadoutList;
namespace Json {
    class Value;
}
/**
 * @class S800TriggerRegisters
 * 
 *   This class:
 * * Processes the S800 trigger definition JSOn file and given a base address for the module:
 * * Provides read write operations for the various registers we care about.
 * 
 * This is all in the context of the VMUSB/MVLC acquisition frameworks.
 * 
 * @note the class is supposed to be used as a utility class for those environments,
 * not as an actual readout module.
 * @note, the assumption is that actual speed is not that important, that is we don't
 * need to cache actual register offsets. only the JSON document. 
 */
class S800TriggerRegisters {
public:
    typedef enum _ClockSource {internal, external} ClockSource;

private:
    std::uint32_t    m_base;                     // Module base address.
    Json::Value&     m_registerSpec;            // Parsed JsON of register def file.
public:
    S800TriggerRegisters(std::uint32_t base, const char* registerDefFile);
    virtual ~S800TriggerRegisters();

    // Canonicals we forbid:
private:
    S800TriggerRegisters(const S800TriggerRegisters&);
    S800TriggerRegisters& operator=(const S800TriggerRegisters&);
    int operator==(const S800TriggerRegisters&);
    int operator!=(const S800TriggerRegisters&);

public:
    // Things which might also be done at event-readout time.

    std::uint64_t readTimestamp(CVMUSB& controller);
    void addReadTimestamp(CVMUSBReadoutList& list);

    std::uint32_t readTriggerMask(CVMUSB& controller);
    void addReadTriggerMask(CVMUSBReadoutList& list);

    void swClear(CVMUSB& controller);
    void addSwClear(CVMUSBReadoutList& list);

    void resetBusy(CVMUSB& controller);
    void addResetBusy(CVMUSBReadoutList& list);
    
    // Things never done at event readout-time.
    
    void enableExternalClear(CVMUSB& controller, bool state);
    void setRunNumber(CVMUSB& controller, std::uint32_t runNumber);
    
    std::string describeJSON(CVMUSB& controller);
private:
    // internal utilities - mostly used for getting register addresses from the
    // Json and adding m_base to them

    std::uint32_t timestampLowBits() const;
    std::uint32_t timestampHighBits() const;
    std::uint32_t triggerMask() const;
    std::uint32_t swClearRegister() const;
    std::uint32_t busyResetRegister() const;
    std::uint32_t runNumberLowBits() const;
    std::uint32_t runNumberHighBits() const;
    std::uint32_t externalClearEnableRegister() const;

    std::uint32_t getRegister(const char* name) const;
};
#endif