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

/** @file  WriteModPar.cpp
 *  @brief Implementation of the CWriteModPar class.
 */

#include "WriteModPar.h"

#include <sstream>

#include <CXIAException.h>
#include <config.h>
#include <config_pixie16api.h>

/**
 * constructor
 *    @param pInterp - pointer to the interpreter on which we register.
 */
CWriteModPar::CWriteModPar(Tcl_Interp *pInterp)
    : CTclCommand(pInterp, "pixie16::writemodpar") {}

/**
 * destructor
 */
CWriteModPar::~CWriteModPar() {}

/**
 * operator()
 *   Executes the command.
 * @param objv - command words.
 * @return int - TCL_OK if all is ok and TCL_ERROR for a problem.
 * @note - The return value is only set if TCL_ERROR is returned and
 *         in that case, a human readable string describing the
 *         error is set as the result.
 */
int CWriteModPar::operator()(std::vector<Tcl_Obj *> &objv) {
  int module;
  try {
    requireExactly(objv, 4);
    module = getInteger(objv[1]);
    const char *parName = Tcl_GetString(objv[2]);
    unsigned int value = getInteger(objv[3]);

    int status = Pixie16WriteSglModPar(parName, value, module);
    if (status) {
      std::stringstream msg;
      msg << "Error writing module parameter " << parName << " with value "
          << value << " to module " << module;
      throw CXIAException(msg.str(), "Pixie16WriteSglModPar()", status);
    }
  } catch (std::string &msg) {
    setResult(msg.c_str());
    return TCL_ERROR;
  } catch (CXIAException &e) {
    setResult(e.ReasonText());
    return TCL_ERROR;
  }
  return TCL_OK;
}