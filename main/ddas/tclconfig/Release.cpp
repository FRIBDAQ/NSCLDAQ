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

/** @file  Release.cpp
 *  @brief Implement the pixie16::release command.
 */
#include "Release.h"

#include <sstream>

#include <CXIAException.h>
#include <Configuration.h>
#include <config.h>
#include <config_pixie16api.h>

/**
 * constructor
 *   Base class construction registers the command and
 *   we squirrel away the configuration so we know which
 *   module numbers exist and need to be released.
 */
CRelease::CRelease(Tcl_Interp *pInterp, DAQ::DDAS::Configuration &config)
    : CTclCommand(pInterp, "pixie16::release"), m_config(config) {}

/**
 * destructor
 *  null for now.
 */
CRelease::~CRelease() {}

/**
 * operator()
 *    Called in response to the command.
 *  @param objv - the command words
 *  @return int - TCL_OK on success TCL_ERROR on failure with an
 *                error message in the result.
 * @throw CXIAException if any of the API calls fail.
 */
int CRelease::operator()(std::vector<Tcl_Obj *> &objv) {
  int index;
  int slot;
  try {
    requireExactly(objv, 1); // No extra parameters.
    auto slots = m_config.getSlotMap();
    for (index = 0; index < slots.size(); index++) {
      slot = slots[index];
      int status = Pixie16ExitSystem(index);
      if (status) {
        std::stringstream msg;
        msg << "Error exiting system for module number " << index;
        throw CXIAException(msg.str(), "Pixie16ExitSystem()", status);
      }
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
