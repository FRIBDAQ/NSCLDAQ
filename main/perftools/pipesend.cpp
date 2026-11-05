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

/** @file  pipesend
 *  @brief Send a stream of bytes to stdout for measuring pipe performance.
 *  @details
 *  Writes a buffer to stdout forever; pipe it into pipeperf to measure the
 *  throughput of a Unix pipe.
 *
 * Usage:
 *    pipesend [--bufsize=BYTES] | pipeperf ...
 *       --bufsize - size in bytes of each write to stdout (default 100 MB).
 */

#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <vector>

#include "pipesend_cmdline.h"

int main(int argc, char **argv) {
  struct gengetopt_args_info args;
  if (cmdline_parser(argc, argv, &args) != 0) {
    exit(EXIT_FAILURE);
  }

  std::vector<uint8_t> buffer(args.bufsize_arg);

  while (1) {
    write(STDOUT_FILENO, buffer.data(), buffer.size());
  }
}
