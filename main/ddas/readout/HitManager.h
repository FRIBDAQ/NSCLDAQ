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
 * @file HitManager.h
 * @brief Defines a class to collect and sort hits from modules and outputs
 * hits within a specified time window.
 */

#ifndef HITMANAGER_H
#define HITMANAGER_H

#include <cstdint>
#include <deque>

/** @namespace DDASReadout */
namespace DDASReadout {
class ZeroCopyHit;

/**
 * @class HitManager
 * @brief Collect hits from modules and retains them in a sorted deque.
 * @details
 * On request, provides hits that were accepted within some sliding
 * time interval. The time interval is defined at construction time
 * and is in units of seconds.
 *
 * This module does no storage management, the receiver of all hits is
 * expected to release any events that have been output.
 */

class HitManager {
private:
  /** Sorted hits for all modules in the system. */
  std::deque<DDASReadout::ZeroCopyHit *> m_sortedHits;
  uint64_t m_nWindow; //!< Emission window for dequeing hits in ns.

public:
  /**
   * @brief Constructor.
   * @param window The hit emission window in nanoseconds (ns).
   */
  HitManager(uint64_t window);
  /**
   * @brief Adds a new set of hits to the m_sortedHits deque maintaining
   * total ordering by calibrated timestamp.
   * @param newHits References the new hits to be added.
   */
  void addHits(std::deque<DDASReadout::ZeroCopyHit *> &newHits);
  /**
   * @brief Returns true if there's at least one hit that can be output.
   * @return True if a hit can be output, false otherwise.
   */
  bool haveHit();

  /**
   * @brief Get the next hit from the queue.
   * @return Pointer to the oldest hit in the m_sortedHits deque.
   * @retval nullptr If there are no hits in m_sortedHits
   */
  DDASReadout::ZeroCopyHit *nextHit();

private:
  /**
   * @brief Given a reference to a deque of hits, sorts that deque in
   * place by increasing timestamp.
   * @param newHits The hits to sort.
   */
  void sortHits(std::deque<DDASReadout::ZeroCopyHit *> &newHits);
  /**
   * @brief Merge a sorted deque of new hits into the existing set of
   * sorted hits.
   * @param newHits Sorted dequeue of new hits.
   */
  void mergeHits(std::deque<DDASReadout::ZeroCopyHit *> &newHits);
};

} // namespace DDASReadout

#endif
