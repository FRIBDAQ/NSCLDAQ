/**
 * @file SystemBooter.cpp
 * @brief Implementation of the system booter class for DDAS.
 */

#include "SystemBooter.h"

#include <unistd.h>
#include <string.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <config.h>
#include <config_pixie16api.h>
#include <Configuration.h>
#include <CXIAException.h>

const size_t FILENAME_STR_MAXLEN = 256;

/**
 * @details
 * Enables verbose output by default.
 */
DAQ::DDAS::SystemBooter::SystemBooter() :
    m_verbose(true), m_offlineMode(0)
{}

/**
 * @details
 * Provided a configuration, all modules will be booted. The configuration 
 * contains the firmware files for each hardware type, the slot map, and the 
 * number of modules. During the course of booting, the hardware will be 
 * queried as well to determine the the serial number, revision, ADC 
 * frequency, and resolution. The revision, ADC frequency, and resolution 
 * will all be parsed and the information will be stored in the configuration
 * as a HardwareRegistry::HardwareType.
 *
 * XIA's "offline mode" consists of an emulated crate containing, at the 
 * moment, 4 modules of "common" type, including a 32-channel module in the 
 * last slot. If offline mode is enabled, the crate simulation is started, 
 * otherwise attempt to boot physical hardware.
 */
void DAQ::DDAS::SystemBooter::boot(Configuration &config, BootType type)
{
    if (m_offlineMode) {
	std::cout << "---------------------------\n";
	std::cout << "Initializing Pixie crate simulation... \n";
	std::cout.flush();
	int nModules = 4;
	int retval = Pixie16InitSystem(nModules, nullptr, 1);
	if (retval < 0) {
	    throw CXIAException("SystemBooter::boot() failed",
				"Pixie16InitSystem()", retval);
	} else {
	    std::cout << "Crate simulation initialized successfully."
		      << std::endl;
	}

	populateHardwareMap(config);

	char parFile[FILENAME_STR_MAXLEN];
	for (int i = 0; i < nModules; i++) {
	    std::cout << "\nBooting simulated module #" << i << std::endl;
	    strcpy(parFile, config.getSettingsFilePath(i).c_str());
	    retval = Pixie16BootModule("sys.bin", "fippi.bin", nullptr,
				       "dsp.ldr", parFile, "dsp.var", i,
				       computeBootMask(type));
	    if (retval < 0) {
		throw CXIAException("SystemBooter::boot() failed",
				    "Pixie16BootModule()", retval);
	    }
	}
	std::cout << "Crate simulation: all modules ok" << std::endl;
    } else {    
	std::cout << "---------------------------\n";
	std::cout << "Initializing PXI access... \n";
	std::cout.flush();

	int NumModules = config.getNumberOfModules();
	int retval = Pixie16InitSystem(
	    NumModules, config.getSlotMap().data(), 0
	    );
	if (retval < 0) {
	    throw CXIAException(
		"SystemBooter::boot() failed", "Pixie16InitSystem()", retval
		);
	} else {
	    std::cout << "System initialized successfully." << std::endl;
	}

	// Give the system some time to settle after initialization.
	usleep(1000);

	populateHardwareMap(config);

	for (int index = 0; index < NumModules; ++index) {
	    bootModuleByIndex(index, config, type);
	}

	if (m_verbose) {
	    std::cout << "All modules ok" << std::endl;
	}
    }
}

/**
 * @details
 * The system is booted into a usable state. The mechanics of booting involve
 * either loading firmware and settings or just settings, depending on the type
 * parameter that was passed as a second argument to the method. If the user
 * chooses to boot with a firmware load, the firmware files stored in the
 * configuration associated with the hardware will be used. The settings
 * file that will be used in any boot type, will be the path stored in the
 * configuration.
 */
void
DAQ::DDAS::SystemBooter::bootModuleByIndex(
    int modIndex, Configuration& config, BootType type
    )
{
    char Pixie16_Com_FPGA_File[FILENAME_STR_MAXLEN];
    char Pixie16_SP_FPGA_File[FILENAME_STR_MAXLEN];
    char Pixie16_DSP_Code_File[FILENAME_STR_MAXLEN];
    char Pixie16_DSP_Var_File[FILENAME_STR_MAXLEN];
    char Pixie16_Trig_FPGA_File[FILENAME_STR_MAXLEN];
    char DSPParFile[FILENAME_STR_MAXLEN];

    // Select firmware and dsp files based on hardware variant
    std::vector<int> hdwrMap = config.getHardwareMap();
    if (hdwrMap[modIndex] == HardwareRegistry::Unknown) {
	std::stringstream errmsg;
	errmsg << "Cannot boot module " << modIndex
	       << ", hardware type not recognized." << std::endl;
	throw std::runtime_error(errmsg.str());
    }

    // Because the Pixie16BootModule takes char* strings, we have to copy our
    // beautiful std::strings into the character arrays. Note that there is
    // no check at the moment to ensure that the firmware file paths are no
    // more than 256 characters. daqdev/DDAS#106 - modified to get the per
    // module firmware configuration which will default to the global config
    // if not specified.
    
    FirmwareConfiguration fwConfig = config.getModuleFirmwareConfiguration(
	hdwrMap[modIndex], modIndex
	);

    checkFWPathLengths(fwConfig, FILENAME_STR_MAXLEN);
    
    strcpy(Pixie16_Com_FPGA_File, fwConfig.s_ComFPGAConfigFile.c_str());
    strcpy(Pixie16_SP_FPGA_File,  fwConfig.s_SPFPGAConfigFile.c_str());
    strcpy(Pixie16_DSP_Code_File, fwConfig.s_DSPCodeFile.c_str());
    strcpy(Pixie16_DSP_Var_File,  fwConfig.s_DSPVarFile.c_str());

    // daqdev/DDAS#106 - modified as above to get a per module setfile:
    
    strcpy(DSPParFile, config.getSettingsFilePath(modIndex).c_str());

    if (m_verbose) {
	if (type == FullBoot) {
	    std::cout << "\nBooting Pixie-16 module #"
		      << modIndex << std::endl;
	    std::cout << "\tComFPGAConfigFile:  "
		      << Pixie16_Com_FPGA_File << std::endl;
	    std::cout << "\tSPFPGAConfigFile:   "
		      << Pixie16_SP_FPGA_File << std::endl;
	    std::cout << "\tDSPCodeFile:        "
		      << Pixie16_DSP_Code_File << std::endl;
	    std::cout << "\tDSPVarFile:         "
		      << Pixie16_DSP_Var_File << std::endl;
	    std::cout << "\tDSPParFile:         "
		      << DSPParFile << std::endl;
	    std::cout
		<< "------------------------------------------------------";
	    std::cout << "\n\n";
	} else {
	    std::cout << "\nEstablishing communication parameters "
		      << "with module #" << modIndex << std::endl;
	    std::cout << "\tSkipping firmware load." << std::endl;
	}
    }

    // Arguments are:
    // 0) Name of communications FPGA config. file
    // 1) Name of signal processing FPGA config. file
    // 2) Placeholder name of trigger FPGA configuration file
    // 3) Name of executable code file for digital signal processor (DSP)
    // 4) Name of DSP parameter file
    // 5) Name of DSP variable names file
    // 6) Pixie module number
    // 7) Fast boot pattern bitmask

    int retval = Pixie16BootModule(
	Pixie16_Com_FPGA_File, Pixie16_SP_FPGA_File, Pixie16_Trig_FPGA_File,
	Pixie16_DSP_Code_File, DSPParFile, Pixie16_DSP_Var_File,
	modIndex, computeBootMask(type)
	);
    if (retval < 0) {	
	std::string msg = "Boot failed module " + std::to_string(modIndex);
	throw CXIAException(msg, "Pixie16BootModule()", retval);
    }
}

/**
 * @details
 * By default, the output verbosity setting is enabled. If it is disabled,
 * there will be no output printed to the terminal.
 */
void DAQ::DDAS::SystemBooter::setVerbose(bool enable)
{
    m_verbose = enable;
}

/**
 * @details
 * By default, the boot mode is set to online mode. If it is set
 * to offline mode, calls to the API functions can still be tested
 * with no modules present.
 */
void DAQ::DDAS::SystemBooter::setOfflineMode(unsigned short mode)
{
    m_offlineMode = mode;
}

/**
 * @details
 * To retrieve information about all of the modules in the system, 
 * Pixie16ReadModuleInfo is called for each module index. The resulting 
 * revision number, ADC bits, and ADC frequency is printed (if verbose output
 * enabled) and the hardware mapping is stored in the configuration that was
 * passed in.
 */
void DAQ::DDAS::SystemBooter::populateHardwareMap(Configuration &config)
{
    unsigned short ModRev;
    unsigned int   ModSerNum;
    unsigned short ModADCBits;
    unsigned short ModADCMSPS;
    unsigned short nchannels;

    int NumModules = config.getNumberOfModules();
    std::vector<int> hdwrMapping(NumModules);

    /** 
     * @todo (ASC 12/14/23): For the API transition we want to read the 
     * module_config struct. We may not even need to log it (just put them 
     * in a vector) 
     */
    for(unsigned short i = 0; i < NumModules; i++) {
	int retval = Pixie16ReadModuleInfo(
	    i, &ModRev, &ModSerNum, &ModADCBits, &ModADCMSPS
	    );
	if (retval < 0)
	{
	    std::string msg = "Failed to read hardware variant module " + i;
	    throw CXIAException(msg, "Pixie16ReadModuleInfo()", retval);
	} else {
	    if (m_verbose) {
		logModuleInfo(i, ModRev, ModSerNum, ModADCBits, ModADCMSPS);
	    }
	    auto type = HardwareRegistry::computeHardwareType(
		ModRev, ModADCMSPS, ModADCBits
		);
	    hdwrMapping[i] = type;
	}
    }

    // Store the hardware map in the configuration so other components of the
    // program can understand more about the hardware being used.
    config.setHardwareMap(hdwrMapping);
}

///
// Private methods
//

/**
 * @todo (ASC 7/7/23): Lots of arguments to this function. Can we pack info 
 * into a struct and pass it around that way?
 */
void DAQ::DDAS::SystemBooter::logModuleInfo(
    int modIndex, unsigned short ModRev, unsigned short ModSerNum,
    unsigned short ModADCBits, unsigned short ModADCMSPS
    )
{
    std::cout << "Found Pixie-16 module #" << modIndex;
    std::cout << ", Rev = " << ModRev;
    std::cout << ", S/N = " << ModSerNum << ", Bits = " << ModADCBits;
    std::cout << ", MSPS = " << ModADCMSPS;
    std::cout << std::endl;
}

/**
 * @details
 * The bootmask is ultimately used in the Pixie16BootModule function.
 */
unsigned int DAQ::DDAS::SystemBooter::computeBootMask(BootType type)
{
    if (type == FullBoot) {
	return 0x7f;
    } else {
	return 0x70;
    }
}

/**
 * @details
 * Fails on first bad path length encountered.
 *
 * @todo (ASC 5/23/24): Unit test needed.
 */
void DAQ::DDAS::SystemBooter::checkFWPathLengths(
    FirmwareConfiguration& fwConfig, const size_t maxLen
    ) {

    std::vector<std::string> paths = {
	fwConfig.s_ComFPGAConfigFile, fwConfig.s_SPFPGAConfigFile,
	fwConfig.s_DSPCodeFile, fwConfig.s_DSPVarFile
    };

    for (const auto& path : paths) {
	if (path.size() > maxLen) {
	    std::stringstream msg;
	    msg << "FW file path " << path << " must be less than "
		<< maxLen << " characters but is " << path.size();
	    throw std::length_error(msg.str());
	}
    }
}
