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

/** @file:  CMPIFanoutTransport_mpi.cpp
 *  @brief: Implement the CMPIFanoutTransportClass.
 */

#include "CMPIFanoutTransport_mpi.h"

#include <stdexcept>
#include <stdlib.h>
#include <iostream>

/**
 * constructor (default)
 *    Just construct the base class
 */
CMPIFanoutTransport::CMPIFanoutTransport() : CMPITransport() {}

/**
 * constructor with communicator.
 *   Construct the base class and set its communicator.
 *
 * @param communicator - the MPI communicator to use.
 */
CMPIFanoutTransport::CMPIFanoutTransport(MPI_Comm communicator) :
    CMPITransport()
{
    setCommunicator(communicator);        
}

/**
 * destructor.
 */
CMPIFanoutTransport::~CMPIFanoutTransport()
{
    
    end();                    // Don't exit with hanging clients.
}
/**
 * recv
 *    @throws std::logic_error - this transport is unidirectional.
 */
void
CMPIFanoutTransport::recv(void** ppData, size_t& size)
{
    throw std::logic_error("CMPIFanoutTransport only can send!");
}
/**
 * send
 *    Get data request and send this data in response:
 *    Note, getDataRequets registers the client and
 *    sets the receiver in the base class.
 *
 *  @param parts - parts of the message
 *  @param numParts - number of message parts.
 */
void
CMPIFanoutTransport::send(iovec* parts, size_t numParts)
{
    getDataRequest();
    CMPITransport::send(parts, numParts);
}
/**
 * end
 *   End of data handling.
 *   While there are still clients, get the next data request
 *   and respond to it with an end, removing that client
 *   from the registry.  Note that the use of
 *   getDataRequest ensures the registry always has the
 *   client requesting data.
 */
void
CMPIFanoutTransport::end()
{
#ifdef DEBUG
  std::cerr << "MPI fanout transport end " << m_clients.size() << " clients to end\n";  
#endif
  while (!m_clients.empty()) {
    getDataRequest();
#ifdef DEBUG
    std::cerr << "End got request from " << getReceiver() << std::endl;
    std::cerr << m_clients.size() << " remaining\n";
#endif
    CMPITransport::end();
    int client = setReceiver(-1);  // Gets set next getDataReq
#ifdef DEBUG
    std::cerr << "Removing " << client << std::endl;
#endif
    m_clients.remove(client);
  }
  std::cerr << " all ended\n";
}

/////////////////////////////////////////////////////////////////
// Private methods.
//

/**
 * getDataRequest
 *    Receives a data request from a client.
 *    - Sets the receiver.
 *    - If necessary, adds the receiver to the client registry.
 *
 *  @throw std::logic_error if the message received as not a
 *                          data request.
 */
void
CMPIFanoutTransport::getDataRequest()
{
    void* pData;
    size_t nBytes;
    
    CMPITransport::recv(&pData, nBytes);
    if (getLastReceivedTag() != dataRequestTag) {
        if (nBytes) free(pData);   // In case we keep running
        throw std::logic_error(
            "CMPIFanoutTransport - received something other than a data request tag"
        );
    }
    if (nBytes) {
        free(pData);
        throw std::logic_error(
            "CMPITransport - nonempty data request"
        );
    }
    int requestor = getLastReceivedRank();
    setReceiver(requestor);
    if (!m_clients.hasClient(requestor) ) {
        m_clients.add(requestor);
    }
    
    
}
