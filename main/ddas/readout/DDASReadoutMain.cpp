/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
	     Aaron Chester
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** 
 * @file DDASReadoutMain.cpp
 * @brief Implementation of the production DDAS readout code.
 */

using namespace std;

#include "DDASReadoutMain.h"

#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <vector>

#include <config.h>
#include <CExperiment.h>
#include <CRunControlPackage.h>
#include <CTimedTrigger.h>
#include <TCLInterpreter.h>

#include "CBootCommand.h"
#include "CDDASStatisticsCommand.h"
#include "CMyBusy.h"
#include "CMyEndCommand.h"
#include "CMyEventSegment.h"
#include "CMyScaler.h"
#include "CMyTrigger.h"
#include "CSyncCommand.h"

// These are nullptr, nullptr... nothing really happens until we setup.
CMyTrigger *mytrigger(0); //!< Newing them here makes order of construction.
/** Un-controlled - now new'd in SetupReadout. */
CMyEventSegment *myeventsegment(0); 
std::vector<CMyScaler*> scalerModules; //!< List of scalar modules.

// Application frameworks require an 'entry point' object instance. 
// This is created below:

CTCLApplication* gpTCLApplication = new DDASReadoutMain;

/**
 * @details 
 * This function must define the trigger as well as the response of the 
 * program to triggers. A trigger is an object that describes when an event 
 * happens. Triggers are objects derived from CEventTrigger. In this case we 
 * use the CMyTrigger class to define the trigger object.
 *
 * @note This function is incompatible with the pre-10.0 software in that for 
 * the 10.0 software, there was a default trigger that did useful stuff. 
 * The default trigger for this version is a NULL trigger (a trigger that 
 * never happens). You _must_ create a trigger object and register it with the 
 * experiment object via its EstablishTrigger member funtion else you'll never 
 * get any events.
 */
void
DDASReadoutMain::SetupReadout(CExperiment* pExperiment)
{
    CReadoutMain::SetupReadout(pExperiment); 
   
    // The user can define an environment variable EVENT_BUFFER_SIZE that
    // can override the default event buffer size. If that env is defined:
    // - Convert to unsigned,
    // - Complain if not integer and exit,
    // - Warn if decreasing from the default,
    // - Set the new size with pExperiment->setBufferSize().
  
    size_t bufferSize = 16934;
    const char* pNewBufferSizeStr = getenv("EVENT_BUFFER_SIZE");
    if (pNewBufferSizeStr) { // new string defined
	std::cout << "Overriding the default event buffer size\n";
	char* end;
	size_t newSize = strtoul(pNewBufferSizeStr, &end, 0);
	if (newSize == 0) {
	    std::cerr << "**ERROR** EVENT_BUFFER_SIZE environment variable"
		" must be an integer > 0\n";
	    exit(EXIT_FAILURE);
	}
	bufferSize = newSize;
    }
    std::cout << "The new event buffer size will be: "
	      << bufferSize << std::endl;
    pExperiment->setBufferSize(bufferSize);
  
    // See: https://git.nscl.msu.edu/daqdev/NSCLDAQ/issues/1005:
  
    mytrigger = new CMyTrigger();
    myeventsegment = new CMyEventSegment(mytrigger, *pExperiment);  
  
    // Establish your trigger here by creating a trigger object
    // and establishing it.

    pExperiment->EstablishTrigger(mytrigger);
    pExperiment->EstablishBusy(new CMyBusy);
    
    // Create and add your event segments here, by creating them and invoking
    // CExperiment's AddEventSegment:
    
    pExperiment->AddEventSegment(myeventsegment);

    // We have to register our commands here because they depend on our event
    // segment and SetupReadout is called _after_ addCommands.

    CTCLInterpreter* pInterp = gpTCLApplication->getInterpreter();
    CRunControlPackage* pRctl = CRunControlPackage::getInstance(*pInterp);
    CMyEndCommand* pMyEnd= new CMyEndCommand(
	*pInterp, myeventsegment, pExperiment
	);
    pRctl->addCommand(pMyEnd);
  
    // Add the ddas_sync and ddas_boot commands:
  
    CSyncCommand* pSyncCommand = new CSyncCommand(*pInterp, myeventsegment);
    CBootCommand* pBootCommand = new CBootCommand(
	*pInterp, "ddas_boot", myeventsegment
	);
}

/**
 * @details
 * We simply use a timed trigger to read out scaler data at regular intervals. 
 * By default the scaler read interval is 16 seconds. This can be overridden 
 * using the environment variable `SCALER_SECONDS` or by specifying a value 
 * using the `-scalerseconds` option when invoking this program with 
 * `ddasReadout`.
 */
void
DDASReadoutMain::SetupScalers(CExperiment* pExperiment) 
{
    // Establishes the default scaler trigger:
    
    CReadoutMain::SetupScalers(pExperiment);
    
    // Sample: Set up a timed trigger at 16 second intervals. Complete DDAS
    // scalers are only understandable every 16 seconds. Polling for scalers
    // every two seconds will only retrieve updated values for ***.
    
    timespec t;
    t.tv_sec  = 16;
    t.tv_nsec = 0;

    const char* scalerenv = getenv("SCALER_SECONDS");
    if (scalerenv) {
	int seconds = atoi(scalerenv);
	if (seconds > 0) {
	    t.tv_sec = seconds;
	}
    }
  
    CTimedTrigger* pTrigger = new CTimedTrigger(t);
    pExperiment->setScalerTrigger(pTrigger);
  
    // Create and add your scaler modules here:
    
    int modules;
    int crateid;
    modules = myeventsegment->getNumberOfModules();
    crateid = myeventsegment->getCrateID();

    cout << "Setup scalers for " << modules << " modules " << endl;

    if (modules > MAX_MODULES_PER_CRATE) {
	cerr << "**ERROR** Attempting to setup scalers for " << modules
	     << " when a max of " << MAX_MODULES_PER_CRATE
	     << " are allowed!" << endl;
    }

    for (int i = 0; i < modules; i++) {
	auto nchan = myeventsegment->getChannelCount(i);
	CMyScaler* pModule = new CMyScaler(crateid, i, nchan);
	scalerModules.push_back(pModule);
	pExperiment->AddScalerModule(pModule);	
    }
}

/**
 * @details
 * Register the statistics command in addition to all the usual stuff from 
 * the base class.
 */
void
DDASReadoutMain::addCommands(CTCLInterpreter* pInterp)
{
    CReadoutMain::addCommands(pInterp); // Add standard commands.
    new CDDASStatisticsCommand(
	*pInterp, "statistics", myeventsegment, scalerModules
	);
}

/**
 * @details
 * A run variable is a Tcl variable whose value is periodically written to 
 * the output event stream. Run variables are intended to monitor things 
 * that can change in the middle of a run.
 *
 * @note The base class may create run variables so see the comments in 
 * the function body about where to add code.
 *
 * See also: SetupStateVariables
 */
void
DDASReadoutMain::SetupRunVariables(CTCLInterpreter* pInterp)
{
    CReadoutMain::SetupRunVariables(pInterp); // Add standard variables.

    // Add any run variable definitions below:
    
}

/**
 * @details
 * A state variable is a Tcl variable whose value is logged whenever the 
 * run transitions to active. While the run is not halted, state variables 
 * are write protected. State variables are intended to log a property of 
 * the run. Examples of state variables created by the production readout 
 * framework are run and title which hold the run number, and the title.
 *
 * @note The base class may create state variables so see the comments in 
 * the function body about where to add code.
 *
 * See also: SetupRunVariables
 */
void
DDASReadoutMain::SetupStateVariables(CTCLInterpreter* pInterp)
{
    CReadoutMain::SetupStateVariables(pInterp); // Add standard variables.

    // Add any state variable definitions below:

}
