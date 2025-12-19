/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CExit.cpp
# @brief  Implement a replacement for the Tcl exit command.
# @author <fox@nscl.msu.edu>
*/

#include "CExit.h"
#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>
#include <Globals.h>
#include <TclServer.h>
#include <CAcquisitionThread.h>
#include <CTheApplication.h>
/**
 * constructor
 *   Create the 'exit' command which should now override the Tcl exit
 *   command:
 *   @param interp - interpreter on which this will be registered.
 */
CExit::CExit(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "exit", true)
    {}
/**
 * destructor
 *   Just lets this chain to the parent destructor.
 */
CExit::~CExit() {}

/**
 * operator()
 *   Executes the command
 *   *  Ensure there's at most one extra argument.
 *   *  If there is an argument that must be an integer and it replaces the
 *      default exit status (0).
 *   *  Send an exit event to the Tcl Server
 *   *  Join with the tcl server.
 *   *  Invoke Tcl_Exit to exit the application.
 *
 *  @param interp - Reference to the encapsulated interpreter that is running
 *                  the command.
 *  @param objv   - Array of CTCLObjects that represent the command words.
 *  @return int   - Though actually this function does not return unless
 *                  there is an error, in which case it returns TCL_ERROR.
 */
int
CExit::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    CTheApplication* pApp = CTheApplication::getInstance();
    try {
        pApp->logStateChangeRequest("Requested exit of Readout");
        bindAll(interp, objv);
        requireAtMost(objv, 2, "exit takes at most the status parameters");
        
        // Default the exit status and override it if there is a param:
        
        int status = 0;
        if (objv.size() == 2) {
            status = objv[1];                // Throws if not integer.
        }
        
        CExit::exit(status);
        pApp->logStateChangeStatus("Exiting - successful.");
    }
    catch (std::string msg) {
        interp.setResult(msg);
        std::string log = "Some part of exiting did not succeed: ";
        log += msg;
        pApp->logStateChangeStatus(log.c_str());
    }
    
    return TCL_ERROR;
}

void CExit::exit(int status) 
{
  // Send an event to the TclServer process's interpreter asking it to exit.
  
  TclServer* pServer = ::Globals::pTclServer;
  CTheApplication* pApp = CTheApplication::getInstance();
  if (pServer) {
    pApp->logProgress("Halting Tcl Server");
    pServer->scheduleExit();
    pServer->join();
    pApp->logProgress("Tcl Server halted");
  }

  // End the run and join with the acquisition thread
  auto pReadout = CAcquisitionThread::getInstance();
  if (pReadout->isRunning()) {
    pApp->logProgress("Ending run and stopping acquisition thread");
    CControlQueues::getInstance()->EndRun();
    CAcquisitionThread::getInstance()->join();
    pApp->logProgress("Acquisition thread exited");
  }

  // Exit the program:

  pApp->logStateChangeStatus("Exiting for real now.");

  // If there's a controller, then we should enable all the
  // interrupts see Issue #404  This allows
  // us to switch off with older versions of mesytec/mdaq.

  auto pController = Globals::pUSBController;
  if (pController) {
    pController->writeIrqMask(0);
  }

  Tcl_Exit(status);
}
