/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321

@file TclWrapper.cpp
@author Ron Fox <fox at frib dot msu dot edu>
@brief Implementation for command to wrap a Tcl driver.
*/
#include "TclWrapper.h"
#include "CReadoutModule.h"
#include <XXUSBConfigurableObject.h>
#include <stdint.h>
#include <tcl.h>
#include <TCLInterpreter.h> 
#include <TCLObject.h>
#include <TCLVariable.h>
#include <Exception.h>
#include <iostream>
#include <stdlib.h>

////////////////////////////////////// Driver implemention.
/**
 *  construction
 * 
 *   Null out the configuration and save the interpreter.
 * @param interp - the interpreter that will be used to execute commands.
 */
TclWrapper::TclWrapper(CTCLInterpreter& interp) :
    m_pConfig(0),
    m_pInterp(&interp)
{}

/**
 * destructor
 *     we don't hold ownership rights over the config so:
 */
TclWrapper::~TclWrapper() {}

/**
 *  onAttach 
 * Add  a parameter, -ensemble with no validation for the ensemble 
 * command.  We only require the ensemble be defined when it's used.
 * 
 * @param configuration - references our configuration.
 */
void
TclWrapper::onAttach(XXUSB::CConfigurableObject& configuration) {
    m_pConfig = &configuration;

    m_pConfig->addParameter("-ensemble", nullptr, nullptr, "");
}

/**
 * Initialize
 *     Initialize the defice hardware.
 * @param controller - VMUSB controller, we actually know that there is a 
 * command ensemble "mvlc" which acts like a CVMUSB object.
 * 
 * If we've not been configured we will exit with a message.
 */
void
TclWrapper::Initialize(CVMUSB& controller) {

    // Build up the command list:

    std::string command = requireConfigured();     // Exits if -ensemble is "".
    CTCLObject oCommand;
    oCommand.Bind(*m_pInterp);
    oCommand = command;
    oCommand += "Initialize";
    oCommand += "mvlc";

    auto fullScript = std::string(oCommand);
    executeCommand(fullScript.c_str());                   // handles errors too.
}


/**
 *  addREazdoutList
 *     Called to generate readout operations.  
 * 
 * @param list - CVMUSBReadoutlist - we just use the command mvlclist to pass an ensmble
 * that behaves like that to the wrapped driver.
 */
void
TclWrapper::addReadoutList(CVMUSBReadoutList& list) {
    std::string command = requireConfigured();     // Exits if -ensemble is "".
    CTCLObject oCommand;
    oCommand.Bind(*m_pInterp);
    oCommand = command;
    oCommand += "addReadoutList";
    oCommand += "mvlclist";

    auto fullScript = std::string(oCommand);
    executeCommand(fullScript.c_str());
}

/**
 *  onEndRun
 *     Called to generate code to shutdown the device.
 * 
 * @param controller - Memorizing controller.  Note that we don't use this but
 * pass the 'mvlc' command ensemble to the Tcl driver's onEndRun subcommand.
 */
void
TclWrapper::onEndRun(CVMUSB& controller) {
    std::string command = requireConfigured();     // Exits if -ensemble is "".
    CTCLObject oCommand;
    oCommand.Bind(*m_pInterp);
    oCommand = command;
    oCommand += "onEndRun";
    oCommand += "mvlc";

    auto fullScript = std::string(oCommand);
    executeCommand(fullScript.c_str());
}

/**
 * executeCommand
 *   Given a fully formed driver command tries to execute it. If the command fails an error message
 * and traceback are emitted to stderrand the program exits with EXIT_FAILURE
 * 
 * @param command - the command to execute.
 */
void
TclWrapper::executeCommand(const char* command) {
    try {
        m_pInterp->GlobalEval(command);
    }
    catch (CException& e) {
        std::cerr << "Command : '" << command << "' failed: " << std::endl;
        std::cerr << e.ReasonText() << std::endl;

        CTCLVariable errorInfo(m_pInterp, "errorInfo", TCLPLUS::kfFALSE);
        const char* traceback = errorInfo.Get();
        if (traceback) {
            std::cerr << traceback << std::endl;
        }
        exit(EXIT_FAILURE);
    }
}
/**
 *  requireConfigured
 *     @return std::string - the ensemble command.
 * 
 * If the ensemble command is an empty string, an error message is emitted to stderr
 * and the program exits with EXIT_FAILURE
 */
std::string
TclWrapper::requireConfigured() {
    std::string result = m_pConfig->cget("-ensemble");
    if (result == "") {
        std::cerr << "TclWrapper instances need to have -ensemble configured with a Tcl command ensemble\n";
        exit(EXIT_FAILURE);
    }
    return result;
}

 ////////////////////////////////////// Driver command implementation
 /**
  * constructor 
  *     We just do a base class construction.
  * @param interp - interpreter our command, 'addtcldriver' is registered on.
  * @param parser - references the parser object that is going to run the configuration script.
  */
 AddTclDriver::AddTclDriver(CTCLInterpreter& interp, TCLConfigParser& parser)  :
    DeviceCommand(interp, "addtcldriver", parser)
 {

 }
 /** destructor is a noop */
 AddTclDriver::~AddTclDriver() {}

 /**
  * createDevice
  *     Create a readout module encapsulating an instance of the driver.
  * @param name - Name of the device being created.
  */
 CReadoutModule*
 AddTclDriver::createDevice(std::string name) {
    auto driver= new TclWrapper(*getInterpreter());
    auto result = new CReadoutModule;
    result->SetDriver(driver);

    return result;
 }