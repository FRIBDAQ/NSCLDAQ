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

  try {
    CSocket sock;
    if (args.sndbuf_given)
      // Set SO_SNDBUF before Connect so TCP window scaling is negotiated
      // for the larger buffer at connection time.
      sock.setSndBufSize(args.sndbuf_arg);
    sock.Connect(host, port);

    while (1) {
      sock.Write(buffer.data(), buffer.size());
    }
  } catch (CException &e) {
    std::cerr << "socksend: ** ERROR ** " << e.ReasonText() << std::endl;
    return EXIT_FAILURE;
  }
}
