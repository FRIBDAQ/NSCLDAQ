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

/** @file:  ReferenceCountedBuffer.cpp
 *  @brief: Implement the reference counted buffer object.
 */
#include "ReferenceCountedBuffer.h"
#include <stdlib.h>
#include <new>
#include <iostream>
#include <stdexcept>
#include <assert.h>
namespace DDASReadout {
    
/**
 * constructor
 *   Constructs the buffer.
 *   @param initialSize - number of _bytes_ that will be allocated to the
 *                        buffer initiallly.  If zero (the default) no
 *                        storage is allocated and the user must do a resize
 *                        at some point to get storage.
 */
ReferenceCountedBuffer::ReferenceCountedBuffer(size_t initialSize) :
    s_size(0), s_references(0), s_pData(nullptr)
{
    if (initialSize) {
        s_pData = malloc(initialSize);
        if (!s_pData) {
            throw std::bad_alloc();
        }
        s_size = initialSize;
    }
}

/**
 * destructor
 *   Note that it's currently an error to destroy the object if it has references
 *   Otherwise, just free any storage that's been allocated.
 */
ReferenceCountedBuffer::~ReferenceCountedBuffer()
{
    // C++ does not allow exceptions to leave destructors -- in standard.
    
    if (s_references) {
        std::cerr << "FATAL - Reference counted buffer is being destroyed with referencres active!\n";
        assert(s_references == 0);
    }
    free(s_pData);                      // harmless if its nullptr.
}
/**
 * reference
 *   - add a reference to the storage.
 */
void
ReferenceCountedBuffer::reference()
{
    s_references++;
}
/**
 * dereference
 *    Remove a reference to the storage:
 */
void
ReferenceCountedBuffer::dereference()
{
    --s_references;
}
/**
 * isReferenced
 *   @return bool - true if there are references to the object.
 */
bool
ReferenceCountedBuffer::isReferenced()
{
    return s_references > 0;
}
/**
 *  resize
 *     Resize the storage.  This must not be done when there are references
 *     as the resize will invalidate all pointers. Therefore if there are
 *     references, an std::logic_error is thrown.
 *
 *  @param newSize
 *  @note  If newSize < s_size no resize is done but the exception can still be thrown.
 *  @throws std::logic_error - if there are references.
 *  @throws std::bad_alloc   - if memory allocation fails.
 */
void
ReferenceCountedBuffer::resize(size_t newSize)
{
    if (s_references) {
        throw std::logic_error(
            "Attempted resize of a referenced counted buffer with active references"
        );
    }
    if (newSize > s_size) {
        free(s_pData);                   // Harmless if s_size == 0/s_pData == nullptr.
        s_pData = nullptr;              // prevent double free on throw/destruct.
        s_pData = malloc(newSize);
        if (!s_pData) {
	  std::cout << "bad_alloc() of size " << newSize << std::endl;
	  throw std::bad_alloc();       
        } 
        s_size = newSize;
    }
}




}    // namespace DDASReadout
