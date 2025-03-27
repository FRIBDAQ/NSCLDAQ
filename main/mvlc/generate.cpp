
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
@brief Main driver for turning VMUSBReadout daqconfig scripts -> mvlc fribdaq-readout .yaml configs
*/

#include "options.h"
#include "utilities.h"
#include <iostream>
#include <string>


#include <stdlib.h>


int main(int argc, char** argv) {
    gengetopt_args_info args;
    if (cmdline_parser(argc, argv, &args)) {
        exit(EXIT_FAILURE);
    }
    if (args.inputs_num != 1) {
        std::cerr << "You must provide a parameter that is the input file\n";
        cmdline_parser_print_help();
    }
    // Extract the input file and the output file.

    std::string infile = args.inputs[0];
    std::string outfile = computeOutfile(args);


    std::cout << "infile: " << infile << " outfile: " << outfile << std::endl;

}