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

/** @file:  ritemMaker.cpp
 *  @brief: Makes a stream of ring items.
 */
/**
 *  Creates a stream of ring items with body headers.  The body headers
 *  have incrementing timestamps, and a fixed sourceid:
 *  The items are output to stdout.
 *  
 *  Usage:
 *     ritemMaker datasize sid tsincrement
 *
 *  datasize size of the ring item payload (stuff after body  header).
 *  sid      source id given to the ring item body headers.
 *  tsincrement timestamp increment between events.
 *
 * @note  ring items are created in a 1Mbyte buffer which is written to
 *        stdout.
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

uint8_t* buffer[1024*1024];

struct descriptor {
    size_t   s_nBytes;                 // Used bytes in data block.
    uint64_t s_lastTimestamp;          // Prior timestamp.
    int      s_tsdelta;                // Timstamp delta.
    void*    s_pData;                  // Pointer to data.
};

static void usage()
{
    std::cerr << "Usage:\n";
    std::cerr << "    ritemMaker datasize sid tsinc\n";
    std::cerr << "Creates ring items on stdout where:\n";
    std::cerr << "   datasize - is the size of the data part of the ring item\n";
    std::cerr << "   sid      - is the source id to put in the body  header\n";
    std::cerr << "   tsinc    - Timestamp increment between events\n";
}

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
    argc--; argv++;
    
    if (argc != 3) {
        usage();
        exit(EXIT_FAILURE);
    }
    int dataSize = atoi(argv[0]);
    int sid      = atoi(argv[1]);
    int tsInc    = atoi(argv[2]);
    
    // datasize and tsinc must be > 0 and sid must be >= 0:
    
    if ((dataSize <= 0) || (sid < 0) || (tsInc <=0)) {
        usage();
        exit(EXIT_FAILURE);
    }
    
    descriptor desc = {sizeof(buffer), 0, tsInc, buffer};
    initRingItems(desc, dataSize, sid);
    
    while(1) {
        io::writeData(STDOUT_FILENO, buffer, desc.s_nBytes);
        updateTimestamps(desc);
    }
}



