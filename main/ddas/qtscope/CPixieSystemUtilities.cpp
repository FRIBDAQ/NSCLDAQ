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

using namespace DAQ::DDAS;
namespace HR = DAQ::DDAS::HardwareRegistry;

/**
 * @details
 * Default: boot in online mode and read the settings file specified in
 * cfgPixie16.txt.
 */
CPixieSystemUtilities::CPixieSystemUtilities()
    : m_bootMode(0), m_booted(false), m_ovrSetFile(false), m_numModules(0) {}

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
  if (fwFile) {
    m_config =
        *(Configuration::generate(fwFile, "cfgPixie16.txt", "modevtlen.txt"));
  } else {
    m_config =
        *(Configuration::generateManagedFW("cfgPixie16.txt", "modevtlen.txt"));
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
  if (getenv("DDAS_BOOT_WHEN_REQUESTED"))
    type = SystemBooter::SettingsOnly;
  SystemBooter booter;
  booter.setOfflineMode(m_bootMode); // 1: offline, 0: online
  try {
    booter.boot(m_config, type);
  } catch (const CXIAException &e) {
    std::cerr << e.ReasonText() << std::endl;
    return -1;
  }

  m_numModules = m_config.getNumberOfModules();
  m_modEvtLength = m_config.getModuleEventLengths();

  // The hardware map is set up during boot time:

  std::vector<int> hdwrMap = m_config.getHardwareMap();
  for (size_t i = 0; i < hdwrMap.size(); i++) {
    HR::HardwareSpecification spec = HR::getSpecification(hdwrMap.at(i));
    m_modADCMSPS.push_back(spec.s_adcFrequency);
    m_modADCBits.push_back(spec.s_adcResolution);
    m_modRev.push_back(spec.s_hdwrRevision);
    m_modClockCal.push_back(spec.s_clockCalibration);
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
  int retval;
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
    std::cerr << e.ReasonText() << std::endl;
  }

  return retval;
}

/**
 * @details
 * Check and see if the system is booted. If so, load the parameters from
 * the settings file. If not flag that a new settings file path (potentially
 * different from that in the cfgPixie16.txt) has been set. The flag is
 * checked at boot to load the new settings file.
 */
int CPixieSystemUtilities::LoadSetFile(char *fileName) {
  int retval;
  try {
    if (m_booted) { // If system is booted just apply the params.
      retval = Pixie16LoadDSPParametersFromFile(fileName);

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
    } else { // Not booted so hold on to the name.
      m_ovrSetFile = true;
      m_config.setSettingsFilePath(fileName);
      std::cout << "New DSP parameter file " << fileName
                << " will be loaded on system boot" << std::endl;
    }
  } catch (const CXIAException &e) {
    std::cerr << e.ReasonText() << std::endl;
    return e.ReasonCode();
  }

  return retval;
}

/**
 * @details
 * If the call to Pixie16ExitSystem() fails for any module, return the error
 * code and set the booted state flag to false. The system is likely in a bad
 * state.
 */
int CPixieSystemUtilities::ExitSystem() {
  int retval;
  try {
    if (m_booted) {
      for (int i = 0; i < m_numModules; i++) {
        retval = Pixie16ExitSystem(i);
        if (retval < 0) {
          std::stringstream msg;
          msg << "CPixieSystemUtilities::ExitSystem() failed"
              << " to exit " << "module " << i;
          throw CXIAException(msg.str(), "Pixie16ExitSystem()", retval);
        }
      }
    }
    m_booted = false;
  } catch (const CXIAException &e) {
    std::cerr << e.ReasonText();
    m_booted = false;
    return e.ReasonCode();
  }

  return retval; // All good.
}

int CPixieSystemUtilities::GetModuleMSPS(int module) {
  // A correctly booted system must by definition contain >= 1 module
  // so I'm _pretty_ sure this is a good check for that too:

  if (!m_booted) {
    std::string msg(
        "CPixieSystemUtilities::GetModuleMSPS() system not booted.");
    return -1;
  } else if ((module < 0) || (module >= m_numModules)) {
    std::stringstream msg;
    msg << "CPixieSystemUtilities::GetModuleMSPS() ";
    msg << "invalid module number ";
    msg << module << " for " << m_numModules << " module system.";
    std::cerr << msg.str() << std::endl;
    return -2;
  } else {
    // Implicit conversion probably OK but:
    return static_cast<int>(m_modADCMSPS[module]);
  }
}

int CPixieSystemUtilities::GetModuleChannelCount(int module) {
  // A correctly booted system must by efinition contain >= 1 module
  // so I'm _pretty_ sure this is a good check for that too:

  if (!m_booted) {
    std::string msg(
        "CPixieSystemUtilities::GetModuleChannelCount() system not booted.");
    std::cerr << msg << std::endl;
    return -1;
  } else if ((module < 0) || (module >= m_numModules)) {
    std::stringstream msg;
    msg << "CPixieSystemUtilities::GetModuleChannelCount() ";
    msg << "invalid module number ";
    msg << module << " for " << m_numModules << " module system.";
    std::cerr << msg.str() << std::endl;
    return -2;
  } else {
    // Implicit conversion probably OK but:
    return static_cast<int>(m_config.getModuleChannelCount(module));
  }
}