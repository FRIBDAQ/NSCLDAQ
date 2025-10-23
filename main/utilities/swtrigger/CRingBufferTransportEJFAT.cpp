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

/** @file:  CRingBufferTransportEJFAT.cpp
 *  @brief: Transport class for ring buffers.
 */

#include "CRingBufferTransportEJFAT.h"
#include <CRingBuffer.h>

#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <new>

#include <chrono>
#include <iostream>

static const int CHUNK_SIZE(1024*1024);
static const int POLL_COUNT(100);
static const int POLL_TIMING(10);

static size_t totalBytes(0); // debugging really

namespace ch = std::chrono;

/**
 * constructor (write)
 *    @param writer - the ring buffer object we're going to write items to.
 */
CRingBufferTransportEJFAT::CRingBufferTransportEJFAT(CRingBuffer& writer) :
    m_pWriter(&writer), m_pReader(nullptr), m_pCurrentChunk(nullptr),
    m_pIterator(nullptr), m_timeoutMs(2000), m_timedOut(false), m_seenData(false)
{}
/**
 * constructor (read)
 *   @param reader - The Chunkaccessor we're going to use to get high
 *                   performance access to the ring buffer (single copy).
 */
CRingBufferTransportEJFAT::CRingBufferTransportEJFAT(CRingBufferChunkAccess& reader) :
    m_pWriter(nullptr), m_pReader(&reader), m_pCurrentChunk(nullptr),
    m_pIterator(nullptr), m_timeoutMs(2000), m_timedOut(false), m_seenData(false)
{
}
/**
 * destructor
 */
CRingBufferTransportEJFAT::~CRingBufferTransportEJFAT()
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
CRingBufferTransportEJFAT::recv(void** ppData, size_t& size)
{
    if (!m_pReader) {
        throw std::logic_error(
            "CRingBufferTransportEJFAT attempted recv from a write-only transport instance"    
	    );
    }
    if (!m_pCurrentChunk) {
        nextChunk(); // waits at most m_timeoutMs ms at "end"
    }

    if (m_timedOut) { // Timed out nextChunk() and returned
	ppData = nullptr;
	size = 0; // trigger to end processing in CRingItemBlockSourceElement
	return;
    }

    if (!m_seenData) m_seenData = true;
    
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

    totalBytes += size;
    
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
CRingBufferTransportEJFAT::send(iovec* parts, size_t numParts)
{
    if (!m_pWriter) {
        throw std::logic_error(
            "CRingBufferTransportEJFAT attempted send from read-only transport"
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
CRingBufferTransportEJFAT::nextChunk()
{    
    size_t dataAvail;
    auto start = ch::high_resolution_clock::now();
    while (!(dataAvail = m_pReader->waitChunk(CHUNK_SIZE, POLL_COUNT, POLL_TIMING))) {
	auto now = ch::high_resolution_clock::now();
	// auto d = ch::duration_cast<ch::milliseconds>(now-start);
	// std::cerr << "seen " << totalBytes << " " << d.count() << "ms\n";
	if (now - start > ch::milliseconds(m_timeoutMs) && m_seenData) {
	    finishChunk(); // nulls everything, maybe not needed ??
	    m_timedOut = true;
	    return;
	}
    }
    
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
CRingBufferTransportEJFAT::finishChunk()
{
    delete m_pIterator;
    m_pIterator = nullptr;
    m_pCurrentChunk = nullptr;                    // Really internal to the chunk.
}
