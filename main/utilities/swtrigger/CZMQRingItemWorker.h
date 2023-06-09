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

/** @file: CZMQRingItemWorker.h
 *  @brief: Worker that processes ring items from a ZMQ Router (we use Dealer).
 */

#ifndef CZMQRINGITEMWORKER_H
#define CZMQRINGITEMWORKER_H

#include "CParallelWorker.h"
#include <stdint.h>


class CProcessor;

/**
 * @class CZMQRingItemWorker
 *    Work son ring items that are sent to us by way of a ZMQ
 *    ROUTER/DEALER fanout (we're the dealer).
 *
 *    The output of this processing element goes to an unspecified CSender
 *    object.  The actual sender type depends on how the computation is
 *    constructed.
 *
 *    The actual processing of items is handled by encapsulating a
 *    CProcessor object.   Note that in fact, this worker is actually fairly
 *    generic rather than, as the name of the class suggests, being confined
 *    to work items consisting of ring items.
 *    
 */
class CZMQRingItemWorker : public CParallelWorker
{
private:
    CProcessor*  m_pProcessor;
public:
    CZMQRingItemWorker(
        const char* routerUri, uint64_t clientId, CSender& sender,
        CProcessor* processor
     );
    virtual ~CZMQRingItemWorker() {}
    virtual void process(void* pData, size_t nBytes);
};


#endif