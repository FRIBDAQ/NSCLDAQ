/**
 * @file CPixieSystemUtilities.cpp
 * @brief Implementation the Pixie DAQ system utilities class.
 */

#include "CPixieSystemUtilities.h"

#include <iostream>
#include <sstream>

#include <config.h>
#include <config_pixie16api.h>

#include <CXIAException.h>
#include <SystemBooter.h>
#include <string>

#include "CPixieErrors.h"

using namespace DAQ::DDAS;
namespace HR = DAQ::DDAS::HardwareRegistry;

/**
 * @details
 * Default: boot in online mode and read the settings file specified in
 * cfgPixie16.txt. The last error message is initialized to an empty string.
 */
CPixieSystemUtilities::CPixieSystemUtilities()
    : m_bootMode(0), m_booted(false), m_ovrSetFile(false) {}

/**
 * @details
 * Reads in configuration information from cfgPixie16.txt, loads settings file
 * information, boots modules and saves configuration info.
 */
int CPixieSystemUtilities::Boot() {
  // If the settings file path has been overwritten pre system boot, use
  // the new path. first we grab it, then reset it after initializing
  // the configuration settings below.

  std::string newSetFile;
  if (m_ovrSetFile) {
    newSetFile = m_config.getSettingsFilePath();
  }

  // If a FW file is specified, use it, otherwise use managed FW:

  const char *fwFile = getenv("FIRMWARE_FILE");
  try {
    if (fwFile) {
      m_config =
          *(Configuration::generate(fwFile, "cfgPixie16.txt", "modevtlen.txt"));
    } else {
      m_config = *(
          Configuration::generateManagedFW("cfgPixie16.txt", "modevtlen.txt"));
    }
  } catch (const std::exception &e) {
    m_lastErrorMessage = e.what();
    std::cerr << "CPixieSystemUtilities::Boot() failed: " << m_lastErrorMessage
              << std::endl;
    return CPIXIEERROR_INVALID_CONFIG;
  }

  // (Re)set the custom settings file path here if used:

  if (m_ovrSetFile) {
    m_config.setSettingsFilePath(newSetFile);
  }

  /**
   * @note (ASC 9/11/24): Check the same envvar as e.g. the readout code to
   * determine whether to perform a full boot or settings-only boot.
   * In principle this could be configurable on the QtScope GUI but for now
   * the boot mode is set the same way as it is for the readout code.
   * An important thing to keep in mind is that
   * `getenv("DDAS_BOOT_WHEN_REQUESTED")` is false iff
   * `DDAS_BOOT_WHEN_REQUESTED` is not set (`getenv()` returns pointer to
   * the value string which evaluates to true regardless of the value
   * itself). When running a containerized NSCLDAQ one needs to make sure the
   * envvar is set _inside_ the container.
   */

  SystemBooter::BootType type = SystemBooter::FullBoot;
  if (getenv("DDAS_BOOT_WHEN_REQUESTED")) {
    type = SystemBooter::SettingsOnly;
  }
  SystemBooter booter;
  booter.setOfflineMode(m_bootMode); // 1: offline, 0: online
  try {
    booter.boot(m_config, type);
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  m_booted = true;

  return 0;
}

/**
 * @details
 * File format depends on what is supported by the version of the XIA API
 * being used. Version 3+ will save the settings file as a JSON file while in
 * version 2 it is binary.
 */
int CPixieSystemUtilities::SaveSetFile(char *fileName) {
  try {
    int retval = Pixie16SaveDSPParametersToFile(fileName);
    if (retval < 0) {
      std::stringstream msg;
      msg << "CPixieSystemUtilities::SaveSetFile() failed to save"
          << " DSP parameter file to: " << fileName;
      throw CXIAException(msg.str(), "Pixie16SaveDSPParametersToFile()",
                          retval);
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return 0;
}

/**
 * @details
 * Check and see if the system is booted. If so, load the parameters from
 * the settings file. If not flag that a new settings file path (potentially
 * different from that in the cfgPixie16.txt) has been set. The flag is
 * checked at boot to load the new settings file.
 */
int CPixieSystemUtilities::LoadSetFile(char *fileName) {
  // If we aren't booted, simply hold onto the name:
  if (!m_booted) {
    m_ovrSetFile = true;
    m_config.setSettingsFilePath(fileName);
    std::cout << "New DSP parameter file " << fileName
              << " will be loaded on system boot" << std::endl;
    return 0;
  }

  try {
    int retval = Pixie16LoadDSPParametersFromFile(fileName);
    if (retval < 0) {
      std::stringstream msg;
      msg << "CPixieSystemUtilities::LoadSetFile() failed to"
          << " load DSP parameter file from: " << fileName;
      throw CXIAException(msg.str(), "Pixie16LoadDSPParametersFromFile()",
                          retval);
    } else {
      std::cout << "Loading new DSP parameter file from: " << fileName
                << std::endl;
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return 0;
}

/**
 * @details
 * If the call to Pixie16ExitSystem() fails for any module, return the error
 * code and set the booted state flag to false. The system is likely in a bad
 * state.
 */
int CPixieSystemUtilities::ExitSystem() {
  // If we aren't booted, we don't need to do anything, just return success:
  if (!m_booted) {
    return 0;
  }

  // Assuming the system is booted then:
  try {
    for (int i = 0; i < m_config.getNumberOfModules(); i++) {
      int retval = Pixie16ExitSystem(i);
      if (retval < 0) {
        std::stringstream msg;
        msg << "CPixieSystemUtilities::ExitSystem() failed to exit module "
            << i;
        throw CXIAException(msg.str(), "Pixie16ExitSystem()", retval);
      }
    }
    m_booted = false;
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    m_booted = false;
    return e.ReasonCode();
  }

  return 0;
}

/**
 * @details
 * Perfoms bounds checking on the module number. The various
 * DAQ::DDAS::Configuration accessor methods throw but ctypes does not handle
 * C++ exceptions so we catch them here and print the error message to stderr.
 * The return value is an error code that can be checked by the caller.
 */
int CPixieSystemUtilities::GetModuleMSPS(int module) {
  if (!m_booted) {
    m_lastErrorMessage =
        "CPixieSystemUtilities::GetModuleMSPS() system not booted.";
    std::cerr << m_lastErrorMessage << std::endl;
    return CPIXIEERROR_NOT_BOOTED;
  }

  auto numModules = m_config.getNumberOfModules();
  if (module < 0 || module >= numModules) {
    std::stringstream msg;
    msg << "CPixieSystemUtilities::GetModuleMSPS()";
    msg << " invalid module number " << module << " for " << numModules
        << " module system.";
    m_lastErrorMessage = msg.str();
    std::cerr << m_lastErrorMessage << std::endl;
    return CPIXIEERROR_INVALID_MODULE;
  }

  const auto &hdwrMap = m_config.getHardwareMap();
  const auto &spec = HR::getSpecification(hdwrMap[module]);

  return spec.s_adcFrequency;
}

/**
 * @details
 * Perfoms bounds checking on the module number. The various
 * DAQ::DDAS::Configuration accessor methods throw but ctypes does not handle
 * C++ exceptions so we catch them here and print the error message to stderr.
 * The return value is an error code that can be checked by the caller.
 */
int CPixieSystemUtilities::GetModuleChannelCount(int module) {
  if (!m_booted) {
    m_lastErrorMessage =
        "CPixieSystemUtilities::GetModuleChannelCount() system not booted.";
    std::cerr << m_lastErrorMessage << std::endl;
    return CPIXIEERROR_NOT_BOOTED;
  }

  auto numModules = m_config.getNumberOfModules();
  if (module < 0 || module >= numModules) {
    std::stringstream msg;
    msg << "CPixieSystemUtilities::GetModuleChannelCount()";
    msg << " invalid module number " << module << " for " << numModules
        << " module system.";
    m_lastErrorMessage = msg.str();
    std::cerr << m_lastErrorMessage << std::endl;
    return CPIXIEERROR_INVALID_MODULE;
  }

  return static_cast<int>(m_config.getModuleChannelCount(module));
}
