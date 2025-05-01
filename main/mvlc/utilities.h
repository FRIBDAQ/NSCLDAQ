/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief header defining some utility functions for mvlcgenerate.
*/

#ifndef MVLC_UTILITY_H
#define MVLC_UTILITY_H
#include <string>
#include "options.h"

std::string computeOutfile(gengetopt_args_info& args);


#endif