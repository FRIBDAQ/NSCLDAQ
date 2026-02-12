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
 * @file  CSyncCommand.h
 * @brief Header for class that implements the ddas_sync command.
 */

#ifndef CSYNCCOMMAND_H
#define CSYNCCOMMAND_H

#include <TCLObjectProcessor.h>

// Forward class definitions:

class CTCLInterpreter;
class CTCLObject;
class CMyEventSegment;

/**
 * @class CSyncCommand
 * @brief Provides the ddas_sync command for the ddas readout program.
 * @details
 * * Construction maintains a pointer to the event segment.
 * * The class registers the "ddas_sync" command on the main interp.
 * * When invoked, simply calls the synchronize method of the event segment.
 * @note A more refined approach would be to refuse to perform the operation
 * when the run is in progress. At this time, however we're going to (heaven
 * help us) rely on the user to know that they really need to do a clock
 * synchronization.
 */
class CSyncCommand : public CTCLObjectProcessor {
private:
  CMyEventSegment *m_pSegment; //!< The event segment we act upon.

public:
  /**
   * @brief Constructor.
   * @param interp Reference to the interpreter.
   * @param pSeg   Event segment to manipulate.
   */
  CSyncCommand(CTCLInterpreter &interp, CMyEventSegment *pSeg);
  /** @brief Destructor. */
  virtual ~CSyncCommand();

  /**
   * @brief Gets control when the command is invoked.
   * @param interp Intepreter that is running this command.
   * @param objv   Words that make up the tcl command.
   * @return Command status.
   * @retval TCL_OK Success.
   * @retval TCL_ERROR Failure.
   */
  int operator()(CTCLInterpreter &interp, std::vector<CTCLObject> &objv);
};

#endif
