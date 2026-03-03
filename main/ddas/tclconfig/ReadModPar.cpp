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

/** @file  ReadModPar.cpp
 *  @brief Reads a module parameter (implementation).
 */
#include "ReadModPar.h"

#include <sstream>

#include <CXIAException.h>
#include <config.h>
#include <config_pixie16api.h>
#include <string>

/**
 * constructor
 *   @param pInterp - pointer to the interpreter.
 */
CReadModPar::CReadModPar(Tcl_Interp *pInterp)
    : CTclCommand(pInterp, "pixie16::readmodpar") {}
/**
 * destructor
 */
CReadModPar::~CReadModPar() {}
/**
 * operator()
 *     Executes the command
 *  @param objv - vector of command words.
 *  @return int - TCL_OK On success, TCL_ERROR on failure.
 *  @note On success, the result is an integer value read from the
 *        module.  On failure, the result is a textual error message
 */
int CReadModPar::operator()(std::vector<Tcl_Obj *> &objv) {
  int modnum;

  try {
    unsigned int data;
    requireExactly(objv, 3);

    modnum = getInteger(objv[1]);
    std::string parname(Tcl_GetString(objv[2]));

    int status = Pixie16ReadSglModPar(parname.c_str(), &data, modnum);
    if (status) {
      std::stringstream msg;
      msg << "Pixie16ReadSglModPar failed for module number " << modnum;
      throw CXIAException(msg.str(), "Pixie16ReadSglModPar()", status);
    }
    setResult(static_cast<int>(data));
  } catch (std::string msg) {
    setResult(msg.c_str());
    return TCL_ERROR;
  } catch (CXIAException &e) {
    setResult(e.ReasonText());
    return TCL_ERROR;
  }

  return TCL_OK;
}
