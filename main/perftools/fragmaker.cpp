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

/** @file:  fragmaker.cpp
 *  @brief: Send some whole cloth event fragments to stdout.
 */


/**
 *   This code is intended to serve as a data source to test the
 *   performance of glom.   We spew a stream of fragments to stdout
 *   The sourceid of all fragments is 0 and the timestamps increase by
 *   a settable value. for each fragment.
 *
 *   Fragment size is adjustable.  We manage a 1 Mbyte block of fragments.
 *
 * @note - it would be wise to get the performance of this program before
 *         hooking it to glom as it might not match that of pipesend.
 * @note - At start time, the data block is initialized with ringitems
 *         that have a fragment header prepended. Subsequenly, after each
 *         write of the data block, we only alter the timestamps in the
 *         fragment headers.
 * @note - The ring items are also very minimal, just  uint32_size and
 *         PHYSICS_EVENT type.
 *
 * Usage:
 *     fragmaker payload-size   tstamp-delta
 *     
 */
#include <fragment.h>
#include <io.h>
#include <DataFormat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>

uint8_t data[1024*1024];              // Raw data block.

struct descriptor {
    size_t   s_nBytes;                 // Used bytes in data block.
    uint64_t s_lastTimestamp;          // Prior timestamp.
    int      s_tsdelta;                // Timstamp delta.
    void*    s_pData;                  // Pointer to data.
};

static void usage()
{
    std::cerr << "Usage:\n";
    std::cerr << "   fragmaker payload-size   tstamp-delta\n";
    std::cerr << "Where:\n";
    std::cerr << "  payload-size - number of bytes in each ring item must be >= 8\n";
    std::cerr << "  tstamp-delta - Tick in timestamp between fragments.\n";
}
//  initializes the ring items and initial timestamps as well.
//  desc.s_nBytes  - on entry is the size of the buffer, on exit bytes used.
//  desc.s_lastTimestamp - on entry; first timestamp is this + desc.s_tsdelta. on exit
//                         timestamp of the last item in the buffer.
//  desc.s_tsdelta - Ticks between timestamps.
//  desc.s_pData   - pointer to the data to init.

static void initRingItems(descriptor& desc, unsigned payloadSize)
{
    uint32_t bytesLeft = desc.s_nBytes;
    uint32_t bytesBuilt = 0;
    
    // The size of a chunk in the buffer is the payload size (which must
    // be at least sizeof(RingItemheader). and the size of a FragmentHeader.
    
    if (payloadSize < sizeof(RingItemHeader)) {
        std::cerr << "Payload size of " << payloadSize
            << " is smaller than a ring item header: "
            << sizeof(RingItemHeader) << std::endl;
        exit(EXIT_FAILURE);
    }
    uint8_t* p = static_cast<uint8_t*>(desc.s_pData);
    
    uint32_t itemsize = payloadSize + sizeof(EVB::FragmentHeader);
    while (bytesLeft >= itemsize)  {
        EVB::pFragmentHeader pF = reinterpret_cast<EVB::pFragmentHeader>(p);
        
        // Fragment header:
        
        desc.s_lastTimestamp += desc.s_tsdelta;
        pF->s_timestamp = desc.s_lastTimestamp;
        pF->s_sourceId  = 0;
        pF->s_size      = payloadSize;
        pF->s_barrier   = 0;
        
        // Ring item:
        
        pRingItemHeader pR = reinterpret_cast<pRingItemHeader>(pF+1);
        pR->s_size = payloadSize;
        pR->s_type = PHYSICS_EVENT;
        
        bytesLeft  -= itemsize;
        bytesBuilt += itemsize;
        p          += itemsize;
    }
    desc.s_nBytes = bytesBuilt;
}
// UpdateTimestamps - just changes the timestamps in all the item headers.
// desc.s_pData -> data.
// desc.s_nBytes = number of bytes of data.
// desc.s_lastTimestamp - last timestamp in prior block sent.
// desc.s_tsdelta - Timestamp increment for each item.
//
static void updateTimestamps(descriptor& desc)
{
    uint8_t* p = static_cast<uint8_t*>(desc.s_pData);
    uint32_t remaining = desc.s_nBytes;
    
    while (remaining) {
        EVB::pFragmentHeader pF = reinterpret_cast<EVB::pFragmentHeader>(p);
        desc.s_lastTimestamp  += desc.s_tsdelta;
        pF->s_timestamp = desc.s_lastTimestamp;
        
        uint32_t itemSize = sizeof(EVB::FragmentHeader) + pF->s_size;
        
        remaining -= itemSize;
        p         += itemSize;
    }
}

//
int main(int argc, char** argv)
{
    argc--; argv++;
    
    if (argc != 2) {
        usage();
        exit(EXIT_FAILURE);
    }
    
    int payloadSize = atoi(argv[0]);
    int tsDelta     = atoi(argv[1]);
    
    if ((payloadSize < 8) || (tsDelta == 0)) {
        usage();
        exit(EXIT_FAILURE);
    }
    
    descriptor desc = {sizeof(data), 0, tsDelta, data};;
    initRingItems(desc, payloadSize);               // First initialization.,
    
    while(1) {
        io::writeData(STDOUT_FILENO, desc.s_pData, desc.s_nBytes);
        updateTimestamps(desc);        // New timestamp values.
    }
}

