/**
 * @file xlmload.cpp 
 * @brief Main program for the MVLC XLM Loader.
 * @author Ron Fox <fox@frib.msu.edu>
 */
#include "xlm_options.h"
#include <stdlib.h>
int main(int argc, char** argv) {
    gengetopt_args_info args;
    if (cmdline_parser(argc, argv, &args)) {
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}