/**
 * @file Configuration.h
 * @brief Defines a class for storing system configuration information.
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "HardwareRegistry.h"

/** @namespace DAQ */
namespace DAQ {
/** @namespace DAQ::DDAS */
namespace DDAS {

/**
 * @addtogroup configuration libConfiguration.so
 * @brief DDAS Pixie-16 hardware configuration library.
 * @details
 * Shared library containing classes to manage the internal
 * configuration of a DDAS system and store information about its
 * hardware. Contains all functions defined in the
 * DAQ::DDAS::HardwareRegistry namespace.
 * @{
 */

/**
 * @brief Storage for hardware-type firmware/settings paths.
 * @details
 * A simple structure to hold the paths to all firmware/settings files
 * for a specific hardware type. These objects will be stored in a map
 * of the Configuration class and keyed by a hardware type defined in
 * HardwareRegistry.h
 */
struct FirmwareConfiguration {
  /** Name of communications FPGRA configuration file. */
  std::string s_ComFPGAConfigFile;
  /** Name of signal processing FPGA configuration file. */
  std::string s_SPFPGAConfigFile;
  std::string s_DSPCodeFile; //!< Name of executable DSP code file.
  std::string s_DSPVarFile;  //!< Name of DSP variable names file.
};

/**
 * @typedef FirmwareMap
 * @brief A map of firmware configurations keyed by the hardware type
 * defined in HardwareRegistry.h
 */
typedef std::map<int, FirmwareConfiguration> FirmwareMap;

/**
 * @class Configuration
 * @brief Store the system configuration information needed by Readout.
 * @details
 * The Configuration class stores all of the system configuration for
 * a Readout program. It maintains the configuration that is read in
 * from the modevtlen.txt, and cfgPixie16.txt configuration files.
 * The configuration keeps track of the crate ID, slot map, settings
 * file path, module event lengths, module count, the hardware types
 * present the crate, and, if specified, the firmware and DSP settings
 * loaded onto those modules.
 *
 * A number of methods are provided to generate a Configuration object
 * based on which of the aforementioned configuration files are
 * present. The new function `generateManagedFW()` can be used to
 * generate a valid Configuration compatible with XIA's automatic
 * firmware management (i.e. no pre-defined default FW).
 *
 * At the moment, modules are expected to output events of equal
 * length for all channels. There is no attempt to read out channels
 * with different lengths in a module.
 */

class Configuration {
private:
  int m_crateId;                            //!< Crate ID from cfgPixie16.
  std::vector<unsigned short> m_slotMap;    //!< Map of physical slots.
  std::vector<unsigned short> m_channelMap; //!< Channels per module.
  std::string m_settingsFilePath;           //!< Path to default .set file.
  std::vector<int> m_modEvtLengths;         //!< Event lengths for modules.
  FirmwareMap m_fwMap;            //!< Map of firmware for hardware types.
  std::vector<int> m_hardwareMap; //!< Map of HardwareRegistry types.

  // These additions support per module firmware maps and set files:

  /** Per-module firmware maps */
  std::map<int, FirmwareMap> m_moduleFirmwareMaps;
  /** Per-module setfiles */
  std::map<int, std::string> m_moduleSetFileMap;

public:
  /** @brief Constructor. */
  Configuration() = default;
  /** @brief Destructor. */
  ~Configuration() = default;

  /**
   * @brief Set the crate id for the module.
   * @param id The id to assign.
   */
  void setCrateId(int id) { m_crateId = id; };
  /**
   * @brief Return the crate id value.
   * @return The crate id.
   */
  int getCrateId() const { return m_crateId; };
  /**
   * @brief Set the number of modules in the crate.
   * @param size Number of modules.
   */
  void setNumberOfModules(size_t size);
  /**
   * @brief Return the number of modules in the crate.
   * @return The number of modules.
   */
  size_t getNumberOfModules() const { return m_slotMap.size(); };
  /**
   * @brief Assign a new slot map.
   * @param map  The slots that are occupied.
   * @throws std::runtime_error When length of argument is
   *   different than the length of stored modevtlen vector.
   */
  void setSlotMap(const std::vector<unsigned short> &map);
  /**
   * @brief Return the vector containing the filled slots.
   * @return std::vector<unsigned short>  The vector containing the
   *   slots that are filled.
   */
  std::vector<unsigned short> getSlotMap() const { return m_slotMap; };
  /**
   * @brief Assign a new channel map (number of channels per module).
   * @param map Map of channels in each module.
   */
  void setChannelMap(const std::vector<unsigned short> &map);
  /**
   * @brief Return the channel map.
   * @return Vector containing the number of channels per module.
   */
  std::vector<unsigned short> getChannelMap() { return m_channelMap; };
  /**
   * @brief Get the number of channels in a module.
   * @param mod The module number (index, not slot).
   * @return Number of channels in module.
   * @throw Runtime error if the module is out of range.
   */
  unsigned short getChannelCount(size_t mod);
  /**
   * @brief Set the path to the DSP settings file.
   * @param path The path to the settings file.
   */
  void setSettingsFilePath(const std::string &path) {
    m_settingsFilePath = path;
  };
  /**
   * @brief Set a per-module DSP settings file.
   * @param modNum Module number.
   * @param path The path to the settings file.
   */
  void setModuleSettingsFilePath(int modnum, const std::string &path);
  /**
   * @brief Return the path to the .set file.
   * @return The settings file path.
   */
  std::string getSettingsFilePath() const { return m_settingsFilePath; };
  /**
   * @brief Returns the DSP settings file path specific to a single
   * module.
   * @param modnum Module number.
   * @return std::string  The full path to the settings file.
   */
  std::string getSettingsFilePath(int modNum);
  /**
   * @brief Set the firmware configuration for a hardware type
   * @param specifier The hardware type.
   * @param config    The new configuration.
   * @details
   * Any previous FirmwareConfiguration stored will be replaced by
   * the new configuration. If there is no previous configuration
   * for the hardware type it will be added.
   */
  void setFirmwareConfiguration(int specifier,
                                const FirmwareConfiguration &config) {
    m_fwMap[specifier] = config;
  };
  /**
   * @brief Retrieve the current firmware specifier for a particular
   * hardware type.
   * @param hdwrType The hardware specifier associated with the
   *   firmware configuration.
   * @throws std::runtime_error If no firmware configuration exists
   *   for the provided hdwrType.
   * @return The firmware configuration associated with the hdwrType.
   */
  FirmwareConfiguration &getFirmwareConfiguration(int hdwrType);
  /**
   * @brief Sets a firmware map specific to a module.
   * @param module  Module index.
   * @param mapping Firmware mapping for that module.
   */
  void setModuleFirmwareMap(int module, const FirmwareMap &mapping);
  /**
   * @brief Get the module firmware configuration information.
   * @param hwType The hardware type detected in the module.
   * @param modnum Module number.
   * @throw std::runtime_error If the module firmware configuraton
   *   is not in the firmware map.
   * @return The firmware configuration associated with the module.
   */
  FirmwareConfiguration &getModuleFirmwareConfiguration(int hdwrType,
                                                        int modnum);
  /**
   * @brief Return the default map of firmware information.
   * @return The firmware map.
   */
  FirmwareMap &getDefaultFirmwareMap() { return m_fwMap; }
  /**
   * @brief Set the lengths of events for each module
   * @param lengths  The module event lengths.
   * @throw std::runtime_error if size of lengths does not match
   *   size of stored slot map.
   */
  void setModuleEventLengths(const std::vector<int> &lengths);
  /**
   * @brief Return a copy of the module event length vector.
   * @return std::vector<int>  Copy of module event lengths vector.
   */
  std::vector<int> getModuleEventLengths() const { return m_modEvtLengths; };
  /**
   * @brief Set the hardware map for each module.
   * @param map The hardware map.
   * @throw std::runtime_error if size of lengths does not match
   *   size of stored slot map.
   */
  void setHardwareMap(const std::vector<int> &map);
  /**
   * @brief Return a copy of the hardware map vector.
   * @return A copy of hardware map vector.
   */
  std::vector<int> getHardwareMap() const { return m_hardwareMap; };
  /**
   * @brief Get the per-module FW maps.
   * @return Per-module FW maps.
   */
  std::map<int, FirmwareMap> getModuleFirmwareMaps() const {
    return m_moduleFirmwareMaps;
  };
  /**
   * @brief Get the per-module settings file map.
   * @return Per-module settings file map.
   */
  std::map<int, std::string> getModuleSetFileMap() const {
    return m_moduleSetFileMap;
  };

  /**
   * @brief Print brief line of information for cfgPixie16.txt
   * @param stream The ostream to write to.
   */
  void print(std::ostream &stream);

  /**
   * @brief Generate a Configuration class object from cfgPixie16.txt.
   * @param cfgPixiePath Path to cfgPixie16.txt.
   * @throw std::runtime_error Any errors opening or parsing the
   *   firmware and configuration files.
   * @return Pointer to the generated Configuration object.
   */
  static std::unique_ptr<Configuration>
  generate(const std::string &cfgPixiePath);
  /**
   * @brief Generate a Configuration class object from a firmware
   * version file and cfgPixie16.txt.
   * @param fwVsnPath    Path to the firmware version file.
   * @param cfgPixiePath Path to cfgPixie16.txt.
   * @throw std::runtime_error Any errors opening or parsing the
   *   firmware and configuration files.
   * @return Pointer to the generated Configuration object.
   */
  static std::unique_ptr<Configuration>
  generate(const std::string &fwVsnPath, const std::string &cfgPixiePath);
  /**
   * @brief Generate a Configuration class object from a firmware
   * version file, cfgPixie16.txt and modevtlen.txt file.
   * @param fwVsnPath     Path to the firmware version file.
   * @param cfgPixiePath  Path to cfgPixie16.txt.
   * @param modEvtLenPath Path to the modevtlen.txt file.
   * @throw std::runtime_error Error opening or parsing the
   *   modevtlen file.
   * @return Pointer to the generated Configuration object.
   */
  static std::unique_ptr<Configuration>
  generate(const std::string &fwVsnPath, const std::string &cfgPixiePath,
           const std::string &modEvtLenPath);
  /**
   * @brief Generate a Configuration class object from
   * cfgPixie16.txt and modevtlen.txt files (managed FW).
   * @param cfgPixiePath  Path to cfgPixie16.txt.
   * @param modEvtLenPath Path to the modevtlen.txt file.
   * @throw std::runtime_error Error opening or parsing the
   *   modevtlen file.
   * @return Pointer to the generated Configuration object.
   */
  static std::unique_ptr<Configuration>
  generateManagedFW(const std::string &cfgPixiePath,
                    const std::string &modEvtLenPath);
};

/** @} */

} // namespace DDAS
} // namespace DAQ

#endif // CONFIGURATION_H
