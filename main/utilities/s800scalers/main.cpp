#include <stdlib.h>
#include "options.h"
#include <string>
#include <tcl.h>  // For wildcarded string match.
#include <set>    // THe register addresses will be in a set<uint32_t>
#include <stdint.h>
#include <iostream>
// Patterns that identify the register names I care about.

static const char* COUNTER_MATCH="*_CTR";
static const char* FREQUENCY_MATCH="*/FRQ";

int main(int argc, char** argv) {
    gengetopt_args_info parsed_args;
    cmdline_parser(argc, argv, &parsed_args);

    // I need one or two unnamed parameters;
    // the first is the output filename.
    // The last, if provided, overrides the
    // default path to the register config file.

    const char* config_file_cstr = DEFAULT_REGISTER_FILE;

    if ((parsed_args.inputs_num == 0) || (parsed_args.inputs_num > 2)) {
        cmdline_parser_print_help();
        std::cerr << "The inputs must be the output file and, optionally the path to the ";
        std::cerr << "register definition file\n";
        exit(EXIT_FAILURE);
    }
    std::string output_file = parsed_args.inputs[0];

    // If needed, override the config file:
    if (parsed_args.inputs_num == 2) {
        config_file_cstr = parsed_args.inputs[1];
    }
    std::string config_file = config_file_cstr;


    std::cout << "Generating " << output_file << " from: " << config_file << std::endl;
    bool include_freqs = parsed_args.include_frequencies_flag == 0? false : true;

    if (include_freqs) {
        std::cout << "Frequencies and counters will both be included\n";
    } else {
        std::cout << "Only counter will be used.\n";
    }

    exit(EXIT_SUCCESS);
}