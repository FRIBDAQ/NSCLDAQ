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
 * This class manages the Pixie DAQ system. It controls loading and saving 
 * settings files, booting and exiting, and stores information about the state
 * of the system which can be accessed across the ctypes interface.
 */

class CPixieSystemUtilities
{
public:
    /** @brief Constructor. */
    CPixieSystemUtilities();
    /** @brief Destructor. */
    ~CPixieSystemUtilities();

    /**
     * @brief Boot the entire system.
     * @return int 
     * @retval 0  On successful boot.
     * @retval -1 If the boot fails.
     */
    int Boot();
    /**
     * @brief Save the currently loaded DSP settings to a settings file. 
     * @param fileName Name of file to save.
     * @return int  
     * @retval 0 Success.
     * @retval !=0 XIA API error code.
     */
    int SaveSetFile(char* fileName);
    /**
     * @brief Load a new settings file.
     * @param fileName  Settings file name we are attempting to open.
     * @return int  
     * @retval 0  Success.
     * @retval !=0  XIA API error code.
     */
    int LoadSetFile(char* fileName);
    /**
     * @brief Exit the system and release resources from the modules.
     * @return int  
     * @retval 0  Success.
     * @retval !=0  XIA API error code.
     */
    int ExitSystem();    
    /**
     * @brief Set the boot mode.
     * @warning Offline boot mode is currently only allowed for XIA API 2!
     * @param mode  Set the boot mode to this value.
     */
    void SetBootMode(int mode) {m_bootMode = mode;};
    /**
     * @brief Get the boot mode.
     * @warning Offline boot mode is currently only allowed for XIA API 2!
     * @return int The boot mode.
     * @retval 0 Online mode.
     * @retval 1 Offline mode (no hardware).
     */
    int GetBootMode() {return m_bootMode;};
    /**
     * @brief Get the crate boot status.
     * @return bool
     * @retval true   If the system has been booted.
     * @retval false  Otherwise.
     */
    bool GetBootStatus() {return m_booted;};
    /**
     * @brief Get the number of installed modules.
     * @return int  The number of modules in the crate.
     */
    int GetNumModules() {return m_numModules;};
    /**
     * @brief Get the module ADC sampling rate in MSPS.
     * @returns unsigned short The module ADC sampling rate in MSPS.
     * @throws std::runtime_error  If the module number is invalid.
     * @retval  0 if the module number is invalid.
     */
    unsigned short GetModuleMSPS(int module);
    
private:
    DAQ::DDAS::Configuration m_config; //!< Hardware configuration information.
    int m_bootMode; //!< Offline (1) or online (0) boot mode.
    bool m_booted;  //!< True when the system is booted, false otherwise.
    bool m_ovrSetFile; //!< True if a settings file has been re-loaded
                       //!< since boot.
    unsigned short m_numModules; //!< Number of modules in the crate.
    std::vector<int> m_modEvtLength; //!< Event length in 32 bit words.
    std::vector<unsigned short> m_modADCMSPS; //!< Sampling rate of a module.
    std::vector<unsigned short> m_modADCBits; //!< ADC bits of a module.
    std::vector<unsigned short> m_modRev; //!< Module revision in hex format.
    std::vector<unsigned short> m_modClockCal; //!< ns per clock tick.
};

/** @} */

extern "C" {
    /** @brief Wrapper for the class constructor. */
    CPixieSystemUtilities* CPixieSystemUtilities_new()
    {
	return new CPixieSystemUtilities();
    }

    /** @brief Wrapper to boot the crate. */
    int CPixieSystemUtilities_Boot(CPixieSystemUtilities* utils)
    {
	return utils->Boot();
    }
    /** @brief Wrapper to save a settings file. */
    int CPixieSystemUtilities_SaveSetFile(
	CPixieSystemUtilities* utils, char* fName
	)
    {
	return utils->SaveSetFile(fName);
    }
    /** @brief Wrapper to load a settings file. */
    int CPixieSystemUtilities_LoadSetFile(
	CPixieSystemUtilities* utils, char* fName
	)
    {
	return utils->LoadSetFile(fName);
    }
    /** @brief Wrapper to exit the system file. */
    int CPixieSystemUtilities_ExitSystem(CPixieSystemUtilities* utils)
    {
	return utils->ExitSystem();
    }

    /** @brief Wrapper to set the boot mode. */
    void CPixieSystemUtilities_SetBootMode(
	CPixieSystemUtilities* utils, int mode
	)
    {
	return utils->SetBootMode(mode);
    }
    /** @brief Wrapper to get the boot mode. */
    int CPixieSystemUtilities_GetBootMode(CPixieSystemUtilities* utils)
    {
	return utils->GetBootMode();
    }
    /** @brief Wrapper to get the boot status. */
    bool CPixieSystemUtilities_GetBootStatus(CPixieSystemUtilities* utils)
    {
	return utils->GetBootStatus();
    }
    /** @brief Wrapper to get the number of modules. */
    unsigned short CPixieSystemUtilities_GetNumModules(
	CPixieSystemUtilities* utils
	)
    {
	return utils->GetNumModules();
    }
    /** @brief Wrapper to get a single module ADC MSPS from the HW map. */
    unsigned short CPixieSystemUtilities_GetModuleMSPS(
	CPixieSystemUtilities* utils, int mod
	)
    {
	return utils->GetModuleMSPS(mod);
    }

    /** @brief Wrapper for the class destructor. */
    void CPixieSystemUtilities_delete(CPixieSystemUtilities* utils)
    {
	if(utils) {
	    delete utils;
	    utils = nullptr;
	}
    };
}

#endif
