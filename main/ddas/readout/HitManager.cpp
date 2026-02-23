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
 * @file HitManager.cpp
 * @brief Implements the CHitManager class.
 */

#include "HitManager.h"

#include <algorithm>

#include <RawChannel.h>
#include <ZeroCopyHit.h>


namespace DDASReadout {

HitManager::HitManager(uint64_t window) : m_nWindow(window) {}

/**
 * @details
 * On return the deque will be empty.
 */
void HitManager::addHits(std::deque<DDASReadout::ZeroCopyHit *> &newHits) {
  sortHits(newHits);  // First sort the incoming hits.
  mergeHits(newHits); // Then merge them into the newHits deque.
}

/**
 * @details
 * Returns false if < 2 hits since no window can be constructed.
 */
bool HitManager::haveHit() {
  if (m_sortedHits.size() < 2)
    return false;

  auto pfront = m_sortedHits.front();
  auto pback = m_sortedHits.back();

  return ((pback->s_time - pfront->s_time) > m_nWindow);
}

/**
 * @details
 * On exit, if a hit is returned it has been popped off the deque.
 */
DDASReadout::ZeroCopyHit *HitManager::nextHit() {
  DDASReadout::ZeroCopyHit *result;
  if (m_sortedHits.empty()) {
    result = nullptr;
  } else {
    result = m_sortedHits.front();
    m_sortedHits.pop_front();
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
//  Private members.
//

/**
 * @brief Static local comparison function to provide < when we have
 * pointers to hits.
 * @details
 * Uses the RawChannel operator< to operate on the timestamps.
 * @param p1, p2 Zero-copy hits for comparison.
 * @return bool
 * @retval true If the timestamp of p1 is < the timestamp of p2.
 * @retval false Otherwise.
 */
static bool hitCompare(DDASReadout::ZeroCopyHit *p1,
                       DDASReadout::ZeroCopyHit *p2) {
  return *p1 < *p2;
}

/**
 * @brief Sort hits using std::sort. Note that this function modifies the
 * input hit queue.
 * @param[in,out] newHits List of hits to sort. Modified by this function.
 */
void HitManager::sortHits(std::deque<DDASReadout::ZeroCopyHit *> &newHits) {
  std::sort(newHits.begin(), newHits.end(), hitCompare);
}

/**
 * @details
 * We assume that, other than when starting up, the time range covered by
 * the output deque is much larger than that of the new hits (the window
 * is seconds where at high rates, which is what we care about optimizing,
 * the data read from a module will be milliseconds?).
 *
 *  Special cases:
 *  - m_sortedHits is empty: assign it to new hits.
 *  - m_sortedHits.front() >= newHits.back(): prepend the new hits.
 *
 * Otherwise a reduced append and merge approach is taken and the following
 * steps are performed:
 *  1. Append the new hits to sorted hits.
 *  2. Search backwards in the existing hits until either we come to the
 *     first existing hit or we come to an element whose time is <= to
 *     newhit's front.
 *  3. Do an in-place merge of those two ranges of the sorted_list.
 */
void HitManager::mergeHits(std::deque<DDASReadout::ZeroCopyHit *> &newHits) {
  if (m_sortedHits.empty()) {
    m_sortedHits = newHits; // Assign
  } else {
    auto oldFront = m_sortedHits.front();
    auto oldBack = m_sortedHits.back();
    auto newFront = newHits.front();
    auto newBack = newHits.back();
    if (hitCompare(newBack, oldFront)) { // Prepend
      m_sortedHits.insert(m_sortedHits.begin(), newHits.begin(), newHits.end());
    } else { // Reduced append and merge
      /**
       * @todo (ASC 3/21/24): Does checking oldBack < newFront,
       * appending and exiting without doing an inplace merge
       * offer any performance boost?
       */
      // Start by appending:
      auto newPosition = m_sortedHits.insert(m_sortedHits.end(),
                                             newHits.begin(), newHits.end());
      auto oldPosition = newPosition;
      --oldPosition;
      while (!hitCompare(*oldPosition, *newPosition) &&
             (oldPosition != m_sortedHits.begin())) {
        --oldPosition;
      }
      /**
       * @todo (ASC 3/20/24): std::inplace_merge() dynamically
       * allocates a temporary buffer and switches to a less
       * efficient algorithm is used. _If_ sorting is rate-limiting,
       * and we're pretty convinced we shouldn't have too much
       * dynamic memory management, can we avoid it with a different
       * merge algorithm?
       */
      std::inplace_merge(oldPosition, newPosition, m_sortedHits.end(),
                         hitCompare);
    }
  }
}

} // namespace DDASReadout
