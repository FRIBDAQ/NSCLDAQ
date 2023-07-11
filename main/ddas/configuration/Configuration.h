/**
 * @addtogroup configuration libConfiguration.so
 * @brief DDAS Pixie-16 hardware configuration library.
 *
 * Shared library containing classes to manage the internal configuration of a 
 * DDAS system and store information about its hardware. Contains all functions
 * defined in the DAQ::DDAS::HardwareRegistry namespace.
 * @{
 */

/**
 * @file Configuration.h
 * @brief Defines a class for storing system configuration information.
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <vector>
#include <map>
#include <iosfwd>
#include <memory>

#include "HardwareRegistry.h"

/** @namespace DAQ */
namespace DAQ {
    /** @namespace DAQ::DDAS */
    namespace DDAS {

/*!
 * \brief The FirmwareConfiguration struct
 *
 * A simple structure to hold the paths to all firmware/settings files
 * for a specific hardware type. These objects will be stored in a map
 * of the Configuration class and keyed by a hardware type.
 */
	struct FirmwareConfiguration
	{
	    std::string s_ComFPGAConfigFile; //!< Name of communications FPGA config. file.
	    std::string s_SPFPGAConfigFile; //!< Name of signal processing FPGA config. file.
	    std::string s_DSPCodeFile; //!< Name of executable code file for digital signal processor (DSP)
	    std::string s_DSPVarFile; //!< Name of DSP variable names file
	};

	/** 
	 * @typedef FirmwareMap
	 * @brief A map of firmware configurations keyed by the hardware type.
	 */
	typedef std::map<int, FirmwareConfiguration> FirmwareMap;

/*!
 * \brief The Configuration class
 *
 * The Configuration class stores all of the system configuration for
 * a Readout program. It maintains the configuration that is read in
 * from the DDASFirmwareVersion.txt, modevtlen.txt, and cfgPixie16.txt
 * configuration files. The configuration therefore keeps track of
 * the crate id, slot map, setting file path, module event lengths,
 * module count, and all of the available firmware files for each 
 * hardware type.
 *
 * It can be configured either manually or by passing it as an argument to
 * a ConfigurationParser::parse(), FirmwareVersionFileParser::parse(), or
 * ModEvtFileParser::parse() methods as it is in the Readout programs.
 *
 * At the moment, modules are expected to output events of equal length for
 * all channels. There is no attempt to read out channels with different
 * lengths in a module.
 */
	class Configuration
	{
	private:
	    int m_crateId; ///< Crate ID from cfgPixie16 (not necessarily what is in settings file).
	    std::vector<unsigned short> m_slotMap; ///< Mapping of what slots are occupied.
	    std::string m_settingsFilePath; ///< Path to default .set file.
	    std::vector<int> m_modEvtLengths; ///< Event length for each module (units of 32-bit integers).

    
	    FirmwareMap m_fwMap; ///< Default map of firmware configuration from hardware type.
	    std::vector<int> m_hardwareMap; ///< Map of HardwareRegistry types.
    
	    // These additions support per module firmware maps and .set files:
    
	    std::map<int, FirmwareMap> m_moduleFirmwareMaps; ///< Per module firmwar maps.
	    std::map<int, std::string> m_moduleSetFileMap; ///< Per module setfiles.

	public:
	    Configuration() = default;
	    /** @brief Copy constructor. */
	    Configuration(const Configuration& rhs) :
		m_crateId(rhs.m_crateId), m_slotMap(rhs.m_slotMap),
		m_settingsFilePath(rhs.m_settingsFilePath),
		m_modEvtLengths(rhs.m_modEvtLengths),
		m_fwMap(rhs.m_fwMap), m_hardwareMap(rhs.m_hardwareMap),
		m_moduleFirmwareMaps(rhs.m_moduleFirmwareMaps),
		m_moduleSetFileMap(rhs.m_moduleSetFileMap)
		{}
        
	    ~Configuration() = default;

	    void setCrateId(int id);
	    int getCrateId() const;

	    void setNumberOfModules(size_t size);
	    size_t getNumberOfModules() const;

	    void setSlotMap(const std::vector<unsigned short>& map);
	    std::vector<unsigned short> getSlotMap() const;

	    void setSettingsFilePath(const std::string& path);   
	    std::string getSettingsFilePath() const; // Default
    
	    // daqdev/DDAS#106
	    void setModuleSettingsFilePath(int modNum, const std::string& path);
	    std::string getSettingsFilePath(int modNum); // Per module.
	    //

	    void setFirmwareConfiguration(
		int specifier, const FirmwareConfiguration &config
		);
	    FirmwareConfiguration& getFirmwareConfiguration(int hdwrType);
    
	    // daqdev/DDAS#106
	    void setModuleFirmwareMap(int module, const FirmwareMap& mapping);
	    FirmwareConfiguration& getModuleFirmwareConfiguration(
		int hdwrType, int modnum
		);
	    /**
	     * @brief Return the default map of firmware information.
	     * @return FirmwareMap  The firmware map.
	     */ 
	    FirmwareMap& getDefaultFirmwareMap() { return m_fwMap; }
	    //
    
	    void setModuleEventLengths(const std::vector<int>& lengths);
	    std::vector<int> getModuleEventLengths() const;

	    void setHardwareMap(const std::vector<int>& map);
	    std::vector<int> getHardwareMap() const;

	    void print(std::ostream& stream);

	    static std::unique_ptr<Configuration>
	    generate(
		const std::string& fwVsnPath, const std::string& cfgPixiePath
		);

	    static std::unique_ptr<Configuration>
	    generate(
		const std::string& fwVsnPath, const std::string& cfgPixiePath,
		const std::string& modEvtLenPath
		);

	};

    } // end DDAS namespace
} // end DAQ namesapce

#endif // CONFIGURATION_H

/** @} */
