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
@brief Implementation for S800 trigger module register file parser..
*/

#include "s800TriggerRegisters.h"
#include <json/json.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <fstream>
#include <stdexcept>
#include <sstream>

static const std::uint8_t amod(CVMUSBReadoutList::a32UserData);

/**
 * constructor:
 *    @param base - base address of the module.
 *    @param registerDefFile Json file containing the register definitions.
 * 
 * -  Stores the base address.
 * -  Allocates a Json::Value object for the parsed register specification
 * -  Attempts to parse the register definition file -> m_registerSpec
 * @throw Json::Exception - (derived from std::exception) on parse failure.
 * @throw std::runtime_error - if not able to open the file.
 *
 */
S800TriggerRegisters::S800TriggerRegisters(std::uint32_t base,  const char* registerDefFile) :
    m_base(base),
    m_registerSpec(*(new Json::Value)) {

        // Open the definition file:

        std::ifstream json(registerDefFile);
        if (!json) {
            std::stringstream error_msg;
            error_msg << "Unable to open the register definition file: " << registerDefFile;
            std::string msg(error_msg.str());
            throw std::runtime_error(msg);
        }

        // Read the Json:

        json >> m_registerSpec;              // Could throw Json::Exception.

}

/** 
 * destructor
 *    Must destroy the m_registerSpec object to prevent leaks:
 */
S800TriggerRegisters::~S800TriggerRegisters() {
    delete &m_registerSpec;
}
//////////////////////////// public methods: ///////////////////////////////////////////////


/**
 *  readTimestamp
 *     @param controller - controller through which the timestamp parts will be read.
 *     @return std::uint64_t - the assembled timestamp.
 *     @note in the MVLC_GENERATOR environment, this is an error:
 */
std::uint64_t
S800TriggerRegisters::readTimestamp(CVMUSB& controller) {
#ifdef MVLC_GENERATOR                               // Can't do that in the MVLC
    throw std::runtime_error("s800TriggerRegisters::readTimestamp is illegal in mvlc generate.");
#else
    std::uint32_t data_low, data_high;
    if (controller.vmeRead32(timestampLowBits(), amod, &data_low) ) {
        throw std::runtime_error("Read of s800 trigger timestamp low bits failed");
    }
    if (controller.vmeRead32(timestampHighBits(), amod, &data_high)) {
        throw std::runtime_error("Read of s800 trigger timestamp high bits failed");
    }
    // Build the 64 bit timestamp:

    std::uint64_t result = data_high;
    result = (result << 32) | data_low;

    return result;
    
#endif    
}
/**
 * addReadTimstamp
 *    Add a read of the timestamp to a readout list:
 * 
 * @param list - CVMUSBReadoutList to which to add this.
 */
void
S800TriggerRegisters::addReadTimestamp(CVMUSBReadoutList& list) {
    list.addRead32(timestampLowBits(), amod);
    list.addRead32(timestampHighBits(), amod);
}

/**
 * readTriggerMask
 *    Read the value of the trigger mask
 * 
 * @param controller - the controller to read through.
 * @return std::uint32_t the mask value.
 * @note this is not legal in the MVLC generator.
 */
std::uint32_t 
S800TriggerRegisters::readTriggerMask(CVMUSB& controller) {
#ifdef MVLC_GENERATOR
    throw std::runtime_error("S800TriggerRegisters::readTriggerMask is not available in mvlctranslate");
#else
    std::uint32_t result;
    if (controller.vmeRead32(triggerMask(), amod, &result)) {
        throw std::runtime_error("Read of trigger mask register failed");
    }
    return result & 0x1ff;  // Only 9 bits.
#endif
}
/**
 *  addReadTriggerMask
 *     Add a read of the trigger mask to a readout list.
 * 
 * @param list - the CVMUSBReadoutList to add to.
 */
void
S800TriggerRegisters::addReadTriggerMask(CVMUSBReadoutList& list) {
    list.addRead32(triggerMask(), amod);   // User gonna have to mask that 
}

/**
 *  swClear
 *    Write to the software Clear register.  Presumably this does some sort of
 * clear operation in the logic.  The register must be toggled, 1, 0.
 *   @param controller - the controller object we write through.
 */
void
S800TriggerRegisters::swClear(CVMUSB& controller) {
    if (controller.vmeWrite32(swClearRegister(), amod, 1)) {
        throw std::runtime_error("Failed to write 1 to the software clear register");
    }
    if (controller.vmeWrite32(swClearRegister(), amod, 0)) {
        throw std::runtime_error("Failed to write 0 to the software clear register");
    }

}
/**
 * addSwClear
 *    Add a software clear to a readoutlist.
 * 
 * @param list - the list to add to.
 */
void
S800TriggerRegisters::addSwClear(CVMUSBReadoutList& list) {
    list.addWrite32(swClearRegister(), amod, 1);
    list.addWrite32(swClearRegister(), amod, 0);
}

/**
 * resetBusy
 *   Have toggle the value 1 then zero.
 * @param controller - controller through which the write is done.
 * 
 */
void
S800TriggerRegisters::resetBusy(CVMUSB& controller) {
    if (controller.vmeWrite32(busyResetRegister(), amod, 1)) {
        throw std::runtime_error("Failed to write 1 to the busy reset register");
    }
    if (controller.vmeWrite32(busyResetRegister(), amod, 0)) {
        throw std::runtime_error("Failed to write 0 to the busy reset register");
    }
}
/**
 * addResetBusy
 *    Add a write to the busy reset register to a readout list.
 * 
 * @param list - Readout list to modify.
 */
void
S800TriggerRegisters::addResetBusy(CVMUSBReadoutList& list) {
    list.addWrite32(busyResetRegister(), amod, 1);
    list.addWrite32(busyResetRegister(), amod, 0);
}
/**
 *  Arm the trigger register:
 * 
 */
void
S800TriggerRegisters::armTrigger(CVMUSB& controller) {
    if (controller.vmeWrite32(armTriggerRegister(), amod, 1)) {
        throw std::runtime_error("Failed to write a 1 to the trigger arm register.");
    }
    if (controller.vmeWrite32(armTriggerRegister(), amod, 0)) {
        throw std::runtime_error("Failed to write a 0 to the trigger arm register");
    }
}
/**
 * Add instructions to the stack to re-arm the trigger
 */
void
S800TriggerRegisters::addArmTrigger(CVMUSBReadoutList& list) {
    list.addWrite32(armTriggerRegister(), amod, 1);
    list.addWrite32(armTriggerRegister(), amod, 0);
}


/**
 * enableExternalClear
 *   Not totally sure what to do here as I don't have documentation
 * for the register itself....but I want to get the API finalized.
 * I'm going to assume for this that writing a 1 to the enableExternalClear
 * register will enable the external clear and a 0 disable it.
 * 
 * @param controller - controller through which the write is done.
 * @param state      - true to enable, false otherwise.
 */
void
S800TriggerRegisters::enableExternalClear(CVMUSB& controller, bool state) {
    if(controller.vmeWrite32(
        externalClearEnableRegister(), amod, state? 1 : 0)
    ) {
        throw std::runtime_error("Could not write the enable external clear register.");
    }
}
/**
 * setRunNumber
 * 
 *     Set the run number register. Note that the FRIB/NSCLDAQ run number is
 * a uint32_t but it seems(?) like there's a high and low set so we'll write
 * the high one to zero and the low one to the actual run number.
 * 
 * @param controller - controller through which the VME is accessed.
 * @param runNumber - the new run number.
 * 
 */
void
S800TriggerRegisters::setRunNumber(CVMUSB& controller, std::uint32_t runNumber) {
    // Write the bottom part:

    if (controller.vmeWrite32(runNumberLowBits(), amod, runNumber)) {
        throw std::runtime_error("Could not write the low order run number bits");
    }

    if (controller.vmeWrite32(runNumberHighBits(), amod, 0)) {
        throw std::runtime_error("Could not write the high order run number bits.");
    }
}
/**
 *  startRun
 *     Start the run by writing a 1 to the go register.
 * @param controller - controller through which the write is done. 
 */
void
S800TriggerRegisters::startRun(CVMUSB& controller) {
    if (controller.vmeWrite32(goRegister(), amod, 1)) {
        throw std::runtime_error("Failed to write 1 to the go register");
    }
}
/** 
 * stopRun
 *    Stop the run by writing a 0 to the go register.
 * @param controller - controller through which the write is done.
 */
void
S800TriggerRegisters::stopRun(CVMUSB& controller) {
    if (controller.vmeWrite32(goRegister(), amod, 0)) {
        throw std::runtime_error("Failed to write 0 to the go register");
    }
}
////
/**
 * describeJSON
 *    Return a string that describes the module.
 * @return std::string
 */
std::string 
S800TriggerRegisters::describeJSON() {
    std::stringstream desc;
    std::string result;

    auto device = m_registerSpec["Device"].asString();
    auto firmware = m_registerSpec["Project"].asString();
    auto built    = m_registerSpec["BuildDate"].asString();
    
    desc << "Device type: " << device << std::endl;
    desc << "Firmware:    " << firmware << std::endl;
    desc << "Build date:  " << built;

    result = desc.str();

    return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////
// Internal private utilities.

/**
 *  timestampLowBits:
 *     Return the address of the register that has the low 32 bits of the timestamp register.
 * @return std::uint32_t
 */
std::uint32_t
S800TriggerRegisters::timestampLowBits()  const {
    // Assume this is "R_TS_0" - can chage it if this is backwards:

    return getRegister("R_TS_0");
}
/**
 * timestamphighBits
 *   @return std::uint32_t - return the address of tyhe high timestamp bits:
 */
std::uint32_t 
S800TriggerRegisters::timestampHighBits() const {
    return getRegister("R_TS_1");
}

/**
 * triggerMask
 *    @return uint32_t - return the trigger mask register address
 */
std::uint32_t
S800TriggerRegisters::triggerMask() const {
    return getRegister("REG_FLAGS");  // Only 9 bits.
}

/**
 * swClearRegister
 *   @return uint32_t - the address of the software clear register.
 */
std::uint32_t
S800TriggerRegisters::swClearRegister() const {
    return getRegister("SW_CLEAR");
}
/**
 * busyResetRegister
 *    @return uint32_t - address of the busy reset register.
 * 
 */
std::uint32_t 
S800TriggerRegisters::busyResetRegister() const {
    return getRegister("BUSY_FORCE_RESET");
}
/** 
 * runNumberLowBits
 *    @return uint32_t - address of the run number low bits.
 */
std::uint32_t
S800TriggerRegisters::runNumberLowBits() const {
    // We assume this is "REG_RUNNUMBER" could be "REG_RUNNUMBER2", however.
    //
    return getRegister("REG_RUNNUMBER");
}
/**
 * runNumberHighBits
 *    @return uint32_t - address of the high bits of the run number.
 */
std::uint32_t
S800TriggerRegisters::runNumberHighBits() const {
    return getRegister("REG_RUNNUMBER2");
}

/**
 * externalClearEnableRegister
 *  @return uint32_t  Return the external clear enable register addresss
 */
std::uint32_t
S800TriggerRegisters::externalClearEnableRegister() const {
    return getRegister( "EXT_CLEAR_EN");
}
/**
 * go Register - the registesr that controls  the run.
 * If this register is 1 the run is active, if 0, the run is halted.
 * @return uint32_t - address of the go register.
 */
std::uint32_t S800TriggerRegisters::goRegister() const {
    return getMMCComponent("TS_CFG_GO");
}
/**
 * return the register that arms the trigger:
 * 
 * @return uint32_t - address of the re-arm trigger registser
 *     which must be pulsed to re-arm.
 */
std::uint32_t
S800TriggerRegisters::armTriggerRegister() const {
    return getRegister("R_CLEAR");
}
///
/**
 * getRegister
 *    Get the address of a register given its name.  E.g. get the
 * Address field of ["MMCComponents"]["Registers"][registername] as an integer and
 * add it to the base address
 * 
 * @param name - register name
 * @return std::uint32_t - address of the register given the value of m_base.
 */
std::uint32_t
S800TriggerRegisters::getRegister(const char* name) const {
    
    auto registers = m_registerSpec["Registers"];
    if (registers.isNull()) {
        throw Json::Exception("No Registers subkey");
    }
    
    // Iterate over registers finding the requested name:

    for (int index = 0; index < registers.size(); index++) {
        if (registers[index]["Name"].asString() == name) {
            return registers[index]["Address"].asUInt() + m_base;
        }
    }

    // no match:

    std::stringstream msg;
    msg << "No register named " << name;
    std::string smsg(msg.str());
    throw Json::Exception (smsg);
}
/**
 * getMMCComponent
 *    Get the address of an MMC component given its name.
 *    E.g. get the Address field of ["MMCComponents"][componentname] as an integer and
 *    add it to the base address.
 *
 * @param name - component name
 * @return std::uint32_t - address of the component given the value of m_base.
 */
std::uint32_t
S800TriggerRegisters::getMMCComponent(const char* name) const {

    auto components = m_registerSpec["MMCComponents"];
    if (components.isNull()) {
        throw Json::Exception("No MMCComponents subkey");
    }

    // Iterate over components finding the requested name:

    for (int index = 0; index < components.size(); index++) {
        if (components[index]["Name"].asString() == name) {
            return components[index]["Address"].asUInt() + m_base;
        }
    }

    // no match:

    std::stringstream msg;
    msg << "No MMC component named " << name;
    std::string smsg(msg.str());
    throw Json::Exception(smsg);
}