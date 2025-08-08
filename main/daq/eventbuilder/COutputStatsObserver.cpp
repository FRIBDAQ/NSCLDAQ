/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "COutputStatsObserver.h"
#include "CFragmentHandler.h"
#include "fragment.h"

/*-------------------------------------------------------------------------------
** Canonical methods:
*/

/**
 * Constructor:
 *   initialize the total fragment count to zero and registers us as a fragment observer
 *   with the fragment handler.
 */
COutputStatsObserver::COutputStatsObserver()  :
  m_nTotalFragments(0)
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->addObserver(this);
}
/**
 * destructor:
 *   Remove us from the observer list:
 */
COutputStatsObserver::~COutputStatsObserver()
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->removeObserver(this);
}

/*--------------------------------------------------------------------------
**  Public methods:
*/

/**
 * operator()
 *
 *   Observer method.  This is called by the fragment handler when it has a batch
 *   of fragments ready to go.  In our case, we just increment our statistics
 *   counters based on the fragments we got.
 *
 * @param event - the fragments passed in.
 */
void
COutputStatsObserver::operator()(const EvbFragments& event)
{
  m_nTotalFragments += event.size();
  // CriticalSection c(m_perSourceStatGuard); 
  // for (auto p = event.begin(); p != event.end(); p++) {
  //   EVB::pFragment pf = p->second;
  //   uint32_t sourceId = pf->s_header.s_sourceId;
  //   m_perSourceStatistics[sourceId]++;
  // }
}
/**
 * clear
 *
 *  Clear all statistics.  Note that for the per source map, all mp entries are erased.
 */
void
COutputStatsObserver::clear()
{
  CriticalSection c(m_perSourceStatGuard);
  m_nTotalFragments = 0;
  m_perSourceStatistics.clear();
}
/**
 * getStatistics
 *
 * Get the  statistics that have been accumulated:
 *
 * @return COutputStatsObserver::Statistics
 */
COutputStatsObserver::Statistics 
COutputStatsObserver::getStatistics() const
{
  Statistics result;
  result.s_nTotalFragments = m_nTotalFragments;
  CriticalSection c(const_cast<CMutex&>(m_perSourceStatGuard));
  for (PerSourceStats::const_iterator p = m_perSourceStatistics.begin(); p != m_perSourceStatistics.end(); 
       p++) {
    result.s_perSourceFragments.push_back(*p);
  }
  return result;
}
