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

/** @file:  CMPIFanoutTransport_mpi.h
 *  @brief: Define a fanout transport using MPI.
 */
#ifndef CMPIFANOUTTRANSPORT_MPI_H
#define CMPIFANOUTTRANSPORT_MPI_H

#include "CFanoutTransport.h"
#include "CMPITransport_mpi.h"
#include "CClientRegistry.h"

#include <sys/uio.h>

/**
 * @class CMPIFanoutTransport
 *    This class is the sender  transport for a fanout.
 *    It pairs with CMPIFanoutClientTransport which receives data.
 *    Each client requests a work item from this transport by
 *    sending an empty message with the dataRequestTag tag.
 *    - Receive operations throw an exception.
 *    - Send operations receive a data request and then respond
 *      to the requesting rank with the work item provided.
 *    - End operations again wait for data requests and send end
 *      messages until there are no more requestors.
 *
 *  @note CMPIFanoutClientTransport must use the same communicator
 *        as the paired CMPIFanoutTransport.
 *  @note as requests come for data they are registered in this
 *        class's client registry.
 *  @note the end code is crafted so that if a data request
 *        comes from a client that has not yet registered,
 *        it's still sent an end.  This allows for the case
 *        when the data set is smaller than the number of
 *        clients e.g.
 *   @note ranks within the communicator are used to identify clients not some
 *        client id set by the client transport.
 *        
 */
class CMPIFanoutTransport : public CMPITransport, public CFanoutTransport
{
private:
    CClientRegistry m_clients;
public:
    CMPIFanoutTransport();
    CMPIFanoutTransport(MPI_Comm communicator);
    virtual ~CMPIFanoutTransport();
    
    virtual void recv(void** ppData, size_t& size);
    virtual void send(iovec* parts,  size_t numParts);
    virtual void end();
    
    static const int dataRequestTag  = 2;   // Tag to use when requesting data from us.
protected:
    void getDataRequest();
};



#endif
