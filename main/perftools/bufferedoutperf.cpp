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

/** @file  bufferedoutperf.cpp
 *  @brief Measure throughput of the buffered file outputter.
 *  @details
 *  Writes --mbytes megabytes to --filename through io::CBufferedOutput and
 *  reports the achieved rate via --logfile (StatsLog format, like the other
 *  perftools).
 *
 *  Options (see bufferedoutperf.ggo / --help):
 *    --buffersize output buffer size in Kbytes (default 8192).
 *    --writesize  size of each write in bytes (default 8192).
 *    --mbytes     (required) total megabytes to write.
 *    --filename   (required) file to write to.
 *    --fsync      fsync to disk before stopping the timer, so the rate reflects
 *                 durable writes to the device.  Without it the data only
 *                 reaches the page cache, so (for --mbytes below RAM) the rate
 *                 measures the buffering + page-cache path, NOT the storage
 *                 media.  O_DIRECT is not offered: CBufferedOutput's buffer is
 *                 malloc'd, not block-aligned, so O_DIRECT writes would fail.
 *    --logfile    append the run summary to this file (StatsLog format);
 *                 without it there is no output.
 *
 *  Note: CBufferedOutput uses a background thread, so the timed region is
 * scoped to include its destructor (which drains that thread); with --fsync the
 * fsync is timed too.
 */

#include <fcntl.h>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <vector>

#include <CBufferedOutput.h>

#include "StatsLog.h"
#include "bufferedoutperf_cmdline.h"
#include "utils.h"

int main(int argc, char **argv) {
  gengetopt_args_info args;
  if (cmdline_parser(argc, argv, &args) != 0) {
    exit(EXIT_FAILURE);
  }

  int bufferK = args.buffersize_arg;
  int writeBytes = args.writesize_arg;
  int mBytesTotal = args.mbytes_arg;
  std::string file = args.filename_arg;

  // All of the sizes must be > 0:

  if ((bufferK <= 0) || (writeBytes <= 0) || (mBytesTotal <= 0)) {
    std::cerr << "buffersize, writesize and mbytes must all be > 0\n";
    exit(EXIT_FAILURE);
  }

  // open the file, make the outputter:

  int fd = creat(file.c_str(), S_IRUSR | S_IWUSR);
  if (fd < 0) {
    perror("File open failed");
    exit(EXIT_FAILURE);
  }

  std::vector<uint8_t> data(writeBytes);
  uint64_t nToWrite = (uint64_t)mBytesTotal * 1024 * 1024; // 64-bit: mbytes
                                                           // can exceed RAM.
  uint64_t n = 0;

  timespec start, stop;
  clock_gettime(CLOCK_REALTIME, &start);
  {
    // Scope the outputter so its destructor flushes and drains the output
    // thread *inside* the timed region -- otherwise the clock would stop
    // before the asynchronous writes finish.
    io::CBufferedOutput o(fd, bufferK * 1024);
    while (n < nToWrite) {
      o.put(data.data(), writeBytes);
      n += writeBytes;
    }
  }
  if (args.fsync_flag) {
    // Force the data to the device so the rate reflects durable-write
    // throughput rather than just the page cache.
    if (fsync(fd) < 0) {
      perror("bufferedoutperf: fsync");
      exit(EXIT_FAILURE);
    }
  }
  clock_gettime(CLOCK_REALTIME, &stop);
  close(fd);

  uint64_t ns = hrDiff(stop, start);

  // Report through StatsLog (one sample: the whole-file write) so the output
  // matches the other tools' --logfile summaries.  The mode is encoded in the
  // summary name.
  std::string logpath = args.logfile_given ? args.logfile_arg : "";
  StatsLog stats(args.fsync_flag ? "bufferedoutput (fsync)"
                                 : "bufferedoutput (buffered)");
  stats.add(n, ns);
  stats.writeSummary(logpath, ns / 1.0e9);
}