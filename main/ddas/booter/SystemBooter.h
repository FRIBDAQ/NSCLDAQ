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
	 * the system.
	 * @todo (ASC 3/20/25): Implement API 4 parallal boot and document
	 * the changes to the boot class.
	 */
      
	class SystemBooter
	{
	public:
	    /** @brief An enum for boot type bitmasks. */
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
	     * @todo (ASC 3/20/25): Work in progress.
	     */
	    void boot(Configuration& config, BootType type);
	    
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
	     * @param mode Boot mode: 0 for online, 1 for offline.
	     */
	    void setOfflineMode(unsigned short mode) { m_offlineMode = mode; };
	    /**
	     * @brief Return the boot mode of the system.
	     * @return Boot mode: 0 for online, 1 for offline.
	     */
	    unsigned short getOfflineMode() const { return m_offlineMode; };

	private:
	    void initSystem(Configuration& config, BootType type);
	    void initSystemOffline(Configuration& config, BootType type);
	    void parallelBoot(Configuration& config, BootType type);
	    void serialBoot(Configuration& config, BootType type);
	    void offlineBoot(Configuration& config, BootType type);
	    /**
	     * @brief Read and store hardware info from each of the modules 
	     * in the system.
	     * @param config The system configuration.
	     * @throw CDDASException If `Pixie16ReadModuleInfo()` returns 
	     *   an error.
	     */
	    void populateHardwareMap(Configuration &config);
	    /**
	     * @brief Print out some basic information regarding the module
	     * @param config The system configuration.
	     */
	    void logModuleInfo(Configuration& config);
	};
	
	/** @} */

    } // end DDAS namespace
} // end DAQ namespace

#endif // SYSTEMBOOTER_H

