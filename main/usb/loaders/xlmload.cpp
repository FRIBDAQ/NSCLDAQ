/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  xlmloader.cpp
 *  @brief: Load an XLM-72 or XLM-72V over the VMUSB.
 *  @note:  This can work with either direct connection to the VMUSB or
 *          via the slow control server of the VMUSBeadout.
 */
#include "xlmloadparams.h"
#include <cstdlib>
#include <CVMUSB.h>
#include <CVMUSBusb.h>
#include <CVMUSBRemote.h>
#include <Exception.h>
#include <stdexcept>
#include <CXLM.h>
#include <CRunState.h>


/**
 * Usage:
 *    xlmload [options...]   firmwarefile
 *
 *    Options:
 *
 *      Only one of the following:
 *      
 *       -base   - Base address of the XLM
 *       or
 *       -slot   - Slot of the XLM 
 *
 *       Only one of the following:
 *       
 *       -serialno - VMUSB Serial number (defaults to the first one found).
 *       or:
 *       -host     - Host VMUSBReadout is running ing
 *       -port     - Port the VMUSBReadout control server is listening on.
 *                   defaults to 27000
 *       -module   - Name of the VMUSB list server module.
 */
/**
 * selectVMUSB
 *   Used in local connection to select the USB device to connect with.
 *   - Enumerates the VMUSBs  if there are none complain.
 *   - If the serial number is not provided, return the first elemwent.
 *   - If the serial number is provided locate the matching VMEUSB and return
 *     that element (complain if no match).
 *     
 * @param parsedArgs - parsed parameter arguments.
 * @return usb_device*
 *
 */
static usb_device*
selectVMUSB(gengetopt_args_info& parsedParams)
{
    std::vector<usb_device*> devices = CVMUSBusb::enumerate();
    if (devices.size() == 0) {
        throw std::string("No VMUSB's are attached to this system");
    }
    
    if (!parsedParams.serial_given) {
        return devices[0];
    }
    
    // Look for a match:
    
    for (int i = 0; i < devices.size(); i++) {
        if (CVMUSBusb::serialNo(devices[i]) == parsedParams.serial_arg) {
            return devices[i];
        }
    }
    std::string except = "No VMUSB with serial number ";
    except += parsedParams.serial_arg;
    except += " is attached to this system";
    throw except;
}
/**
 * createRemoteController
 *    Called when it's determined we're connecting to the VMUSB through
 *    a VMUSBReadout proxy.
 *    We use host_arg, port_arg and module_arg to make the connection.
 *
 *  @param parsedParams - result of gengetargs parsing of command params.
 *  @return CVMUSB*     - pointer to the CVMUSBRemote instantiated.
*/

static CVMUSB*
createRemoteController(gengetopt_args_info& parsedParams)
{
    return new CVMUSBRemote(
        parsedParams.module_arg, parsedParams.host_arg, parsedParams.port_arg
    );
}
/**
 * createLocalcontroller
 *    Creates a directly connected controller.
 *    -  select the correct device (depending on the presernce of --serial).
 *    -  Create a CVMUSBusb object and return it.
 */
static CVMUSB*
createLocalController(gengetopt_args_info& parsedParams)
{
    usb_device* dev = selectVMUSB(parsedParams);
    return new CVMUSBusb(dev);
}
/**
 * createController
 *    Creates and returns the appropriate, connected VMUSB controller object.
 *
 * @return CVMUSB*    - Pointer to the actual controller created.
 * @param parsedParams - references the parsed parameters from gengetopt
 *
 * The controller created depends on the type of connection.
 * --host, --module are minimally provided a remote connection is created
 * otherwise a direct connection is made.
 */
static CVMUSB*
createController(gengetopt_args_info& parsedParams)
{
    // node is not provided, this is a local connection.  Otherwise remote:
    
    if (parsedParams.host_given) {
        return createRemoteController(parsedParams);
    } else {
        return createLocalController(parsedParams);
    }
}

/**
 * loadFirmware
 *    Do the actual firmware load.
 *    -  figure out the base address of the XLM.
 *    -  Construct and XLM::CFirmwareLoader
 *    -  Ask it to load/boot the firmware.
 *
 *  @param controller - the CVMUSB controller/connection object.
 *  @param name       - the path to the firmware file.
 *  @param parsedParams - the parsed command line parameters (for the base addr).
 */
static void
loadFirmware(CVMUSB& usb, const std::string& file, const gengetopt_args_info& parsedParams)
{
    uint32_t baseAddress;
    if (parsedParams.base_given) {
        baseAddress = parsedParams.base_arg;
    } else {
        baseAddress = parsedParams.slot_arg << 27;
    }
    XLM::CFirmwareLoader loader(usb, baseAddress);
    
    loader(file);
}

/**
 * main
 *   - Get a CVMUSB controller.
 *   - Instantiate a loader.
 *   - Load the module.
 *
 *  @param argc - number of command line parameters.
 *  @param argv - The command linea arguments.
 *  @return  exit status:
 *  @retval EXIT_SUCCESS - successful execution.
 *  @retval EXIT_FAILURE - failed execution.
 */
int
main(int argc, char** argv)
{
    gengetopt_args_info parsedParams;
    cmdline_parser(argc , argv, &parsedParams);
    CVMUSB* controller(nullptr);
    
    
    int exitStatus = EXIT_SUCCESS;
    try {
        controller = createController(parsedParams);
        
        // We need to have a filename.
        
        if (parsedParams.inputs_num != 1) {
            std::cerr << "You must supply exactly one firmwware file path\n";
            cmdline_parser_print_help();
            exitStatus = EXIT_FAILURE;
        } else {
            std::string firmwareFile = parsedParams.inputs[0];
            loadFirmware(*controller, firmwareFile, parsedParams);
        }
    }
    catch (CException& e) {
        std::cerr << e.ReasonText() << std::endl;
        exitStatus = EXIT_FAILURE;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        exitStatus = EXIT_FAILURE;
    }
    catch (std::string s) { 
        std::cerr << s << std::endl;
        exitStatus = EXIT_FAILURE;
    }
    delete controller;
    std::exit(exitStatus);
}


// Stubs needed to satisfy undef globals.:

// Run state stubs: Not actually used in tests.
// just satisfy globals.

CRunState* CRunState::m_pTheInstance(nullptr);

CRunState::CRunState() {}
CRunState::~CRunState() {}

CRunState* CRunState::getInstance() {
  if (!m_pTheInstance) m_pTheInstance = new CRunState;
  return m_pTheInstance;
}


uint16_t
CRunState::getRunNumber() {
  return 0;
}