/*
 *    This software is Copyright by the Board of Trustees of Michigan
 *    State University (c) Copyright 2013.
 *
 *    You may use this software under the terms of the GNU public license
 *    (GPL).  The terms of this license are described at:
 *
 *     http://www.gnu.org/licenses/gpl.txt
 *
 *    Author:
 *            Ron Fox
 *            NSCL
 *            Michigan State University
 *            East Lansing, MI 48824-1321
 */

/**
 * @file   CBootCommand.cpp
 * @brief  Implement the ddasboot command.
 */

#include "CBootCommand.h"

#include <stdexcept>

#include <TCLInterpreter.h>
#include <TCLObject.h>

#include "CMyEventSegment.h"
#include "RunState.h"

CBootCommand::CBootCommand(CTCLInterpreter &interp, const char *pCmd,
                           CMyEventSegment *pSeg)
    : CTCLObjectProcessor(interp, pCmd), m_pSegment(pSeg) {}

CBootCommand::~CBootCommand() {}

/**
 * @details
 * Ensures there are no additional command parameters. Invokes the segments's
 * boot method.
 */
int CBootCommand::operator()(CTCLInterpreter &interp,
                             std::vector<CTCLObject> &objv) {
  bindAll(interp, objv);
  try {
    requireExactly(objv, 1, "ddasboot requires no parameters");

    if (RunState::getInstance()->m_state == RunState::inactive) {
      m_pSegment->boot();
    } else {
      throw std::runtime_error(
          "Cannot boot system while a run is active or paused.");
    }
  } catch (std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  } catch (std::exception &e) {
    interp.setResult(e.what());
    return TCL_ERROR;
  }

  return TCL_OK;
}
