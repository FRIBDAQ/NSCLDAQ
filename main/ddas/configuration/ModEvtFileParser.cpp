/**
 * @file ModEvtFileParser.cpp
 * @brief Implementation of the modevtlen.txt file parser.
 */

#include "ModEvtFileParser.h"

#include <iostream>

#include "Configuration.h"

/*!
 * @details
 * The parser will read in as many lines as the value returned by
 * config.getNumberOfModules(). For that reason, the caller must have already
 * set the number of modules in the configuration object.
 */
void
DAQ::DDAS::ModEvtFileParser::parse(
    std::istream &input, DAQ::DDAS::Configuration &config
    )
{
    int NumModules = config.getNumberOfModules();
    std::vector<int> modEvtLenData(NumModules);

    for(int i=0; i<NumModules; i++) {
	input >> modEvtLenData[i];

	if (input.fail() || input.bad()) {
	    std::string errmsg = "Failure while reading module event length ";
	    errmsg += "configuration file. Expected "
		+ std::to_string(NumModules);
	    errmsg += "entries but found only " + std::to_string(i) + ".";
	    throw std::runtime_error(errmsg);
	}
	if (modEvtLenData[i] < 4) {
	    std::string errmsg(
		"Failure while reading module event length "
		);
	    errmsg += "configuration file. Found event length ";
	    errmsg += std::to_string(modEvtLenData[i]);
	    errmsg += " less than 4.";
	    throw std::runtime_error(errmsg);
	}
    }

    config.setModuleEventLengths(modEvtLenData);
}
