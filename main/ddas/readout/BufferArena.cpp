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
 * @file BufferArena.cpp
 * @brief Implement the buffer arena class.
 */

#include "BufferArena.h"

#include <stdexcept>

#include "ReferenceCountedBuffer.h"

namespace DDASReadout {

/**
 * @details
 * Destroy all buffers that are in the buffer pool. Note that there's no
 * good way at present to detect destruction while there are still buffers
 * outstanding.
 */
BufferArena::~BufferArena() {
  while (!m_BufferPool.empty()) {
    delete m_BufferPool.front();
    m_BufferPool.pop_front();
  }
}

/**
 * @details
 * If the pool is not empty the front element is gotten and resized.
 * Otherwise, a new buffer element is created.
 */
ReferenceCountedBuffer *BufferArena::allocate(size_t nBytes) {
  if (m_BufferPool.empty()) {
    m_BufferPool.push_back(new ReferenceCountedBuffer);
  }
  ReferenceCountedBuffer *pResult = m_BufferPool.front();
  m_BufferPool.pop_front();

  pResult->resize(nBytes);
  return pResult;
}

/**
 * @note This is only legal if the buffer is not referenced.
 */
void BufferArena::free(ReferenceCountedBuffer *pBuffer) {
  if (pBuffer->isReferenced()) {
    throw std::logic_error(
        "Freeing a buffer to a buffer arena that's still referenced");
  }
  m_BufferPool.push_back(pBuffer);
}
} // namespace DDASReadout
