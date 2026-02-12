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
 * @file RawChannel.h
 * @brief Defines a raw channel hit storage struct for Readout/sorting.
 */

#ifndef RAWCHANNEL_H
#define RAWCHANNEL_H

#include <cstddef>
#include <cstdint>

namespace DDASReadout {

/**
 * @struct RawChannel
 * @brief A struct containing a pointer to a hit and its properties.
 * @details
 * The struct can be used in either zero copy or copy mode. In zero-copy
 * mode, a channel's data pointer points to some buffer that may have raw
 * data from more than one hit. In copy mode, the data are dynamically
 * allocated to hold the raw hit.
 */

struct RawChannel {
  uint32_t s_moduleType; //!< Type of module this comes from.
  double s_time;         //!< Extracted time, possibly calibrated.
  int s_chanid;          //!< Channel within module.
  bool s_ownData;        //!< True if we own s_data.
  int s_ownDataSize;     //!< If we own data, how many uint32_t's.
  int s_channelLength;   //!< Number of uint32_t in s_data.
  uint32_t *s_data;      //!< Pointer to the hit data.

  /** @brief Default constructor. */
  RawChannel();
  /**
   * @brief Construct a channel for copy-in data.
   * @param nWords Number of data words to pre-allocate.
   * @throws std::bad_alloc If malloc of pre-allocated storage fails.
   */
  RawChannel(size_t nWords);
  /**
   * @brief Constructor initialized with zero-copy hit data.
   * @param nWords Number of data words to pre-allocate.
   * @param pZCopyData Pointer to the data of the hit.
   */
  RawChannel(size_t nWords, void *pZcopyData);
  /** @brief Destructor. */
  virtual ~RawChannel();

  /**
   * @brief Set the 48-bit timestamp data from the hit infomration.
   * @return int
   * @retval 0 Success.
   * @retval 1 If the number of data words is insufficient (< 4).
   */
  int SetTime();
  /**
   * @brief Set time time in ns.
   * @param nsPerTick Clock calibration in nanoseconds per clock tick.
   * @param useExt True if using an external timestamp (default=false).
   * @return int
   * @retval 0 Success.
   * @retval 1 Failure, which can happen in the following ways:
   * * If attempting to use the external timestamp but none is present
   *   in the data (header has insufficient words).
   * * If not using an external timestamp, but the number of data words
   *   are insufficient (< 4).
   */
  int SetTime(double nsPerTick, bool useExt = false);
  /**
   * @brief Set the event length from the data.
   * @return 0 Always.
   */
  int SetLength();
  /**
   * @brief Set the channel value from the data.
   * @return int
   * @retval 0 Success.
   * @retval 1 Insufficent data in the hit or the hit has not been set.
   */
  int SetChannel();
  /**
   * @brief Determine if a channel has the correct amount of data.
   * @param expecting The expected channel length in 32-bit words.
   * @return int
   * @retval 0 Correct length.
   * @retval 1 Incorrect channel length. A message is output
   *     to std::cerr.
   */
  int Validate(int expecting);

  /**
   * @brief Set new data.
   * @param nWords The new channel length (32-bit words).
   * @param pZCopyData Pointer to the data to set.
   */
  void setData(size_t nWords, void *pZCopyData);

  /**
   * @brief Copy in data.
   * @param nWords The new channel length (32-bit words).
   * @param pData Pointer to the data to copy in.
   */
  void copyInData(size_t nWords, const void *pData);

  /**
   * @brief Copy constructor.
   * @param rhs The object we're copying into this.
   */
  RawChannel(const RawChannel &rhs);
  /**
   * @brief Assignment operator.
   * @param rhs The object we'er assigning to this.
   * @return Pointer to lhs (*this).
   */
  RawChannel &operator=(const RawChannel &rhs);

  /**
   * @brief Extract the number of words in a hit.
   * @param pData Pointer to the hit data.
   * @return The length of the hit.
   */
  static uint32_t channelLength(void *pData);
  /**
   * @brief Returns the multiplier used to convert the module raw
   * timestamp into nanoseconds.
   * @param moduleType The module type/speed etc. word that's normally
   *   prepended to hit data.
   * @return The timestamp multiplier.
   */
  static double moduleCalibration(uint32_t moduleType);
};
} // namespace DDASReadout

// Comparison operators -- operate on the timestamp:

/**
 * @brief operator<
 * @param c1, c2 Channels with timestamps for comparison.
 * @return bool
 * @retval true If c1 time < c2 time.
 * @retval false Otherwise.
 */
bool operator<(const DDASReadout::RawChannel &c1,
               const DDASReadout::RawChannel &c2);
/**
 * @brief operator>
 * @param c1, c2 Channels with timestamps for comparison.
 * @return bool
 * @retval true If c1 time > c2 time.
 * @retval false Otherwise.
 */
bool operator>(const DDASReadout::RawChannel &c1,
               const DDASReadout::RawChannel &c2);
/**
 * @brief operator==
 * @param c1, c2 Channels with timestamps for comparison.
 * @return bool
 * @retval true If c1 time, c2 time are equal.
 * @retval false Otherwise.
 */
bool operator==(const DDASReadout::RawChannel &c1,
                const DDASReadout::RawChannel &c2);

#endif
