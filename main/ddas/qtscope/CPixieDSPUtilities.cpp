/**
 * @file CPixieDSPUtilities.cpp
 * @brief Implementation DSP utilities class.
 */

#include "CPixieDSPUtilities.h"

#include <iostream>
#include <sstream>

#include <config.h>
#include <config_pixie16api.h>
#include <CXIAException.h>

int
CPixieDSPUtilities::AdjustOffsets(int module)
{
    int retval;
    try {
	retval = Pixie16AdjustOffsets(module);
	if (retval < 0) {
	    std::stringstream msg;
	    msg << "Failed to adjust offsets in module " << module;
	    throw CXIAException(msg.str(), "Pixie16AdjustOffsets()", retval);
	}
    }
    catch (const CXIAException& e) {
	std::cerr << e.ReasonText() << std::endl;
    }

    return retval;
}

/**
 * @details
 * Channel parameters are doubles. For a list of parameters and their units, 
 * see the Pixie-16 Programmers Manual, pgs. 60-61.
 */
int
CPixieDSPUtilities::WriteChanPar(
    int module, int channel, char* paramName, double value
    )
{
    int retval;
    try {
	retval = Pixie16WriteSglChanPar(paramName, value, module, channel);
	if (retval < 0) {
	    std::stringstream msg;
	    msg << "Failed to write channel parameter " << paramName
		<< " to module " << module << " channel " << channel;
	    throw CXIAException(msg.str(), "Pixie16WriteSglChanPar()", retval);
	}
    }
    catch (const CXIAException& e) {
	std::cerr << e.ReasonText() << std::endl;
    }
    
    return retval;
}

/**
 * @details
 * Channel parameters are doubles. For a list of parameters and their units, 
 * see the Pixie-16 Programmers Manual, pgs. 60-61.
 */
int
CPixieDSPUtilities::ReadChanPar(
    int module, int channel, char* paramName, double& value
    )
{
    int retval;
    try {
	retval = Pixie16ReadSglChanPar(paramName, &value, module, channel);
  	if (retval < 0) {
	    std::stringstream msg;
	    msg << "Failed to read channel parameter " << paramName
		<< " from module " << module << " channel " << channel;
	    throw CXIAException(msg.str(), "Pixie16ReadSglChanPar()", retval);
	}
    }
    catch (const CXIAException& e) {
	std::cerr << e.ReasonText() << std::endl;
    }
    
    return retval;
}

/**
 * @details
 * Module parameters are unsigned ints. For a list of parameters and their 
 * units, see the Pixie-16 Programmers Manual, pgs. 62-63. 
 */
int
CPixieDSPUtilities::WriteModPar(
    int module, char* paramName, unsigned int value
    )
{
    int retval;
    try {
	retval = Pixie16WriteSglModPar(paramName, value, module);
  	if (retval < 0) {
	    std::stringstream msg;
	    msg << "Failed to write module parameter " << paramName
		<< " to module " << module;
	    throw CXIAException(msg.str(), "Pixie16WriteSglModPar()", retval);
	}
    }
    catch (const CXIAException& e) {
	std::cerr << e.ReasonText() << std::endl;
    }
  
    return retval;
}

/**
 * @details
 * Module parameters are unsigned ints. For a list of parameters and their 
 * units, see the Pixie-16 Programmers Manual, pgs. 62-63. 
 */
int
CPixieDSPUtilities::ReadModPar(
    int module, char* paramName, unsigned int& value
    )
{
    int retval;
    try {
	retval = Pixie16ReadSglModPar(paramName, &value, module);
  	if (retval < 0) {
	    std::stringstream msg;
	    msg << "Failed to read module parameter " << paramName
		<< " from module " << module;
	    throw CXIAException(msg.str(), "Pixie16ReadSglModPar()", retval);
	}
    }
    catch (const CXIAException& e) {
	std::cerr << e.ReasonText() << std::endl;
    }
    
    return retval;
}
