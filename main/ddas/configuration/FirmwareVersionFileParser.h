/**
 * @file FirmwareVersionFileParser.h
 * @brief Defines the class used to parse the DDASFirmwareVersion.txt file 
 * defnining the firmware and DSP configuration code used by the Pixie modules.
 */

#ifndef FIRMWAREVERSIONFILEPARSER_H
#define FIRMWAREVERSIONFILEPARSER_H

#include <iosfwd>
#include <regex>

#include "Configuration.h"

/** @namespace DAQ */
namespace DAQ {
    /** @namespace DAQ::DDAS */
    namespace DDAS {

	/**
	 * @addtogroup configuration libConfiguration.so
	 * @{
	 */
	
	/**
	 * @class FirmwareVersionFileParser FirmwareVersionFileParser.h
	 * @brief The FirmwareVersionFileParser class.
	 *
	 * The FirmwareVersionFileParser is designed to parse the
	 * DDASFirmwareVersions.txt file that is installed by the project. 
	 * The DDASFirmwareVersions.txt file has two major sections. The top 
	 * section contains fpga firmware file paths and the bottom section 
	 * provides the paths to dsp configuration code. The format of this
	 * file can be observed in DDASFirmwareVersions.txt.in , which is the 
	 * template file used by automake to generate the
	 * DDASFirmwareVersion.txt file.
	 *
	 * Ultimately, the contents of the DDASFirmwareVersions.txt file will 
	 * be stored in the Configuration object passed in as an argument to 
	 * the parse() method. That object will keep a database of the firmware
	 * files organized by their associated hardware type.
	 */
    	class FirmwareVersionFileParser
	{
	    std::regex m_matchExpr; //!< Expression for pattern-matching. 
	    
	public:
	    /** @brief Constructor. */
	    FirmwareVersionFileParser();
	    /**
	     * @brief Main entry point for parsing DDASFirmwareVersions.txt.
	     * @param input  The stream to read file content from.
	     * @param config The FirmwareMap in which to store this.
	     * @throw std::runtime_error If the DDASFirmwareVersionFile.txt is 
	     *   missing any expected field.
	     */
	    void parse(std::istream& input, FirmwareMap& config);
	};
	
	/** @} */

    } // end DDAS namespace
} // end DAQ namespace

#endif // FIRMWAREVERSIONFILEPARSER_H
