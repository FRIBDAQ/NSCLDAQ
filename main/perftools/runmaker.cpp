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

/** @file:  runmaker.cpp
 *  @brief: Create a run - a begin run followed by a stream of
 *          events.  This is basically ritemMaker preceding its output
 *          with a begin run item running for a fixed number of items
 *          and then ending with an end run item.
 */

#include <DataFormat.h>
#include <io.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include <string.h>

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
    std::cerr << "    runMaker datasize sid tsinc\n";
    std::cerr << "Creates ring items on stdout where:\n";
    std::cerr << "   datasize - is the size of the data part of the ring item\n";
    std::cerr << "   mbytes   - Mbytes of data to create.\n";
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

int main(int argc, char** argv)
{
    argc--; argv++;
    
    if (argc != 4) {
        usage();
        exit(EXIT_FAILURE);
    }
    int dataSize = atoi(argv[0]);
    int mBytes   = atoi(argv[1]);
    int sid      = atoi(argv[2]);
    int tsDelta  = atoi(argv[3]);
    
    if ((dataSize <= 0) || (mBytes <= 0) || (sid < 0) || (tsDelta <= 0)) {
        usage();
        exit(EXIT_FAILURE);
    }
    
    // set up the ring items.
    
    descriptor desc = {sizeof(buffer), 0, tsDelta, buffer};
    initRingItems(desc, dataSize, sid);
    
    // Write the begin run.
    
    StateChangeItem bor;
    bor.s_header.s_size = sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(StateChangeItemBody);
    bor.s_header.s_type = BEGIN_RUN;
    bor.s_body.u_noBodyHeader.s_mbz = 0;
    pStateChangeItemBody pBody = &(bor.s_body.u_noBodyHeader.s_body);
    pBody->s_runNumber  = 0;
    pBody->s_timeOffset = 0;
    pBody->s_Timestamp = time(nullptr);
    pBody->s_offsetDivisor = 1;
    strcpy(pBody->s_title, "Test run");
    
    io::writeData(STDOUT_FILENO, &bor, bor.s_header.s_size);
    
    // write the data:
    
    uint64_t nWritten = 0;
    uint64_t nToWrite = (uint64_t)(mBytes) * 1024*1024;
    
    while(nWritten < nToWrite) {
        io::writeData(STDOUT_FILENO, buffer, desc.s_nBytes);
        updateTimestamps(desc);
        nWritten +=  desc.s_nBytes;
    }
    
    // Write the end run.
    
    time_t now = time(nullptr);
    uint32_t offset = now - pBody->s_Timestamp;
    StateChangeItem eor;
    eor.s_header.s_size = sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(StateChangeItemBody);
    eor.s_header.s_type = END_RUN;
    eor.s_body.u_noBodyHeader.s_mbz = 0;
    pBody = &(eor.s_body.u_noBodyHeader.s_body);
    pBody->s_runNumber  = 0;
    pBody->s_timeOffset = offset;
    pBody->s_Timestamp = now;
    pBody->s_offsetDivisor = 1;
    strcpy(pBody->s_title, "Test run");
    
    io::writeData(STDOUT_FILENO, &eor, eor.s_header.s_size);
    
}