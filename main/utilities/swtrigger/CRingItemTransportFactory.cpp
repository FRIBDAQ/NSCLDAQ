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

/** @file:  CRingItemTransportFactory.cpp
 *  @brief: Implement the factory.
 */
#include "CRingItemTransportFactory.h"
#include "CRingBufferTransport.h"
#include "CRingItemFileTransport.h"
#include <CRingBuffer.h>
#include <CRingFileBlockReader.h>
#include <CBufferedOutput.h>
#include <CRingBufferChunkAccess.h>
#include <CRemoteAccess.h>
#include <URL.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdexcept>
#include <system_error>

#include <signal.h>
#include <os.h>
#include <stdio.h>

#include "CRingBufferTransportEJFAT.h"

// Stupid class to ensure that the chunk accessors delete the ring buffers
// we make -- otherwise killing the transprot kills us.  We don't
// want the base class to assume the ring buffer is dynamic.

class _CRingBufferChunkAccess : public CRingBufferChunkAccess
{
private:
    CRingBuffer* m_pRing;
public:
    _CRingBufferChunkAccess(CRingBuffer* p) :
        CRingBufferChunkAccess(p), m_pRing(p) {}
    virtual ~_CRingBufferChunkAccess() {delete m_pRing;}
};

/**
 * createTransport
 *
 * @parm uri - null terminated source/sink specification.
 * @param accessMode - Producer/consumer access mode?
 * @return CRingItemtransport* - pointer to the created transport.
 * @note all errors are reported via exceptions.
 */
CRingItemTransport*
CRingItemTransportFactory::createTransport(
    const char* uri, CRingBuffer::ClientMode accessMode
)
{
    URL ringUri(uri);
    
    // First decision is based on protocol.
    
    std::string proto = ringUri.getProto();
    if (proto == "tcp") {
        // live data - If we want to be a producer, the host must be local.
        
        if (accessMode == CRingBuffer::producer) {
            if (CRingAccess::local(ringUri.getHostName())) {
                std::string ringName = ringUri.getPath();
                CRingBuffer* pRing = CRingBuffer::createAndProduce(ringName);
                return new CRingBufferTransport(*pRing);
            } else {
                throw std::invalid_argument(
                    "CRingItemTransportFactory: Producer hosts must be local!"    
                );
            }
        } else {
            // Consumer so use the Remote access library:
            
            CRingBuffer* pRing = CRingAccess::daqConsumeFrom(uri);
            CRingBufferChunkAccess* accessor =
                new _CRingBufferChunkAccess(pRing);
            return new CRingBufferTransportEJFAT(*accessor);
            
        }
    } else if (proto == "file") {
        // accessing file
        
        std::string path = ringUri.getPath();
        if (accessMode == CRingBuffer::producer) {
            int fd = creat(path.c_str(), S_IRUSR | S_IWUSR | S_IRGRP);
            if (fd < 0) {
                throw std::system_error(
                    errno, std::generic_category(),
                    "CRingItemTransportFactory: Creating output file"
                );
            }
            io::CBufferedOutput* pWriter = new io::CBufferedOutput(fd, 8192);
            return new CRingItemFileTransport(*pWriter);
        } else {          
            CRingFileBlockReader* pReader =
                new CRingFileBlockReader(path.c_str());
            return new CRingItemFileTransport(*pReader);
        }
        
    } else {
        throw std::invalid_argument(
            "CRingItemTransportFactory:  protocol must be tcp or localhost"
        );
    }
    
    
}
