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

/** @file  ReadChanPar.cpp
 *  @brief Implement the readchanpar command.
 */

#include "ReadChanPar.h"

#include <sstream>

#include <CXIAException.h>
#include <config.h>
#include <config_pixie16api.h>

/**
 * constructor
 *   @param pInterp - tcl interpreter we're registering on.
 */
CReadChanPar::CReadChanPar(Tcl_Interp *pInterp)
    : CTclCommand(pInterp, "pixie16::readchanpar") {}

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
int CReadChanPar::operator()(std::vector<Tcl_Obj *> &objv) {
  int index;
  int chan;
  try {
    requireExactly(objv, 4); // command module slot parname
    index = getInteger(objv[1]);
    chan = getInteger(objv[2]);
    const char *name = Tcl_GetString(objv[3]);

    double result;
    int status = Pixie16ReadSglChanPar(name, &result, index, chan);
    if (status) {
      std::stringstream msg;
      msg << "Failed to read channel parameter " << name << " for module "
          << index << " channel " << chan;
      throw CXIAException(msg.str(), "Pixie16ReadSglChanPar()", status);
    }
    setResult(result);

  } catch (std::string msg) {
    setResult(msg.c_str());
    return TCL_ERROR;
  } catch (CXIAException &e) {
    setResult(e.ReasonText());
    return TCL_ERROR;
  }

  return TCL_OK;
}