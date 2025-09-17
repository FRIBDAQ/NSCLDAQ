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

/** @file:  CRingItemBlockSourceElement.h
 *  @brief: Base class for sources that send blocks of ring items.
 */
#ifndef CRINGITEMBLOCKSOURCELEMENT_H
#define CRINTITEMBLOCKSOURCELEMENT_H
#include "CDataSourceElement.h"
#include <stdint.h>
#include <vector>
/**
 * @class CRingItemBlockSourceElement
 *     Fanout data source that sends blocks of ring items in order to reduce
 *     send overhead per item.   This can be used as a base class
 *     for sources that use arbitrary transports.
 */
class CRingItemBlockSourceElement : public CDataSourceElement
{
public:
    typedef struct _Message {
        uint64_t  s_timestamp;
        size_t    s_nBytes;
        void*     s_pData;
    } Message, *pMessage;

private:
    size_t m_nChunkSize;
    uint64_t m_nLastTimestamp;
    
 
    std::vector<Message>   m_chunk;

    int m_beginSeen;
    int m_endSeen;
    
public:
    CRingItemBlockSourceElement(
        const char* ringUri, CFanoutTransport& fanout, size_t chunkSize = 1
    );
    virtual ~CRingItemBlockSourceElement() {}
    virtual void operator()();             // Override b/c process frees memory.
    virtual void process(void* pData, size_t nBytes);
private:
    void sendChunk();
    void clearChunk();
    
};


#endif
