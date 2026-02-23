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

/**
 * @file CDDASStatisticsCommand.h
 * @brief Provide DDAS specific 'statistics' command for getting statistics.
 */

#ifndef CDDASSTATISTICSCOMMAND_H
#define CDDASSTATISTICSCOMMAND_H

#include <vector>

#include <TCLObjectProcessor.h>

class CMyEventSegment;
class CMyScaler;

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CDDASStatisticsCommand
 * @brief Provides a statistics command processor that overrides the SBS one.
 * @details
 * We need to override because the concept of a trigger within DDAS is
 * completely different than the triggering used to invoke the readouts.
 * For DDAS readout, the triggering information comes from the module
 * pseudo scalers. Thus we'll function by grabbing byte statistics from
 * CMyEventSegment and trigger information from the collection of
 * CMyScaler objects.
 */

class CDDASStatisticsCommand : public CTCLObjectProcessor {
private:
  CMyEventSegment *m_pEventSegment;    //!< Event segment we act upon.
  std::vector<CMyScaler *> &m_Scalers; //!< Run statistics stored here.

public:
  /**
   * @brief Constructor.
   * @param interp Interpreter on which the command is registered.
   * @param command Name of the command 'should/must' be "statistics" to
   *   smoothly replace the SBSReaout framework command.
   * @param pSeg Pointer to the event segment which provides byte counters.
   * @param scalers Reference to the array of scaler segments that provide
   *   the individual module trigger statistics information.
   */
  CDDASStatisticsCommand(CTCLInterpreter &interp, const char *command,
                         CMyEventSegment *pSeg,
                         std::vector<CMyScaler *> &scalers);
  /** @brief Destructor. */
  virtual ~CDDASStatisticsCommand();

  /**
   * @brief Called to execute the Tcl command.
   * @param interp references The interpreter executing the command.
   * @param objv Command line parameters.
   * @return Status of the command.
   * @retval TCL_OK Successful completion.
   * @retval TCL_ERROR Failure. Human readable reason is in the intepreter
   *   result.
   */
  int operator()(CTCLInterpreter &interp, std::vector<CTCLObject> &objv);

private:
  /**
   * @brief  Computes and formats the result.
   * @param interp Interpreter executing the command.
   * @param result Reference to the CTCLObject in to which the result is
   *   formatted, must already be bound.
   * @param bytes Total number of bytes this program instance acquired.
   * @param runBytes Number of bytes acquire over the last (or current) run.
   */
  void formatResult(CTCLInterpreter &interp, CTCLObject &result, size_t bytes,
                    size_t runBytes);
  /**
   * @brief Format a three element list from the individual counters for a
   *   statistics sublist. See formatResult for a description of the
   *   resulting list.
   * @param result Object into which the list will be created. Must already
   *   be bound to an interpreter.
   * @param triggers Number of triggers.
   * @param accepted Number of accepted triggers.
   * @param bytes Number of bytes.
   */
  void formatCounters(CTCLObject &result, size_t triggers, size_t accepted,
                      size_t bytes);
};

#endif
