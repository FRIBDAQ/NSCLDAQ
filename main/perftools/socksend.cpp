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

/** @file:  socksend.cpp
 *  @brief: send data over a socket.
 */

/**
 *  Purpose of this program is to see what the performance of sockets
 *  TCP/sock-stream is.  This end of the test just streams data over
 *  the socket to a performance measuring program.
 *
 * Usage:
 *   socksend --host=HOST --port=PORT [--bufsize=BYTES] [--sndbuf=BYTES]
 *      --host, --port - connection parameters of the measuring peer.
 *      --bufsize      - size in bytes of each write to the socket
 *                       (default 100 MB).
 *      --sndbuf       - set the socket send buffer (SO_SNDBUF) in bytes
 *                       (default: leave kernel autotuning).  Pairs with
 *                       sockperf's --rcvbuf for tuning large transfers.
 *
 * socksend streams until the consumer closes the connection (e.g. sockperf
 * finishing its passes); that is reported and treated as a clean exit (0).
 * Setup failures (bad --sndbuf, connection refused) exit non-zero.
 *
 * We'll attempt large sends but experience says those will be chunked
 * up...so we'll probably not suceed.
 */
#include <CSocket.h>
#include <Exception.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "socksend_cmdline.h"

int main(int argc, char **argv) {
  struct gengetopt_args_info args;
  if (cmdline_parser(argc, argv, &args) != 0) {
    exit(EXIT_FAILURE);
  }

  std::string host = args.host_arg;
  std::string port = std::to_string(args.port_arg);
  std::vector<uint8_t> buffer(args.bufsize_arg);

  // Setup: a failure here (bad --sndbuf, connection refused, ...) is a real
  // error.
  CSocket sock;
  try {
    if (args.sndbuf_given)
      // Set SO_SNDBUF before Connect so TCP window scaling is negotiated
      // for the larger buffer at connection time.
      sock.setSndBufSize(args.sndbuf_arg);
    sock.Connect(host, port);
  } catch (CException &e) {
    std::cerr << "socksend: ** ERROR ** " << e.ReasonText() << std::endl;
    return EXIT_FAILURE;
  }

  // Stream until the consumer stops reading and closes the socket.  Once we are
  // connected, the normal way to finish is the peer going away: a bounded
  // receiver (e.g. sockperf) reads its fixed amount then exits, which resets or
  // closes the connection (ECONNRESET -> CErrnoException, EPIPE ->
  // CTCPConnectionLost).  That is a clean end for a generator, not an error.
  try {
    while (1) {
      sock.Write(buffer.data(), buffer.size());
    }
  } catch (CException &e) {
    std::cerr << "socksend: peer closed the connection; done (" << e.ReasonText()
              << ").\n";
    return EXIT_SUCCESS;
  }
}
