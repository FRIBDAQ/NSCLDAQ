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

/** @file:  CRingBufferTransport.h
 *  @brief: Ring item transport via a ring buffer.
 */
#ifndef CRINGBUFFERTRANSPORT_H
#define CRINGBUFFERTRANSPORT_H
#include "CRingItemTransport.h"

#include <CRingBufferChunkAccess.h>
#include <sys/uio.h>

class CRingBuffer;


/**
 * @class CRingBufferTransport
 *    Handles unidirectional transport via ring buffers.
 *    - Read accesses are done via a ring chunk accessor for
 *      single copy access.
 *    - Write accesses are done with low level ring buffer
 *      operations.
 */
class CRingBufferTransport : public CRingItemTransport
{
private:
    CRingBuffer*                             m_pWriter;
    CRingBufferChunkAccess*                  m_pReader;
    
    CRingBufferChunkAccess::Chunk           m_CurrentChunk;  // Reads...
    CRingBufferChunkAccess::Chunk*          m_pCurrentChunk;
    CRingBufferChunkAccess::Chunk::iterator* m_pIterator;
    
public:
    CRingBufferTransport(CRingBuffer& writer);           // For writes.
    CRingBufferTransport(CRingBufferChunkAccess& reader); // for reads.
    virtual ~CRingBufferTransport();
    
    virtual void recv(void** ppData, size_t& size);
    virtual void send(iovec* parts, size_t numParts);
private:
    void nextChunk();
    void finishChunk();
};


#endif
