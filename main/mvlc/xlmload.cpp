/**
 * @file xlmload.cpp 
 * @brief Main program for the MVLC XLM Loader.
 * @author Ron Fox <fox@frib.msu.edu>
 */
#include "xlm_options.h"
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <mesytec-mvlc/mesytec-mvlc.h>
#include <string>

using namespace mesytec::mvlc;

int main(int argc, char** argv) {
    gengetopt_args_info args;
    if (cmdline_parser(argc, argv, &args)) {
        exit(EXIT_FAILURE);
    }
    // Need a firmware file:

    if (args.inputs_num != 1) {
        std::cerr << "There must be at least a single firmware file\n";
        exit(EXIT_FAILURE);
    }
    std::string firmware = args.inputs[0];
    std::cerr << "Firmware file " << firmware << std::endl;


    // Figure out the base address of the XLM based on the options:

    uint32_t xlmBase = 0;
    if (args.slot_given) {
        xlmBase = args.slot_arg << 27;     // base adress given the slot.
    } else {
        xlmBase = args.base_arg;
    }
    std::cerr << "Loading XLM at : 0x" << std::hex << xlmBase << std::dec << std::endl;
    
    // Creaete and connect the MVLC based on the connection argument given:
    // and connect to the controller:

    MVLC interface;
    if (args.host_given) {
        std::cerr << "Connect ethernet to " << args.host_arg << std::endl;
        interface = make_mvlc_eth(std::string(args.host_arg));
    } else if (args.usb_given) {
        std::cerr << "Connect USB  to index " << args.usb_arg << std::endl;
        if (args.usb_arg > 0) {
            interface = make_mvlc_usb(args.usb_arg);
        } else {
            interface = make_mvlc_usb();
        }
    } else if (args.serial_given) {
        std::cerr << "Connect USB to serial number: " << args.serial_arg << std::endl;
        interface = make_mvlc_usb(std::string(args.serial_arg));
    }
    auto ec = interface.connect();
    if (ec) {
        std::cerr << "Unable to connect to the MVLC: " << ec.message() << std::endl;
        exit(EXIT_FAILURE);
    }

    
    return EXIT_SUCCESS;
}