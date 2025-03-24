/**
 * @file SystemBooter.cpp
 * @brief Implementation of the system booter class for DDAS.
 */

#include "SystemBooter.h"

#include <unistd.h>
#include <string.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <config.h>
#include <config_pixie16api.h>
#include <Configuration.h>
#include <CXIAException.h>

using namespace DAQ::DDAS;

PIXIE_BOOT_MODE
static inline getBootMode(SystemBooter::BootType type) {
    if (type == SystemBooter::FullBoot) {
	return PIXIE_BOOT_RESET_LOAD;
    } else {
	return PIXIE_BOOT_SETTINGS_LOAD;
    }
}

module_config
static inline getModuleConfig(unsigned short modNum)
{
    module_config cfg;
    int rv = PixieGetModuleInfo(modNum, &cfg);
    if (rv < 0) {
	std::stringstream msg;
	msg << "SystemBooter::getModuleConfig() failed to read module "
	    << "configuration for module " << modNum;
	throw CXIAException(msg.str(), "PixieGetModuleInfo()", rv);
    }

    return cfg;
}

/**
 * @details
 * Default settings: verbose output enabled, boot in online mode.
 */
DAQ::DDAS::SystemBooter::SystemBooter() :
    m_verbose(true), m_offlineMode(0)
{}

void
DAQ::DDAS::SystemBooter::boot(Configuration& config, BootType type)
{   
    initSystem(config);
    usleep(1000); // Wait to ensure system is initialized.
    populateHardwareMap(config);
    if (m_offlineMode) {
	offlineBoot(config, type);
    } else {
	parallelBoot(config, type);
    }
    logModuleInfo(config);
    std::cout << "All modules ok" << std::endl;
}

///
// Private functions
//

void
DAQ::DDAS::SystemBooter::initSystem(Configuration& config)
{
    std::cout << "---------------------------\n";
    std::cout << "Initializing PXI access... \n";
    std::cout.flush();
    
    int rv;
    if (m_offlineMode) {
	int nModules = 3; // 4th module is Pixie-32.
	config.setNumberOfModules(nModules);
	rv = Pixie16InitSystem(nModules, nullptr, 1);
    } else {
	rv = Pixie16InitSystem(config.getNumberOfModules(),
			       config.getSlotMap().data(), 0);
    }    

    if (rv < 0) {
	throw CXIAException("SystemBooter::initSystem() failed",
			    "Pixie16InitSystem()", rv);
    } else {
	std::cout << "System initialized successfully." << std::endl;
    }
}

void
DAQ::DDAS::SystemBooter::parallelBoot(Configuration& config, BootType type)
{
    std::cout << "Attempting parallel boot for Pixie crate..." << std::endl;
    int rv = Pixie16LoadModuleFirmware(FIRMWARE_PATH);
    if (rv < 0) {
	throw CXIAException("SystemBooter::parallelBoot() failed",
			    "Pixie16LoadModuleFirmware()", rv);
    } else {
	std::cout << "Found module firmware in " << FIRMWARE_PATH
		  << std::endl;
    }

    // If there are any per-module firmware sets:
    setPerModuleFirmware(config);

    rv = PixieBootCrate(config.getSettingsFilePath().c_str(),
			getBootMode(type));
    if (rv < 0) {
	throw CXIAException("SystemBooter::boot() failed",
			    "PixieBootCrate()", rv);
    }

    // Once the system is booted, if there are per-module settings we
    // load them onto the boards with a settings-only boot:
    setPerModuleDSP(config);
}

void
DAQ::DDAS::SystemBooter::offlineBoot(Configuration& config, BootType type)
{
    char parFile[PIXIE16_API_MOD_CONFIG_MAX_STRING];
    strcpy(parFile, config.getSettingsFilePath().c_str());
    int rv = PixieBootCrate(parFile, getBootMode(type));
    if (rv < 0) {
	throw CXIAException("SystemBooter::offlineBoot() failed",
			    "PixieBootCrate()", rv);
    }
}

void DAQ::DDAS::SystemBooter::setPerModuleFirmware(Configuration& config)
{
    // Per-module map is map<modNum, map<hwTag, FWConfig> >
    auto fwMaps = config.getModuleFirmwareMaps();
    if (fwMaps.empty()) {
	return; // No map, nothing to do.
    } else {
	std::cout << "Detected per-module firmware..." << std::endl;
    }

    // Attempt to set firmware for all modules with a FW map:
    for (const auto& map : fwMaps) {
	int modNum = map.first;
	std::cout << "Found FW map for module " << modNum << std::endl;
	
	// Get the module configuration:
	auto cfg = getModuleConfig(modNum);
	unsigned short rev = cfg.revision;
	unsigned short msps = cfg.adc_sampling_frequency;
	unsigned short bits = cfg.adc_bit_resolution;
	
	// Module type must be known:
	auto type = HardwareRegistry::computeHardwareType(rev, msps, bits);	
	if (type == HardwareRegistry::Unknown) {
	    std::stringstream msg;
	    msg << "SystemBooter::parallelBoot(): Unknown module type "
		<< msps << "m-" << bits	<< "b-rev" << rev;
	    throw std::runtime_error(msg.str());
	}

	// If the FW map is loaded and the module type is recognized, the
	// mapped FW is (God help us) valid. Fish out the paths from the FW
	// struct and set for this module:
	auto fwConfig = config.getModuleFirmwareConfiguration(type, modNum);
	setFirmware(fwConfig.s_ComFPGAConfigFile, cfg.slot, "sys");
	setFirmware(fwConfig.s_SPFPGAConfigFile, cfg.slot, "fippi");
	setFirmware(fwConfig.s_DSPCodeFile, cfg.slot, "dsp");
	setFirmware(fwConfig.s_DSPVarFile, cfg.slot, "var");
    }
}

/**
 * @details
 * Wrapper for `Pixie16SetModuleFirmware()` which throws CXIAExceptions
 * with the XIA error code and error message.
 */
void
DAQ::DDAS::SystemBooter::setFirmware(
    std::string fwFile, unsigned int slot, std::string device
    )
{
    int rv = Pixie16SetModuleFirmware(fwFile.c_str(), slot, device.c_str());
    if (rv < 0) {
	std::stringstream msg;
	msg << "SystemBooter::setPerModuleFirmware() failed to set slot "
	    << slot << " '" <<  device << "' FW from " << fwFile;
	throw CXIAException(msg.str(), "Pixie16SetModuleFirmware()", rv);
    }
}

/**
 * @details
 * Wrapper for `Pixie16BootModuleFirmware()` which throws CXIAExceptions
 * with the XIA error code and error message. DSP settings are loaded onto
 * the modules via a settings-only boot.
 */
void
DAQ::DDAS::SystemBooter::setPerModuleDSP(Configuration& config)
{    
    std::map<int, std::string> dspMap = config.getModuleSetFileMap();
    if (dspMap.empty()) {
	return; // No map, nothing to do.
    } else {
	std::cout << "Found per-module DSP settings..." << std::endl; 
    }
    
    for (const auto& entry : dspMap) {
	int modNum = entry.first;
	std::string dspPath = entry.second;
	PIXIE_BOOT_MODE mode = PIXIE_BOOT_SETTINGS_LOAD;
	int rv = Pixie16BootModuleFirmware(dspPath.c_str(), modNum, mode);
	if (rv < 0) {
	    std::stringstream msg;
	    msg << "SystemBooter::setPerModuleDSP() failed to set mod "
		<< modNum << " DSP settings from " << dspPath;
	    throw CXIAException(msg.str(), "Pixie16BootModuleFirmware()", rv);
	}
    }
}

void
DAQ::DDAS::SystemBooter::populateHardwareMap(Configuration& config)
{
    int nModules = config.getNumberOfModules();
    std::vector<int> hwMap(nModules);
    for (unsigned short i = 0; i < nModules; i++) {
	auto cfg = getModuleConfig(i);
	if (m_verbose) {
	    std::cout << "Found Pixie module #" << cfg.number;
	    std::cout << ", Rev = " << cfg.revision;
	    std::cout << ", S/N = " << cfg.serial_number;
	    std::cout << ", Bits = " << cfg.adc_bit_resolution;
	    std::cout << ", MSPS = " << cfg.adc_sampling_frequency;
	    std::cout << std::endl;
	}
	auto type = HardwareRegistry::computeHardwareType(
	    cfg.revision, cfg.adc_sampling_frequency, cfg.adc_bit_resolution
	    );
	hwMap[i] = type;	    
    }
    // Store the hardware map in the configuration so other components of the
    // program can understand more about the hardware being used.
    config.setHardwareMap(hwMap);
}

void
DAQ::DDAS::SystemBooter::logModuleInfo(Configuration& config)
{
    std::cout << std::endl;
    for (int i = 0; i < config.getNumberOfModules(); i++) {
	auto cfg = getModuleConfig(i);
	std::cout << "----- Module " << cfg.number << " -----" << std::endl;
	std::cout << "ADC resolution : " << cfg.adc_bit_resolution
		  << std::endl;
	std::cout << "ADC MSPS       : " << cfg.adc_sampling_frequency
		  << std::endl;
	std::cout << "Number         : " << cfg.number << std::endl;
	std::cout << "Channels       : " << cfg.number_of_channels
		  << std::endl;
	std::cout << "Revision       : " << cfg.revision << std::endl;
	std::cout << "Serial No.     : " << cfg.serial_number << std::endl;
	std::cout << "Slot           : " << cfg.slot << std::endl;
	std::cout << "FW revision    : " << cfg.fw_revision << std::endl;
	std::cout << "FW tag         : " << cfg.fw_tag << std::endl;
	std::cout << "FW type        : " << cfg.fw_type << std::endl;
	// As of API 4.4.0 on 3/21/25 last two device data are placeholders:
	for (int i = 0; i < PIXIE16_API_MOD_CONFIG_MAX_DEVICES - 2; i++) {
	    std::cout << cfg.fw_device[i] << ":\t" << cfg.fw_device_file[i]
		      << std::endl;
	}
	std::cout << std::endl;
    }
}
