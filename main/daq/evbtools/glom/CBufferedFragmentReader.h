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

/** @file:  CBufferedFragmentReader.h
 *  @brief: Read blocks of data and provide fragments from them.
 */
#ifndef CBUFFEREDFRAGMENTREADER_H
#define CBUFFEREDFRAGMENTREADER_H
#include <stddef.h>

namespace ufmt {
    namespace EVB {
	typedef struct _FlatFragment FlatFragment, *pFlatFragment; 
    }
}

/**
 * CBufferedFragmentReader
 *    Returns a stream of flat fragments from a buffered read.
 *    note that pointers to the fragment are returned rather than
 *    copying them.
 */
class CBufferedFragmentReader
{
private:
    int     m_nFd;              // File descriptor
    void*   m_pBuffer;          // Read/partial fragment buffer.
    size_t  m_nBufferSize;      // Current size of fragment buffer.
    size_t  m_nBytesInBuffer;   // Number of bytes from last read.
    size_t  m_nReadSize;        // How many bytes max to read.
    size_t  m_nOffset;          // Offset to next fragment to return.
public:
    CBufferedFragmentReader(int fd);
    virtual ~CBufferedFragmentReader();
    
    const ufmt::EVB::pFlatFragment getFragment();
private:
    bool mustRead();
    void fillBuffer();
    void* readPointer();
    void  readData();
    const ufmt::EVB::pFlatFragment cursor();
    void updateOffset(const ufmt::EVB::pFlatFragment pFrag);
    size_t fragSize(const ufmt::EVB::pFlatFragment pFrag);
};

#endif
