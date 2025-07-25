/**
 * @file xlmload.cpp 
 * @brief Main program for the MVLC XLM Loader.
 * @author Ron Fox <fox@frib.msu.edu>
 */
#include "xlm_options.h"
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
int main(int argc, char** argv) {
    gengetopt_args_info args;
    if (cmdline_parser(argc, argv, &args)) {
        exit(EXIT_FAILURE);
    }
    // Figure out the base address of the XLM based on the options:

    uint32_t xlmBase = 0;
    if (args.slot_given) {
        xlmBase = args.slot_arg << 27;     // base adress given the slot.
    } else {
        xlmBase = args.base_arg;
    }
    std::cerr << "Loading XLM at : 0x" << std::hex << xlmBase << std::dec << std::endl;

    return EXIT_SUCCESS;
}