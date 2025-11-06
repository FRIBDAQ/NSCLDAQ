/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     Aaron Chester
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CBufferedFragmentReader.cpp
 *  @brief: Implement a buffered fragment reader.
 */

#include "CBufferedFragmentReader.h"
#include <fragment.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <new>
#include <system_error>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <poll.h>
#include <ios>
#include <iostream>

static const size_t DefaultReadSize(32768);

using namespace ufmt::EVB;

/**
 * constructor
 *   Allocate the initial buffer and set up the conditions so that
 *   mustRead will immediately return true.   The read size
 *   is determined as follows:
 *   - If This is a pipeline, the pipe buffersize is used.
 *   - If not a default buffersize of 32768 is used.
 *   - The fd is put into nonblocking mode regardless so that
 *     once data are available, reads will not wait for the entire
 *     block to be available (this means we can't use io::readData as it will
 *     retry until all the data are in).
 *     
 *  @param fd   - File descriptor from which we'll be reading.
 */   
CBufferedFragmentReader::CBufferedFragmentReader(int fd) :
    m_nFd(fd), m_pBuffer(nullptr), m_nBufferSize(0), m_nBytesInBuffer(0),
    m_nReadSize(0), m_nOffset(0)
{
    // Figure out the read size and allocate the block etc. If reading
    // from a pipe, try to set the buffer size to 1MB (see issue #307):

    fcntl(STDIN_FILENO, F_SETPIPE_SZ, 1024*1024);
    int nPipeDepth = fcntl(m_nFd, F_GETPIPE_SZ); 
    if (nPipeDepth > 0) {
        m_nReadSize = nPipeDepth;
    } else {
        m_nReadSize = DefaultReadSize;
    }

    // We know its stdin so lets allocate the block but make it look like
    // it's been exactly fully read:

    m_pBuffer = malloc(m_nReadSize);
    if (!m_pBuffer) {
        throw std::bad_alloc();
    }
    m_nBufferSize = m_nReadSize;
    m_nBytesInBuffer = 0;               // No data.
    m_nOffset        = 0;               // All read.
    
    // Set the file descriptor to non blocking.
    
    int fdFlags = fcntl(m_nFd, F_GETFD);
    if (fdFlags < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            "CBufferedFragmentReader - getting flags from file descriptor."
        );
    }
    fdFlags |= O_NONBLOCK;
    int stat = fcntl(m_nFd, F_SETFD, fdFlags);
    if (stat < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            "CBufferedFragmentReader - setting nonblocking on file descriptor."
        );
    }
}
/**
 * Destructor:
 *    Free the data buffer.
 */
CBufferedFragmentReader::~CBufferedFragmentReader()
{
   free(m_pBuffer); 
}
const pFlatFragment
CBufferedFragmentReader::getFragment()
{
    // This blocks until we have at least one fragment:
    
    while (mustRead()) {
        fillBuffer();    
    }
    // At thist time, m_nOffset is an offset in to m_pBuffer
    // at which we can find a fragment.
    
    const pFlatFragment result = cursor();
    updateOffset(result);
    
    return result;
}
///////////////////////////////////////////////////////////////////////////////
// Internally used methods:
//


/**
 * mustRead
 *    @return bool - true if to satisfy the next getFragment() call requires
 *            us to read more data.
 *
 *   - If There's not enough used room in the buffer to hold a fragment header,
 *     we have to read.
 *   - If we have a fragment header but the unread space in the buffer is less
 *     than the size of that header + the size of its payload, we must read.
 */
bool
CBufferedFragmentReader::mustRead()
{
    size_t unread = m_nBytesInBuffer - m_nOffset;
    if (unread < sizeof(FragmentHeader)) return true;
    
    // So we have a header,  let's see how much more space we need:
    
    pFlatFragment here = cursor();
    size_t s = fragSize(here);
    if (unread < s) return true;
    
    return false;
}
/**
 * fillBuffer
 *    - If necessary moves the current partial fragment to the front of the
 *      buffer adjusting m_nBytesInBuffer (see readPointer).
 *    - Uses poll to block until data are available on the fd.
 *    - Does a read to append data into the buffer.  Note that
 *      since read data are available, a 0 back from this read
 *      indicates and end file condition.  If that happens we throw
 *      an std::stream failure with a text message that indicates an end file.
 *
 *
 *    Our poll uses a long timeout and retries in any event.
 */
void
CBufferedFragmentReader::fillBuffer()
{
    pollfd p = {m_nFd, POLLIN ,0};
    int status;
    while ((status = poll(&p, 1, 3600*24*1000)) == 0) 
        ;
    // We got something:
    //  status == -1  is an error which we report as errno.
    //  f.revents not contaning POLLIN indicates an EOF??
    
    if (status < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            "CBufferedFragmentReader - poll/wait for input failed"
        );
    }
    if ((p.revents & POLLIN) == 0) {
        throw  std::ios_base::failure(
            "CBufferedFragmentReader: EOF - maybe detected in poll."
        );
    }
    // We have data so read what is availsable:
    
    readData();
}
/**
 * readPointer
 *    Returns a pointer to where the next read should happen.
 *    -  If there are byts in the buffer, they are moved to the buffer front.
 *    -  m_nReadSize is computed from m_nBufferSize and the number of
 *       unread byrtes.
 *    -  m_nOffset is reset to zero.
 *    -  m_nBytesInBuffer is set to the amount of data we need to move.
 * @return void* - pointer just past any data that was moved by use to the
 *       front of the buffer.
 */
void*
CBufferedFragmentReader::readPointer()
{
    size_t unread = m_nBytesInBuffer - m_nOffset;   // un-processed bytes.
    
    if (unread > 0) {
        uint8_t* pSrc = static_cast<uint8_t*>(m_pBuffer);
        pSrc         += m_nOffset;
        memmove(m_pBuffer, pSrc, unread);
    }
    // reset all the book keeping stuff:
    
    m_nBytesInBuffer = unread;
    m_nOffset        = 0;
    
    uint8_t* result = static_cast<uint8_t*>(m_pBuffer);
    result         += unread;                  // Append data here.
    m_nReadSize     = m_nBufferSize - unread;  // Can read this much.
    
    return result;
}
/**
 * readData
 *    Reads data from the fd
 *    -  If 0 bytes are read, we signal the end of file.
 *    -  If <0 bytes are read and the reason is EAGAIN or EINTR we try again
 *    -  until we get 0 or >0.
 *    -  If > 0, we update m_nBytesInBuffer.
 */
void
CBufferedFragmentReader::readData()
{
    
    while (1) {
        void* p = readPointer();                   // sets m_nReadSize
        ssize_t nRead = read(m_nFd, p, m_nReadSize);
        if (nRead == 0) {
            throw std::ios_base::failure(
                "CBufferedFragmentReader - end file on read"
            );
        } else if (nRead < 0) {
            if ((errno != EINTR) && (errno != EAGAIN) && (errno != EWOULDBLOCK)) {
                throw std::system_error(
                    errno, std::generic_category(),
                    "CBufferedFragmentReader - read(2) failed"
                );
                                        
            }
        } else {
            m_nBytesInBuffer += nRead;
            break;
        }
    }
}
/**
 * cursor
 *   @return const pFlatFragment - Where the current fragment lives.
 */
const pFlatFragment
CBufferedFragmentReader::cursor()
{
    uint8_t* p = static_cast<uint8_t*>(m_pBuffer);
    p         += m_nOffset;
    return reinterpret_cast<pFlatFragment>(p);
}
/**
 * updateOffset
 *   Given the most recent fragment pointer updates m_nOffset to point
 *   to the next fragment (if there is one).
 *
 * @param pFrag - a pointer to a flat fragment that must be entirely within the
 *                buffer, and must be represented, as well, by m_nOffset.
 *   
 */
void
CBufferedFragmentReader::updateOffset(const pFlatFragment pFrag)
{
    m_nOffset += fragSize(pFrag);
}
/**
 * fragSize
 *    @param pFrag - pointer to a flat fragment.
 *    @return size_t - number of bytes in the fragment pointed to by pFrag
 */
size_t
CBufferedFragmentReader::fragSize(const pFlatFragment pFrag)
{
    return sizeof(FragmentHeader) + pFrag->s_header.s_size;
}
