/**
 * @file ModEvtFileParser.h
 * @brief Defines a parser for the modevtlen.txt file.
 */

#ifndef MODEVTFILEPARSER_H
#define MODEVTFILEPARSER_H

#include <iosfwd>

/** @namespace DAQ */
namespace DAQ {
    /** @namespace DAQ::DDAS */
    namespace DDAS {
	
	class Configuration;

	/**
	 * @addtogroup configuration libConfiguration.so
	 * @{
	 */
	
	/*!
	 * @class ModEvtFileParser ModEvtFileParser.h
	 * \brief A parser for the modevtlen.txt file.
	 *
	 * The modevtlen.txt contains the length of each event to expect from 
	 * each module. For each channel in a digitizer, the assumption is 
	 * that the of the same length will be emitted. That implies that each 
	 * channel will be configured to have the same settngs for trace 
	 * capture, QDC, energy summing, and external clock. If that is not 
	 * the case, Readout programs will fail miserably!
	 *
	 * The structure of the file is very simple. There should be a line 
	 * for each module in the crate with a single integer value 
	 * representing the length of the event in units of 32-bit integers. 
	 * The minimum value any line can have is 4, because that is the 
	 * minimum length of data a module can output for each channel.
	 *
	 * It is not an error for the file to contain more lines than there 
	 * are modules in the system. It _IS_ an error for the file to contain 
	 * fewer lines than there are in the system.
	 */
	class ModEvtFileParser
	{
	public:
	    /** @brief Constructor. */
	    ModEvtFileParser() = default;
	    /** 
	     * \brief Parse and store the contents of the modevtlen.txt file 
	     *   in a configuration object
	     * \param input  The stream from which the contents of the file 
	     *   are read.
	     * \param config The configuration to store the results in.
	     * \throws std::runtime_error If fewer lines than there are modules
	     *    are found.
	     * \throws std::runtime_error If a line is encountered with a value
	     *    less than 4.
	     */
	    void parse(std::istream& input, Configuration& config);
	};

	/** @} */

    } // end DDAS namespace
} // end DAQ namespace

#endif // MODEVTFILEPARSER_H

