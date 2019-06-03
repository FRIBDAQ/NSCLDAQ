/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
* @file  CRingItemMarkingWorker.cpp
* @brief Implements the CRingItemMarkingWorker class.
*/

#include "CRingItemMarkingWorker.h"
#include "CSender.h"
#include "CRingItemSorter.h" // message format.

#include <CRingItem.h>
#include <CRingItemFactory.h>
#include <DataFormat.h>
#include <stdexcept>
#include <vector>



// Shorthand for the messages:

typedef CRingItemSorter::Item Message, *pMessage;



/**
 * constructor
 *    Construct the base class and save our classifier instance.
 *
 * @param fanin - Transport that is a client to a fanout so we run in parallel.
 * @param sink  - Data sink - we'll put data here.
 * @param clientId - Our id with the fanout.
 * @param pClassifier - pointer to the classifier functional.
 */
CRingMarkingWorker::CRingMarkingWorker(
    CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId,
    Classifier* pClassifier
) : CParallelWorker(fanin, sink, clientId), m_pClassifier(pClassifier) {}

/**
 * process
 *    - Non physics items just pass through.
 *    - Physics items with no body header are an std::invalid_argument exception.
 *    - Physics items with body headers are passed to the classifier
 *      and emitted with the classification added  to the body header.
 */
void
CRingMarkingWorker::process(void* pData, size_t nBytes)
{
    if (nBytes) {
        // Figure out how many Items we have.  The maximum iovec size is
        // 3*nItems.  For each item:
        // - Stuff before the body header extension.
        // - body header extension value.
        // - Stuff after the body header extension.
        //
        // Note that not all items get a vbody header extension, but 3*n is the max.
        // so that's what we'll allocate.
        
        size_t nItems = countItems(pData, nBytes);
        iovec messageParts[nItems*3];            // None yet.
        size_t nParts(0);                        // None yet.
        std::vector<uint32_t> classifications;   // Holds the classifications until sent.
        
        pMessage p = static_cast<pMessage>(pData);
        
        for (int i =0; i < nItems; i++) {
            // Items that are not PHYSICS_EVENTS are not classified:
            
            if (p->s_item.s_header.s_type != PHYSICS_EVENT) {
                messageParts[nParts].iov_base = p;
                messageParts[nParts].iov_len  = messageSize(p);
                nParts++;                                  // Only need one part.
            } else {
                // If there's no body  header, that's a fatal error.
                
                pRingItem pRItem = &p->s_item;
                if (pRItem->s_body.u_noBodyHeader.s_mbz == 0) {
                    throw std::invalid_argument("Got a physics event with no body header!!");
                } else {
                    CRingItem* pRingItemObj = CRingItemFactory::createRingItem(pRItem);
                    classifications.push_back((*m_pClassifier)(*pRingItemObj));
                    delete pRingItemObj;
                    nParts += createClassifiedParts(
                        &messageParts[nParts], p, (classifications.back())
                    );
                }
            }
            
            p = static_cast<pMessage>(nextItem(p));
        }
        // Send the data.  Caller frees the message.
        
        getSink()->sendMessage(messageParts, nParts);
    }
}
///////////////////////////////////////////////////////////////////////////////
// Private members:

/**
 * countItems
 *    Count the numgber of Message items in a message.
 *
 * @param pData - points to the data received.
 * @param nBytes - number of bytes in the data
 * @return size_t number of items in the message
 * @note each item is of type 'Message' defined at the start of this file.
 */
size_t
CRingMarkingWorker::countItems(const void* pData, size_t nBytes)
{
    const Message* pM = reinterpret_cast<const Message*>(pData);
    size_t result(0);
    while (nBytes) {
        result++;
        
        size_t itemSize = messageSize(pM);
        pM              = reinterpret_cast<const Message*>(nextItem(pM));
        nBytes         -= itemSize;
    }
    return result;
}
/**
 * nextItem
 *   Given a pointer to a message, returns a pointer to th e next message.
 *   It's up to the caller to prevent this from falling off the end of the world.
 *
 *  @param pData pointer to a message item.
 *  @return void* pointer to the next message item.
 *                really a pointer to the byte following the message item.
 */
void*
CRingMarkingWorker::nextItem(const void* pData)
{
    size_t nBytes = messageSize(pData);
    uint8_t* p    = const_cast<uint8_t*>(static_cast<const uint8_t*>(pData));
    p += messageSize(pData);
    return p;
}
/**
 * messageSize
 *   Given what is actually a pointe to a const pMessage, compute the
 *   number of bytes required by that message.
 *   The asumptiomn is there's no padding between the timestamp and the
 *   ringitem.
 *
 * @param pData - pointer to the message item.
 */
size_t
CRingMarkingWorker::messageSize(const void* pData)
{
    const Message* pm = static_cast<const Message*>(pData);
    return sizeof(uint64_t) + pm->s_item.s_header.s_size;
}
/**
 * createClassifiedParts
 *    Given a message part and its classifier, create the
 *    iovector elemnts that can be used to ouutput the item.
 *    Note that the original item's body header size will be modified
 *    to include the additional uint32_t as well.
 *
 *  @param[out] vec - pointer to the io vector elements to fill in.
 *  @param[inout]  pData - pointer to the original message (pMessage).
 *  @param[in] classification - the classification to put on this ring item.
 *  @return size_t number of io vector items used.
 */
size_t
CRingMarkingWorker::createClassifiedParts(
    iovec* vec, void* pData, uint32_t& classification
)
{
    pMessage p   = static_cast<pMessage>(pData);
 
    // Adjust the body header size and size upwards for the uint32_t:
    
    size_t ringItemSize = p->s_item.s_header.s_size;   // Original size.
    p->s_item.s_header.s_size += sizeof(uint32_t);
    p->s_item.s_body.u_hasBodyHeader.s_bodyHeader.s_size += sizeof(uint32_t);
    
    
    // Describe the message up through the old body header:
    
    vec[0].iov_base = p;
    vec[0].iov_len  = sizeof(uint64_t) + sizeof(RingItemHeader) + sizeof(BodyHeader);
    
    // The classification is just the uint32:
    
    vec[1].iov_base = &classification;
    vec[1].iov_len  = sizeof(uint32_t);
    
    // Now the remainder:
    
    vec[2].iov_base = p->s_item.s_body.u_hasBodyHeader.s_body;
    vec[2].iov_len  = ringItemSize - (sizeof(RingItemHeader) + sizeof(BodyHeader));
    

    return 3;    
}