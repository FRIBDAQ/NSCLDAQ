/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   prebegin.cpp
# @brief  Command utility to prepare to begin a run.
# @author <fox@nscl.msu.edu>
*/


#include "prebeginopts.h"

#include "CStateManager.h"
#include <iostream>
#include <cstdlib>
#include "tracker.h"
#include "transition.h"


int main(int argc, char** argv)
{
    // Parse the args and create the stat manager object:
    
    struct gengetopt_args_info pa;
    cmdline_parser(argc, argv, &pa);
    
    CStateManager sm(pa.request_uri_arg, pa.subscribe_uri_arg);
    bool verbose = pa.verbose_flag != 0;
    
    // Require the state to be "Ready"
    
    std::string gstate = sm.getGlobalState();
    if ((gstate != "Ready") ) { 
        std::cerr << "To prebegin a run the global state must be 'Ready' ;
        std::cerr << " however it is: '" << gstate << "'\n";
        std::exit(EXIT_FAILURE);
    }
    // initiate the state transition:
    
        transition(sm, "Beginning", verbose);
    
    return EXIT_SUCCESS;
    
}