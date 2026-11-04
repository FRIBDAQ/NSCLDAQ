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

/** @file:  fragsrcperf.cpp
 *  @brief: Measure performance of ring fragment source.
 */

/**
 *  We'll pose as an event builder for a ring fragment source.
 *  -  we won't advertise with the port manager.
 *  -  Message from the ring fragment source come in blocks that consist
 *     of a request and body.  The first two uint32_t's of the block
 *     are the size of the request and body respectively.
 *  - All messages must be acknowledged with an OK\n string.
 *  - The first message is a connection identification message and we'll drop
 *    it on the floor.
 *  - The rest of the messages are data essages and we'll measure the
 *    bytes/sec of body
 *  - Note that this ignores the fact that the ring items have been prefaced
 *    with body headers that are 32 bytes each.  For small fragments that may
 *    be important and we'll think about t later.
 *
 * Usage:
 *    fragsrcperf my-port block-count meter-port
 *       my-port - the port on which we accept a connection from the
 *                 ring fragment source (we emulate an event builder).
 *      block-count - number of blocks of data we receive from the source
 *                    per measurement.
 *      meter-host - host that's running the meter server.
 *      meter-port - port on which the meter server is listening for input.
 */
#include "MeterApi.h"
#include "utils.h"
#include <CSocket.h>
#include <Exception.h>

#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static const int NUM_MEASUREMENTS(100);   // Number of measurement blocks.

static void usage()
{
    std::cerr << "Usage\n";
    std::cerr << "    fragsrcperf my-port block-count meter-host meter-port\n";
    std::cerr << "Where:\n";
    std::cerr << "    my-port is the port on which we listen for ringfragsrc connections\n";
    std::cerr << "    block-count is the number of received blocks per measurement\n";
    std::cerr << "    meter-host, meter-port - specify how to connect to the meter server.\n";
}
/**
 * since we're not actually processing data;
 * Reads a block of data from the socket (request and body) and returns
 * the size of the body.
 *
 * @param pSocket - socket connected to ring fragment source.
 * @return size_t - Size of the body.
 * @note all storage management is taken care of by this function.
 */
static size_t readBlock(CSocket* pSocket)
{
    uint32_t rsize;
    uint32_t bsize;
    
    pSocket->Read(&rsize, sizeof(uint32_t));
    uint8_t request[rsize+sizeof(uint32_t)];
    pSocket->Read(request, rsize + sizeof(uint32_t));  // Read req and body size.
    
    // Last 32 bits of request are the body size:
    
    memcpy(&bsize, request+rsize, sizeof(uint32_t));
    void* body = malloc(bsize);
    int bread = 0;
    while (bread < bsize) {
        bread += pSocket->Read(body, bsize);    
    }
    
    //std::cerr << std::hex << "Req: " << rsize << " body: " << bsize
    //    << std::dec << std::endl;
    
    
    const char* pReply = "OK\n";
    pSocket->Write(pReply, strlen(pReply));
   // pSocket->Flush();
    free(body);
    
    return bsize;
}

/**
 * acceptConnection
 *    Accepts a single connnection on our listen port.
 *  @param port - stringified port namne
 *  @return CSocket*  - pointer to the socket that's connected to the peer.
 *  @note after accepting the first connection, the listen socket is closed
 *        ensuring future connection requests result in ECONREFUSED
 */
static
CSocket* acceptConnection(std::string port)
{
    CSocket listener;
    listener.Bind(port);
    listener.Listen(1);
    std::string client;
    
    CSocket* pConnection = listener.Accept(client);
    
    std::cerr << "Accepted connection from " << client << std::endl;
    
    
    return pConnection;
}
/**
 *   Do the timing.
 *
 *   @param pSock - pointer to the socket from which to read data.
 *   @param blocks - number of blocks to read.
 *   @param meter - Reference to the meter object.
 */
static void timeBlocks(CSocket* pSock, ssize_t blocks, Meter& meter)
{
    timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);
    size_t bytes = 0;
    for (int i = 0; i < blocks; i++) {
        bytes += readBlock(pSock);
    }
    clock_gettime(CLOCK_REALTIME, &stop);
    uint64_t nsdiff = hrDiff(stop, start);
    
    double rate = double(bytes) / nsdiff * 1.0e9;
    
    std::cerr << bytes << " read in " << nsdiff << "ns -> "
        << rate << " bytes/sec\n";
    meter.set(rate);
}

/**
 * main
 *   Entry point
 *   - advertise/accept a connection
 *   - Drop the connection data block on the floor.
 *   - Time the performance of successive block-counyt blocks of data.
 *
 *  See file level comments for usage.
 */
int main (int argc, char** argv)
{
    argc--; argv++;           // Skip the program name.
    if(argc != 4) {
        usage();
        exit(EXIT_FAILURE);
    }
    
    int port = atoi(argv[0]);
    if (port <= 0) {
        std::cerr << "Bad port number\n";
        usage();
        exit(EXIT_FAILURE);
    }
    std::string strPort = argv[0];
    ssize_t blocks      = atoi(argv[1]);
    if (blocks <= 1) {
        std::cerr << "Bad block count\n";
        usage();
        exit(EXIT_FAILURE);
    }
    try {
        Meter meter(argv[2], atoi(argv[3]), "ringfragsrc", 1.0, 1.0e9, true);
        
        CSocket* pSock = acceptConnection(strPort);
        
        readBlock(pSock);
        std::cerr << "Got connection block\n";
        
        // Since the time to start the run is in human times,
        // don't start timing until after we get the first data block>
        
        readBlock(pSock);
        std::cerr << "Got the first data block\n";
        for (unsigned i =0; i < NUM_MEASUREMENTS; i++) {
            timeBlocks(pSock, blocks , meter);
        }
        
    }
    catch (CException& e) {
        std::cerr << e.ReasonText() << std::endl;
        exit(EXIT_FAILURE);
    }
}



