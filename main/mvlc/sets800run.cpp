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
    @file sets800run 
    @brief - fribdaq_readout slow controls client to set the run number in the new s800 trigger module
*/
#include <fribdaq/CVMEClient.h>
#include <s800TriggerRegisters.h>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <CVMUSBReadoutList.h>    // I only want this for the address modifier def
#include <string.h>

//Note the structure of this is very much like sets800triggerrun.cpp and therefore:
/** @todo - see if there's sufficient common code with ssets800triggerrun.cpp to 
 *         be worth a refactoring.
 */
/**
  * Usage:
  *    $DAQBIN/mvlc_sets800run host port devname base-address run
  * Where:
  *     host - is the host in which VMUSBReadout is runing.
  *     port - is the port on which the slow control server is listening for connections.
  *     devname - is the name  of the VMUSB device in the slow controls system.
  *     base_address - is the VME base address of the trigger module.
  *     run    - is the run number to set.
  */

static void
usage(const char* msg) {
    std::cerr << "**** Error: " << msg << std::endl << std::endl;
    std::cerr << "Usage:\n";
    std::cerr << "   $DAQBIN/mvlc_sets800run host port dev-name base-address run\n";
    std::cerr << "Sets the run number in the s800 trigger module.\nWHere:\n";
    std::cerr << "   host - is the host in which the VMUSBReadout is running\n";
    std::cerr << "   port - Is the slow controls port of the VMUSBReadout\n";
    std::cerr << "   dev-name - Is the devicename of the 'vmusb' device created in the ctlconfig.tcl script\n";
    std::cerr << "   run  - Is the new run number to set in the trigger register.\n";
    exit(EXIT_FAILURE);
}

static void
checkArgcount(int argc, char** argv) {
if (argc != 6) {
        usage("Icorrect number of command line parameters!");
    }
}

/**
 * createClient
 *  Given the program parameters, create the CVMEClient object that will connect us
 * to the fribdaq_readout slow controls server.
 * @param argc, argv - command line parameters.
 * @return CVMEClient* - pointer tothe VME client object.  Recommended to put this in a
 *     smart pointer to ensure it is released.  In any event the caller is
 *     responsible for deleting it.
 * @note Errors are reported via usage.
 */
static CVMEClient*
createClient(int argc, char** argv) {
    checkArgcount(argc, argv);

    // Decode the port.
    
    unsigned int port = strtoul(argv[2], nullptr, 0);

    if (port == 0) {
        usage("Either port does not translate to a number it has the illegal value of zero");
    }
    //                    host     devname  
    return new CVMEClient(argv[1], argv[3],  port);


}
/**
 * setList
 *    Given the client, and command parameters, figure out what to add to it to 
 * set the run number:
 * 
 * @param client - CVMEClient&  references the VME client.
 * @param argc, argv - command line paramters.
 * @note errors are reported via usage which exits.
 * 
 */
static void 
setList(CVMEClient& client, int argc, char** argv) {
    checkArgcount(argc, argv);

    // Try to pull out the base address and new run number.
    char* endptr;
    unsigned base = strtoul(argv[4], &endptr, 0);
    if ((endptr - argv[4]) != strlen(argv[4])) {
        usage("Base address cannot be converted to an unsigned int.");
    }

    unsigned run = strtoul(argv[5], &endptr, 0);
    if ((endptr - argv[5]) != strlen(argv[5])) {
        usage("Run number cannot be converted to an unsigned int.");
    }
    S800TriggerRegisters regs(base, DEFAULT_S800REGFILE);

    client.addWrite(
        regs.runNumberLowBits(), CVMUSBReadoutList::a32UserData, run, CVMEClient::DataWidth::D32
    );
    client.addWrite(
        regs.runNumberHighBits(), CVMUSBReadoutList::a32UserData, 0, CVMEClient::DataWidth::D32
    );

}

// Entry point:

int main(int argc, char** argv) {
    std::unique_ptr<CVMEClient> controller(createClient(argc, argv));
    setList(*controller, argc, argv);

    // I think CVMEClient throws std::exception derived objects:

    try {
        auto junk = controller->execute();
    } 
    catch (std::exception& e) {
        std::cerr << "Communication with the control server failed: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    exit(EXIT_FAILURE);
}
