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
#include "CGetCommand.h"

#include "TclServer.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include "CControlModule.h"
#include <CRunState.h>
#include <CControlQueues.h>
#include <CCCUSB.h>
#include <tcl.h>

using namespace std;

/*!
  Construct the command.
*/
CGetCommand::CGetCommand(CTCLInterpreter& interp,
			 TclServer&       server,
			 CCCUSB&          vme) :
  CTCLObjectProcessor(interp, "Get"),
  m_Server(server),
  m_Vme(vme)
{}
CGetCommand::~CGetCommand()
{}

/*
   Execute the get command.  See the class comments for syntax.
*/
int
CGetCommand::operator()(CTCLInterpreter& interp,
			vector<CTCLObject>& objv)
{
  // Need 3 words on the command line:

  if (objv.size() != 3) {
    m_Server.setResult(
	      "ERROR Get: Incorrect number of command parameters : need Get name point");
    return TCL_ERROR;
  }

  // Get the pieces of the command:


  string name  = objv[1];
  string point = objv[2];
  
  // Locate the object:

  CControlModule* pModule = m_Server.findModule(name);
  if (!pModule) {
    string msg("ERROR Get: unable to find module ");
    msg += name;
    m_Server.setResult( msg);
    return TCL_ERROR;
  }


  // If we are in the middle of a run, we need to halt data collection
  // before using the vmusb
  bool mustRelease(false);
  if (CRunState::getInstance()->getState() == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }

  string result =   pModule->Get(m_Vme, point.c_str());
  m_Server.setResult( result);

  // if we need to, put the vmusb back into acquisition mode
  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }
  return TCL_OK;

}

