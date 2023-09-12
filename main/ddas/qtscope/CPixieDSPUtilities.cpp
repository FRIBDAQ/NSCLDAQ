/**
 * @file CPixieDSPUtilities.cpp
 * @brief Implementation DSP utilities class.
 */

#include "CPixieDSPUtilities.h"

#include <iostream>
#include <sstream>

#include <config.h>
#include <config_pixie16api.h>

int
CPixieDSPUtilities::AdjustOffsets(int module)
{
    int retval = Pixie16AdjustOffsets(module);

    if (retval < 0) {
	std::stringstream errmsg;
	errmsg << "CPixieDSPUtilities::AdjustOffsets() failed";
	errmsg << " to adjust offsets in module: " << module
	       << " with retval " << retval;
	std::cerr << errmsg.str() << std::endl;
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
    int retval = Pixie16WriteSglChanPar(paramName, value, module, channel);
  
    if (retval < 0) {
	std::stringstream errmsg;
	errmsg << "CPixieDSPUtilities::WriteChanPar() failed";
	errmsg << " to write parameter " << paramName
	       << " to module " << module
	       << " channel " << channel
	       << " with retval " << retval;
	std::cerr << errmsg.str() << std::endl;
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
    int retval = Pixie16ReadSglChanPar(paramName, &value, module, channel);
  
    if (retval != 0) {
	std::stringstream errmsg;
	errmsg << "CPixieDSPUtilities::ReadChanPar() failed";
	errmsg << " to read parameter " << paramName
	       << " from module " << module
	       << " channel " << channel
	       << " with retval " << retval;
	std::cerr << errmsg.str() << std::endl;
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
    int retval = Pixie16WriteSglModPar(paramName, value, module);
  
    if (retval < 0) {
	std::stringstream errmsg;
	errmsg << "CPixieDSPUtilities::WriteModPar() failed";
	errmsg << " to write parameter " << paramName
	       << " to module " << module
	       << " with retval " << retval;
	std::cerr << errmsg.str() << std::endl;
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
    int retval = Pixie16ReadSglModPar(paramName, &value, module);
  
    if (retval != 0) {
	std::stringstream errmsg;
	errmsg << "CPixieDSPUtilities::ReadModPar() failed";
	errmsg << " to read parameter " << paramName
	       << " from module " << module
	       << " with retval " << retval;
	std::cerr << errmsg.str() << std::endl;
    }
 
    return retval;
}
