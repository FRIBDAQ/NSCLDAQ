/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <config.h>
#include "CBeginRun.h"
#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>
#include <TCLVariable.h>
#include <Exception.h>
#include <CMutex.h>

#include <Globals.h>
#include "tclUtil.h"
#include <CAcquisitionThread.h>
#include <CRunState.h>
#include <CConfiguration.h>
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>
#include <CReadoutModule.h>
#include <CPreBeginCommand.h>
#include <stdexcept>

#include <thread>
#include <chrono>

static const size_t MAX_STACK_STORAGE(1024);

using std::vector;
using std::string;

static const string usage("Usage:\n\t begin");

/////////////////////////////////////////////////////////////////////////
//////////////////////////////// Canonicals /////////////////////////////
/////////////////////////////////////////////////////////////////////////

/*!
<<<<<<< HEAD
  Construct the begin command 
  \param interp : CTCLInterpreter&
     Interpreter on which this command will be registered.
  @param preBegin - pointer to a pre begin command object.
*/
CBeginRun::CBeginRun(CTCLInterpreter& interp, CPreBeginCommand* preBegin) :
  CTCLObjectProcessor(interp, "begin"),
  m_pPreBegin(preBegin)
=======
          Construct the begin command
          \param interp : CTCLInterpreter&
             Interpreter on which this command will be registered.
        */
CBeginRun::CBeginRun(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "begin")
>>>>>>> master
{}
/*!
           Destructor does nothing important.
        */
CBeginRun::~CBeginRun()
{}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////// Command execution /////////////////////////////
/////////////////////////////////////////////////////////////////////////////


void CBeginRun::reconnect()
{
    std::cout << "Begin reconnect acquiring" << std::flush << std::endl;
    CriticalSection lock(CCCUSB::getGlobalMutex());
    std::cout << "Begin reconnect acquired" << std::flush << std::endl;
    Globals::pUSBController->reconnect();
    std::cout << "Begin done " << std::flush << std::endl;
}

/*!
           Process the begin run.
           - Ensure that the preconditions for starting the run are met these are:
             - The begin command has no additional command line parameters.
             - The current run state is Idle
           - Create the Adc/Scaler configuration.
             - Configure it from the file stored in Globals::configurationFilename.
             - Store that configuration in Globals::configuration
           - Start up the readout thread to take data.

           \param interp : CTCLInterpreter&
                Interpreter that is exeuting this command.
           \param objv : std::vector<CTCLObject>&
                Reference to an object vector that contains the command parameters.
        */

int
CBeginRun::operator()(CTCLInterpreter& interp,
                      vector<CTCLObject>& objv)
{
<<<<<<< HEAD
  // Make sured all precoditions are met.

  if (objv.size() != 1) {
    tclUtil::Usage(interp, 
		   "Incorrect number of command parameters",
		   objv,
		   usage);
    return TCL_ERROR;
  }


  CRunState* pState = CRunState::getInstance();
  CRunState::RunState state = pState->getState();
  
  if ((state != CRunState::Idle)  && (state != CRunState::Starting)){
    tclUtil::Usage(interp,
		   "Invalid run state for begin be sure to stop the run",
		   objv,
		   usage);
    return TCL_ERROR;
  }
  try {
    // If pre-begin not yet performed do it for compatibility with old
    // controls.
    
    if (state == CRunState::Idle) {
      m_pPreBegin->performPreBeginInitialization();
    }
    
  }
  catch (std::exception& e) {
    interp.setResult(e.what());
    return TCL_ERROR;
  }
  catch (std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
  catch (...) {
    interp.setResult("begin command - unanticipated exception thrown");
    return TCL_ERROR;
  }
  // Set the state to match the appropriate set of variables:
  //
  CTCLVariable run(&interp, "run", false);
  const char* runNumberString = run.Get(TCL_GLOBAL_ONLY);
  if (!runNumberString) {
    runNumberString = "0";	// If no run variable, default run number-> 0.
  }
  uint16_t runNumber;
  sscanf(runNumberString, "%hu", &runNumber);
  pState->setRunNumber(runNumber);

  CTCLVariable title(&interp, "title", false);
  const char *titleString = title.Get(TCL_GLOBAL_ONLY);
  if (!titleString) {
    titleString = "No Title Set"; // If no title variable default it.
  }
  pState->setTitle(string(titleString));
  

  CAcquisitionThread* pReadout = CAcquisitionThread::getInstance();
  pReadout->start(Globals::pUSBController, Globals::pController);

  interp.setResult("Begin - Run started");

  return TCL_OK;
=======
    // Make sured all precoditions are met.

    if (objv.size() != 1) {
        tclUtil::Usage(interp,
                       "Incorrect number of command parameters",
                       objv,
                       usage);
        return TCL_ERROR;
    }

    CRunState* pState = CRunState::getInstance();
    if (pState->getState() != CRunState::Idle) {
        tclUtil::Usage(interp,
                       "Invalid run state for begin be sure to stop the run",
                       objv,
                       usage);
        return TCL_ERROR;
    }
    // Set the state to match the appropriate set of variables:
    //
    CTCLVariable run(&interp, "run", false);
    const char* runNumberString = run.Get(TCL_GLOBAL_ONLY);
    if (!runNumberString) {
        runNumberString = "0";	// If no run variable, default run number-> 0.
    }
    uint16_t runNumber;
    sscanf(runNumberString, "%hu", &runNumber);
    pState->setRunNumber(runNumber);

    CTCLVariable title(&interp, "title", false);
    const char *titleString = title.Get(TCL_GLOBAL_ONLY);
    if (!titleString) {
        titleString = "No Title Set"; // If no title variable default it.
    }
    pState->setTitle(string(titleString));

    // Re-establish connection to the controller:

    reconnect();

    // Check that the configuration file processes correctly:

    CConfiguration* pConfig = new CConfiguration;
    Globals::pConfig = pConfig;
    string errorMessage = "Begin - configuration file processing failed: ";
    try {
        pConfig->processConfiguration(Globals::configurationFilename);
    }
    catch (string msg) {
        errorMessage += msg;
        tclUtil::setResult(interp, errorMessage);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        errorMessage += msg;
        tclUtil::setResult(interp, errorMessage);
        return TCL_ERROR;
    }
    catch (CException& e) {
        errorMessage += e.ReasonText();
        tclUtil::setResult(interp, errorMessage);
        return TCL_ERROR;
    }
    catch (...) {
        // Configuration file processing error of some sort...

        tclUtil::setResult(interp, errorMessage);
        return TCL_ERROR;

    }
    // Figure out how big the stacks are...the two stack + any headers must
    // fit into the 1Kx16 stack memory the CUSB has:

    std::vector<CReadoutModule*> stackModules = pConfig->getStacks();
    size_t totalSize = 0;
    CCCUSBReadoutList stacks;

    // Just build one gimungous stack from one or both stacks:

    for (int i = 0; i < stackModules.size(); i++) {
        stackModules[i]->addReadoutList(stacks);
    }
    if (stacks.size() > MAX_STACK_STORAGE) {
        tclUtil::setResult(interp, "**** Your configuration file exceeds the maximum stack storage space ****");
        return TCL_ERROR;
    }

    // It did so we can kill it off and start the run.
    // we kill it off because supporting Tcl drivers requires the configuration be processed a bit specially
    // so that the interpreter is still around and the Tcl thread model isn't violated.
    //
    // delete pConfig;
    Globals::pConfig = 0;

    CAcquisitionThread* pReadout = CAcquisitionThread::getInstance();
    pReadout->start(Globals::pUSBController);

    tclUtil::setResult(interp, string("Begin - Run started"));
    return TCL_OK;
>>>>>>> master
}

