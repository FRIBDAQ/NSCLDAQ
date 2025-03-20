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
    if (m_offlineMode) {
	initSystemOffline(config, type);
	usleep(1000); // Wait to ensure initialized.
	offlineBoot(config, type);
    } else {
	initSystem(config, type);	
	usleep(1000); // Wait to ensure initialized.
	if (!config.getModuleSetFileMap().empty()) {
	    // serial boot
	} else {
	    parallelBoot(config, type);
	}
	std::cout << "All modules ok" << std::endl;
    }

    Pixie16ExitSystem(config.getNumberOfModules());
    std::cout << "Exiting...\n";
}

///
// Private functions
//

void
DAQ::DDAS::SystemBooter::initSystem(Configuration& config, BootType type)
{
    std::cout << "---------------------------\n";
    std::cout << "Initializing PXI access... \n";
    std::cout.flush();

    int rv = Pixie16InitSystem(config.getNumberOfModules(),
			       config.getSlotMap().data(), 0);
    if (rv < 0) {
	throw CXIAException("SystemBooter::initSystem() failed",
			    "Pixie16InitSystem()", rv);
    } else {
	std::cout << "System initialized successfully." << std::endl;
    }
    populateHardwareMap(config);
}

void
DAQ::DDAS::SystemBooter::initSystemOffline(Configuration& config,
						 BootType type)
{
    std::cout << "---------------------------\n";
    std::cout << "Initializing PXI access... \n";
    std::cout.flush();
    int nModules = 3; // 4th simulated module is 32 channels, exclude for now.
    int rv = Pixie16InitSystem(nModules, nullptr, 1);
    if (rv < 0) {
	throw CXIAException("SystemBooter::initSystemOffline() failed",
			    "Pixie16InitSystem()", rv);
    } else {
	std::cout << "Crate simulation initialized successfully."
		  << std::endl;
    }
    config.setNumberOfModules(nModules);
    populateHardwareMap(config);
}

void
DAQ::DDAS::SystemBooter::parallelBoot(Configuration& config, BootType type)
{
    std::cout << "Attempting parallel boot for Pixie crate..." << std::endl;
    std::cout << "Looking for module firmware..." << std::endl;
    int rv = Pixie16LoadModuleFirmware(FIRMWARE_PATH);
    if (rv < 0) {
	throw CXIAException("SystemBooter::parallelBoot() failed",
			    "Pixie16LoadModuleFirmware()", rv);
    } else {
	std::cout << "Found module firmware in " << FIRMWARE_PATH
		  << std::endl;
    }

    std::cout << "Checking for per-module firmware..." << std::endl;
    auto perModMaps = config.getModuleFirmwareMaps();
    for (const auto modMap : perModMaps) {
	////
	// if any modules have a fw map, get the paths and override default
	//
	int modNum = modMap.first;
	std::cout << "Found FW map for module " << modNum << std::endl;
    }
    
    rv = PixieBootCrate(config.getSettingsFilePath().c_str(),
			getBootMode(type));
    if (rv < 0) {
	throw CXIAException("SystemBooter::boot() failed",
			    "PixieBootCrate()", rv);
    }

    std::cout << "Loading per-module settings..." << std::endl;
    ////
    // check setfile map and settings-only boot module fw for each entry
    //

    logModuleInfo(config);
}

void
DAQ::DDAS::SystemBooter::serialBoot(Configuration& config, BootType type)
{
    // implement
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
    logModuleInfo(config);
    std::cout << "Crate simulation: all modules ok" << std::endl;
}

**
 * @details
 * To retrieve information about all of the modules in the system, 
 * Pixie16ReadModuleInfo is called for each module index. The resulting 
 * revision number, ADC bits, and ADC frequency is printed (if verbose output
 * enabled) and the hardware mapping is stored in the configuration that was
 * passed in.
 */
void DAQ::DDAS::SystemBooter::populateHardwareMap(Configuration &config)
{
    int nModules = config.getNumberOfModules();
    std::vector<int> hdwrMapping(NumModules);
    
    for (unsigned short i = 0; i < nModules; i++) {
	module_config cfg;
	int rv = PixieGetModuleInfo(i, &cfg);
	if (rv < 0) {
	    std::string msg = "Failed to read module info " + i;
	    throw CXIAException(msg, "PixieGetModuleInfo()", rv);
	}
	if (m_verbose) {
	    logModuleInfo(i, cfg.revision, cfg.serial_number,
			  cfg.adc_bit_resolution, cfg.adc_sampling_frequency);
	}
	auto type = HardwareRegistry::computeHardwareType(
	    cfg.revision, cfg.adc_sampling_frequency, cfg.adc_bit_resolution
	    );
	hdwrMapping[i] = type;
    }
    
    // Store the hardware map in the configuration so other components of the
    // program can understand more about the hardware being used.
    config.setHardwareMap(hdwrMapping);
}

void
DAQ::DDAS::SystemBooter::logModuleInfo(Configuration& config)
{
    int nModules = config.getNumberOfModules();
    for (int i = 0; i < nModules; i++) {
	module_config cfg;
	int rv = PixieGetModuleInfo(i, &cfg);
	if (rv < 0) {
	    std::stringstream msg;
	    msg << "SystemBooter::logModuleInfo() failed to read module "
		<< "configuration for module " << i;
	    throw CXIAException(msg.str(), "PixieGetModuleInfo", rv);
	}
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
	for (int i = 0; i < PIXIE16_API_MOD_CONFIG_MAX_DEVICES-2; i++) {
	    std::cout << "FW device " << i << "    : " << cfg.fw_device[i]
		      << std::endl;
	    std::cout << "FW file   " << i << "    : "
		      << cfg.fw_device_file[i] << std::endl;
	}
	std::cout << std::endl;
    }
}
