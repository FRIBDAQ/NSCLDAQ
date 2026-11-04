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

/** @file:  pipeperf.cpp
 *  @brief: Measure performance of pipe data transfers.
 */
#include <stdint.h>
#include "utils.h"
#include "MeterApi.h"
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>


static  uint8_t buffer[100*1024*1024];

/**
 * measures the byte/sec through a unix pipe.
 *
 * Usage:
 *    pipesend | pipeperf nbyets meter-host meter-port
 */

//
static void usage()
{
    std::cerr << "Usage:\n";
    std::cerr << "    pipesend|pipperf nbytes meter-host meter-port\n";
    std::cerr << "Where:\n";
    std::cerr << "    nbytes is the transfer size measured per pass\n";
    std::cerr << "    meter-host, meter-port - are connection params to the meter server\n";
    
}


static double measure(int bytes)
{
    int nread = 0;
    int readsize = sizeof(buffer) < bytes ? sizeof(buffer) : bytes;
    
    timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);
    
    while (nread < bytes) {
        nread += read(STDIN_FILENO, buffer, readsize);
    }
    clock_gettime(CLOCK_REALTIME, &stop);
    
    uint64_t ns = hrDiff(stop, start);
    
    double rate = double(nread)/ns * 1.0e9;
    
    std::cerr << nread << " Bytes read in  " << ns << "ns -> " << rate << " bytes/sec\n";
    
    return rate;
    
}

int main(int argc, char** argv)
{
    argc--; argv++;
    
    if (argc != 3) {
        usage();
        exit(EXIT_FAILURE);
    }
    
    int nbytes = atoi(argv[0]);
    Meter m(argv[1], atoi(argv[2]), "pipe", 1.0, 1.0e10, true);
    
    while(1) {
        m.set(measure(nbytes));
    }
}

