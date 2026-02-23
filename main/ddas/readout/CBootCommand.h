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
 * @file  CBootCommand.h
 * @brief Class definition for the ddasboot command.
 */

#ifndef CBOOTCOMMAND_H
#define CBOOTCOMMAND_H

#include <TCLObjectProcessor.h>

class CMyEventSegment;
class CTCLInterpreter;
class CTCLObject;

/**
 * @class CBootCommand
 * @brief This class implements the ddasboot command. It is added to to the Tcl
 * interpreter that runs ddasreadout so that the DDAS modules can be booted
 * on-demand rather than every time the Readout program starts.
 */

class CBootCommand : public CTCLObjectProcessor {
private:
  CMyEventSegment *m_pSegment; //!< The event segment we act upon.

public:
  /**
   * @brief Constructor.
   * @param interp Reference to the interpreter.
   * @param pCmd   Command string.
   * @param pSeg   Event segment to manipulate.
   */
  CBootCommand(CTCLInterpreter &interp, const char *pCmd,
               CMyEventSegment *pSeg);
  /** @brief Destructor. */
  virtual ~CBootCommand();

  /**
   * @brief Gets control when the command is invoked.
   * @param interp Interpreter executing the command.
   * @param objv   Command words.
   * @return Status of the command.
   * @retval TCL_OK Successful completion.
   * @retval TCL_ERROR Failure. Human readable reason is in the intepreter
   *   result.
   */
  virtual int operator()(CTCLInterpreter &interp,
                         std::vector<CTCLObject> &objv);
};

#endif
