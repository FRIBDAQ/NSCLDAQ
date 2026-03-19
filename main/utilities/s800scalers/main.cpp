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

static const char* COUNTER_MATCH="*_CNT";
static const char* FREQUENCY_MATCH="*_FRQ";

static const char* RAW_TRIGGERS="R_RAW";   // Raw triggers.
static const char* LIVE_TRIGGERS="R_LIVE"; // Live triggers.

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

// Build a set of the register addresses that match a specific name.
// we use a set because these will be sorted by addresss so when we dump them
// we can aggregate any blocks.
static void
make_register_set(std::set<uint32_t>& result, Json::Value& root, const char* match) {
    if (! root.isMember("Registers")) {
        throw std::invalid_argument("The JSON does not have a 'Registers' key!");
    }
    auto regs = root["Registers"];
    for (auto& v : regs) {
        std::string name = v["Name"].asString();
        if (Tcl_StringMatch(name.c_str(), match)) {
            result.insert(v["Address"].asUInt());
        }
    }
}
// Dump the registers in the form of 
//  address n 
//Where:
// addresss is a base addresss and n is the number of consecutive uint32_t
// addresses that follow.
//   f = references the file stream to which the dump is done.
//   regs - are the set of register addresses.
static void
dump_register_set(std::ostream& f, std::set<uint32_t>& regs) {
    // Aggreation hinges on the idea that the regs will iterate out of the
    // set in numeric order.

    // COunt will be the count of consecutives.
    // base - the current base.
    // next - the next expected if it's consecutive.

    unsigned count = 0;               // Number of consecutives.
    unsigned base  = 0xffffffff;      // Special for the first one.
    unsigned next  = 0xffffffff;
    for(auto r: regs) {
        // New clump.
        if (r != next) {
            if (base != 0xffffffff) {
                f << base << " " << count << std::endl;
            }
            // Set up for the next clump:

            base = r;
            next = r + sizeof(uint32_t);
            count = 1;
        }  else {           // Continue current clump:
            count++;
            next += sizeof(uint32_t);
        }
    }
    // Dump the last clump:

    if (count != 0) {
        f << base << " " << count << std::endl;
    }
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

    // The live and raw triggers:

    try {
        std::set<uint32_t> counters;   // Note that make_register_set accumulates but does not preserve order:
        make_register_set(counters, root, RAW_TRIGGERS);  // Raw trigger counter.
        dump_register_set(out, counters); 
        counters.clear();             // Reset for the live triggers.
        make_register_set(counters, root, LIVE_TRIGGERS); // live trigger counter.
        dump_register_set(out, counters);

    }
    catch (std::exception& e) {
        std::cerr << "Failed to process the trigger counters: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    try {    
        std::set<uint32_t> counters;
        make_register_set(counters, root, COUNTER_MATCH);
        dump_register_set(out, counters);
    }
    catch (std::exception& e) {
        std::cerr << " Failed to process the counters: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    // Process the frequencies if asked and write them:

    if (include_freqs) {
        try {
            std::set<uint32_t> frequencies;
            make_register_set(frequencies, root, FREQUENCY_MATCH);
            dump_register_set(out, frequencies);
        }
        catch (std::exception& e) {
            std::cerr << "Failed to process the frequencies: " << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}