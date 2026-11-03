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

/** @file:  rdoperf.cpp
 *  @brief: Monitor the rate of data into a ring buffer.
 */

#include "MeterApi.h"
#include "utils.h"
#include <unistd.h>
#include <time.h>
#include <CRingBuffer.h>
#include <iostream>
#include <stdlib.h>
#include <Exception.h>
#include <stdio.h>



// True if the CLOCK_REALTIME - has 10x the resolution ms or better.
static bool resolutionOk(int ms)
{
    timespec res;
    int stat = clock_getres(CLOCK_REALTIME, &res);
    if (stat) {
        perror("Getting clock resolution");
        exit(EXIT_FAILURE);
    }
    long ns = ms * 1000*1000;    // number of ns in the ms.
    long resns = res.tv_nsec + 1.0e9*res.tv_sec;
    return (resns * 10.0 ) < ns;
}


static void usage()
{
    std::cerr << "Usage\n";
    std::cerr << "   rdoperf ring ms host port\n";
    std::cerr << "Where:\n";
    std::cerr << "   ring - name of the ring being monitored\n";
    std::cerr << "   ms - milliseconds between measurements\n";
    std::cerr << "   host, port - specify the host/port on which the meter server runs\n";
}

/**
 * main
 *    Usage:
 *       rdoperf ring-name ms host port
 *
 *    - ring-name - ring buffer into which data are being put.
 *    - ms        - number of millisecs/sample.
 *    - host      - host running the meter server.
 *    - port      - Port the meter server is listening on.
 * @note -each interval is of the form:
 *        - Empty ring
 *        - sleep
 *        - get bytes in ring
 *        - figure out time difference
 *        - report time difference to meter server.
 *
 *  Now some caveats:
 *        
 * @note - we require the clock resolution be at least 10x the sampling
 *         interval.
 * @note - There's some small skew in the time measurement; specifically
 *         the time between emptying the ring buffer and reading the interval
 *         start time - during which the producer is happily stuffing data
 *         into the ring.
 */
int main(int argc, char** argv)
{
    argv++; argc--;             // Skip the command name string.
    if (argc != 4) {
        usage();
        exit(EXIT_FAILURE);
    }
    try {
        CRingBuffer ring(argv[0]);        // We're timing this ring.
        
        int ms = atoi(argv[1]);           // ms per interval.
        if (ms <= 0 ) {
            usage();
            exit(EXIT_FAILURE);
        }
        
        Meter meter(argv[2], atoi(argv[3]), argv[0], 1.0, 1.0e10, true);  // Meter named after ring.
        
        // Be sure that clocks have needed resolution.
        
        if (!resolutionOk(ms)) {
            std::cerr << "High precision clock does not have sufficient resolution\n";
            std::cerr << "to assure timing accuracy for " << ms << "ms measurment interval\n";
            exit(EXIT_FAILURE);
        }
        timespec start, stop;
        while(1) {
            clock_gettime(CLOCK_REALTIME, &start);
            ring.skip(ring.availableData());
            usleep(ms*1000);            // Let data come in.
            size_t bytesInRing = ring.availableData();
            clock_gettime(CLOCK_REALTIME, &stop);
            
            uint64_t nsDiff = hrDiff(stop, start);
            double rate = (double)(bytesInRing)/(nsDiff) * 1.0e9;
            meter.set(rate);
            
            std::cout << bytesInRing << " in " << nsDiff << " -> "
                      << rate << "bytes/sec\n";
            
        }
    }
    catch (CException& e) {
        std::cerr << "** ERROR ** " << e.ReasonText() << std::endl;
        exit(EXIT_FAILURE);
    }
}
