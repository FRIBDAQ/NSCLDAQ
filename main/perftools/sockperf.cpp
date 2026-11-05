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
 *  Measures the rate at which data can be transferred over a sock_stream TCP
 *  socket, over a fixed number of passes.  We listen on a port, accept one
 *  connection from the sender (e.g. socksend) and time successive blocks.
 *
 *  Options (see sockperf.ggo / --help):
 *    --blocksize  bytes measured per transfer pass (default 8MB).
 *    --port       (required) port on which we accept the sender connection.
 *    --nsamples   number of passes to run (default 1000); bounds the test.
 *    --bufsize    largest single read() request in bytes (default 8MB).
 *    --meter-host meter-server host; omit to disable the meter GUI (log only).
 *    --meter-port meter-server port (default 2000).
 *    --logfile    file the run summary is appended to.
 *    --verbose    print each pass's measurement to stderr (off by default).
 *    --rcvbuf     set the socket receive buffer (SO_RCVBUF) in bytes.
 *
 *  The --blocksize (8MB) default is tuned for a steady-state throughput test:
 *  the block is large enough that the fixed per-pass overhead (two clock reads
 *  and a division) is negligible. --bufsize caps each read() request and sizes
 *  the read buffer (allocated as the smaller of bufsize and blocksize). Unlike
 *  a pipe (capped at its ~64KiB capacity), a socket read can return up to the
 *  full request, so a larger --bufsize up to blocksize means fewer, larger
 *  reads; beyond blocksize it is inert.  The actual per-read chunk is finally
 *  bounded by the socket receive buffer (SO_RCVBUF, which TCP autotunes up to
 *  net.ipv4.tcp_rmem's max -- commonly ~6MB); the 8MB blocksize/bufsize
 *  defaults sit just above that so the request is never the limiter.  Use
 *  --rcvbuf to raise SO_RCVBUF (set on the listening socket before accept, so
 *  the accepted connection inherits it) when you want reads larger than the
 *  stock kernel ceiling; the kernel caps it at net.core.rmem_max.  Lower
 *  --bufsize to force small reads and measure syscall-bound behavior.
 *  --verbose is off by default so the per-pass line does not add terminal I/O
 *  between passes.  If the sender closes the connection before --nsamples
 *  passes complete, sockperf stops and summarizes what it collected.
 *
 *  Typical use (defaults give a steady-state test); start this, then the
 * sender: sockperf --port=9000 --logfile=run.log socksend
 * --host=<sockperf-host> --port=9000
 */

#include <iostream>
#include <memory>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>

#include <CSocket.h>
#include <CTCPConnectionLost.h>
#include <Exception.h>

#include "MeterApi.h"
#include "StatsLog.h"
#include "sockperf_cmdline.h"
#include "utils.h"

// Time one pass of 'nbytes' bytes read from the socket, in chunks of at most
// 'bufsize'.  A socket read can return up to the requested size (unlike a pipe,
// which is capped at its ~64KiB capacity), so a larger bufsize means fewer,
// larger reads -- up to blocksize, beyond which it is inert.
static double timeBlock(CSocket *peer, long nbytes, uint8_t *buf, long bufsize,
                        bool verbose, uint64_t &outBytes, uint64_t &outNs) {
  long bytesRead = 0;

  timespec start, stop;
  clock_gettime(CLOCK_REALTIME, &start);
  while (bytesRead < nbytes) {
    long want = nbytes - bytesRead;
    if (want > bufsize)
      want = bufsize; // never read past the buffer or past blocksize.
    bytesRead += peer->Read(buf, want);
  }
  clock_gettime(CLOCK_REALTIME, &stop);

  uint64_t ns = hrDiff(stop, start);
  double rate = double(bytesRead) / ns * 1.0e9;

  if (verbose)
    std::cerr << bytesRead << " bytes read in " << ns << "ns for: " << rate
              << " Bytes/sec\n";

  outBytes = bytesRead;
  outNs = ns;

  return rate;
}

//
int main(int argc, char **argv) {
  struct gengetopt_args_info args;
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(EXIT_FAILURE);

  // Each read requests at most --bufsize bytes; a pass never needs more than
  // --blocksize, so the buffer only has to be as large as the smaller of the
  // two (avoids over-allocating when bufsize > blocksize).
  long bufbytes = args.bufsize_arg < args.blocksize_arg ? args.bufsize_arg
                                                        : args.blocksize_arg;
  std::vector<uint8_t> buffer(bufbytes);
  std::string myport = std::to_string(args.port_arg);
  std::string logpath = args.logfile_given ? args.logfile_arg : "";

  std::unique_ptr<Meter> pMeter;
  CSocket *peer;
  try {
    if (args.meter_host_given)
      pMeter.reset(new Meter(args.meter_host_arg, args.meter_port_arg, "socket",
                             1.0, 1.0e10, true));

    std::string who;
    CSocket listener;
    listener.Bind(myport);
    if (args.rcvbuf_given)
      // Set SO_RCVBUF before Listen so the accepted socket inherits it (and TCP
      // window scaling is negotiated for the larger buffer at connect time).
      listener.setRcvBufSize(args.rcvbuf_arg);
    listener.Listen();
    peer = listener.Accept(who);
    std::cerr << "Connection from " << who << std::endl;
  } catch (CException &e) {
    std::cerr << "sockperf: ** ERROR ** " << e.ReasonText() << std::endl;
    exit(EXIT_FAILURE);
  }

  StatsLog stats("sockperf");
  timespec tStart, tNow;
  clock_gettime(CLOCK_REALTIME, &tStart);
  try {
    for (long i = 0; i < args.nsamples_arg; ++i) {
      uint64_t bytes, ns;
      double rate = timeBlock(peer, args.blocksize_arg, buffer.data(), bufbytes,
                              args.verbose_flag, bytes, ns);
      if (pMeter)
        pMeter->set(rate);
      stats.add(bytes, ns);
    }
  } catch (CTCPConnectionLost &) {
    // Sender closed the connection before nsamples passes completed; stop and
    // summarize what we collected (same graceful-EOF policy as pipeperf).
    std::cerr << "sockperf: sender closed the connection after "
              << stats.samples() << " sample(s); stopping early.\n";
  } catch (CException &e) {
    std::cerr << "sockperf: ** ERROR ** " << e.ReasonText() << std::endl;
    exit(EXIT_FAILURE);
  }
  clock_gettime(CLOCK_REALTIME, &tNow);
  double elapsed = hrDiff(tNow, tStart) / 1.0e9;

  stats.writeSummary(logpath, elapsed);

  return 0;
}
