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
 * @file ZeroCopyHit.h
 * @brief Class to manage a zero copy RawChannel that comes from inside a
 * buffer from a buffer arena.
 */
#ifndef ZEROCOPYHIT_H
#define ZEROCOPYHIT_H

#include "RawChannel.h"

namespace DDASReadout {

struct ReferenceCountedBuffer;
class BufferArena;

/**
 * @class ZeroCopyHit
 * @brief This class extends RawChannel to produce a raw channel that is
 * zero-copied from a reference counted buffer that comes from a buffer
 * arena. This is a key data structure in the zero-copy DDAS readout.
 *
 * @details
 * This acts like a RawChannel, but on destruction, if it is the last
 * reference to the buffer, returns it to the arena from whence it came.
 *
 * Copy construction and assignment are supported with appropriate
 * semantics to handle proper reference counting.
 *
 * I'm sure I'm hearing a pile of people ask me why not use
 * std::shared_pointer -- The answer is that I don't actually want the
 * buffers to be destroyed as that involves expensive dynamic memory
 * management, I want storage returned to a pre-allocated buffer arena
 * from which it can be quickly re-gotten. Yeah I suppose I could use
 * custom new/delete methods but that seems pretty painful and at least
 * as error prone as this code.
 */
class ZeroCopyHit : public RawChannel {
private:
  ReferenceCountedBuffer *m_pBuffer; //!< The hit lives in here.
  BufferArena *m_pArena;             //!< The buffer came from this arena.

public:
  /** @brief Default constructor. */
  ZeroCopyHit();
  /**
   * @brief Construct from parameters.
   * @param nWords   Number of uint32_t's in the hit.
   * @param pHitData Pointer to the hit data.
   * @param pBuffer  Underlying reference counted buffer.
   * @param pArena   Buffer arena to which the  buffer is released when
   *   no longer referenced.
   */
  ZeroCopyHit(size_t nWords, void *pHitData, ReferenceCountedBuffer *pBuffer,
              BufferArena *pArena);
  /**
   * @brief Copy constructor.
   * @param rhs Reference ot ZeroCopyHit object to copy.
   */
  ZeroCopyHit(const ZeroCopyHit &rhs);
  /**
   * @brief Assignment operator.
   * @param rhs Reference to ZeroCopyHit object assigned to lhs.
   * @return Reference to ZeroCopyHit object.
   */
  ZeroCopyHit &operator=(const ZeroCopyHit &rhs);
  /** @brief Destructor. */
  virtual ~ZeroCopyHit();

  // Support for recycling ZeroCopyHits:

  /**
   * @brief Sets a new hit.
   * @param nWords   Number of words in the new hit.
   * @param pHitData Pointer to the hit data.
   * @param pBuffer  Buffer from whence the hit data came.
   * @param pArena   Arena to which the buffer is returned.
   */
  void setHit(size_t nWords, void *pHitData, ReferenceCountedBuffer *pBuffer,
              BufferArena *pArena);
  /** @brief Free an existing hit. */
  void freeHit();

private:
  /** @brief Add a reference to the underlying buffer. */
  void reference();
  /**
   * @brief Release our reference to m_pBuffer and return it to its arena
   * if we were the last reference.
   */
  void dereference();
};

} // namespace DDASReadout

#endif
