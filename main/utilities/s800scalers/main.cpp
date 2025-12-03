#include <stdlib.h>
#include "options.h"
#include <string>
#include <tcl.h>  // For wildcarded string match.
#include <set>    // THe register addresses will be in a set<uint32_t>
#include <stdint.h>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <json/json.h>

// Patterns that identify the register names I care about.

static const char* COUNTER_MATCH="*_CTR";
static const char* FREQUENCY_MATCH="*/FRQ";


// Read the JSON configuration file.
// throws std::exception derived exception on failures
// name - path to the configuration file.
// result(out) - internal representation of the json.
static void
parse_definition_file(const std::string& name, Json::Value& result) {
    std::ifstream f(name);
    if (!f) {
        std::stringstream msg_s;
        msg_s << "Unable to open configuration file: " << name;
        std::string msg(msg_s.str());

        throw std::invalid_argument(msg);
    }

    // the parse could also throw

    f >> result;
}

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

    // Read in the definition file:

    Json::Value root;
    try {
        parse_definition_file(config_file, root);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    // open the output file:
    
    std::ofstream out(output_file);
    if (!out) {
        std::cerr << "Unable to open the output file: " << output_file << std::endl;
        exit(EXIT_FAILURE);
    }

    // Process the counters and write them.


    // Process the frequencies if asked and write them:

    if (include_freqs) {

    }

    exit(EXIT_SUCCESS);
}