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

/** @file:  CDataSourceElement.cpp
 *  @brief: Implement the data source fanout processing element.
 */

#include "CDataSourceElement.h"
#include "CFanoutTransport.h"
#include "CReceiver.h"
#include "CSender.h"

#include <stdlib.h>

/**
 * constructor
 *    @param source - where the data comes from.
 *    @param fanout - Fanout transport on which to instantiate the sender.
 *    @note We assume the receiver and transports are dynamic and delete them
 *          in the destructor.
 */
CDataSourceElement::CDataSourceElement(
    CReceiver& source, CFanoutTransport& fanout
) : m_pSenderTransport(&fanout), m_pDataSource(&source), m_pFanout(nullptr)
{
    m_pFanout = new CSender(fanout);
}
/**
 * destructor
 *   See the assumption sin the constructor.  Note that the
 *   sender's destructor will kill off our transport.
 */
CDataSourceElement::~CDataSourceElement()
{
    delete m_pDataSource;
    delete m_pFanout;
}

/**
 * operator()
 *     Get message from the receiver and pass them on to the
 *     process method.  When the message size is 0 we return.
 *     Note that the message with size 0 is still passed to process.
 */
void
CDataSourceElement::operator()()
{
    void* pData;
    size_t nBytes(0);
    do {
        m_pDataSource->getMessage(&pData, nBytes);
        process(pData, nBytes);
        free(pData);        
    } while(nBytes > 0);
}

/**
 * process
 *    - If nBytes is nonzero pass it on to the sender.
 *    - If nBytes is zero tell the sender's transport we're at end.
 *
 * @param pData - Pointer to the data (message).
 * @param nBytes - Number of bytes in the message.
 */
void
CDataSourceElement::process(void* pData, size_t nBytes)
{
    if (nBytes > 0) {
        m_pFanout->sendMessage(pData, nBytes);
    } else {
        m_pSenderTransport->end();        // Tell transport we're done.
    }
}
