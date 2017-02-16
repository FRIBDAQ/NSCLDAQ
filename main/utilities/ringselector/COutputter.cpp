/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "COutputter.h"
#include "RingBufferQueue.h"
#include <io.h>
#include <V12/CRingItem.h>
#include <V12/CRawRingItem.h>
#include <V12/DataFormat.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * constructor.
 * 
 * @param ringQueues - the inter thread buffer queue contains class we use to 
 *                     communicate with the main thread.
 * @param oneshot    - Boolean indicating if we should exit when we see the first end of run.,
 *                     ring item.  The assumption in this logic is that the application is not
 *                     being used in critical data flow applications so we don't need to do 
 *                     Begin/End counting/matching.
 *
 */
COutputter::COutputter(Queues& ringQueues, bool oneshot) :
  m_RingQueues(ringQueues), m_exitOnEnd(oneshot)
{
}

/**
 * Destructor.
 *  Join with the thread (hopefully that looks at living and returns if it's false without trying
 *  to do a pthread join.
 */

COutputter::~COutputter()
{
  join();			// Wait for thread exit.
}

/**
 * run
 *   Entry point for the thread.  Our operation is pretty simple.  Get data from  the
 *   queues.  Write the ring item to stdout and free it for re-use.
 */
void
COutputter::run()
{
    using DAQ::V12::CRingItem;
    using DAQ::V12::CRawRingItem;
    using DAQ::V12::END_RUN;

  while(1) {
    CRingItem* pItem = m_RingQueues.receive(); // blocks.
    uint32_t   type  = pItem->type();

    auto& rawItem = dynamic_cast<CRawRingItem&>(*pItem);

    std::array<char,20> header;
    serializeHeader(rawItem, header.begin());
    io::writeData(STDOUT_FILENO, header.data(), 20);  // Failures throw exiting.

    auto& body = rawItem.getBody();
    io::writeData(STDOUT_FILENO, body.data(), body.size() );  // Failures throw exiting.

    m_RingQueues.Free(pItem);		// Free's the item adds to free queue.

    if (m_exitOnEnd && (type == END_RUN)) {
      exit(0);
    }
  }
}
