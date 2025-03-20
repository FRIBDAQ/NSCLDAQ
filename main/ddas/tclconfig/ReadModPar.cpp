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

/** @file:  ReadModPar.cpp
 *  @brief: Reads a module parameter (implementation).
 */
#include "ReadModPar.h"
#include <config.h>
#include <config_pixie16api.h>
#include <sstream>

static const char* errorMessages[3] = {
    "Success",
    "Invalid module number",
    "Invalid module parameter name"
};

/**
 * constructor
 *   @param pInterp - pointer to the interpreter.
 */
CReadModPar::CReadModPar(Tcl_Interp* pInterp) :
    CTclCommand(pInterp, "pixie16::readmodpar")
{}
/**
 * destructor
 */
CReadModPar::~CReadModPar()
{
    
}
/**
 * operator()
 *     Executes the command
 *  @param objv - vector of command words.
 *  @return int - TCL_OK On success, TCL_ERROR on failure./
 *  @note On success, the result is an integer vaule read from the
 *        module.  On failure, the result is a textual error message
 */
int
CReadModPar::operator()(std::vector<Tcl_Obj*>& objv)
{
    int modnum;
    
    try {
        unsigned int data;
        requireExactly(objv, 3);
        
        modnum = getInteger(objv[1]);
        std::string parname(Tcl_GetString(objv[2]));
        
        int status = Pixie16ReadSglModPar(parname.c_str(), &data, modnum);
        if (status) throw -status;
        
        setResult(static_cast<int>(data));
    }
    catch (std::string msg) {
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    catch (int status) {
        std::string msg = apiMessage(modnum, status);
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
//////////////////////////////////////////////////////////////////

/**
 * apiMessage
 *   @param module -number of the module.
 *   @param status - status value.
 *   @return human readable error message.
 */
std::string
CReadModPar::apiMessage(int module, int status)
{
    std::stringstream s;
    s << "Error reading module parameter from module number "
        << module << ": " << errorMessages[status];
        
    std::string result(s.str());
    return result;
}
