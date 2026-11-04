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

/** @file  rdoperf.cpp
 *  @brief Monitor the rate of data into a ring buffer.
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <unistd.h>

#include <CRingBuffer.h>
#include <Exception.h>

#include "MeterApi.h"
#include "StatsLog.h"
#include "utils.h"

// True if the CLOCK_REALTIME - has 10x the resolution ms or better.
static bool resolutionOk(int ms) {
  timespec res;
  int stat = clock_getres(CLOCK_REALTIME, &res);
  if (stat) {
    perror("Getting clock resolution");
    exit(EXIT_FAILURE);
  }
  long ns = ms * 1000 * 1000; // number of ns in the ms.
  long resns = res.tv_nsec + 1.0e9 * res.tv_sec;
  return (resns * 10.0) < ns;
}

static void usage() {
  std::cerr << "Usage\n";
  std::cerr << "   rdoperf ring ms host port [seconds] [logfile]\n";
  std::cerr << "Where:\n";
  std::cerr << "   ring - name of the ring being monitored\n";
  std::cerr << "   ms - milliseconds between measurements\n";
  std::cerr << "   host, port - specify the host/port on which the meter "
               "server runs\n";
  std::cerr
      << "            (use '-' for host to skip the meter GUI entirely)\n";
  std::cerr << "   seconds - optional test duration; when given, run that long "
               "then\n";
  std::cerr
      << "            write a summary and exit (0/omitted => run forever)\n";
  std::cerr << "   logfile - optional file to append the summary block to\n";
}

/**
 * main
 *    Usage:
 *       rdoperf ring-name ms host port [seconds] [logfile]
 *
 *    - ring-name - ring buffer into which data are being put.
 *    - ms        - number of millisecs/sample.
 *    - host      - host running the meter server ('-' skips the meter GUI).
 *    - port      - Port the meter server is listening on.
 *    - seconds   - (optional) test duration; then a summary is written.
 *    - logfile   - (optional) file the summary is appended to.
 * @note -each interval is of the form:
 *        - Empty ring
 *        - sleep
 *        - get bytes in ring
 *        - figure out time difference
 *        - report the rate to the meter server (when enabled).
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
int main(int argc, char **argv) {
  argv++;
  argc--; // Skip the command name string.
  if (argc < 4 || argc > 6) {
    usage();
    exit(EXIT_FAILURE);
  }
  try {
    CRingBuffer ring(argv[0]); // We're timing this ring.

    int ms = atoi(argv[1]); // ms per interval.
    if (ms <= 0) {
      usage();
      exit(EXIT_FAILURE);
    }

    std::string mhost = argv[2];
    int mport = atoi(argv[3]);
    int seconds = (argc >= 5) ? atoi(argv[4]) : 0; // 0 => run forever
    std::string logpath = (argc >= 6) ? argv[5] : "";

    Meter *meter = 0; // Meter named after ring.
    if (mhost != "-")
      meter = new Meter(argv[2], mport, argv[0], 1.0, 1.0e10, true);

    // Be sure that clocks have needed resolution.

    if (!resolutionOk(ms)) {
      std::cerr << "High precision clock does not have sufficient resolution\n";
      std::cerr << "to assure timing accuracy for " << ms
                << "ms measurement interval\n";
      exit(EXIT_FAILURE);
    }
    StatsLog stats(argv[0]);
    timespec start, stop, tStart;
    clock_gettime(CLOCK_REALTIME, &tStart);
    double elapsed = 0.0;
    do {
      clock_gettime(CLOCK_REALTIME, &start);
      ring.skip(ring.availableData());
      usleep(ms * 1000); // Let data come in.
      size_t bytesInRing = ring.availableData();
      clock_gettime(CLOCK_REALTIME, &stop);

      uint64_t nsDiff = hrDiff(stop, start);
      double rate = (double)(bytesInRing) / (nsDiff)*1.0e9;
      if (meter)
        meter->set(rate);
      stats.add(bytesInRing, nsDiff);

      std::cout << bytesInRing << " in " << nsDiff << " -> " << rate
                << "bytes/sec\n";

      elapsed = hrDiff(stop, tStart) / 1.0e9;
    } while (seconds <= 0 || elapsed < seconds);

    stats.writeSummary(logpath, elapsed);
    delete meter;
  } catch (CException &e) {
    std::cerr << "** ERROR ** " << e.ReasonText() << std::endl;
    exit(EXIT_FAILURE);
  }
}
