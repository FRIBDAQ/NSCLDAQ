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

/** @file:  bufferedoutperf.cpp
 *  @brief: Test performance of the buffered outputter.
 */


#include <CBufferedOutput.h>
#include "utils.h"
#include <time.h>


#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "bufferedoutperf_cmdline.h"


/**
 * usage:
 *     bufferedoutperf --mbytes=M --filename=F [--buffersize=K] [--writesize=B]
 *
 *   -   --buffersize - the size of the output buffer in Kbytes (default 8192).
 *   -   --writesize  - size of each write in bytes (default 8192).
 *   -   --mbytes     - total number of megabytes (MB) to write.
 *   -   --filename   - file to write it to.
 */

//

int main(int argc, char** argv)
{
    gengetopt_args_info args;
    if (cmdline_parser(argc, argv, &args) != 0) {
        exit(EXIT_FAILURE);
    }

    int bufferK    = args.buffersize_arg;
    int writeBytes = args.writesize_arg;
    int mBytesTotal = args.mbytes_arg;
    std::string file = args.filename_arg;

    // All of the sizes must be > 0.

    if ((bufferK <= 0) || (writeBytes <= 0) || (mBytesTotal <= 0)) {
        std::cerr << "buffersize, writesize and mbytes must all be > 0\n";
        exit(EXIT_FAILURE);
    }
    // open the file, make the outputter.
    
    int fd = creat(file.c_str(), S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }
    
    io::CBufferedOutput o(fd, bufferK*1024);
    std::vector<uint8_t> data(writeBytes);


    size_t n = 0;
    timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);
    while (n < (mBytesTotal*1024*1024)) {
        o.put(data.data(), writeBytes);
        n+= writeBytes;
    }
    o.flush();
    clock_gettime(CLOCK_REALTIME, &stop);
    
    uint64_t ns = hrDiff(stop, start);
    
    double rate = double(n)/ns * 1.0e9;
    std::cout << n << " bytes written in " << ns << " nanoseconds for "
        << rate << "bytes/sec\n";
    
}