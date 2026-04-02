/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
             FRIB
             Michigan State University
             East Lansing, MI 48824-1321
*/

/** @file CAdjustOffsets.cpp
 *  @brief Implements the CAdjustOffsets class.
 */

#include "CAdjustOffsets.h"

#include <sstream>

#include <CXIAException.h>
#include <config.h>
#include <config_pixie16api.h>

/**
 * constructor
 *  @param pInterp - interpreter this command is registered on.
 */
CAdjustOffsets::CAdjustOffsets(Tcl_Interp *pInterp)
    : CTclCommand(pInterp, "pixie16::adjustOffsets") {}

/**
 * destructor:
 */
CAdjustOffsets::~CAdjustOffsets() {}

/**
 * operator()
 *    Execute the command:
 *    - Ensure there's a slot number parameter and that it is
 *      a valid integer (positive).
 *    - Invoke Pixie16AdjustOffsets
 *    - analyze and report status.
 * @param objv - the command line parameter objects.
 * @return int (TCL_OK On success, TCL_ERROR if not).
 *        On error the result contains a descriptive error message.
 */
int CAdjustOffsets::operator()(std::vector<Tcl_Obj *> &objv) {
  int module;
  try {
    requireExactly(objv, 2);
    module = getInteger(objv[1]);
    int status = Pixie16AdjustOffsets(module);
    if (status < 0) {
      std::stringstream msg;
      msg << "Pixie16AdjustOffsets failed on module: " << module;
      throw CXIAException(msg.str(), "Pixie16AdjustOffsets()", status);
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
