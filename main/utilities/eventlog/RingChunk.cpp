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

/** @file:  RingChunk.cpp
 *  @brief: Implement the ring chunk class.
 */
#include "RingChunk.h"
#include <CRingBuffer.h>
#include <DataFormat.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>


/**
 * constructor
 *   Just initializes the data:
 *
 * @param pBuffer - ring buffer we find chunks in.
 * @param combine - True if runs can be combined.
 */
CRingChunk::CRingChunk(CRingBuffer* pBuffer, bool combine) :
    m_pRing(pBuffer),
    m_fChangeRunOk(combine),
    m_nRunNumber(0),
    m_nFd(-1)
{}

/**
 * setRunNumber
 *    Sets the new run number.
 *  @param newRun - new run number.
 */
void
CRingChunk::setRunNumber(uint32_t newRun)
{
    m_nRunNumber = newRun;
}
/**
 * setFd
 *   Sets the file descriptor to which data are written.
 *   This is used when unexpected run number changes are seen
 *   and m_fChangeRunOk is false.
 *
 * @int newFd  - The new file descrpitor.
 */
void
CRingChunk::setFd(int newFd)
{
    m_nFd = newFd;
}

/**
 * getChunk
 *    Get the next contiguous chunk of ring items.
 *    The search stops when either we have no more data available or
 *    the next ring item wraps.  Ring items are considered to wrap if
 *    either there's less than a uint32_t before the top of the ring
 *    or if there's a uint32_t ring item size that shows there's not enough
 *    space before the ring top to hold the item.
 *
 *  @param[out] nextChunk - will describe the chunk of data we've gotten.
 */
void
CRingChunk::getChunk(Chunk& nextChunk)
{
  nextChunk.s_pStart  = m_pRing->getPointer();
  nextChunk.s_nBytes  = 0;
  nextChunk.s_nEnds   = 0;
  nextChunk.s_nBegins = 0;
  
  size_t bytesAvail = m_pRing->availableData();
  
  
  // Adjust the bytes available by the amount left before a wrap, if that's
  // < a ring item header we have a wrap:
  
  size_t bytesToWrap = m_pRing->bytesToTop();
  if (bytesToWrap < bytesAvail) bytesAvail = bytesToWrap;
  if (bytesAvail < sizeof(RingItemHeader)) return;   // No complete items yet.
  
  /*
   *  Now we look at the data availalble;
   *  - Count bytes that contain full ring items:
   *  - Count any state transition items we see.
   */
  
  uint8_t* p = static_cast<uint8_t*>(nextChunk.s_pStart);
  while(1) {
    pRingItemHeader h = reinterpret_cast<pRingItemHeader>(p);
    // Not even enough space for a header:

    if ((sizeof(RingItemHeader)) > bytesAvail) return;  
    
    if ((h->s_size) > bytesAvail) return; // no full items left.
    nextChunk.s_nBytes += h->s_size;
    bytesAvail -= h->s_size;
    p += h->s_size;
    if (h->s_type == BEGIN_RUN) {
      // - have to do this in the caller. m_nBeginsSeen++;
      nextChunk.s_nBegins++;
      if(badBegin(h)) {
        closeEventSegment();
        std::cerr << " Begin run changed run number without --combine-runs "
          << " or too many begin runs for the data source count\n";
        exit(EXIT_FAILURE);
      }
    }
    if (h->s_type == END_RUN)   nextChunk.s_nEnds++;
  }
  
}
/**
 * nextItemWraps
 *    @return bool:  true if the next wring item wraps;
 *
 *    The next item wraps if:
 *    -  There's less than the size of a ring item header to the top.
 *    -  When we get the next uint32_t there's less than that amount of space
 *       left to the top:
 */
bool
CRingChunk::nextItemWraps()
{
  // Wait for at least a uint32_t:
  
  waitForData(sizeof(uint32_t));
  size_t nToTop = m_pRing->bytesToTop();
  uint32_t* p   = static_cast<uint32_t*>(m_pRing->getPointer());
  
  // Counting on short circuit here which is ok by C++ standard.
  
  return (nToTop < sizeof(RingItemHeader)) || (*p > nToTop);
  
}
/**
 *  closeEventSegment
 *     Truncate the event segment to its current size and close it.
 *
 */
void
CRingChunk::closeEventSegment()
{
  off_t fileSize = lseek(m_nFd, 0, SEEK_CUR);  // Tricky way to get the offset.
  ftruncate(m_nFd, fileSize);
  close(m_nFd);
  m_nFd = -1;           // Has to be re-set by the client.
  
}
/**
 * waitForData
 *    Wait until the ring item has at least a required number of bytes
 *    for us.  The blocking part of the wait increases with each failure
 *    in order to adapt to actual data rates.
 *
 *  @param nBytes - desired minimum number of bytes in the ring.
 */
void
CRingChunk::waitForData(size_t nBytes)
{
  while (m_pRing->availableData() < nBytes)
    ;

}
/**
 * getBody
 *    Given a ring item returns a pointer to the body.
 *
 * @param pItem - pointer to the ring item to check
 * @return void* - Pointer to the body of the ring item.
 */
void*
CRingChunk::getBody(void* p)
{
    pRingItem pItem = static_cast<pRingItem>(p);
    return bodyPointer(pItem);
}

/**
 * badBegin
 *    @param p - pointer to a state transition item with type BEGIN_RUN
 *    @return bool - true if the begin_run item indicates a  problem.
 */
bool
CRingChunk::badBegin(void* p)
{
  pStateChangeItem pItem = static_cast<pStateChangeItem>(p);
  
  // If run changes are allowed or we're not exiting on end of run
  // this is always ok:
  
  if (m_fChangeRunOk ) {
    return false;
  }

  
  // If the run number changed that's also bod:
  
  pStateChangeItemBody pBody =
    reinterpret_cast<pStateChangeItemBody>(getBody(pItem));
  
  return (pBody->s_runNumber != m_nRunNumber);
  
  
}
