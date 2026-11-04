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

/** @file  pipeperf.cpp
 *  @brief Measure performance of pipe data transfers.
 *  @details
 *  Measures the byte/sec through a unix pipe.
 *
 * Usage:
 *    pipesend | pipeperf nbytes meter-host meter-port [seconds] [logfile]
 */

#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <unistd.h>

#include "MeterApi.h"
#include "StatsLog.h"
#include "utils.h"

static uint8_t buffer[100 * 1024 * 1024];

static void usage() {
  std::cerr << "Usage:\n";
  std::cerr << "    pipesend | pipeperf nbytes meter-host meter-port [seconds] "
               "[logfile]\n";
  std::cerr << "Where:\n";
  std::cerr << "    nbytes is the transfer size measured per pass\n";
  std::cerr << "    meter-host, meter-port - are connection params to the "
               "meter server\n";
  std::cerr << "            (use '-' for meter-host to skip the meter GUI "
               "entirely)\n";
  std::cerr << "    seconds - optional test duration; when given, run that "
               "long then\n";
  std::cerr
      << "            write a summary and exit (0/omitted => run forever)\n";
  std::cerr << "    logfile - optional file to append the summary block to\n";
}

static double measure(int bytes, uint64_t &outBytes, uint64_t &outNs) {
  int nread = 0;
  int readsize = sizeof(buffer) < bytes ? sizeof(buffer) : bytes;

  timespec start, stop;
  clock_gettime(CLOCK_REALTIME, &start);

  while (nread < bytes) {
    nread += read(STDIN_FILENO, buffer, readsize);
  }
  clock_gettime(CLOCK_REALTIME, &stop);

  uint64_t ns = hrDiff(stop, start);

  double rate = double(nread) / ns * 1.0e9;

  std::cerr << nread << " Bytes read in  " << ns << "ns -> " << rate
            << " bytes/sec\n";

  outBytes = nread;
  outNs = ns;
  return rate;
}

int main(int argc, char **argv) {
  argc--;
  argv++;

  if (argc < 3 || argc > 5) {
    usage();
    exit(EXIT_FAILURE);
  }

  int nbytes = atoi(argv[0]);
  std::string mhost = argv[1];
  int mport = atoi(argv[2]);
  int seconds = (argc >= 4) ? atoi(argv[3]) : 0; // 0 => run forever
  std::string logpath = (argc >= 5) ? argv[4] : "";

  Meter *meter = 0;
  if (mhost != "-")
    meter = new Meter(mhost.c_str(), mport, "pipe", 1.0, 1.0e10, true);

  StatsLog stats("pipe");
  timespec tStart, tNow;
  clock_gettime(CLOCK_REALTIME, &tStart);
  double elapsed = 0.0;
  do {
    uint64_t bytes, ns;
    double rate = measure(nbytes, bytes, ns);
    if (meter)
      meter->set(rate);
    stats.add(bytes, ns);
    clock_gettime(CLOCK_REALTIME, &tNow);
    elapsed = hrDiff(tNow, tStart) / 1.0e9;
  } while (seconds <= 0 || elapsed < seconds);

  stats.writeSummary(logpath, elapsed);
  delete meter;
  return 0;
}
