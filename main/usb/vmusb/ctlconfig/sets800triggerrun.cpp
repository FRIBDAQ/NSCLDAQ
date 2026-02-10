/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
/**
 * @file sets800triggerrun.cpp
 * @brief Set the run number for an s800 new trigger module.
 */
#include <stdlib.h>
#include <CVMUSBReadoutList.h>
#include <CVMUSBRemote.h>
#include <iostream>
#include <string>
#include <s800TriggerRegisters.h>
#include <memory>
#include <string.h>
 /**
  * Usage:
  *    $DAQBIN/sets800triggerrun host port devname base-address run
  * Where:
  *     host - is the host in which VMUSBReadout is runing.
  *     port - is the port on which the slow control server is listening for connections.
  *     devname - is the name  of the VMUSB device in the slow controls system.
  *     base_address - is the VME base address of the trigger module.
  *     run    - is the run number to set.
  */

static void usage(const char* msg) {
    std::cerr << "**** Error: " << msg << std::endl << std::endl;
    std::cerr << "Usage:\n";
    std::cerr << "   $DAQBIN/sets800triggerrun host port dev-name base-address run\n";
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
 * Given the command line parameters, return, (if possible) ad CVMUSBRemot4e client
 * through which operations can be performe4d.
 * 
 * Errors result in calls 'usage' with the appropriate error message.
 * @return CVMUSBRemote* - pointer to the object created.  caller is responsible for
 *       deleting it so maybe store it in a unique_ptr e.g.
 */
static CVMUSBRemote*
createClient(int argc, char** argv) {
    checkArgcount(argc, argv);

    std::string host(argv[1]);
    unsigned int port = strtoul(argv[2], nullptr, 0);
    std::string dev(argv[3]);

    if (port == 0) {
        usage("Either port does not translate to a number it has the illegal value of zero");
    }
    return new CVMUSBRemote(dev, host, port);
}

/**
 * createReadoutList
 * 
 *    Given the command parameters, fill in the CVMUSBReadoutList with what's needed to
 * write the run number.
 * 
 * @param list - references the list to fill in.
 * @param argc, argv - command parameters.
 * 
 * @note failures are reported via usage().
  */
static void
createReadoutList(CVMUSBReadoutList& list, int argc, char** argv) {
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
    
    // Run Number seems to be 32 bits(?).
    list.addWrite32(regs.runNumberLowBits(), CVMUSBReadoutList::a32UserData, run);
    //?  list.addWrite32(regs.runNumberHighBits(), CVMUSBReadoutList::a32UserData, 0);

}

int main(int argc, char** argv) {
    std::unique_ptr<CVMUSBRemote> controller(createClient(argc, argv));
    CVMUSBReadoutList list;
    createReadoutList(list, argc, argv);

    char* junk[100];           // Won't be anything read back but...
    size_t junkSize = sizeof(junk);

    int status = controller->executeList(list, &junk, junkSize, &junkSize);

    if (status != 0) {
        std::cerr << "Failed to write the run number \n";
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

// crap we have to do because of the framwork:

namespace Globals {
    void* pUSBController(0);
}