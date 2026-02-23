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
 * @file  RawChannel.cpp
 * @brief Implement the raw channel hit storage struct.
 */

#include "RawChannel.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <new>
#include <sstream>
#include <stdexcept>

namespace DDASReadout {
static const uint32_t CHANNELIDMASK(0xF); //!< Bits 0-3 inclusive.
/** Bits 17-29 inclusive. */
static const uint32_t CHANNELLENGTHMASK(0x3FFE0000);
/** Shift channel length to right justify. */
static const uint32_t CHANNELLENGTHSHIFT(17);
static const uint32_t LOWER16BITMASK(0xFFFF); //!< Lower 16 bits.

/**
 * @details
 * Constructs a new raw channel that could be used in either zerocopy or
 * copy mode. The size and data are not yet set and the ownData flag is
 * set false since we don't need to delete.
 */
RawChannel::RawChannel()
    : s_moduleType(0), s_time(0.0), s_chanid(0), s_ownData(false),
      s_ownDataSize(0), s_channelLength(0), s_data(nullptr) {}

/**
 * @details
 * Construts a channel for copy in data. Data are pre-allocated as
 * demanded but not initialized. We use malloc rather than new because
 * new will construct (initialize) ints to zero and we don't want to
 * take that time.
 *
 * Data must eventually be provided by calling copyInData.
 *
 * @note after this call, m_ownData is true and m_ownDataSize is set to
 * nWords.
 */
RawChannel::RawChannel(size_t nWords)
    : s_moduleType(0), s_time(0.0), s_chanid(0), s_ownData(true),
      s_ownDataSize(nWords), s_channelLength(0), s_data(nullptr) {
  s_data = static_cast<uint32_t *>(malloc(nWords * sizeof(uint32_t)));
  if (!s_data) {
    throw std::bad_alloc();
  }
}

/**
 * @note The data pointed to by pZCopyData must be in scope for the
 * duration of this object's lifetime else probably segfaults or bus
 * errors will happen in the best case.
 */
RawChannel::RawChannel(size_t nWords, void *pZCopyData)
    : s_moduleType(0), s_time(0.0), s_chanid(0), s_ownData(false),
      s_ownDataSize(nWords), s_channelLength(nWords),
      s_data(static_cast<uint32_t *>(pZCopyData)) {}

/**
 * @details
 * If we own the data, this will free it.
 */
RawChannel::~RawChannel() {
  if (s_ownData)
    free(s_data);
}

/**
 * @details
 * Assumes that the data are set using the `setData()` or `copyInData()`
 * methods of this class. Determine the raw timestamp from the 48-bit
 * timestamp data in the hit and set it in s_time. The timestamp is
 * extracted from data words 1 and 2 of the Pixie-16 list mode event
 * header structure in the hit.
 *
 * @note If the data have not yet been set, number of words is 0 so this
 * is well behaved.
 */
int RawChannel::SetTime() {
  if (s_channelLength >= 4) {
    uint64_t t = s_data[2] & LOWER16BITMASK;
    t = t << 32;
    t |= (s_data[1]);
    s_time = t;
    return 0;
  } else {
    return 1;
  }
}

/**
 * @details
 * This function assumes that the data are set using either the `setData()`
 * or `copyInData()` methods of this class. Note that `setData()` is used
 * for zero-copy. Determine the calibrated timestamp from the 48-bit
 * timestamp  data in the hit and set it in s_time. The timestamp is
 * extracted from data words 1 and 2 of the Pixie-16 list mode event header
 * structure in the hit. The clock calibration passed to this function is
 * used to convert the time to nanoseconds from clock ticks.
 *
 * @note In Pixie systems, the value of nsPerTick is module-dependent.
 * @note If the data have not yet been set, number of words is 0 so this
 * is well behaved.
 */
int RawChannel::SetTime(double nsPerTick, bool useExt) {
  // The external timestamp requires a header length of at least 6 words
  // and is always the last two words of the header:
  if (useExt) {
    uint32_t headerSize = (s_data[0] & 0x1f000) >> 12;
    if (headerSize >= 6) {
      uint64_t extStampHi = s_data[headerSize - 1] & LOWER16BITMASK;
      uint64_t extStampLo = s_data[headerSize - 2];
      uint64_t stamp = (extStampHi << 32) | (extStampLo);
      s_time = stamp;
    } else {
      return 1; // There's no external timestamp!
    }
  } else if (SetTime()) {
    return 1; // SetTime() fails: channel length < 4.
  }

  // Otherwise we're good, calibrate the time and return success:

  s_time *= nsPerTick;

  return 0;
}

int RawChannel::SetLength() {
  s_channelLength = channelLength(s_data);
  return 0;
}

int RawChannel::SetChannel() {
  if (s_channelLength >= 4) {
    s_chanid = (s_data[0] & CHANNELIDMASK);
    return 0;
  } else {
    return 1;
  }
}

/**
 * @details
 * Retains rough compatibility with the old channel class.
 *
 * @todo (ASC 1/23/24): An old and somewhat cryptic comment about "hating
 * the output" but maintaining it for compatibility. Perhaps because
 * we write to stderr and return 1 instead of raising an exception?
 */
int RawChannel::Validate(int expecting) {
  if (s_channelLength == expecting) {
    return 0;
  } else {
    std::cerr << "Data is corrupt or the setting in modevtlen.txt "
                 "is wrong! Expected: "
              << expecting << " got: " << s_channelLength << std::endl;
    return 1;
  }
}

/**
 * @details
 * Perform at most two tasks:
 * - If we own data already, it's freed and the channel length is reset
 *   to zero (0).
 * - The data and channel length of the data are set from the parameters.
 */
void RawChannel::setData(size_t nWords, void *pZCopyData) {
  if (s_ownData) {
    free(s_data);
    s_ownData = false;
    s_ownDataSize = 0;
  }

  s_channelLength = nWords;
  s_data = static_cast<uint32_t *>(pZCopyData);
}

/**
 * @details
 * - If s_ownData is false, then allocate sufficient storage for the hit.
 * - If s_ownData is true, and the amount of data we have is too small,
 *   allocate new data to hold it.
 * - Copy the hit into our owned data.
 */
void RawChannel::copyInData(size_t nWords, const void *pData) {
  // We need to allocate unless we're already dynamic and have a big
  // enough block allocated. This minimizes allocations.

  bool mustAllocate = !(s_ownData && (nWords <= s_ownDataSize));

  if (mustAllocate) {
    if (s_ownData)
      free(s_data);
    s_data = static_cast<uint32_t *>(malloc(nWords * sizeof(uint32_t)));
    s_ownData = true;
    s_ownDataSize = nWords;
  }
  s_channelLength = nWords;
  memcpy(s_data, pData, nWords * sizeof(uint32_t));
}

/**
 * @details
 * This is just assignment to *this once we're appropriately initialized.
 */
RawChannel::RawChannel(const RawChannel &rhs)
    : s_ownData(false), s_ownDataSize(0), s_data(nullptr) {
  *this = rhs;
}

/**
 * @details
 * Only works if this != &rhs. There are piles of cases to consider:
 *   - rhs is zero copy: we'll zero copy.
 *   - rhs is dynamic: we'll be a dynamic deep copy.
 *
 * These two cases and their subcases are handled by setData and
 * copyInData respectively.
 */
RawChannel &RawChannel::operator=(const RawChannel &rhs) {
  if (this != &rhs) {
    if (rhs.s_ownData) {
      copyInData(rhs.s_channelLength, rhs.s_data);
    } else {
      const void *p = static_cast<const void *>(rhs.s_data);
      setData(rhs.s_channelLength, const_cast<void *>(p));
    }
    // now all the other stuff not set by the above:

    s_time = rhs.s_time;
    s_chanid = rhs.s_chanid;
  }
  return *this;
}

uint32_t RawChannel::channelLength(void *pData) {
  uint32_t *p = static_cast<uint32_t *>(pData);
  uint32_t result = (*p & CHANNELLENGTHMASK) >> CHANNELLENGTHSHIFT;
  return result;
}

/** Map of module frequency to clock calibration. */
static std::map<uint32_t, double> freqToCalibration = {
    {100, 10.0}, {250, 8.0}, {500, 10.0}

};

double RawChannel::moduleCalibration(uint32_t moduleType) {
  uint32_t freq = moduleType & 0xffff;
  double result = freqToCalibration[freq];
  if (result == 0.0) { // No map entry!!
    std::stringstream err;
    err << " No frequency calibration for " << freq << "MSPS modules ";
    err << " update freqToCalibration in RawChannel.cpp";
    throw std::invalid_argument(err.str());
  }
  return result;
}

} // namespace DDASReadout

/**
 * @details
 * Assumes SetTime() has been called.
 */
bool operator<(const DDASReadout::RawChannel &c1,
               const DDASReadout::RawChannel &c2) {
  return c1.s_time < c2.s_time;
}

/**
 * @details
 * Assumes SetTime() has been called.
 */
bool operator>(const DDASReadout::RawChannel &c1,
               const DDASReadout::RawChannel &c2) {
  return c1.s_time > c2.s_time;
}

/**
 * @details
 * Assumes SetTime() has been called.
 */
bool operator==(const DDASReadout::RawChannel &c1,
                const DDASReadout::RawChannel &c2) {
  return c1.s_time == c2.s_time;
}
