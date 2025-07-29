/**
 * @file xlmload.cpp 
 * @brief Main program for the MVLC XLM Loader.
 * @author Ron Fox <fox@frib.msu.edu>
 */
#include "xlm_options.h"
#include "CMVLCDirect.h"
#include "CXLM.h"
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <mesytec-mvlc/mesytec-mvlc.h>
#include <string>

/**
 *  validateOptionCombinations
 *     Some argument combinations can't be checked by gengetopt,
 * like the requirement that --signature and --verify-offset
 * must both or neithr be present.   This checks that combination
 * 
 * @param args - references the parsed argument struct cmdline_parser
 * crated.
 * @return bool - true if those options are in a valid combination.
 */
static bool validateOptionCombintations(gengetopt_args_info& args) {
    // If either is given but not both that's bad.
    if ((args.signature_given || args.verify_offset_given)  &&
        !(args.signature_given && args.verify_offset_given)
    ) {
        return false;
    }
    return true;
}
using namespace mesytec::mvlc;

int main(int argc, char** argv) {
    gengetopt_args_info args;
    if (cmdline_parser(argc, argv, &args)) {
        exit(EXIT_FAILURE);
    }
    if (!validateOptionCombintations(args) ) {
        std::cerr << 
            "If --signature or --verify-offset are given, both must be given\n";
        exit(EXIT_FAILURE);
    }
    // Need a firmware file:

    if (args.inputs_num != 1) {
        std::cerr << "There must be at exactly a single firmware file\n";
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
    // Create the encapsulated controlle rand the loader:

    CMVLCDirect controller(interface);
    XLM::CFirmwareLoader loader(controller, xlmBase);

    // Finally load the XLM

    loader(firmware);

    // If requested, verify the signature after a second.

    if (args.signature_given && args.verify_offset_given) {
        if (!loader.validate(xlmBase + args.verify_offset_arg, args.signature_arg)) {
            std::cerr << "Signature verification failed!!!\n";
            exit(EXIT_FAILURE);
        }
    }
        
    return EXIT_SUCCESS;
}