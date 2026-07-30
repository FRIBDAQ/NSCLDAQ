/**
 * @file CPixieSystemUtilities.h
 * @brief Defines a class for managing the state of Pixie DAQ systems and
 * a ctypes interface for the class.
 */

#ifndef CPIXIESYSTEMUTILITIES_H
#define CPIXIESYSTEMUTILITIES_H

#include <vector>

#include <Configuration.h>

/**
 * @addtogroup utilities libPixieUtilities.so
 * @{
 */

/**
 * @class CPixieSystemUtilities CPixieSystemUtilities.h
 * @brief System manager class for DDAS.
 *
 * @details
 * This class manages the Pixie DAQ system. It controls loading and saving
 * settings files, booting and exiting, and stores information about the state
 * of the system which can be accessed across the ctypes interface.
 */

class CPixieSystemUtilities {
private:
  DAQ::DDAS::Configuration m_config; //!< Hardware configuration information.
  std::string m_lastErrorMessage;    //!< Last error message from the system.
  int m_bootMode;                    //!< Offline (1) or online (0) boot mode.
  bool m_booted;     //!< True when the system is booted, false otherwise.
  bool m_ovrSetFile; //!< True if loading a settings file after booting.

public:
  /** @brief Constructor. */
  CPixieSystemUtilities();

  /**
   * @brief Boot the entire system.
   * @return int
   * @retval 0 On successful boot.
   * @retval CPIXIEERROR_INVALID_CONFIG Failed to generate the system
   * configuration.
   * @retval !=0 XIA API error code on failure.
   */
  int Boot();
  /**
   * @brief Save the currently loaded DSP settings to a settings file.
   * @param fileName Name of file to save.
   * @return int
   * @retval 0   Success.
   * @retval !=0 XIA API error code on failure.
   */
  int SaveSetFile(char *fileName);
  /**
   * @brief Load a new settings file.
   * @param fileName  Settings file name we are attempting to open.
   * @return int
   * @retval 0   Success.
   * @retval !=0 XIA API error code on failure.
   */
  int LoadSetFile(char *fileName);
  /**
   * @brief Exit the system and release resources from the modules.
   * @return int
   * @retval 0   Success.
   * @retval !=0 XIA API error code on failure.
   */
  int ExitSystem();

  /**
   * @brief Set the boot mode.
   * @param mode Set the boot mode to this value.
   */
  void SetBootMode(int mode) { m_bootMode = mode; };
  /**
   * @brief Set the last error message. Used by the extern "C" shims when
   * their catch-all fires.
   * @param msg Message to store.
   */
  void SetLastErrorMessage(const char *msg) { m_lastErrorMessage = msg; };

  /**
   * @brief Get the boot mode.
   * @warning Offline boot mode is currently only allowed for XIA API 2!
   * @return The boot mode.
   * @retval 0 Online mode.
   * @retval 1 Offline mode (no hardware).
   */
  int GetBootMode() { return m_bootMode; };
  /**
   * @brief Get the crate boot status.
   * @return bool
   * @retval true If the system has been booted.
   * @retval false Otherwise.
   */
  bool GetBootStatus() { return m_booted; };
  /**
   * @brief Get the number of installed modules.
   * @return The number of modules in the crate.
   */
  int GetNumModules() { return m_config.getNumberOfModules(); };
  /**
   * @brief Get the module ADC sampling rate in MSPS.
   * @param module Module number (zero-indexed).
   * @returns The module ADC sampling rate in MSPS.
   * @retval CPIXIEERROR_NOT_BOOTED if the system is not booted.
   * @retval CPIXIEERROR_INVALID_MODULE if the module number is invalid.
   */
  int GetModuleMSPS(int module);
  /**
   * @brief Get the number of channels on the module.
   * @param module Module number (zero-indexed).
   * @returns The number of channels on the module.
   * @retval CPIXIEERROR_NOT_BOOTED if the system is not booted.
   * @retval CPIXIEERROR_INVALID_MODULE if the module number is invalid.
   */
  int GetModuleChannelCount(int module);
  /**
   * @brief Get the reason text from the most recent failed operation.
   * @return Pointer to the stored message; empty string if none.
   */
  const char *GetLastErrorMessage() { return m_lastErrorMessage.c_str(); };
};

/** @} */

extern "C" {
/** @brief Wrapper for the class constructor. */
CPixieSystemUtilities *CPixieSystemUtilities_new();
/** @brief Wrapper to boot the crate. */
int CPixieSystemUtilities_Boot(CPixieSystemUtilities *utils);
/** @brief Wrapper to save a settings file. */
int CPixieSystemUtilities_SaveSetFile(CPixieSystemUtilities *utils,
                                      char *fName);
/** @brief Wrapper to load a settings file. */
int CPixieSystemUtilities_LoadSetFile(CPixieSystemUtilities *utils,
                                      char *fName);
/** @brief Wrapper to exit the system file. */
int CPixieSystemUtilities_ExitSystem(CPixieSystemUtilities *utils);
/** @brief Wrapper to set the boot mode. */
void CPixieSystemUtilities_SetBootMode(CPixieSystemUtilities *utils, int mode);
/** @brief Wrapper to get the boot mode. */
int CPixieSystemUtilities_GetBootMode(CPixieSystemUtilities *utils);
/** @brief Wrapper to get the boot status. */
bool CPixieSystemUtilities_GetBootStatus(CPixieSystemUtilities *utils);
/** @brief Wrapper to get the number of modules. */
int CPixieSystemUtilities_GetNumModules(CPixieSystemUtilities *utils);
/** @brief Wrapper to get a single module ADC MSPS from the HW map. */
int CPixieSystemUtilities_GetModuleMSPS(CPixieSystemUtilities *utils, int mod);
/** @brief Wrapper to get the channel count for a single module. */
int CPixieSystemUtilities_GetModuleChannelCount(CPixieSystemUtilities *utils,
                                                int mod);
/** @brief Wrapper to get the last error message. Cannot throw; unguarded. */
const char *
CPixieSystemUtilities_GetLastErrorMessage(CPixieSystemUtilities *utils);
/** @brief Wrapper for the class destructor. */
void CPixieSystemUtilities_delete(CPixieSystemUtilities *utils);
}

#endif
