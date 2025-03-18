/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  WriteChanPar.cpp
 *  @brief: Implemennt pixie16::writechanpar.
 */

#include "WriteChanPar.h"
#include <sstream>
#include <config.h>
#include <config_pixie16api.h>

static const char* errorMessages[7] = {
    "Success",
    "Invalid module number",
    "Invalid channel number",
    "Invalid channel parameter name",
    "Fippi programming failed",
    "Failed to find Baseline cut",
    "SetDACS failed"
};

/**
 * constructor
 * @param pInterp - interpreter on which to register the command.
 */
CWriteChanPar::CWriteChanPar(Tcl_Interp* pInterp) :
    CTclCommand(pInterp, "pixie16::writechanpar")
    {}
    
CWriteChanPar::~CWriteChanPar() {}

/**
 * operator()
 *    Execute the command
 *  @param objv - the command words.
 *  @return int - TCL_OK On success, TCL_ERROR if failed.
 *  @note the interpreter result is only non-null if the command
 *        failed, in which case it's the human readable failure reason
 */
int
CWriteChanPar::operator()(std::vector<Tcl_Obj*>& objv)
{
    int module;
    int chan;
    
    try {
        requireExactly(objv, 5);
        module = getInteger(objv[1]);
        chan   = getInteger(objv[2]);
        const char* name = Tcl_GetString(objv[3]);
        double value = getDouble(objv[4]);
        
        int status = Pixie16WriteSglChanPar(
            name, value, module, chan
        );
        if (status) throw - status;
    }
    catch (std::string msg) {
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    catch (int status) {
        std::string msg = apiMessage(module, chan, status);
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
/////////////////////////////////////////////////////////////

/**
 * apiMessage
 *    Return a message appropriate to a failed. WriteSglChanPar.
 *
 *  @param mod - module number
 *  @param chan - channel number.
 *  @param status - status code negated.
 *  @return std::string - human readable status message.
 */
std::string
CWriteChanPar::apiMessage(int mod, int chan, int status)
{
    std::stringstream s;
    s << "Pixie16WriteSglChanPar failed in module: " << mod
      << " channel " << chan << ": " << errorMessages[status];
    std::string result(s.str());
    return result;
}
