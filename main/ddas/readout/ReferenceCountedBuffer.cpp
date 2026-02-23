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
 * @file ReferenceCountedBuffer.cpp
 * @brief Implement the reference counted buffer object.
 */

#include "ReferenceCountedBuffer.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <new>
#include <stdexcept>

namespace DDASReadout {

/**
 * @details
 * If initailSize is zero (the default) no storage is allocated and the
 * user must do a resize at some point to get storage.
 */
ReferenceCountedBuffer::ReferenceCountedBuffer(size_t initialSize)
    : s_size(0), s_references(0), s_pData(nullptr) {
  if (initialSize) {
    s_pData = malloc(initialSize);
    if (!s_pData) {
      throw std::bad_alloc();
    }
    s_size = initialSize;
  }
}

/**
 * @details
 *   It's currently an error to destroy the object if it has references.
 *   Otherwise, just free any storage that's been allocated.
 */
ReferenceCountedBuffer::~ReferenceCountedBuffer() {
  // C++ does not allow exceptions to leave destructors -- in standard.

  if (s_references) {
    std::cerr << "FATAL - Reference counted buffer "
              << "is being destroyed with referencres active!\n";
    assert(s_references == 0);
  }
  free(s_pData); // Harmless if its nullptr.
}

/**
 * @details
 * Increment the reference counter.
 */
void ReferenceCountedBuffer::reference() { s_references++; }

/**
 * @details
 * Deincrement the reference counter.
 */
void ReferenceCountedBuffer::dereference() { --s_references; }

bool ReferenceCountedBuffer::isReferenced() { return s_references > 0; }

/**
 * @details
 * This must not be done when there are references as the resize will
 * invalidate all pointers. Therefore if there are references, a
 * std::logic_error is thrown.
 *
 * @note If newSize < s_size no resize is done but the exception can
 * still be thrown.
 */
void ReferenceCountedBuffer::resize(size_t newSize) {
  if (s_references) {
    throw std::logic_error("Attempted resize of a referenced counted buffer "
                           "with active references");
  }

  if (newSize > s_size) {
    free(s_pData);     // Harmless if s_size == 0/s_pData == nullptr.
    s_pData = nullptr; // Prevent double free on throw/destruct.
    s_pData = malloc(newSize);
    if (!s_pData) {
      throw std::bad_alloc();
    }
    s_size = newSize;
  }
}

} // namespace DDASReadout
