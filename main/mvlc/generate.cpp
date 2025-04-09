
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

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
#include "MVLCConfigParser.h"
#include <iostream>
#include <string>
#include <vector>

#include <Exception.h>
#include <stdlib.h>
#include "CStack.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include "MVLCGenerator.h"
#include <stdexcept>


static void generateYaml(TCLConfigParser& parser, std::string outfile) {
    MVLCGenerate generator(outfile, &parser);
    try {
        generator.generate();    
    }
    catch(std::exception& e) {
        std::cerr << "Failed to generate output file: "<< e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

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

    // Now we need to process the input file.  This is done in a TclConfigParser object
    // Once that's done we can start to pull bits out of the object and push them into the templated
    // yaml configuration file...which is then written to outfile.

    try {
        MVLCConfigParser tclparser(infile);
        tclparser.initialize();
        tclparser();
        generateYaml(tclparser, outfile);
    }
    catch (CException& e) {
        std::cerr << "Failed to parse Tcl configuration file: " << infile 
            << " : " << e.ReasonText() << std::endl;
        exit(EXIT_FAILURE);
    }


    exit(EXIT_SUCCESS);

}