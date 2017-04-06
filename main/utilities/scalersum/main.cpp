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
# @file   main.cpp
# @brief  Main driver for the sum program.
# @author <fox@nscl.msu.edu>
*/

#include "App.h"

#include "options.h"

#include <iostream>
#include <cstdlib>
#include <memory>
#include <exception>


using namespace std;


/**
 * main
 *    Program entry point:
 *       -  Process command line parameters.
 *       -  Instantiate an application object.
 *       -  Invoke the application object.
 *       -  Ask the application object to output its results.
 */
int main(int argc, char** argv)
{
    struct gengetopt_args_info processParams;
    unique_ptr<DAQ::App> pApp;
    
    if (cmdline_parser(argc, argv, &processParams)) {
        exit(EXIT_FAILURE);             // cmdline_parser writes error msgs.
    }
    try {
        pApp.reset(new DAQ::App(processParams));
        DAQ::App& app(*pApp);
        app();
        app.outputResults(std::cout);
        
    }
    catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        
        std::cerr << "Fatal error, dumping what we've got so far";
        pApp->outputResults(std::cout);
        
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}
