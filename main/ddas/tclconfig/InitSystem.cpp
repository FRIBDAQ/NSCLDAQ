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

/** @file:  InitSystem.cpp
 *  @brief: Implement the init command.
 */
#include "InitSystem.h"

#include <cstdlib>

#include <CXIAException.h>
#include <Configuration.h>
#include <config.h>
#include <config_pixie16api.h>

/**
 * constructor
 *   @param pInterp - interpreter on which to execute the command.
 *   @param config  - References the configuration.
 */
CInitSystem::CInitSystem(Tcl_Interp *pInterp, DAQ::DDAS::Configuration &config)
    : CTclCommand(pInterp, "pixie16::init"), m_config(config) {}

/**
 * destructor
 *  null -- for now
 */
CInitSystem::~CInitSystem() {}

/**
 * operator()
 *   - There must be no parameters.
 *   - Grab the slot map from the configuration
 *   - Invoke the InitSystem call.
 *   - Analyxe results.
 * @param argv the command line words.
 * @return int - TCL_OK if successful TCL_ERROR if not.
 * @note a result is left only if an error is returned.
 */
int CInitSystem::operator()(std::vector<Tcl_Obj *> &argv) {
  int offlineMode = 0;
  if (getenv("PIXIE16_TESTING")) {
    offlineMode = 1;
  }
  try {
    requireExactly(argv, 1);
    auto slots = m_config.getSlotMap();
    int status = Pixie16InitSystem(slots.size(), slots.data(), offlineMode);
    if (status) {
      throw CXIAException("Failed to initialize system", "Pixie16InitSystem()",
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
