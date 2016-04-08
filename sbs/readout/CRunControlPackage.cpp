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
#include "CRunControlPackage.h"
#include "RunState.h"
#include "CExperiment.h"
#include <TCLInterpreter.h>
#include <TCLObjectProcessor.h>
#include <TCLApplication.h>
#include <CReadoutMain.h>
#include <StateException.h>
#include "CBeginCommand.h"
#include "CPreBeginCommand.h"
#include "CPrePauseCommand.h"
#include "CPauseCommand.h"
#include "CResumeCommand.h"
#include "CPreEndCommand.h"
#include "CEndCommand.h"
#include "CInitCommand.h"
#include <TCLTimer.h>

using namespace std;

/*------------------------------------------------
 * Helper class: Extends TCLTimer to maintain the
 * elapsed run time in the run state.
 */
class RunTimer : public CTCLTimer
{
public:
  RunTimer(CTCLInterpreter* pInterp) :
    CTCLTimer(pInterp, 1000)
  {
    Set();
  }
  virtual void operator()() {
    RunState* pState = RunState::getInstance();
    pState->m_timeOffset++;
    Set();
  }
};

// Static member data:

CRunControlPackage*  CRunControlPackage::m_pInstance(0);

// Application global data:

extern CTCLApplication* gpTCLApplication;



/*!
  Constructs the singleton.  This is private so that nobody
  but getInstance() can invoke it.
  \param Interp Interpreter to which the commands will be bound:

*/

CRunControlPackage::CRunControlPackage(CTCLInterpreter& Interp) :
  m_pTheState(RunState::getInstance()),
  m_pTheExperiment(0),
  m_pInterp(&Interp),
  m_pTimer(0)
{

  CReadoutMain* pReadout = reinterpret_cast<CReadoutMain*>(gpTCLApplication);
  m_pTheExperiment = pReadout->getExperiment();
  createCommands(Interp);
}
/*!
   I think I May need a destructor to avoid undefined globals:

*/
CRunControlPackage::~CRunControlPackage()
{
}

/*!
    Return the singleton instance of the package,createing it if necessary:
    \param interp - interpreter on which commands are registered.

    \note while this is a true singleton, the inclusion of the interp parameter
          opens the door architecturally to a per interpreter singleton instead
          (where perhaps only the run state and experiment objects are singleton).

*/
CRunControlPackage*
CRunControlPackage::getInstance(CTCLInterpreter& interp)
{
  if (!m_pInstance) {
    m_pInstance = new CRunControlPackage(interp);
  }
  return m_pInstance;
}

/**
 *  Setup to start the run(hardware initialization).
 */
void
CRunControlPackage::preBegin()
{
  if (m_pTheState->m_state == RunState::inactive) {
    m_pTheExperiment->PreStart();
  } else {
    throw CStateException(
      m_pTheState->stateName().c_str(),
      RunState::stateName(RunState::inactive).c_str(),
      "Attempting pre-begin initialization"
    );
  }
}

/*!
   Start a new run; Run state must be halted.
*/
void
CRunControlPackage::begin()
{
  if (m_pTheState->m_state == RunState::starting) {
    m_pTheExperiment->Start(false);	// not a resume.
    m_pTimer = new RunTimer(m_pInterp);
  }
  else {
    std::string validStates = RunState::stateName(RunState::inactive);
    validStates             += " or ";
    validStates             += RunState::stateName(RunState::starting);
    throw CStateException(m_pTheState->stateName().c_str(),
			 validStates.c_str(),
			 "Attempting to start a run");
			 
  }
}
/**
 * preEnd
 *    Perform any operations requried prior to the end of the run.  Note that
 *    this is currently a stub.  At some point we may expand the CEventSegment
 *    to provide for code to be attached to this state transition.  If/When that
 *    happens, the code should just disable the digitizers from acquiring data.
 */
void
CRunControlPackage::preEnd()
{
  // Must be called in the halted Active state:
  
  RunState::State state = m_pTheState->m_state;
  if ((state != RunState::active) && state != (RunState::paused)) {
    throw CStateException(
      m_pTheState->stateName().c_str(),
      RunState::stateName(RunState::active).c_str(),
      "Attempting run pre-end actions."
    );
   
  }
   m_pTheState->m_state = RunState::halting;
  
}

/*!
   Halt a run.  The state must be activfe or paused for this to be legal.
*/
void
CRunControlPackage::end()
{
  RunState::State state = m_pTheState->m_state;
  if((state == RunState::active) || (state == RunState::paused) || (state == RunState::halting)) {
    if (state != RunState::halting) {
      preEnd();
    }
    m_pTheExperiment->Stop(false);	// Not a pause.
    delete m_pTimer;
    m_pTimer = reinterpret_cast<RunTimer*>(0);
    m_pTheState->m_state = RunState::inactive;
  }
  else {
    string validstates = RunState::stateName(RunState::active);
    validstates += ", ";
    validstates += RunState::stateName(RunState::paused);
    validstates += " or ";
    validstates += RunState::stateName(RunState::halting);
    throw CStateException(m_pTheState->stateName().c_str(),
			 validstates.c_str(),
			 "Attempting to end a run.");
  }
}
/**
 *  prePause
 *     Prepare to pause a run.
 */
void
CRunControlPackage::prePause()
{
  // Pause can only happen in an active run:
  
  if (m_pTheState->m_state != RunState::active) {
    throw CStateException(m_pTheState->stateName().c_str(),
                          "Active",
                          "Attempting to pre-pause a run");
  }
  m_pTheState->m_state = RunState::pausing;
}
/*!
    Pause a run.  The run state must be active.
*/
void
CRunControlPackage::pause()
{
  
  if(
    (m_pTheState->m_state == RunState::active) ||
    (m_pTheState->m_state == RunState::pausing) 
  ) {
    if (m_pTheState->m_state != RunState::pausing) {
      prePause();                        // Run the prepause method.
    }
    m_pTheExperiment->Stop(true);
    delete m_pTimer;
    m_pTimer = reinterpret_cast<RunTimer*>(0);
    m_pTheState->m_state = RunState::paused;
  }
  else {
    std::string validStates = m_pTheState->stateName(RunState::paused);
    validStates += " or ";
    validStates += m_pTheState->stateName(RunState::pausing);
    throw CStateException(m_pTheState->stateName().c_str(),
			 validStates.c_str(),
			 "Attempting to pause a run");
  }
}

/*!
   Resume a run --state willing.  The state must be paused.
*/
void
CRunControlPackage::resume()
{
  if (m_pTheState->m_state == RunState::paused) {
    m_pTheExperiment->Start(true);
    m_pTimer = new RunTimer(m_pInterp);
  }
  else {
    throw CStateException(m_pTheState->stateName().c_str(),
			  m_pTheState->stateName(RunState::paused).c_str(),
			  "Attempting to resume a run");
  }
}


/*!
  Return the state object. This can be done via the state singleton, or
  via the run control package in case in the future the run state somehow 
  stops being a singleton.
  \return RunState*
  \retval The saved pointer to the run state object.
*/
const RunState*
CRunControlPackage::getState() const
{
  return m_pTheState;
}

/*!
  Create all the commands in the package and add them:
*/

void CRunControlPackage::createCommands(CTCLInterpreter& interp)
{
  addCommand(new CBeginCommand(interp));
  addCommand(new CPreBeginCommand(interp));
  CPrePauseCommand* prePause = new CPrePauseCommand(interp);
  addCommand(prePause);
  addCommand(new CPauseCommand(interp, prePause));
  addCommand(new CResumeCommand(interp));
  addCommand(new CEndCommand(interp));
  addCommand(new CInitCommand(interp));
  addCommand(new CPreEndCommand(interp));
}
