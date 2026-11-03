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

/** @file:  metertest.cpp
 *  @brief: test meter server client library.
 */

#include "MeterApi.h"
#include <stdlib.h>
#include <unistd.h>


int
main(int argc, char**argv)
{
    Meter m("localhost", 2000, "test", 0.0 ,100.0, false);
    m.set(50.0);         // Start midscale
    double value = 50.0;
    
    while(1) {
        sleep (1);
        value += drand48()*10 - 5.0;
        m.set(value);
    }
}