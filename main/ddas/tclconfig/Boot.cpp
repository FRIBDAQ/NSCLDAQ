/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  Boot.cpp
 *  @brief: Implement the boot command.
 */
#include "Boot.h"
#include <Configuration.h>
#include <HardwareRegistry.h>

#include <sstream>

#include <config.h>
#include <config_pixie16api.h>

static const char* infoErrors[3] = {
    "Success",
    "Invalid module number",
    "Failed to read I2C Serial EEPROM"
};

static const char* bootErrors[25] = {
    "Success",
    "Invalid Pixie16 module number",
    "Size of COMFPGA config file was wrong",
    "Failed to boot COMM FGPA",
    "Failed to allocate memory to store data for COMFPGA config file",
    "Failed to open COMMFPGA config file",
    "Unused error code 6",
    "Unused error code 7",
    "Unused error code 8",
    "Unused error code 9",
    "Size of SFGPA Config file is invalid",
    "Failed to boot signal processing FPGA",
    "Failed to allocate memory for SFGPA config file",
    "Failed to open SFGPA config file",
    "Failed to boot DSP",
    "Failed to allocate memory to store DPS executable code",
    "Failed to open DSP code file",
    "Size of DSP Parameter file is invalid",
    "Failed to open DSP Parameter file",
    "Can't initialize DSP Variable indices",
    "Can't copy DSP Variable indices",
    "Failed to program FIPPI in",
    "Failed to set the offset DACs",
    "Failed to start RESET_ADC",
    "RESET_ADC run timed out"
};

/**
 * constructor
 *  @param pInterp - pointer to the interpreter our command
 *  @param config  - references the module configuration.
 */
CBoot::CBoot(Tcl_Interp* pInterp, DAQ::DDAS::Configuration& config) :
    CTclCommand(pInterp, "pixie16::boot"),
    m_config(config)
{}

/**
 * destructor
 *    null for now.
 */
CBoot::~CBoot() {}

/**
 * operator()
 *   - Validate the parameter, and parameter type.
 *   - Figure out the module hardware type.
 *   - Fetch the names of the various files we need to boot.
 *   - Boot the module.
 * @param objv - the command line words.
 * @return int - TCL_OK for success, TCL_ERROR for failure.
 * @note -there's only a result if errors are throw and then it's
 *        a textual error message.
 */
int CBoot::operator()(std::vector<Tcl_Obj*>& objv)
{
    int index, slot;
    const char** messages;
    const char* pDoing;
    Tcl_Interp*  pInterp = getInterpreter();
    try {
        requireExactly(objv, 2);
        index = getInteger(objv[1]);
        
        auto slots = m_config.getSlotMap();
        if ((index < 0) || (index >= slots.size())) {
            throw std::string("Module index is invalid");
        }
        slot = slots[index];
        
        messages = infoErrors;
        pDoing   = "getting module hardware type";
        int hwtype = getHardwareType(index);  // throws on error.
        
        messages = bootErrors;
        pDoing   = "booting module";
        bootModule(index, hwtype);
        
    }
    catch (std::string msg) {
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    catch (int status) {
        std::string msg = apiMsg(index, slot, status, pDoing, messages);
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    return TCL_OK;
}
//////////////////////////////////////////////////////////
// Private utilities.

/**
 * apiMsg
 *    Returns a string appropriate to an API error status.
 * @param index -module index.
 * @param slot  - corresponding module slot.
 * @param status - API status value negated.
 * @param doing  -  String describing what failed.
 * @param msgs   - Table of status messages.
 * @return std::string - the error message.
 */
std::string
CBoot::apiMsg(int index, int slot, int status, const char* doing, const char* msgs[])
{
    std::stringstream s;
    
    s << "Error " << doing << " module number " << index
      << " (slot: " << slot << "): " << msgs[status]; 
    
    std::string result = s.str();
    return result;
}
/**
 * getHardwareType
 *   Get the computed hardware type.  This is an NSCL specific
 *   value that combines the properties of the module into a single
 *   integer that can be used to lookup stuff like the firmware files
 *   appropriate to the module.
 *   -  Use ReadModuleInfo to get the module information.
 *   -  Ask the hardware registry to compute the hardware type.
 * @param index - module number.
 * @return int  - Hardware type of the module.
 * @throw int   - negative status code of failing calls to ReadModuleInfo.
 * @throw std::string - if we can't figure out a valid hardware type.
 */
int
CBoot::getHardwareType(int index)
{
    unsigned short rev, bits, mhz;
    unsigned int   serial;
    
    int status = Pixie16ReadModuleInfo(index, &rev, &serial, &bits, &mhz);
    if (status) throw -status;
    
    // Get the type:
    
    int type = DAQ::DDAS::HardwareRegistry::computeHardwareType(
        rev, mhz, bits
    );
    if (type == DAQ::DDAS::HardwareRegistry::Unknown) {
        throw "Module hardware type is unknown";
    }
    
    return type;
}
/**
 * bootModule
 *   Given we know the hardware type of a module, fetch the firmware
 *   files needed and try to boot the module.
 *
 * @param index - Index of module to boot.
 * @param type  - hardware type of module.
 * @throw int   - The negative of the status from BootModule.
 */
void
CBoot::bootModule(int index, int type)
{
    auto firmware = m_config.getFirmwareConfiguration(type);
    std::string setFile = m_config.getSettingsFilePath();
    
    int status = Pixie16BootModule(
        firmware.s_ComFPGAConfigFile.c_str(),
        firmware.s_SPFPGAConfigFile.c_str(),
        "",                       // Trigger fpga file.
        firmware.s_DSPCodeFile.c_str(),
        setFile.c_str(),
        firmware.s_DSPVarFile.c_str(),
        index,
        0x7d
    );
    if (status) throw -status;
}

