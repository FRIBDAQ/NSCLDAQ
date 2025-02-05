/**
 * @file SystemBooter.h
 * @brief Defines a class to manage the booting process for DDAS.
 */

#ifndef SYSTEMBOOTER_H
#define SYSTEMBOOTER_H

#include <stddef.h>

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
	 * the system. There are two separate boot types: FullBoot and 
	 * SettingsOnly. The former loads firmware and settings into the 
	 * system while the latter just loads the settings. The basic usage 
	 * pattern is demonstrated below.
	 *
	 * @code
	 * using namespace DAQ::DDAS;
	 * unique_ptr<Configuration> pConfig = Configuration::generator(
	 *     "DDASFirmwareVersions.txt", "cfgPixie16.txt"
	 * );
	 * SystemBooter booter;
	 * booter.boot(*pConfig, SystemBooter::FullBoot);
	 * @endcode
	 *
	 * One should realize that this does not handle any of the logic 
	 * regarding when and when not to synchronize or load firmware. 
	 * External logic to this class will determine whether the system 
	 * should load the firmware or not. Synchronization is unrelated to 
	 * the boot process besides the fact that a firmware load could ruin
	 * synchronization.
	 */
      
	class SystemBooter
	{
	public:
	    /** @brief An enum for boot type bitmasks. */
	    enum BootType {
		FullBoot = 0x7f, //!< Bitmask for full boot with firmware load.
		SettingsOnly = 0x70 //!< Bitmask for boot with settings only.
	    }; 

	private:
	    bool m_verbose; //!< Enable or disable output.
	    /** 
	     * 0 for online, 1 for offline (no hardware). Only supported for 
	     * XIA API v2 as of 3/13/24.
	     */ 
	    unsigned short m_offlineMode;
	    
	public:
	    /** @brief Constructor. */
	    SystemBooter();
	    /**
	     * @brief Boot the entire system with hardware.
	     * @param config A configuration describing the system.
	     * @param type Style of boot.
	     * @throw CXIAException If `Pixie16InitSystem()` call returns 
	     *   an error.
	     * @throw CXIAException If `populateHardwareMap()` throws.
	     * @throw std::runtime_error If registered hardware is 
	     *   unrecognized when attempting to boot.
	     * @throws CXIAException If the Pixie boot fails.
	     */
	    void boot(Configuration& config, BootType type);
	    /**
	     * @brief Boot a single module
	     * @param modIndex Index of the module in the system.
	     * @param m_config The system configuration.
	     * @param type Boot style (load firmware or settings only).
	     * @throw std::runtime_error If hardware type is unknown.
	     * @throw CDDASException If `Pixie16BootModule()` returns an error.
	     */
	    void bootModuleByIndex(
		int modIndex, Configuration& config, BootType type
		);
	    /**
	     * @brief Enable or disable verbose output
	     * @param enable Enables output messages if true.
	     */
	    void setVerbose(bool enable);
	    /**
	     * @brief Return the state of verbosity.
	     * @return The state.
	     */
	    bool isVerbose() const { return m_verbose; };
	    /**
	     * @brief Enable or disable online boot
	     * @param mode Boot mode, 0 for online, 1 for offline.
	     * @warning Offline boot mode is only supported in XIA API v2!
	     */
	    void setOfflineMode(unsigned short mode);
	    /**
	     * @brief Return the boot mode of the system.
	     * @return The boot mode.
	     */
	    unsigned short getOfflineMode() const { return m_offlineMode; };
	    /**
	     * @brief Read and store hardware info from each of the modules 
	     * in the system.
	     * @param config The system configuration.
	     * @throw CDDASException If `Pixie16ReadModuleInfo()` returns 
	     *   an error.
	     */
	    void populateHardwareMap(Configuration &config);
	    
	private:
	    /**
	     * @brief Convert BootType enumeration to usable boot mask.
	     * @param type Either `BootType::FullBoot` or 
	     *   `BootType::SettingsOnly`.
	     * @return unsigned int  
	     * @retval 0x7f FullBoot
	     * @retval 0x70 SettingsOnly.
	     */
	    unsigned int computeBootMask(BootType type);
	    /**
	     * @brief Print out some basic information regarding the module
	     * @param modIndex   Index of the module in the system.
	     * @param ModRev     Hardware revision.
	     * @param ModSerNum  Serial number.
	     * @param ModADCBits ADC resolution (number of bits).
	     * @param ModADCMSPS ADC frequency (MSPS).
	     */
	    void logModuleInfo(
		int modIndex, unsigned short ModRev, unsigned short ModSerNum,
		unsigned short ModADCBits, unsigned short ModADCMSPS
		);
	    /**
	     * @brief Check that the firmware path lengths can be copied 
	     * into fixed-length arrays of size maxLen.
	     * @param fwConfig Refernces the FW configuration.
	     * @param maxLen   Maximum file path length.
	     * @throw std::length_error Some path is too long.
	     */
	    void checkFWPathLengths(
		FirmwareConfiguration& fwConfig, const size_t maxLen
		);
	};
	
	/** @} */

    } // end DDAS namespace
} // end DAQ namespace

#endif // SYSTEMBOOTER_H

