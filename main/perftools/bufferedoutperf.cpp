/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  bufferedoutperf.cpp
 *  @brief: Test performance of the bufered outputter.
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


/**
 * usage:
 *     bufferedoutperf buffersize writesize mbytes fname
 *
 *   -   buffersize - the size of the buffer in Kbytes
 *   -   writesize  - size of the write in bytes.
 *   -   mbytes     - total number of bytes to write.
 *   -   fname      - Filename to write it to.
 */

static void usage()
{
    std::cerr << "Usage:\n";
    std::cerr << "   bufferedoutperf buffersize writesize mbytes fname\n";
    std::cerr << "Where\n";
    std::cerr << "   buffersize - the size of the buffer in Kbytes\n";
    std::cerr << "   writesize  - size of the write in bytes.\n";
    std::cerr << "   mbytes     - total number of bytes to write\n";
    std::cerr << "   fname      - Filename to write it to.\n";
}

//

int main(int argc, char** argv)
{
    argc--; argv++;
    
    if (argc != 4) {
        usage();
        exit(EXIT_FAILURE);
    }
    
    int bufferK    = atoi(argv[0]);
    int writeBytes = atoi(argv[1]);
    int mBytesTotal = atoi(argv[2]);
    std::string file = argv[3];
    
    // All of the sizes must be > 0.
    
    if ((bufferK <= 0) || (writeBytes <= 0) || (mBytesTotal <= 0)) {
        usage();
        exit(EXIT_FAILURE);
    }
    // open the file, make the outputter.
    
    int fd = creat(file.c_str(), S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }
    
    io::CBufferedOutput o(fd, bufferK*1024);
    uint8_t data[writeBytes];
    
    
    size_t n = 0;
    timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);
    while (n < (mBytesTotal*1024*1024)) {
        o.put(data, writeBytes);
        n+= writeBytes;
    }
    o.flush();
    clock_gettime(CLOCK_REALTIME, &stop);
    
    uint64_t ns = hrDiff(stop, start);
    
    double rate = double(n)/ns * 1.0e9;
    std::cout << n << " bytes written in " << ns << " nanoseconds for "
        << rate << "bytes/sec\n";
    
}