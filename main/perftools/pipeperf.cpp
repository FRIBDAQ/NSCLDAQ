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
 *  Measures the byte/sec through a unix pipe over a fixed number of passes.
 *
 *  Options (see pipeperf.ggo / --help):
 *    --blocksize  bytes read (and timed) per measurement pass (default 4MB).
 *    --nsamples   number of passes to run (default 1000); bounds the test.
 *    --bufsize    largest single read() request in bytes (default 4MB).
 *    --meter-host meter-server host; omit to disable the meter GUI (log only).
 *    --meter-port meter-server port (default 2000).
 *    --logfile    file the run summary is appended to.
 *    --verbose    print each pass's measurement to stderr (off by default).
 *    --pipe-size  resize the stdin pipe (F_SETPIPE_SZ) in bytes (rounded up to
 *                 a power of two, capped at /proc/sys/fs/pipe-max-size).
 *
 *  The --blocksize (4MB) default is tuned for a steady-state throughput test:
 *  the block is large enough that the fixed per-pass overhead (two clock reads
 *  and a division, all outside the timed region) is negligible. --bufsize caps
 *  each read() request and sizes the read buffer (allocated as the smaller of
 *  bufsize and blocksize). A Unix pipe hands back at most its capacity (~64KiB)
 *  per read regardless, so a --bufsize at or above that is inert on a pipe; set
 *  --bufsize below ~64KiB to force small reads and measure syscall-bound
 *  behavior instead; --pipe-size changes that ~64KiB capacity, which moves
 *  where read-size effects appear. With the defaults a bare
 *  "pipesend | pipeperf" runs a steady-state test.
 *
 *  Typical use (defaults give a steady-state test):
 *    pipesend | pipeperf --logfile=run.log
 */

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <unistd.h>
#include <vector>

#include <Exception.h>

#include "MeterApi.h"
#include "StatsLog.h"
#include "pipeperf_cmdline.h"
#include "utils.h"

// Read and time one pass of 'blocksize' bytes from stdin, in chunks of at most
// 'bufsize'.  Returns true and fills rate/outBytes/outNs on a full pass;
// returns false on EOF (the sender closed the pipe), leaving the incomplete
// pass to be discarded by the caller.
static bool measure(long blocksize, uint8_t *buf, long bufsize, bool verbose,
                    double &rate, uint64_t &outBytes, uint64_t &outNs) {
  long nread = 0;

  timespec start, stop;
  clock_gettime(CLOCK_REALTIME, &start);
  while (nread < blocksize) {
    long want = blocksize - nread;
    if (want > bufsize)
      want = bufsize; // never read past the buffer or past blocksize.
    ssize_t n = read(STDIN_FILENO, buf, want);
    if (n < 0) {
      if (errno == EINTR)
        continue; // interrupted by a signal; retry.
      perror("pipeperf: read");
      exit(EXIT_FAILURE);
    }
    if (n == 0)
      return false; // EOF: pipe closed before the pass completed.
    nread += n;
  }
  clock_gettime(CLOCK_REALTIME, &stop);
  uint64_t ns = hrDiff(stop, start);
  rate = double(nread) / ns * 1.0e9;

  if (verbose)
    std::cerr << "pipeperf: read " << nread << " bytes in " << ns
              << "ns for: " << rate << " Bytes/sec\n";

  outBytes = nread;
  outNs = ns;

  return true;
}

int main(int argc, char **argv) {
  struct gengetopt_args_info args;
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(EXIT_FAILURE);

  if (args.pipe_size_given) {
    // Resize the pipe's kernel buffer.  The kernel rounds up to a power of two
    // and caps at /proc/sys/fs/pipe-max-size; the return value is the actual
    // capacity set.
    int actual = fcntl(STDIN_FILENO, F_SETPIPE_SZ, (int)args.pipe_size_arg);
    if (actual < 0) {
      perror("pipeperf: F_SETPIPE_SZ");
      exit(EXIT_FAILURE);
    }
    if (verbose)
      std::cerr << "pipeperf: stdin pipe capacity set to " << actual
                << " bytes\n";
  }

  // Each read requests at most --bufsize bytes; a pass never needs more than
  // --blocksize, so the buffer only has to be as large as the smaller of the
  // two (avoids over-allocating when bufsize > blocksize).
  long bufbytes = args.bufsize_arg < args.blocksize_arg ? args.bufsize_arg
                                                        : args.blocksize_arg;
  std::vector<uint8_t> buffer(bufbytes);
  std::string logpath = args.logfile_given ? args.logfile_arg : "";

  std::unique_ptr<Meter> pMeter;
  try {
    if (args.meter_host_given)
      pMeter.reset(new Meter(args.meter_host_arg, args.meter_port_arg, "pipe",
                             1.0, 1.0e10, true));
  } catch (CException &e) {
    std::cerr << "pipeperf: cannot connect to meter server: " << e.ReasonText()
              << std::endl;
    exit(EXIT_FAILURE);
  }

  StatsLog stats("pipe");
  timespec tStart, tNow;
  clock_gettime(CLOCK_REALTIME, &tStart);
  for (long i = 0; i < args.nsamples_arg; ++i) {
    double rate;
    uint64_t bytes, ns;
    if (!measure(args.blocksize_arg, buffer.data(), bufbytes, args.verbose_flag,
                 rate, bytes, ns)) {
      std::cerr << "pipeperf: end of input after " << stats.samples()
                << " sample(s); stopping early.\n";
      break;
    }
    if (pMeter)
      pMeter->set(rate);
    stats.add(bytes, ns);
  }
  clock_gettime(CLOCK_REALTIME, &tNow);
  double elapsed = hrDiff(tNow, tStart) / 1.0e9;

  stats.writeSummary(logpath, elapsed);

  return 0;
}
