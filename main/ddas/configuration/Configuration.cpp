/**
 * @file Configuration.cpp
 * @brief Implementation of the system storage configuration.
 */

#include "Configuration.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "ConfigurationParser.h"
#include "FirmwareVersionFileParser.h"
#include "ModEvtFileParser.h"

/**
 * @todo (ASC 3/21/25): Clean up this class and remove everything which we
 * do not need (e.g. default maps) and remove any hardware types which are
 * not real or we do not support (e.g. 500m-16b rev F??)
 */

/*!
 * @details
 * This resizes the vectors storing the slot map, module event lengths, and
 * hardware map to be consistent. The caller *must* call `setNumberOfModules()`
 * prior to setting the slot map, channel map, event lengths, or hardware map.
 * Failure to do so will at best result in an exception and/or the caller
 * application fatally terminating.
 */
void DAQ::DDAS::Configuration::setNumberOfModules(size_t size) {
  m_numModules = size;
  m_slotMap.resize(size);
  m_channelMap.resize(size);
  m_modEvtLengths.resize(size);
  m_hardwareMap.resize(size);
}

/**
 * @details
 * It is important for the caller to first call `setNumberOfModules()`
 * before calling this to avoid an exception being thrown as this method ensures
 * that the length of the slot map is the same as the expected number of modules
 * in the system. If the user has not set the number of modules prior to calling
 * this function, this cannot be guaranteed and the method will almost always
 * throw.
 *
 * @code
 *  Configuration config;
 *  config.setNumberOfModules(2);
 *  config.setSlotMap({2, 3});
 * @endcode
 */
void DAQ::DDAS::Configuration::setSlotMap(
    const std::vector<unsigned short> &map) {
  if (map.size() != m_numModules) {
    throw std::runtime_error(
        "Configuration::setSlotMap(): Inconsistent data "
        "for module evt lengths and slot mapping. Set number of modules "
        "first using Configuration::setNumberOfModules().");
  }

  m_slotMap = map;
}

/**
 * @details
 * It is important for the caller to first call `setNumberOfModules()`
 * before calling this to avoid an exception being thrown as this method ensures
 * that the length of the channel map is the same as the expected number of
 * modules in the system. If the user has not set the number of modules prior to
 * calling this function, this cannot be guaranteed and the method will almost
 * always throw.
 *
 * @code
 *  Configuration config;
 *  config.setNumberOfModules(2);
 *  config.setChannelMap({16, 16});
 * @endcode
 */
void DAQ::DDAS::Configuration::setChannelMap(
    const std::vector<unsigned short> &map) {
  if (map.size() != m_numModules) {
    throw std::runtime_error(
        "Configuration::setChannelMap(): Inconsistent "
        "data for module evt lengths and slot mapping. Set number of "
        "modules first using Configuration::setNumberOfModules().");
  }

  m_channelMap = map;
}

unsigned short DAQ::DDAS::Configuration::getModuleChannelCount(size_t mod) {
  if (mod >= m_numModules) {
    throw std::out_of_range(
        "Configuration::getModuleChannelCount(): Module index " +
        std::to_string(mod) + " is out of range for system with " +
        std::to_string(m_numModules) + " modules!");
  }

  return m_channelMap[mod];
}

/**
 * @details
 * The filename path should be checked for readability by the caller.
 */
void DAQ::DDAS::Configuration::setModuleSettingsFilePath(
    int modNum, const std::string &path) {
  m_moduleSetFileMap[modNum] = path;
}

/**
 * @details
 * If there's a per-module set file it's returned otherwise return the
 * default settings file.
 */
std::string DAQ::DDAS::Configuration::getModuleSettingsFilePath(int modnum) {
  if (m_moduleSetFileMap.count(modnum) > 0) {
    return m_moduleSetFileMap[modnum];
  } else {
    return m_settingsFilePath;
  }
}

/**
 * @details Searches the firmware map using std::find.
 */
DAQ::DDAS::FirmwareConfiguration &
DAQ::DDAS::Configuration::getFirmwareConfiguration(int hdwrType) {
  auto pSpec = m_fwMap.find(hdwrType);
  if (pSpec == m_fwMap.end()) {
    throw std::runtime_error(
        "Unable to locate firmware configuration for firmware specifier");
  }

  return pSpec->second;
}

/**
 * @details
 * An existing map is ovewritten.
 */
void DAQ::DDAS::Configuration::setModuleFirmwareMap(
    int module, const FirmwareMap &mapping) {
  m_moduleFirmwareMaps[module] = mapping;
}

/**
 * @details
 * It is an error to have a firmware configuration map file but not to have
 * a configuration for the hardware type. If a per-module firmware map does
 * not exist, return the default configuration.
 */
DAQ::DDAS::FirmwareConfiguration &
DAQ::DDAS::Configuration::getModuleFirmwareConfiguration(int hwType,
                                                         int modnum) {
  if (m_moduleFirmwareMaps.count(modnum) > 0) {
    FirmwareMap &mapping = m_moduleFirmwareMaps[modnum];
    if (mapping.count(hwType) > 0) {
      return mapping[hwType];
    } else {
      throw std::runtime_error("Unable to locate firmware configuration "
                               "for firmware specifier in per-module map");
    }
  }

  return getFirmwareConfiguration(hwType);
}

/**
 * @details
 * It is important for the caller to first call `setNumberOfModules()`
 * before calling this to avoid an exception being thrown as this method ensures
 * that the length of the module event length map is the same as the expected
 * number of modules in the system. If the user has not set the number of
 * modules prior to calling this function, this cannot be guaranteed and the
 * method will almost always throw.
 *
 * @code
 *  Configuration config;
 *  config.setNumberOfModules(2);
 *  config.setModuleEventLengthsMap({4, 4});
 * @endcode
 */
void DAQ::DDAS::Configuration::setModuleEventLengths(
    const std::vector<int> &lengths) {
  if (lengths.size() != m_numModules) {
    throw std::runtime_error(
        "Configuration::setModuleEventLengths() "
        "Inconsistent data for module evt lengths and slot mapping. "
        "Set number of modules first using "
        "Configuration::setNumberOfModules().");
  }

  m_modEvtLengths = lengths;
}

/**
 * @details
 * It is necessary that the caller has previously invoked
 * `setNumberOfModules()` before calling this. The logic of this method aims
 * to keep the slot map and number of modules in the system the same length.
 * Without invoking `setNumberOfModules()` this is most likely not going to be
 * the case.
 *
 * @code
 *  Configuration config;
 *  config.setNumberOfModules(2);
 *  config.setModuleHardwareMap({RevD_100MHz_12Bit, RevF_250MHz_14Bit});
 * @endcode
 */
void DAQ::DDAS::Configuration::setHardwareMap(const std::vector<int> &map) {
  if (map.size() != m_numModules) {
    throw std::runtime_error(
        "Configuration::setHardwareMap() "
        "Inconsistent data for hardware mapping and slot mapping. "
        "Set number of modules first using "
        "Configuration::setNumberOfModules().");
  }

  m_hardwareMap = map;
}

/*!
 * @details
 * Prints out a message similar to:
 * "Crate number 1: 2 modules, in slots:2 3 DSPParFile: /path/to/file.set"
 */
void DAQ::DDAS::Configuration::print(std::ostream &stream) {
  stream << "Crate number " << m_crateId;
  stream << ": " << m_numModules << " modules, in slots: ";
  for (auto &slot : m_slotMap) {
    stream << slot << " ";
  }
  stream << "Channel map: ";
  for (auto c : m_channelMap) {
    stream << c << " ";
  }
  stream << "DSPParFile: " << m_settingsFilePath;
}

std::unique_ptr<DAQ::DDAS::Configuration>
DAQ::DDAS::Configuration::generate(const std::string &cfgPixiePath) {
  std::unique_ptr<Configuration> pConfig(new Configuration);
  ConfigurationParser configParser;
  std::ifstream cfg(cfgPixiePath.c_str(), std::ios::in);

  if (cfg.fail()) {
    throw std::runtime_error("Configuration::generate() Failed to open the "
                             "system configuration file : " +
                             cfgPixiePath);
  }

  configParser.parse(cfg, *pConfig);

  return std::move(pConfig);
}

/**
 * @details
 * `std::move()` ensures correct ownership of the returned pointer,
 * though we _may_ be able to take advantage of some copy elision here.
 */
std::unique_ptr<DAQ::DDAS::Configuration>
DAQ::DDAS::Configuration::generate(const std::string &fwVsnPath,
                                   const std::string &cfgPixiePath) {
  std::unique_ptr<Configuration> pConfig = generate(cfgPixiePath);
  FirmwareVersionFileParser fwFileParser;
  std::ifstream fwvsn(fwVsnPath.c_str(), std::ios::in);

  if (fwvsn.fail()) {
    throw std::runtime_error("Configuration::generate() Failed to open the "
                             "firmware version file : " +
                             fwVsnPath);
  }

  fwFileParser.parse(fwvsn, pConfig->m_fwMap);
  fwvsn.close();

  return std::move(pConfig);
}

/**
 * @details
 * `std::move()` ensures correct ownership of the returned pointer,
 * though we _may_ be able to take advantage of some copy elision here.
 */
std::unique_ptr<DAQ::DDAS::Configuration>
DAQ::DDAS::Configuration::generate(const std::string &fwVsnPath,
                                   const std::string &cfgPixiePath,
                                   const std::string &modEvtLenPath) {
  std::unique_ptr<Configuration> pConfig = generate(fwVsnPath, cfgPixiePath);

  // Read a configration file to tell Pixie16 how big an event is in
  // a particular module.  Within one module all channels MUST be set to
  // the same event length

  ModEvtFileParser modEvtParser;
  std::ifstream modevt(modEvtLenPath.c_str(), std::ios::in);

  if (!modevt.is_open()) {
    throw std::runtime_error("Configuration::generate() Failed to open the "
                             "module event length configuration file: " +
                             modEvtLenPath);
  }

  modEvtParser.parse(modevt, *pConfig);
  modevt.close();

  return std::move(pConfig);
}

/**
 * @details
 * `std::move()` ensures correct ownership of the returned pointer,
 * though we _may_ be able to take advantage of some copy elision here.
 */
std::unique_ptr<DAQ::DDAS::Configuration>
DAQ::DDAS::Configuration::generateManagedFW(const std::string &cfgPixiePath,
                                            const std::string &modEvtLenPath) {
  std::unique_ptr<Configuration> pConfig = generate(cfgPixiePath);

  // Read a configration file to tell Pixie16 how big an event is in
  // a particular module.  Within one module all channels MUST be set to
  // the same event length

  ModEvtFileParser modEvtParser;
  std::ifstream modevt(modEvtLenPath.c_str(), std::ios::in);

  if (!modevt.is_open()) {
    throw std::runtime_error("Configuration::generate() Failed to open the "
                             "module event length configuration file: " +
                             modEvtLenPath);
  }

  modEvtParser.parse(modevt, *pConfig);
  modevt.close();

  return std::move(pConfig);
}
