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
 * @file ReferenceCountedBuffer.h
 * @brief Provides a reference counted buffer.
 */

#ifndef REFERENCECOUNTEDBUFFER_H
#define REFERENCECOUNTEDBUFFER_H

#include <cstddef>
#include <cstdint>

/** @namespace DDASReadout */
namespace DDASReadout {

/**
 * @struct ReferenceCountedBuffer
 * @brief A reference-counted buffer with dynamic storage.
 * @details
 * The idea is that rather than doing data movement, pointers can be
 * registered with the buffer and unregistered. The buffer can then
 * be queried to determine if it can be released, or put in a free list.
 *
 * One feature of this buffer is that, if there are not references,
 * it can be resized. Since resizing is only legal if there are
 * no references, the contents of the buffer are not preserved across
 * a resize.
 *
 * The intended use for this data structure is to provide support for
 * zero copy operations in the DDAS Readout program. The idea is that
 * a block of data can be read from a digitizer into one of these and
 * then pointers to each hit created rather than performing copies into
 * other data structures.
 */

struct ReferenceCountedBuffer {
  size_t s_size;       //!< Number of bytes of data.
  size_t s_references; //!< Number of references.
  void *s_pData;       //!< Data pointer.

  /**
   * @brief Construct the buffer.
   * @param initialSize Initial size in bytes allocated to the buffer
   *     (default = 0).
   * @throw std::bad_alloc Buffer malloc fails for initialSize > 0.
   */
  ReferenceCountedBuffer(size_t initialSize = 0);
  /** @brief Destructor. */
  virtual ~ReferenceCountedBuffer();

  // Reference count management:

  /** @brief Add a reference to the storage. */
  void reference();
  /** @brief Remove a reference to the storage. */
  void dereference();
  /**
   * @brief Are there references to the object?
   * @return bool
   * @retval true If the number of references is > 0.
   * @retval false Otherwise.
   */
  bool isReferenced();

  // Other:

  /**
   * @brief Resize the storage.
   * @param newSize The new size in bytes.
   * @throw std::logic_error If there are references.
   * @throw std::bad_alloc If the memory allocation fails.
   */
  void resize(size_t newSize);

  // Syntactical sugar for the most common casts; still nothing to stop
  // someone from mytyp* p = static_cast<mytyp*>(somebuffer.s_pData);

  operator uint8_t *() { return static_cast<uint8_t *>(s_pData); }
  operator uint16_t *() { return static_cast<uint16_t *>(s_pData); }
  operator uint32_t *() { return static_cast<uint32_t *>(s_pData); }
};

} // namespace DDASReadout

#endif
