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

/** @file  fragsrcperf.cpp
 *  @brief Measure performance of ring fragment source.
 *  @details
 *  We'll pose as an event builder for a ring fragment source.
 *  -  we won't advertise with the port manager.
 *  -  Message from the ring fragment source come in blocks that consist
 *     of a request and body.  The first two uint32_t's of the block
 *     are the size of the request and body respectively.
 *  - All messages must be acknowledged with an "OK\n" string.
 *  - The first message is a connection identification message and we'll drop
 *    it on the floor.
 *  - The rest of the messages are data messages and we'll measure the
 *    bytes/sec of body
 *  - Note that this ignores the fact that the ring items have been prefaced
 *    with body headers that are 32 bytes each.  For small fragments that may
 *    be important and we'll think about it later.
 *
 * Usage:
 *    fragsrcperf --port=PORT --blocks=N [--nsamples=N] \
 *                [--meter-host=HOST] [--meter-port=PORT] [--logfile=FILE]
 *       --port   - the port on which we accept a connection from the
 *                  ring fragment source (we emulate an event builder).
 *      --blocks  - number of blocks of data we receive from the source
 *                  per measurement pass (must be > 1).
 *      --nsamples - number of measurement passes to run.
 *      --meter-host, --meter-port - meter server connection (omit --meter-host
 *                  to disable the meter GUI).
 *      --logfile - (optional) file the summary is appended to.
 *      --verbose - print each pass's measurement to stderr (off by default).
 */

#include <iostream>
#include <memory>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>

#include <CSocket.h>
#include <Exception.h>

#include "MeterApi.h"
#include "StatsLog.h"
#include "fragsrcperf_cmdline.h"
#include "utils.h"

/**
 * since we're not actually processing data;
 * Reads a block of data from the socket (request and body) and returns
 * the size of the body.
 *
 * @param pSocket - socket connected to ring fragment source.
 * @return size_t - Size of the body.
 * @note all storage management is taken care of by this function.
 */
static size_t readBlock(CSocket *pSocket) {
  uint32_t rsize;
  uint32_t bsize;

  pSocket->Read(&rsize, sizeof(uint32_t));
  uint8_t request[rsize + sizeof(uint32_t)];
  pSocket->Read(request, rsize + sizeof(uint32_t)); // Read req and body size.

  // Last 32 bits of request are the body size:

  memcpy(&bsize, request + rsize, sizeof(uint32_t));
  void *body = malloc(bsize);
  int bread = 0;
  while (bread < bsize) {
    bread += pSocket->Read(body, bsize);
  }

  // std::cerr << std::hex << "Req: " << rsize << " body: " << bsize
  //     << std::dec << std::endl;

  const char *pReply = "OK\n";
  pSocket->Write(pReply, strlen(pReply));
  // pSocket->Flush();
  free(body);

  return bsize;
}

/**
 * acceptConnection
 *    Accepts a single connnection on our listen port.
 *  @param port - stringified port namne
 *  @return CSocket*  - pointer to the socket that's connected to the peer.
 *  @note after accepting the first connection, the listen socket is closed
 *        ensuring future connection requests result in ECONREFUSED
 */
static CSocket *acceptConnection(std::string port) {
  CSocket listener;
  listener.Bind(port);
  listener.Listen(1);
  std::string client;

  CSocket *pConnection = listener.Accept(client);

  std::cerr << "Accepted connection from " << client << std::endl;

  return pConnection;
}

/**
 *   Do the timing.
 *
 *   @param pSock - pointer to the socket from which to read data.
 *   @param blocks - number of blocks to read.
 *   @param outBytes - (out) body bytes read in this measurement.
 *   @param outNs - (out) nanoseconds the measurement took.
 *   @return double - the measured rate in bytes/sec.
 */
static double timeBlocks(CSocket *pSock, ssize_t blocks, bool verbose,
                         uint64_t &outBytes, uint64_t &outNs) {
  timespec start, stop;
  clock_gettime(CLOCK_REALTIME, &start);
  size_t bytes = 0;
  for (int i = 0; i < blocks; i++) {
    bytes += readBlock(pSock);
  }
  clock_gettime(CLOCK_REALTIME, &stop);
  uint64_t nsdiff = hrDiff(stop, start);

  double rate = double(bytes) / nsdiff * 1.0e9;

  if (verbose)
    std::cerr << bytes << " read in " << nsdiff << "ns -> " << rate
              << " bytes/sec\n";
  outBytes = bytes;
  outNs = nsdiff;
  return rate;
}

/**
 * main
 *   Entry point
 *   - accept a connection (we do not advertise with the port manager)
 *   - Drop the connection data block on the floor.
 *   - Time the performance of successive block-count blocks of data.
 *
 *  See file level comments for usage.
 */
int main(int argc, char **argv) {
  gengetopt_args_info args;
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(EXIT_FAILURE);

  if (args.blocks_arg <= 1) {
    std::cerr << "blocks must be > 1\n";
    exit(EXIT_FAILURE);
  }
  std::string strPort = std::to_string(args.port_arg);
  std::string logpath = args.logfile_given ? args.logfile_arg : "";
  try {
    std::unique_ptr<Meter> pMeter;
    if (args.meter_host_given)
      pMeter.reset(new Meter(args.meter_host_arg, args.meter_port_arg,
                             "ringfragsrc", 1.0, 1.0e9, true));

    CSocket *pSock = acceptConnection(strPort);

    readBlock(pSock);
    std::cerr << "Got connection block\n";

    // Since the time to start the run is in human times,
    // don't start timing until after we get the first data block>

    readBlock(pSock);
    std::cerr << "Got the first data block\n";

    StatsLog stats("ringfragsrc");
    timespec tStart, tNow;
    clock_gettime(CLOCK_REALTIME, &tStart);
    for (long i = 0; i < args.nsamples_arg; ++i) {
      uint64_t bytes, ns;
      double rate = timeBlocks(pSock, args.blocks_arg, args.verbose_flag, bytes, ns);
      if (pMeter)
        pMeter->set(rate);
      stats.add(bytes, ns);
    }
    clock_gettime(CLOCK_REALTIME, &tNow);
    double elapsed = hrDiff(tNow, tStart) / 1.0e9;

    stats.writeSummary(logpath, elapsed);
  } catch (CException &e) {
    std::cerr << e.ReasonText() << std::endl;
    exit(EXIT_FAILURE);
  }
}
