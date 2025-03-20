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

/** @file:  RestoreParams.cpp
 *  @brief: Implement the CRestoreParams class.
 */
#include "RestoreParams.h"
#include <sstream>
#include <config.h>
#include <config_pixie16api.h>

static const char* errorMessages[5] =
{
    "Success",
    ".Set file length is invalid",
    "Failed to program FPPI On load",
    "Failed to se DACS in a moulde",
    "Failed to open the .set file."
};

/**
 * constructor
 *   @param pInterp - interpreter on which we're registering.
 */
CRestoreParams::CRestoreParams(Tcl_Interp* pInterp) :
    CTclCommand(pInterp, "pixie16::restore")
    {}
    
/**
 * destructor
 */
CRestoreParams::~CRestoreParams() {}

/**
 * operator()
 *    Perform the command.
 * @param objv - the command line words.
 * @return int
 */
int
CRestoreParams::operator()(std::vector<Tcl_Obj*>& objv)
{
    const char* pFilename;
    try {
        requireExactly(objv, 2);
        pFilename = Tcl_GetString(objv[1]);
        int status = Pixie16LoadDSPParametersFromFile(pFilename);
        if (status) throw -status;
    }
    catch (std::string msg) {
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    catch (int status) {
        std::string msg = apiMessage(pFilename, status);
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    return TCL_OK;
}

//////////////////////////////////////////////////////////
/**
* apiMessage
*    @param pFilename - name of the4 file being restored.
*    @param status    - status of the attempt.
*    @return std::string
*/
std::string
CRestoreParams::apiMessage(const char* pFilename, int status)
{
    std::stringstream s;
    s << "Unable to load DSP parametes from : " << status
    << " : " << errorMessages[status];
    
    std::string msg = s.str();
    return msg;
}


