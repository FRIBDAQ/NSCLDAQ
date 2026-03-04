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

/** @file  RestoreParams.cpp
 *  @brief Implement the CRestoreParams class.
 */

#include "RestoreParams.h"

#include <sstream>

#include <CXIAException.h>
#include <config.h>
#include <config_pixie16api.h>

/**
 * constructor
 *   @param pInterp - interpreter on which we're registering.
 */
CRestoreParams::CRestoreParams(Tcl_Interp *pInterp)
    : CTclCommand(pInterp, "pixie16::restore") {}

/**
 * destructor
 */
CRestoreParams::~CRestoreParams() {}

/**
 * operator()
 *    Perform the command.
 * @param objv - the command line words.
 * @return int
 * @throw CXIAException if the API call fails.
 */
int CRestoreParams::operator()(std::vector<Tcl_Obj *> &objv) {
  const char *pFilename;
  try {
    requireExactly(objv, 2);
    pFilename = Tcl_GetString(objv[1]);
    int status = Pixie16LoadDSPParametersFromFile(pFilename);
    if (status) {
      std::stringstream msg;
      msg << "Error loading DSP parameters from " << pFilename;
      throw CXIAException(msg.str(), "Pixie16LoadDSPParametersFromFile()",
                          status);
    }
  } catch (std::string msg) {
    setResult(msg.c_str());
    return TCL_ERROR;
  } catch (CXIAException &e) {
    setResult(e.ReasonText());
    return TCL_ERROR;
  }
  return TCL_OK;
}
