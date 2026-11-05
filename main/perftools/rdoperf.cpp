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
 *  @details
 *  Samples the data rate into a ring buffer --nsamples times, --interval-ms
 *  milliseconds apart, and reports the aggregate rate and per-sample
 *  statistics.  The rate can also be shown live on a meter server.
 *
 * Usage:
 *    rdoperf --ring=NAME [--nsamples=N] [--interval-ms=MS] \
 *            [--meter-host=HOST] [--meter-port=PORT] [--logfile=FILE] [--verbose]
 */

#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <unistd.h>

#include <CRingBuffer.h>
#include <Exception.h>

#include "MeterApi.h"
#include "StatsLog.h"
#include "rdoperf_cmdline.h"
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

/**
 * main
 *    Samples the ring (--ring) --nsamples times, --interval-ms apart, then
 *    writes a summary (and appends it to --logfile when given).
 *
 * @note - each sample is of the form:
 *        - Empty ring
 *        - sleep
 *        - get bytes in ring
 *        - figure out time difference
 *        - report the rate to the meter server (when --meter-host is given).
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
  gengetopt_args_info args;
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(EXIT_FAILURE);

  int ms = args.interval_ms_arg; // ms per sample.
  if (ms <= 0) {
    std::cerr << "interval-ms must be > 0\n";
    exit(EXIT_FAILURE);
  }
  try {
    CRingBuffer ring(args.ring_arg); // We're timing this ring.

    std::string logpath = args.logfile_given ? args.logfile_arg : "";

    std::unique_ptr<Meter> pMeter; // Meter named after the ring.
    if (args.meter_host_given)
      pMeter.reset(new Meter(args.meter_host_arg, args.meter_port_arg,
                             args.ring_arg, 1.0, 1.0e10, true));

    // Be sure that clocks have needed resolution.

    if (!resolutionOk(ms)) {
      std::cerr << "High precision clock does not have sufficient resolution\n";
      std::cerr << "to assure timing accuracy for " << ms
                << "ms measurement interval\n";
      exit(EXIT_FAILURE);
    }
    StatsLog stats(args.ring_arg);
    timespec start, stop, tStart, tNow;
    clock_gettime(CLOCK_REALTIME, &tStart);
    for (long i = 0; i < args.nsamples_arg; ++i) {
      clock_gettime(CLOCK_REALTIME, &start);
      ring.skip(ring.availableData());
      usleep(ms * 1000); // Let data come in.
      size_t bytesInRing = ring.availableData();
      clock_gettime(CLOCK_REALTIME, &stop);

      uint64_t nsDiff = hrDiff(stop, start);
      double rate = (double)(bytesInRing) / (nsDiff)*1.0e9;
      if (pMeter)
        pMeter->set(rate);
      stats.add(bytesInRing, nsDiff);

      if (args.verbose_flag)
        std::cout << bytesInRing << " in " << nsDiff << " -> " << rate
                  << "bytes/sec\n";
    }
    clock_gettime(CLOCK_REALTIME, &tNow);
    double elapsed = hrDiff(tNow, tStart) / 1.0e9;

    stats.writeSummary(logpath, elapsed);
  } catch (CException &e) {
    std::cerr << "** ERROR ** " << e.ReasonText() << std::endl;
    exit(EXIT_FAILURE);
  }
}
