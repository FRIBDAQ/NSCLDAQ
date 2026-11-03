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

/** @file:  sockperf.cpp
 *  @brief: Measure socket data transfer performance.
 */
#include <CSocket.h>
#include <iostream>
#include "utils.h"
#include <time.h>
#include <stdlib.h>
#include "MeterApi.h"


static uint8_t buffer[100*1024*1024];           // Attempt big  buffer..

/**
 * measures the rate at which data can be transferred over a sock_stream
 * TCP socket.
 *    Usage:
 *      sockperf nbytes my-port meter-host meter-port
 *
 *     - nbytes is the number of bytes per transfer measuered.
 *     - my-port is the port on which we accept a connection.
 *     - meter-host meter-port specify where the meter server runs
 */

static void usage()
{
    std::cerr << "Usage:\n";
    std::cerr << "   sockperf nbytes my-port meter-host meter-port\n";
    std::cerr << "Where\n";
    std::cerr << "  nybtes - is the size of the block being timed.\n";
    std::cerr << "  my-port - is the port number on which we listen for a connection\n";
    std::cerr << "  meter-host, meter-port are the connection params to a meter server\n";
}

//
// Time block - times the time required to read nbytes of data.
//   Reads are done in min of nbytes or 100*1024*1024 byte blocks
//   but we care about the actual number of bytes read.


static double timeBlock(CSocket* peer, int nbytes)
{
    int bytesRead = 0;
    int blocksize = sizeof(buffer) < nbytes ? nbytes : sizeof(buffer);
    
    timespec start, stop;
    
    clock_gettime(CLOCK_REALTIME, &start);
    while(bytesRead < nbytes) {
        bytesRead += peer->Read(buffer, blocksize);
    }
    clock_gettime(CLOCK_REALTIME, &stop);
    
    uint64_t ns = hrDiff(stop, start);
    
    double rate = double(bytesRead)/ns * 1.0e9;
    
    std::cerr << bytesRead << " bytes read in " << ns << "ns for: "
        << rate << " Bytes/sec\n";
    return rate;
}

//
int main(int argc, char** argv)
{
    argc--; argv++;
    if (argc != 4) {
        usage();
        exit(EXIT_FAILURE);
    }
    
    int nbytes = atoi(argv[0]);
    if (nbytes <=0) {
        usage();
        exit(EXIT_FAILURE);
    }
    
    std::string myport    = argv[1];
    
    
    Meter m(argv[2], atoi(argv[3]), "socket", 1.0, 1.0e10, true);
    
    CSocket* peer;
    {
        std::string who;
        CSocket listener;
        listener.Bind(myport);
        listener.Listen();
        peer = listener.Accept(who);
        std::cerr << "Connection from " << who << std::endl;
    }
    
    while(1) {
        m.set(timeBlock(peer, nbytes));
    }
    
}