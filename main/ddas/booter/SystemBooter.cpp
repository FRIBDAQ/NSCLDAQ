/**
 * @file SystemBooter.cpp
 * @brief Implementation of the system booter class for DDAS.
 */

#include "SystemBooter.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "CXIAException.h"
#include "Configuration.h"
#include "config.h"
#include "config_pixie16api.h"

using namespace DAQ::DDAS;

/**
 * @brief Untility function to get a legacy boot flag from a BootType enum.
 * @param type The boot type.
 * @return Legacy boot flag.
 * @retval 0x7F Full boot.
 * @retval 0x70 Settings-only boot.
 */
unsigned int static inline getLegacyBootPattern(SystemBooter::BootType type) {
  if (type == SystemBooter::FullBoot) {
    return 0x7F;
  } else {
    return 0x70;
  }
}

/**
 * @brief Utilitiy function to get the PIXIE_BOOT_MODE from a BootType enum.
 * @param type The boot type.
 * @return PIXIE_BOOT_MODE corresponding to the boot type.
 * @retval PIXIE_BOOT_RESET_LOAD Full boot.
 * @retval PIXIE_BOOT_SETITNGS_LOAD Settings-only boot.
 * @note (ASC 3/25/25): PIXIE_BOOT_MODE enum only used by `PixieBootCrate()`
 * in API 4.4.0, other booting functions use the legacy flags.
 */
PIXIE_BOOT_MODE
static inline getBootMode(SystemBooter::BootType type) {
  if (type == SystemBooter::FullBoot) {
    return PIXIE_BOOT_RESET_LOAD;
  } else {
    return PIXIE_BOOT_SETTINGS_LOAD;
  }
}

/**
 * @brief Utility function wrapping the `PixieGetModuleInfo()` call which
 * throws CXIAException on failure.
 * @param mod The module index.
 * @throws CXIAException If the `PixieGetModuleInfo()` call fails.
 * @return XIA module_config struct containing module info.
 */
module_config static inline getModuleConfig(unsigned short mod) {
  module_config cfg;
  int rv = PixieGetModuleInfo(mod, &cfg);
  if (rv < 0) {
    std::stringstream msg;
    msg << "SystemBooter::getModuleConfig() failed to read module "
        << "configuration for module " << mod;
    throw CXIAException(msg.str(), "PixieGetModuleInfo()", rv);
  }

  return cfg;
}

/**
 * @details
 * Default settings: verbose output enabled, boot in online mode.
 */
DAQ::DDAS::SystemBooter::SystemBooter() : m_verbose(true), m_offlineMode(0) {}

/**
 * @details
 * Perform parallel boot of DDAS crate. Offline mode supported as of XIA
 * API 4.4.0.
 */
void DAQ::DDAS::SystemBooter::boot(Configuration &config, BootType type) {
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

/**
 * @details
 * Two possibilities: 1) if m_offlieMode == 0, initialze system with hardware
 * and slot map defined in the cfgPixie16.txt file. 2) If m_offlineMode == 1,
 * initialize the crate simulation (no hardware required).
 */
void DAQ::DDAS::SystemBooter::initSystem(Configuration &config) {
  std::cout << "---------------------------\n";
  std::cout << "Initializing PXI access... \n";
  std::cout.flush();

  int rv;
  if (m_offlineMode) {
    int nModules = 4; // 4th module is Pixie-32.
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

/**
 * @details
 * The system firmware path can be overridden by setting the environment
 * variable FIRMWARE_PATH to point to a new locaiton. Note that this system
 * path is passed to `Pixie16LoadModuleFirmware()` which expects subfolders
 * extracted from firmware packages released by XIA after 05/27/24.
 * See https://docs.pixie16.xia.com/latest/pixie-sdk/fw.html and
 * `Pixie16LoadModuleFirmware()` API documentation for more details.
 *
 * This function is also responsible for reading and loading the per-module
 * firmware and settings files.
 *
 * @note It is an error to specify two global firmware overrides
 * simultaneously. One can override the default system path using
 * FIRMWARE_PATH or override the default system path and load FW from a
 * file using the envvar FIRMWARE_FILE, but not both.
 */
void DAQ::DDAS::SystemBooter::parallelBoot(Configuration &config,
                                           BootType type) {
  std::cout << "Attempting parallel boot for Pixie crate..." << std::endl;

  // Check environ for alternative search path for FW installs:
  const char *fwPath = FIRMWARE_PATH;
  char *alternateFirmwarePath = getenv("FIRMWARE_PATH");
  if (alternateFirmwarePath) {
    fwPath = alternateFirmwarePath;
  }

  int rv;
  if (config.getDefaultFirmwareMap().empty()) {
    rv = Pixie16LoadModuleFirmware(fwPath);
    if (rv < 0) {
      throw CXIAException("SystemBooter::parallelBoot() failed",
                          "Pixie16LoadModuleFirmware()", rv);
    } else {
      std::cout << "Found module firmware in " << fwPath << std::endl;
    }
  } else {
    std::cout << "Setting firmware from map file" << std::endl;
    if (alternateFirmwarePath) {
      throw std::runtime_error("ERROR: Multiple default global "
                               "firmware overrides!");
    }
    for (int i = 0; i < config.getNumberOfModules(); i++) {
      setModuleFirmware(config, i);
    }
  }

  // If there are any per-module firmware sets:
  setPerModuleFirmware(config);

  // Set the channel map:
  setChannelMap(config);

  // Now we can boot:
  rv = PixieBootCrate(config.getSettingsFilePath().c_str(), getBootMode(type));
  if (rv < 0) {
    throw CXIAException("SystemBooter::boot() failed", "PixieBootCrate()", rv);
  }

  // Once the system is booted, if there are per-module settings we
  // load them onto the boards with a settings-only boot:
  setPerModuleDSP(config);
}

/**
 * @note Offline mode and crate simulation is a development and debugging
 * feature not intended for users.
 */
void DAQ::DDAS::SystemBooter::offlineBoot(Configuration &config,
                                          BootType type) {
  char parFile[PIXIE16_API_MOD_CONFIG_MAX_STRING];
  strcpy(parFile, config.getSettingsFilePath().c_str());
  int rv = PixieBootCrate(parFile, getBootMode(type));
  if (rv < 0) {
    throw CXIAException("SystemBooter::offlineBoot() failed",
                        "PixieBootCrate()", rv);
  }
  setChannelMap(config);
}

/**
 * @details
 * No surprises here:
 * - Get the module firmware maps,
 * - If the map is empty, return,
 * - Otherwise, iterate over the map and set module firmware.
 */
void DAQ::DDAS::SystemBooter::setPerModuleFirmware(Configuration &config) {
  // Per-module map is map<mod, map<hwTag, FWConfig> >
  auto fwMaps = config.getModuleFirmwareMaps();
  if (fwMaps.empty()) {
    return; // No map, nothing to do.
  } else {
    std::cout << "Detected per-module firmware..." << std::endl;
  }

  // Attempt to set firmware for all modules with a FW map:
  for (const auto &map : fwMaps) {
    int mod = map.first;
    std::cout << "Found FW map for module " << mod << std::endl;
    setModuleFirmware(config, mod);
  }
}

/**
 * @details
 * - Read the module configuration,
 * - Based on the revision, MSPS, and bits, compute a hardware type,
 * - Get the firmware configuration for that hardware type,
 * - Extract the firmware paths and set the device firmware.
 */
void DAQ::DDAS::SystemBooter::setModuleFirmware(Configuration &config,
                                                unsigned int mod) {
  // Get the module configuration:
  auto cfg = getModuleConfig(mod);
  unsigned short rev = cfg.revision;
  unsigned short msps = cfg.adc_sampling_frequency;
  unsigned short bits = cfg.adc_bit_resolution;

  // Module type must be known:
  auto type = HardwareRegistry::computeHardwareType(rev, msps, bits);
  if (type == HardwareRegistry::Unknown) {
    std::stringstream msg;
    msg << "SystemBooter::setModuleFirmware(): Unknown module type " << msps
        << "m-" << bits << "b-rev" << rev;
    throw std::runtime_error(msg.str());
  }

  // If the FW map is loaded and the module type is recognized, the
  // mapped FW is (God help us) valid. Fish out the paths from the FW
  // struct and set for this module:
  auto fwConfig = config.getModuleFirmwareConfiguration(type, mod);
  setDeviceFirmware(fwConfig.s_ComFPGAConfigFile, mod, cfg.slot, "sys");
  setDeviceFirmware(fwConfig.s_SPFPGAConfigFile, mod, cfg.slot, "fippi");
  setDeviceFirmware(fwConfig.s_DSPCodeFile, mod, cfg.slot, "dsp");
  setDeviceFirmware(fwConfig.s_DSPVarFile, mod, cfg.slot, "var");
}

/**
 * @details
 * Wrapper for setting device firmware with `Pixie16SetModuleFirmware()` which
 * throws CXIAExceptions with the XIA error code and error message.
 */
void DAQ::DDAS::SystemBooter::setDeviceFirmware(std::string fwFile,
                                                unsigned int mod,
                                                unsigned int slot,
                                                std::string device) {
  int rv = Pixie16SetModuleFirmware(fwFile.c_str(), slot, device.c_str());
  if (rv < 0) {
    std::stringstream msg;
    msg << "SystemBooter::setPerModuleFirmware() failed to set module " << mod
        << " (slot " << slot << ") '" << device << "' FW from " << fwFile;
    throw CXIAException(msg.str(), "Pixie16SetModuleFirmware()", rv);
  }
}

/**
 * @details
 * Wrapper for `Pixie16BootModuleFirmware()` which throws CXIAExceptions
 * with the XIA error code and error message. DSP settings are loaded onto
 * the modules via a settings-only boot.
 */
void DAQ::DDAS::SystemBooter::setPerModuleDSP(Configuration &config) {
  std::map<int, std::string> dspMap = config.getModuleSetFileMap();
  if (dspMap.empty()) {
    return; // No map, nothing to do.
  } else {
    std::cout << "Found per-module DSP settings..." << std::endl;
  }

  for (const auto &entry : dspMap) {
    int mod = entry.first;
    std::cout << "Found DSP path for module " << mod << std::endl;
    std::string dspPath = entry.second;
    unsigned int pat = getLegacyBootPattern(SystemBooter::SettingsOnly);
    int rv = Pixie16BootModuleFirmware(dspPath.c_str(), mod, pat);
    if (rv < 0) {
      std::stringstream msg;
      msg << "SystemBooter::setPerModuleDSP() failed to set module " << mod
          << " DSP settings from " << dspPath;
      throw CXIAException(msg.str(), "Pixie16BootModuleFirmware()", rv);
    } else {
      std::cout << "Module " << mod << " DSP settings loaded from " << dspPath
                << std::endl;
    }
  }
}

void DAQ::DDAS::SystemBooter::setChannelMap(Configuration &config) {
  std::vector<unsigned short> channelMap;
  for (auto i = 0; i < config.getNumberOfModules(); i++) {
    auto cfg = getModuleConfig(i);
    channelMap.push_back(cfg.number_of_channels);
  }
  config.setChannelMap(channelMap);
}

void DAQ::DDAS::SystemBooter::populateHardwareMap(Configuration &config) {
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
        cfg.revision, cfg.adc_sampling_frequency, cfg.adc_bit_resolution);
    hwMap[i] = type;
  }
  // Store the hardware map in the configuration so other components of the
  // program can understand more about the hardware being used.
  config.setHardwareMap(hwMap);
}

/**
 * @detials
 * As of API 4.4.0 on 3/21/25 last two device data are reserved so we
 * only print the first 4 corresponding to the 4 firmware device files.
 */
void DAQ::DDAS::SystemBooter::logModuleInfo(Configuration &config) {
  std::cout << std::endl;
  for (int i = 0; i < config.getNumberOfModules(); i++) {
    auto cfg = getModuleConfig(i);
    std::cout << "----- Module " << cfg.number << " -----" << std::endl;
    std::cout << "ADC resolution : " << cfg.adc_bit_resolution << std::endl;
    std::cout << "ADC MSPS       : " << cfg.adc_sampling_frequency << std::endl;
    std::cout << "Number         : " << cfg.number << std::endl;
    std::cout << "Channels       : " << cfg.number_of_channels << std::endl;
    std::cout << "Revision       : " << cfg.revision << std::endl;
    std::cout << "Serial No.     : " << cfg.serial_number << std::endl;
    std::cout << "Slot           : " << cfg.slot << std::endl;
    std::cout << "FW revision    : " << cfg.fw_revision << std::endl;
    std::cout << "FW tag         : " << cfg.fw_tag << std::endl;
    std::cout << "FW type        : " << cfg.fw_type << std::endl;
    for (int j = 0; j < PIXIE16_API_MOD_CONFIG_MAX_DEVICES - 2; j++) {
      std::cout << cfg.fw_device[j] << ":\t" << cfg.fw_device_file[j]
                << std::endl;
    }
    std::cout << "DSP settings: " << config.getSettingsFilePath(i) << std::endl;
    std::cout << std::endl;
  }
}
