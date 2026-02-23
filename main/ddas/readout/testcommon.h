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

#ifndef TESTCOMMON_H
#define TESTCOMMON_H

#include <cstdint>

/**
 * @brief Given parameters for a hit, creates the data for a 4 longword hit.
 * @param[out] hit - Pointer to a uint32_t[4]  which will receive the hit.
 * @param[in]  crate   Hit crate number.
 * @param[in]  slot    Hit slot number.
 * @param[in]  chan    Hit channel number.
 * @param[in]  rawTime The hit time from the clock.
 * @param[in]  energy  Energy value.
 * @param[in]  cfdTime CFD fractional time (default=0).
 */
void makeHit(uint32_t *hit, int crate, int slot, int chan, uint64_t rawTime,
             uint16_t energy, uint16_t cfdTime = 0);

#endif
