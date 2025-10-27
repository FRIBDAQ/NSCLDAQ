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

/** @file:  CRingBufferTransport.cpp
 *  @brief: Transport class for ring buffers.
 */

#include "CRingBufferTransport.h"
#include <CRingBuffer.h>

#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <new>

#include <chrono>

static const int CHUNK_SIZE(1024*1024);
static const int POLL_COUNT(100);
static const int POLL_TIMING(10);

/**
 * constructor (write)
 *    @param writer - the ring buffer object we're going to write items to.
 */
CRingBufferTransport::CRingBufferTransport(CRingBuffer& writer) :
    m_pWriter(&writer), m_pReader(nullptr), m_pCurrentChunk(nullptr),
    m_pIterator(nullptr)
{}
/**
 * constructor (read)
 *   @param reader - The Chunkaccessor we're going to use to get high
 *                   performance access to the ring buffer (single copy).
 */
CRingBufferTransport::CRingBufferTransport(CRingBufferChunkAccess& reader) :
    m_pWriter(nullptr), m_pReader(&reader), m_pCurrentChunk(nullptr),
    m_pIterator(nullptr)
{
}
/**
 * destructor
 */
CRingBufferTransport::~CRingBufferTransport()
{
    delete m_pWriter;
    delete m_pReader;
    delete m_pIterator;
    
    // The chunk  will get deallocated by m_pReader.
}

/**
 * recv
 *    Get the next ring item from the ring buffer.
 *    - If a chunk has not already gotten one is waited for and gotten.
 *    - The next ring item is gotten from the chunk.
 *    - If that leaves the chunk empty, data are setup so that the next time
 *      a new chunk will be gotten.
 *
 *  @param ppData - pointer to where a pointer to the ring item will be stored.
 *  @param size   - Reference to where the ring item size will be put.
 *  @note Caller must free(3) the data it gets.
 */
void
CRingBufferTransport::recv(void** ppData, size_t& size)
{
    if (!m_pReader) {
        throw std::logic_error(
            "CRingBufferTransport attempted recv from a write-only transport instance"    
	    );
    }
    if (!m_pCurrentChunk) {
        nextChunk();                // Note this can take a _long_ time.
    }
    RingItemHeader& rHeader(**m_pIterator);
    void*           pResult = malloc(rHeader.s_size);
    if (!pResult) {
	throw std::bad_alloc();
    }
    memcpy(pResult, &rHeader, rHeader.s_size);
    *ppData = pResult;
    size    = rHeader.s_size;
    
    // If that was the last item in the ring buffer ensure we get a new chunk
    
    (*m_pIterator)++;
    if ((*m_pIterator) == m_pCurrentChunk->end()) {
	finishChunk();
    }
    
}
/**
 * send
 *   Send the I/O vector to the ring buffer.
 *   No assumption is made about the structure of the items in the
 *   iovec
 *
 *  @param parts - Pointers to the parts to send.
 *  @param numParts - number of part to send.
 *  @throw std::logic_error if this is a reader not a writer.
 */
void
CRingBufferTransport::send(iovec* parts, size_t numParts)
{
    if (!m_pWriter) {
        throw std::logic_error(
            "CRingBufferTransport attempted send from read-only transport"
        );
    }
    for (int i = 0; i < numParts; i++) {
        m_pWriter->put(parts[i].iov_base, parts[i].iov_len);
    }
        
}
/////////////////////////////////////////////////////////////////////
// Utilities for m_pReader
//

/**
 * nextChunk
 *    Wait for data (could be a long time)
 *    Get the next chunk from the ring buffer.
 *    Create an iterator to the beginning of that chunk.
 */
void
CRingBufferTransport::nextChunk()
{    
    size_t dataAvail;
    while(!(dataAvail = m_pReader->waitChunk(CHUNK_SIZE, POLL_COUNT, POLL_TIMING)));
    
    // Now a chunk should be ready... doing this allows for m_pCurrentChunk
    // to be a nullptr indicating we need a next chunk.
    
    m_CurrentChunk  = m_pReader->nextChunk();
    m_pCurrentChunk = &m_CurrentChunk;
    
    // The stuff below is just like getting begin...
    
    m_pIterator     =
        new CRingBufferChunkAccess::Chunk::iterator(
            m_pCurrentChunk->getStorage(), m_pCurrentChunk->size()
	    );
}
/**
 * finishChunk
 *    Delete the iterator and null the chunk poiner indicating that
 *    we need to get a chunk next time.
 */
void
CRingBufferTransport::finishChunk()
{
    delete m_pIterator;
    m_pIterator = nullptr;
    m_pCurrentChunk = nullptr;                    // Really internal to the chunk.
}
