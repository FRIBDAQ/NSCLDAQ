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

/** @file:  pipesend
 *  @brief: Send crap to stdout for measuring pipeline performance.
 */
#include <unistd.h>
#include <stdint.h>

static uint8_t buffer[1024*1024*100];

int main(int argc, char** argv)
{
    while(1) {
        write(STDOUT_FILENO, buffer, sizeof(buffer));
    }
}