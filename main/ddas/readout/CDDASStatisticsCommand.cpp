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
 * @file  CDDASStatisticsCommand.cpp
 * @brief Implement the statistics command specific to DDASReadout.
 */

#include "CDDASStatisticsCommand.h"

#include <cstring>
#include <string>

#include <Exception.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>

#include "CMyEventSegment.h"
#include "CMyScaler.h"

CDDASStatisticsCommand::CDDASStatisticsCommand(
    CTCLInterpreter &interp, const char *command, CMyEventSegment *pSeg,
    std::vector<CMyScaler *> &scalers)
    : CTCLObjectProcessor(interp, command, true), m_pEventSegment(pSeg),
      m_Scalers(scalers) {}

CDDASStatisticsCommand::~CDDASStatisticsCommand() {}

int CDDASStatisticsCommand::operator()(CTCLInterpreter &interp,
                                       std::vector<CTCLObject> &objv) {
  try {
    requireExactly(objv, 1,
                   "DDAS 'statistics' - incorrect command parameter count");
    auto bytes = m_pEventSegment->getStatistics();
    CTCLObject result;
    result.Bind(interp);
    formatResult(interp, result, bytes.first, bytes.second);
    interp.setResult(result);

  } catch (std::exception &e) {
    interp.setResult(e.what());
    return TCL_ERROR;
  } catch (CException &e) {
    interp.setResult(e.ReasonText());
    return TCL_ERROR;
  } catch (std::string &msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  } catch (...) {
    interp.setResult(
        "Unanticipated exception type caugh in DDAS 'statistics' command");
    return TCL_ERROR;
  }
  return TCL_OK;
}

/*--------------------------------------------------------------------------
 * Private utilities.
 */

/**
 * @details
 *
 * Note that we have to sum the trigger statistics over the modules in the
 * system. The result is a two element list. Each element is a three element
 * sublist of statistics.  The first element contains cumulative statistics,
 * the second the statistics from the current run or most recently ended run
 * if data taking is not active.
 *
 * Each list has, in order, the following three subelements:
 *  - Number of triggers.
 *  - Number of accepted triggers.
 *  - Number of bytes of data transferred.
 */
void CDDASStatisticsCommand::formatResult(CTCLInterpreter &interp,
                                          CTCLObject &result, size_t bytes,
                                          size_t runBytes) {
  // Collect triggers statistic sums:

  CMyScaler::Statistics totals;
  memset(&totals, 0, sizeof(CMyScaler::Statistics));
  for (int i = 0; i < m_Scalers.size(); i++) {
    auto moduleStats = m_Scalers[i]->getStatistics();
    totals.s_cumulative.s_nTriggers += moduleStats.s_cumulative.s_nTriggers;
    totals.s_cumulative.s_nAcceptedTriggers +=
        moduleStats.s_cumulative.s_nAcceptedTriggers;

    totals.s_perRun.s_nTriggers += moduleStats.s_perRun.s_nTriggers;
    totals.s_perRun.s_nAcceptedTriggers += moduleStats.s_perRun.s_nTriggers;
  }
  // Now we can format the two sublists and append them to result.

  CTCLObject totalobj;
  totalobj.Bind(interp);
  CTCLObject perRunObj;
  perRunObj.Bind(interp);

  formatCounters(totalobj, totals.s_cumulative.s_nTriggers,
                 totals.s_cumulative.s_nAcceptedTriggers, bytes);
  formatCounters(perRunObj, totals.s_perRun.s_nTriggers,
                 totals.s_perRun.s_nAcceptedTriggers, runBytes);

  result += totalobj;
  result += perRunObj;
}

void CDDASStatisticsCommand::formatCounters(CTCLObject &result, size_t triggers,
                                            size_t accepted, size_t bytes) {
  result += int(triggers);
  result += int(accepted);
  result += int(bytes);
}
