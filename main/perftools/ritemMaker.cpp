/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
             Aaron Chester
             FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  ritemMaker.cpp
 *  @brief: Makes a stream of ring items.
 */
/**
 *  Creates a stream of ring items with body headers.  The body headers
 *  have incrementing timestamps, and a fixed sourceid:
 *  The items are output to stdout.
 *
 *  Usage:
 *     ritemMaker --datasize=N [--sid=S] [--tsinc=T] [--bufsize=B]
 *
 *  --datasize size of the ring item payload (stuff after body  header).
 *  --sid      source id given to the ring item body headers (default 0).
 *  --tsinc    timestamp increment between events (default 1).
 *  --bufsize  size in bytes of the ring-item buffer (default 1 MB).
 *
 * @note  ring items are created in a buffer whose size is set by --bufsize
 *        (default 1 MB) which is written to stdout.
 * @note  Ring item contents are not initialized.
 * @note  Between passes, only the timestamps are adjusted.
 * @note  All items are PHYSICS_EVENT items.
 */
#include <DataFormat.h>
#include <io.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>

#include "ritemMaker_cmdline.h"

struct descriptor {
    size_t   s_nBytes;                 // Used bytes in data block.
    uint64_t s_lastTimestamp;          // Prior timestamp.
    int      s_tsdelta;                // Timestamp delta.
    void*    s_pData;                  // Pointer to data.
};

// Fully initializes the buffer with ring items.

static void
initRingItems(descriptor& desc, int dataSize, int sid)
{
    uint8_t* p = static_cast<uint8_t*>(desc.s_pData);
    
    // Full size of a ring item:
    
    uint32_t rItemSize = sizeof(RingItemHeader) +  sizeof(BodyHeader) + dataSize;
    int bytesLeft = desc.s_nBytes;
    int bytesBuilt = 0;
    
    while (bytesLeft >= rItemSize) {
        pRingItem pI = reinterpret_cast<pRingItem>(p);
        pI->s_header.s_size = rItemSize;
        pI->s_header.s_type = PHYSICS_EVENT;
                
        desc.s_lastTimestamp += desc.s_tsdelta;
        pBodyHeader pBh = &(pI->s_body.u_hasBodyHeader.s_bodyHeader);
        pBh->s_size       = sizeof(BodyHeader);
        pBh->s_timestamp  = desc.s_lastTimestamp;
        pBh->s_sourceId   = sid;
        pBh->s_barrier    = 0;
        
        p += rItemSize;
        bytesBuilt += rItemSize;
        bytesLeft  -= rItemSize;
    }
    desc.s_nBytes = bytesBuilt;
    
}
// Updates only the timestamps in the ring item body headers in the buffer.

static void
updateTimestamps(descriptor& desc)
{
    size_t bytesLeft = desc.s_nBytes;
    uint8_t*       p = static_cast<uint8_t*>(desc.s_pData);
    
    while (bytesLeft) {
        pRingItem pI = reinterpret_cast<pRingItem>(p);
        
        desc.s_lastTimestamp += desc.s_tsdelta;
        pI->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp = desc.s_lastTimestamp;
        
        bytesLeft -= pI->s_header.s_size;
        p         += pI->s_header.s_size;
    }
}

//
int main (int argc, char** argv)
{
    gengetopt_args_info args;
    if (cmdline_parser(argc, argv, &args) != 0) {
        exit(EXIT_FAILURE);
    }
    int dataSize = args.datasize_arg;
    int sid      = args.sid_arg;
    int tsInc    = args.tsinc_arg;

    // datasize and tsinc must be > 0 and sid must be >= 0:

    if ((dataSize <= 0) || (sid < 0) || (tsInc <=0)) {
        std::cerr << "datasize and tsinc must be > 0 and sid must be >= 0\n";
        exit(EXIT_FAILURE);
    }

    std::vector<uint8_t> buffer(args.bufsize_arg);
    descriptor desc = {buffer.size(), 0, tsInc, buffer.data()};
    initRingItems(desc, dataSize, sid);

    while(1) {
        io::writeData(STDOUT_FILENO, buffer.data(), desc.s_nBytes);
        updateTimestamps(desc);
    }
}



