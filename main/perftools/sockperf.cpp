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

/** @file  sockperf.cpp
 *  @brief Measure socket data transfer performance.
 *  @details
 * Measures the rate at which data can be transferred over a sock_stream TCP
 * socket.
 *
 * Usage: sockperf nbytes my-port meter-host meter-port [seconds] [logfile]
 *     - nbytes is the number of bytes per transfer measured.
 *     - my-port is the port on which we accept a connection.
 *     - meter-host meter-port specify where the meter server runs
 *       (meter-host '-' skips the meter GUI).
 *     - seconds (optional) bounds the test; then a summary is written.
 *     - logfile (optional) file the summary is appended to.
 */

#include <iostream>
#include <stdlib.h>
#include <string>
#include <time.h>

#include <CSocket.h>

#include "MeterApi.h"
#include "StatsLog.h"
#include "utils.h"

static uint8_t buffer[100 * 1024 * 1024]; // Attempt big buffer...

static void usage() {
  std::cerr << "Usage:\n";
  std::cerr << "   sockperf nbytes my-port meter-host meter-port [seconds] "
               "[logfile]\n";
  std::cerr << "Where\n";
  std::cerr << "  nbytes - is the size of the block being timed.\n";
  std::cerr
      << "  my-port - is the port number on which we listen for a connection\n";
  std::cerr << "  meter-host, meter-port are the connection params to a meter "
               "server\n";
  std::cerr << "            (use '-' for meter-host to skip the meter GUI "
               "entirely)\n";
  std::cerr
      << "  seconds - optional test duration; when given, run that long then\n";
  std::cerr
      << "            write a summary and exit (0/omitted => run forever)\n";
  std::cerr << "  logfile - optional file to append the summary block to\n";
}

//
// Time block - times the time required to read nbytes of data.
//   Reads are done in min of nbytes or 100*1024*1024 byte blocks
//   but we care about the actual number of bytes read.
static double timeBlock(CSocket *peer, int nbytes, uint64_t &outBytes,
                        uint64_t &outNs) {
  int bytesRead = 0;
  int blocksize = sizeof(buffer) < nbytes ? nbytes : sizeof(buffer);

  timespec start, stop;

  clock_gettime(CLOCK_REALTIME, &start);
  while (bytesRead < nbytes) {
    bytesRead += peer->Read(buffer, blocksize);
  }
  clock_gettime(CLOCK_REALTIME, &stop);

  uint64_t ns = hrDiff(stop, start);

  double rate = double(bytesRead) / ns * 1.0e9;

  std::cerr << bytesRead << " bytes read in " << ns << "ns for: " << rate
            << " Bytes/sec\n";
  outBytes = bytesRead;
  outNs = ns;
  return rate;
}

//
int main(int argc, char **argv) {
  argc--;
  argv++;
  if (argc < 4 || argc > 6) {
    usage();
    exit(EXIT_FAILURE);
  }

  int nbytes = atoi(argv[0]);
  if (nbytes <= 0) {
    usage();
    exit(EXIT_FAILURE);
  }

  std::string myport = argv[1];
  std::string mhost = argv[2];
  int mport = atoi(argv[3]);
  int seconds = (argc >= 5) ? atoi(argv[4]) : 0; // 0 => run forever
  std::string logpath = (argc >= 6) ? argv[5] : "";

  Meter *meter = 0;
  if (mhost != "-")
    meter = new Meter(mhost.c_str(), mport, "socket", 1.0, 1.0e10, true);

  CSocket *peer;
  {
    std::string who;
    CSocket listener;
    listener.Bind(myport);
    listener.Listen();
    peer = listener.Accept(who);
    std::cerr << "Connection from " << who << std::endl;
  }

  StatsLog stats("sockperf");
  timespec tStart, tNow;
  clock_gettime(CLOCK_REALTIME, &tStart);
  double elapsed = 0.0;
  do {
    uint64_t bytes, ns;
    double rate = timeBlock(peer, nbytes, bytes, ns);
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