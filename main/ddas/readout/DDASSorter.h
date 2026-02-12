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
 * @file DDASSorter.h
 * @brief Define the class that does all the timestamp sorting.
 */

#ifndef DDASSORTER_H
#define DDASSORTER_H

#include <deque>

#include <CRingBufferChunkAccess.h>

class CRingBuffer;

namespace DDASReadout {
class BufferArena;
class ReferenceCountedBuffer;
class ZeroCopyHit;
class HitManager;
} // namespace DDASReadout

typedef struct _RingItemHeader *pRingItemHeader;

/**
 * @class DDASSorter
 * @brief Class to manage data flow and timestamp ordering.
 * @details
 * The DDASSorter class processes ring items:
 * - Non event ring items are just passed on through.
 * - Event items are parsed for hits which are added to the hit manager.
 * - If hits are available from the hit manager they are passsed as output
 *   ring items.
 * - When the end of run item is seen, the hit manager is flushed prior to
 *   sending the end run item to the output file.
 */

class DDASSorter {
private:
  CRingBuffer &m_source;              //!< Ringbuffer data source.
  CRingBuffer &m_sink;                //!< Ringbuffer data sink.
  DDASReadout::HitManager *m_pHits;   //!< Management of sorted hits.
  DDASReadout::BufferArena *m_pArena; //!< Raw ring item storage buffers.
  std::deque<DDASReadout::ZeroCopyHit *> m_hits; //!< Reusable hit pool.
  uint32_t m_sid;                                //!< Source ID.
  double m_lastEmittedTimestamp; //!< Timestamp of last emitted hit.

public:
  /**
   * @brief Constructor.
   * @param source Input data ringbuffer.
   * @param sink   Output data ringbuffer.
   * @param window Accumulation window in seconds.
   */
  DDASSorter(CRingBuffer &source, CRingBuffer &sink, float window = 10.0);
  /** @brief Destructor. */
  ~DDASSorter();

  /** @brief Defines the dataflow of the sorter. */
  void operator()();

private:
  /**
   * @brief Processes a chunk of ring items from the ring buffer.
   * @param chunk References a chunk of ring items gotten from the ringbuffer.
   */
  void processChunk(CRingBufferChunkAccess::Chunk &chunk);
  /**
   * @brief Output a ring item to the sink.
   * @param pItem Pointer to the raw ring item to output.
   */
  void outputRingItem(pRingItemHeader pItem);
  /**
   * @brief Process a ring item for outputting.
   * @param pItem References the ring item to process.
   * @throw std::string With a message if we're about to run off the end
   *   of a ring item while processing.
   */
  void processHits(pRingItemHeader pItem);
  /** @brief Flush hits on the end of run. */
  void flushHitManager();
  /**
   * @brief Attempts to allocate a hit from the pool of hits in m_hits.
   * If that pool is exhausted a new one is created.
   * @return Pointer to the allocated hit.
   */
  DDASReadout::ZeroCopyHit *allocateHit();
  /**
   * @brief Returns a hit to the free pool where it can be allocated again
   * without dynamic memory management.
   * @param pHit Hit to return to the pool.
   */
  void freeHit(DDASReadout::ZeroCopyHit *pHit);
  /** @brief Create a ring item from a ZeroCopyHit and output it. */
  void outputHit(DDASReadout::ZeroCopyHit *pHit);
};

#endif
