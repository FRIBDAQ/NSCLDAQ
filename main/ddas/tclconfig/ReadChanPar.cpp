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

/** @file:  ReadChanPar.cpp
 *  @brief: Implement the readchanpar command.
 */
#include "ReadChanPar.h"
#include <config.h>
#include <config_pixie16api.h>
#include <sstream>

static const char* apiMessages[4] = {
  "Success",
  "Invalid Pixie16 module number",
  "Invalid Pixie16 channel number",
  "Invalid Pixie16 parameter name"
};


/**
 * constructor
 *   @param pInterp - tcl interpreter we're registering on.
 */
CReadChanPar::CReadChanPar(Tcl_Interp* pInterp) :
    CTclCommand(pInterp, "pixie16::readchanpar")
{}

/**
 * destructor
 */
CReadChanPar::~CReadChanPar() {}

/**
 * operator()
 *   - Get the module and channel number
 *   - Get the parameter name string.
 *   - Request the value and set it as the result.
 * @param objv - the command words.
 * @return int - TCL_OK on success, TCL_ERROR on failure.
 */
int
CReadChanPar::operator()(std::vector<Tcl_Obj*>& objv)
{
    int index;
    int chan;
    try {
        requireExactly(objv, 4); // command module slot parname
        index = getInteger(objv[1]);
        chan  = getInteger(objv[2]);
        const char* name = Tcl_GetString(objv[3]);
        
        double result;
        int status = Pixie16ReadSglChanPar(name, &result, index, chan);
        if (status) throw -status;
        
        setResult(result);
        
    }
    catch (std::string msg) {
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    catch (int status) {
        std::string msg = apiMsg(index, chan, status);
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
/////////////////////////////////////////////////////////////

/**
 * apiMsg
 *    Return string describing the error from ReadSglChanPar.
 * @param index - module number.
 * @param chan  - channel number.
 * @param status - status code.
 * @return std::string
 */
std::string
CReadChanPar::apiMsg(int index, int chan, int status)
{
    std::stringstream s;
    s << "Pixie16ReadSglChanPar failed for module number "
        << index << " channel " << chan << ": "
        << apiMessages[status];
        
    std::string result = s.str();
    return result;
}
