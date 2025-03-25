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

/** @file:  SaveParams.cpp
 *  @brief: Implementation of CSaveParams.
 */
#include <SaveParams.h>
#include <sstream>
#include <config.h>
#include <config_pixie16api.h>

const char* errorMessages[3] = {
    "Success",
    "Failed to read DSP Parameters from Modules",
    "Failed to open DSP parameter file"
};

/**
 * constructor
 * @param pInterp - interpreter on which we'r registering.
 */
CSaveParams::CSaveParams(Tcl_Interp* pInterp) :
    CTclCommand(pInterp, "pixie16::save")
{}
/**
 * destructor
 */
CSaveParams::~CSaveParams() {}

/**
 * operator()
 *    Perform the command.
 *  @param objv - the command words.
 *  @return int- TCL_OK on ok or TCL_ERROR on failure.
 *  @note the result is only set on error and then contains a string
 *         describing the error.
 */
int
CSaveParams::operator()(std::vector<Tcl_Obj*>& objv)
{
    const char* pFilename;
    try {
        requireExactly(objv, 2);
        pFilename = Tcl_GetString(objv[1]);
        int status = Pixie16SaveDSPParametersToFile(pFilename);
        if (status) throw -status;
    }
    catch (std::string msg) {
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    catch (int status) {
        std::string result = apiMessage(pFilename, status);
        setResult(result.c_str());
        return TCL_ERROR;
    }
    return TCL_OK;
}
////////////////////////////////////////////////////////////////
/**
* apiMessage
*
* @param filename - filename we're trying to save to.
* @param status   - absolute value of status.
* @return std::string - Appropriate error message.
*/
std::string
CSaveParams::apiMessage(const char* filename, int status)
{
    std::stringstream s;
    s << "Unable to write to .set file: " << filename
      << ": " << errorMessages[status];
    std::string result = s.str();
    return result;
}
