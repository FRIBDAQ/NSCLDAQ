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

/** @file  WriteChanPar.cpp
 *  @brief Implemennt pixie16::writechanpar.
 */

#include "WriteChanPar.h"

#include <sstream>

#include <CXIAException.h>
#include <config.h>
#include <config_pixie16api.h>

/**
 * constructor
 * @param pInterp - interpreter on which to register the command.
 */
CWriteChanPar::CWriteChanPar(Tcl_Interp *pInterp)
    : CTclCommand(pInterp, "pixie16::writechanpar") {}

CWriteChanPar::~CWriteChanPar() {}

/**
 * operator()
 *    Execute the command
 *  @param objv - the command words.
 *  @return int - TCL_OK On success, TCL_ERROR if failed.
 *  @note the interpreter result is only non-null if the command
 *        failed, in which case it's the human readable failure reason
 * @throw CXIAException if the API call fails.
 */
int CWriteChanPar::operator()(std::vector<Tcl_Obj *> &objv) {
  int module;
  int chan;

  try {
    requireExactly(objv, 5);
    module = getInteger(objv[1]);
    chan = getInteger(objv[2]);
    const char *name = Tcl_GetString(objv[3]);
    double value = getDouble(objv[4]);

    int status = Pixie16WriteSglChanPar(name, value, module, chan);
    if (status) {
      std::stringstream msg;
      msg << "Error writing channel parameter " << name << " with value "
          << value << " to module " << module << " channel " << chan;
      throw CXIAException(msg.str(), "Pixie16WriteSglChanPar()", status);
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
