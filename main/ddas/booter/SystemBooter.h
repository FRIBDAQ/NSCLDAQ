/**
 * @file SystemBooter.h
 * @brief Defines a class to manage the booting process for DDAS.
 */

#ifndef SYSTEMBOOTER_H
#define SYSTEMBOOTER_H

#include <stddef.h>
#include <string>

/** @namespace DAQ */
namespace DAQ {
/** @namespace DAQ::DDAS */
namespace DDAS {

class Configuration;
struct FirmwareConfiguration;

/**
 * @addtogroup libSystemBooter libSystemBooter.so
 * @brief DDAS Pixie-16 system booter library.
 * @details
 * A library containing code used by other DDAS programs which boots
 * Pixie modules and sets hardware configuration for the booted system.
 * @{
 */

/**
 * @class SystemBooter SystemBooter.h
 * @brief Manages the booting process for DDAS.
 * @details
 * All Readout and slow controls programs rely on this class to boot
 * the system.
 */

class SystemBooter {
public:
  /** @brief An enum for boot type masks, legacy flags. */
  enum BootType {
    FullBoot,    //!< Full boot with firmware load.
    SettingsOnly //!< Boot with settings only.
  };

private:
  bool m_verbose;               //!< Enable or disable output.
  unsigned short m_offlineMode; //!< 0: online 1: offline (no HW).

public:
  /** @brief Constructor. */
  SystemBooter();

  /**
   * @brief Boot the entire system.
   * @param config References the system configuration.
   * @param type Boot type (full or settings-only).
   */
  void boot(Configuration &config, BootType type);

  /**
   * @brief Enable or disable verbose output.
   * @param enb Enables output messages if true.
   */
  void setVerbose(bool enb) { m_verbose = enb; };
  /**
   * @brief Return the verbose state.
   * @return True if verbose output enabled, false otherwise.
   */
  bool isVerbose() const { return m_verbose; };
  /**
   * @brief Enable or disable online boot
   * @param mode Boot mode: 0 for online, anything else (typically 1) for
   * offline.
   */
  void setOfflineMode(unsigned short mode) { m_offlineMode = mode; };
  /**
   * @brief Return the boot mode of the system.
   * @return Boot mode: 0 for online, anything else for offline.
   */
  unsigned short getOfflineMode() const { return m_offlineMode; };

private:
  /**
   * @brief Perform system initializaiton.
   * @param config References the system configuration.
   * @throw CXIAException if system initialization fails.
   */
  void initSystem(Configuration &config);
  /**
   * @brief Parallel boot of the crate.
   * @param config References the system configuration.
   * @param type BootType (full or settings-only)
   * @throw CXIAException if firmware load fails.
   * @throw CXIAException if crate boot fails.
   */
  void parallelBoot(Configuration &config, BootType type);
  /**
   * @brief Offline boot of the crate simulation.
   * @param config References the system configuration.
   * @param type BootType (full or settings-only).
   * @throw CXIAException if crate boot fails.
   */
  void offlineBoot(Configuration &config, BootType type);
  /**
   * @brief Set firmware from per-module firmware maps.
   * @param config References the system configuration.
   */
  void setPerModuleFirmware(Configuration &config);
  /**
   * @brief Set firmware for a single module.
   * @param config References the system configuration.
   * @param mod Module index.
   * @throw std::runtime_error If the hardware type is not present in the
   * hardware registry.
   */
  void setModuleFirmware(Configuration &config, unsigned int mod);
  /**
   * @brief Set firmware for a single device on a module.
   * @param fwFile Path to the device firmware file.
   * @param mod Module index.
   * @param slot Slot number for this module.
   * @param device Firmware device name (sys, fippi, dsp, var).
   * @throw CXIAException If the device firmware cannot be set.
   */
  void setDeviceFirmware(std::string fwFile, unsigned int mod,
                         unsigned int slot, std::string device);
  /**
   * @brief Set DSP settings from per-module settings map.
   * @param config References the system configuration.
   * @throw CXIAException If the firmware boot fails.
   */
  void setPerModuleDSP(Configuration &config);
  /**
   * @brief Set the channel map.
   * @param config References the system configuration.
   */
  void setChannelMap(Configuration &config);
  /**
   * @brief Read and store hardware info from each of the modules
   * in the system.
   * @param config The system configuration.
   */
  void populateHardwareMap(Configuration &config);
  /**
   * @brief Print out some basic information regarding the module
   * @param config The system configuration.
   */
  void logModuleInfo(Configuration &config);
};

/** @} */

} // namespace DDAS
} // namespace DAQ

#endif // SYSTEMBOOTER_H
