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

/** @file:  socksend.cpp
 *  @brief: send data over a socket.
 */

/**
 *  Purpose of this program is to see what the performance of sockets
 *  TCP/sock-stream is.  This end of the test just streams data over
 *  the socket to a performance measuring program.
 *
 * Usage:
 *   socksend host port
 *
 * We'll attempt 100Mbyte sends but experience says those will be chunked
 * up...so we'll probably not suceed.
 */
#include <iostream>
#include <CSocket.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t buffer[100*1024*1024];


static void usage()
{
    std::cerr << "Usage:\n";
    std::cerr << "   socksend host port\n";
    std::cerr << "Where:\n";
    std::cerr << "   host,port specify the connection parmeters of our peer\n";
}

int
main(int argc, char** argv)
{
    argc--; argv++;                       // Skip the program name.
    if (argc != 2) {
        usage();
        exit(EXIT_FAILURE);
    }
    std::string host = argv[0];
    std::string port = argv[1];
    
    CSocket sock;
    sock.Connect(host, port);
    
    while(1) {
        sock.Write(buffer, sizeof(buffer));
    }
}