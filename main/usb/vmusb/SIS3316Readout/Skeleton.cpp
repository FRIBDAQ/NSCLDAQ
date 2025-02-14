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
#include <Skeleton.h>
#include <CExperiment.h>
#include <TCLInterpreter.h>
#include <CTimedTrigger.h>
#include <CConfigurableCompoundEventSegment.h>
#include <CSISScalerBank.h>
#include <CVMUSBBusy.h>
#include <stdlib.h>
#include <iostream>
#include <CVMUSBusb.h>
/*
/*
** This file is a skeleton for the production readout software for
** NSCLDAQ 10.0 and later.  The programmatic interface
** to NSCLDAQ 10.0 at the application level is a 'close match' to that
** of earlier versions.  The software itself is a complete re-write so
** some incompatibilities may exist.  If you find an incompatibility,
** please post it at daqbugs.nscl.msu.edu so that it can be documented,
** and addressed.  Note that this does not necessarily mean that
** the incompatibility will be 'fixed'.
**
**   ------------------------------------------------------------
**
** How to use this skeleton:
**
**  This skeleton is the 'application' class for the production readout software.
**  The application class has several member functions you can override
**  and implement to perform user specific initialization.
**  These are:
**    SetupRunVariables   - Creates an initial set of run variables.
**    SetupStateVariables - Creates an initial set of state variables.
**    SetupReadout        - Sets up the software's trigger and its response to 
**                          that trigger.
**    SetupScalers        - Sets up the response to the scaler trigger and, if desired,
**                          modifies the scaler trigger from a periodic trigger controlled
**                          by the 'frequency' Tcl variable to something else.
**
** For more information about how to tailor this skeleton, see
** the comments in front of each member function.
**
*/

////////////////////////////////////////////////////////////////////////////////////////

/* These globals are neede by the VMUSB version of the SIS vme class:*/
/* the only member that matters is the pUSBController member */
/* They drag over from the VMUSBReadout */
class CConfiguration;
class CVMUSB;
class TclServer;
class CTCLInterpreter;
class CTheApplication;


namespace Globals {
  CConfiguration*    pConfig(0);
  string             configurationFilename;
  string             controlConfigFilename;
  CVMUSB*            pUSBController(0);
  bool               running(false);
  TclServer*         pTclServer(0);     
  unsigned           scalerPeriod(0);
  size_t             usbBufferSize(0);
  unsigned           sourceId = 0;
  char*              pTimestampExtractor = 0;
  Tcl_ThreadId           mainThreadId = 0;
  CTCLInterpreter*       pMainInterpreter = 0;
  CTheApplication*   pApplication(0);
};

/*
** Application frameworks require an 'entry point' object instance.  This
** is created below:
*/

CTCLApplication* gpTCLApplication = new Skeleton;

////////////////////////////////////////////////////////////////////////////////////////

/*!
  Setup the Readout This function must define the trigger as well as
  the response of the program to triggers.  A trigger is an object that
  describes when an event happens.  Triggers are objects derived from
  CEventTrigger
 
  \note  This function is incompatible with the pre 10.0 software in that
         for the 10.0 software, there was a default trigger that did useful stuff.
	 The default trigger for this version is a NULL trigger (a trigger that
	 never happens.  You _must_ create a trigger object and register it with the
	 experiment object via its EstablishTrigger member funtion else you'll never
	 get any events.

   The following are trigger classes you can use:
   - CNullTrigger - never fires. This is the default.
   - CTimedTrigger - Really intended for scaler triggers, but maybe there's an application
                     you can think of for a periodic event trigger.
   - CTestTrigger  - Always true. That's intended for my use, but you're welcome to use it
                     if you want a really high event rate.
   - CV262Trigger  - Uses the CAEN V262 as a trigger module.
   - CV977Trigger  - Uses the CAEN V977 as a trigger module.

   \param pExperiment - Pointer to the experiment object.

*/

void
Skeleton::SetupReadout(CExperiment* pExperiment)
{
  CReadoutMain::SetupReadout(pExperiment);

  // Probably have to increase the event buffer in 
  // pExperiment for the FADCs e.g.:
  //

  size_t MaxEventSize = 1024*1024;     // May need even bigger?
  pExperiment->setBufferSize(MaxEventSize);

  // Set up access to the VMUSB -- the first one we find:
  // One could also iterate over controllers and use
  // CVMUSB::serialNo to find a specific one instead.
  // E.g. (untested but probably ok):
  //    CVMUSB* pUSB(0);
  //    for(auto c: controllers) {
  //       if (CVMUSB::seralNo(c) == TheOneIWantString) {
  //           pUSB = new CVMUSBusb(c);
  //       }
  //    }
  //    if (!pUSB) .... some error..and exit.
  //    Globals::pUSBController = pUSB;
  //

  // We're only doing local VMUSB modules not ethernet served ones.
  
  auto controllers = CVMUSBusb::enumerate();
  if (controllers.size() == 0) {
    std::cerr << "There are no VMUSB controllers connected to the system\n";
    exit(EXIT_FAILURE);
  }
  Globals::pUSBController = new CVMUSBusb(controllers[0]);

  // Establish your trigger here by creating a trigger object
  // and establishing it.

  // Create and add your event segments here, by creating them and invoking CExperiment's 
  // AddEventSegment
  

  /*
  *  The code below makes a compound event segment that can create and configure 
  *  modules via a config file.  The config file name
  * (feel free to change this); is gotten from the environment variable
  *   DAQCONFIG
  *   Add more event segments as you choose.
  */
  const char* sis_configfile = getenv("DAQCONFIG");
  if (!daq_configfile) {
    std::cerr << 
       "********************************* FATAL ***************************\n"
      << "The environment variable DAQCONFIG must be set to the Tcl config file\n"
      <<"and has not\nExiting";
      exit(EXIT_FAILURE);
  }
  auto pSegment = new CConfigurabvleCompoundEventSegment(daq_configfle);
  pExperiment->AddEventSegment(pSegment);
  pExperiment->EstablishBusy(new CVMUSBBusy);
}

/*!
  Very likely you will want some scalers read out.  This is done by
  creating scalers and adding them to the CExperiment object's
  list of scalers via a call to that object's AddScalerModule.

  By default, the scalers are read periodically every few seconds.  The interval
  between scaler readouts is defined by the Tcl variable frequency.

  You may replace this default trigger by creating a CEventTrigger derived object
  and passing it to the experiment's setScalerTrigger member function.

  \param pExperiment - Pointer to the experiment object.
*/
void
Skeleton::SetupScalers(CExperiment* pExperiment) 
{
  CReadoutMain::SetupScalers(pExperiment);	// Establishes the default scaler trigger.

  // Sample: Set up a timed trigger at 2 second intervals.

  timespec t;
  t.tv_sec  = 2;
  t.tv_nsec = 0;
  CTimedTrigger* pTrigger = new CTimedTrigger(t);
  pExperiment->setScalerTrigger(pTrigger);

  // Create and add your scaler modules here.

  // Add a CSISScalerBank - which can configure multiple scalers....the env SISSCALERCONFIG
  // points to the scaler configuration file:

  const char* pConfigFile = getenv("SISSCALERCONFIG");
  if(!pConfigFile) {
    std::cerr << "***** ERROR the SISSCALERCONFIG environment variable needs to be defined and \n";
    std::cerr << "***** and must point to a scaler configuration file\n";
    exit(EXIT_FAILURE); 
  }
  auto pBank = new CSIS3820ScalerBank(pConfigFile);
  pExperiment->addScalerModule(pBank);

}
/*!
   Add new Tcl Commands here.  See the CTCLObjectProcessor class.  You can create new
   command by deriving a subclass from this abstract base class.  The base class
   will automatically register itself with the interpreter.  If you have some
   procedural commands you registered with Tcl_CreateCommand or Tcl_CreateObjCommand, 
   you can obtain the raw interpreter (Tcl_Interp*) of a CTCLInterpreter by calling
   its getInterp() member.

   \param pInterp - Pointer to the CTCLInterpreter object that encapsulates the
                    Tcl_Interp* of our main interpreter.

*/

void
Skeleton::addCommands(CTCLInterpreter* pInterp)
{
  CReadoutMain::addCommands(pInterp); // Add standard commands.
}

/*!
  Setup run variables:  A run variable is a Tcl variable whose value is periodically
  written to to the output event stream.  Run variables are intended to monitor things
  that can change in the middle of a run.  One use of a run variable is to
  monitor control system values.  A helper process can watch a set of control system
  variables, and issue set commands to the production readout program via its
  Tcl server component.  Those run variables then get logged to the event stream.

  Note that the base class may create run variables so see the comments in the function
  body about where to add code:

  See also:

     SetupStateVariables

     \param pInterp - pointer to the TCL interpreter.
*/

void
Skeleton::SetupRunVariables(CTCLInterpreter* pInterp)
{
  // Base class will create the standard commands like begin,end,pause,resume
  // runvar/statevar.

  CReadoutMain::SetupRunVariables(pInterp);

  // Add any run variable definitions below.

}

/*!
  Setup state variables: A state variable is a Tcl variable whose value is logged 
  whenever the run transitions to active.  While the run is not halted,
  state variables are write protected.  State variables are intended to 
  log a property of the run.  Examples of state variables created by the
  production readout framework are run and title which hold the run number,
  and the title.

  Note that the base class may create state variables so see the comments in the function
  body about where to add code:

  See also

  SetupRunVariables

  \param pInterp - Pointer to the tcl interpreter.
 
*/
void
Skeleton::SetupStateVariables(CTCLInterpreter* pInterp)
{
  CReadoutMain::SetupStateVariables(pInterp);

  // Add any state variable definitions below:

  
}
