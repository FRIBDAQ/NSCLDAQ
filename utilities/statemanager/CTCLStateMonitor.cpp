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
# @file   CTCLStateMonitor.cpp
# @brief  Implements the Tcl state monitor api.
# @author <fox@nscl.msu.edu>
*/

#include "CTCLStateMonitor.h"
#include "TCLObjectProcessor.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include "TCLException.h"
#include <exception>
#include <string.h>

/*-----------------------------------------------------------------------------
   Implementation of CTCLStateMonitor - this should probably actually be
   called CThreadsafeStateMonitor because it's not actually Tcl specific.
*/

/**
 * constructor
 *     Simply invoke the base class constructor
 *  @param transitionRequestURI   - URI for the transition request port.
 *  @param statePublisherURI      - URI for state publication.
 *  @param cb                     - Initialization callback.
 */
CTCLStateMonitor::CTCLStateMonitor(
        std::string transitionRequestURI, std::string statePublisherURI,
        CStateMonitorBase::InitCallback cb) :
  CStateMonitor(transitionRequestURI, statePublisherURI, cb)
  {}
  
  /**
   * destructor
   *   finalize the mutex.
   */
  CTCLStateMonitor::~CTCLStateMonitor()
  {
    Tcl_MutexFinalize(m_lock);
  }
  
  /**
   * Register
   *    Register a callback.  Now this is done with the mutex held so that
   *    the the thread that runs the event loop won't trip over changes to the
   *    data structures that hold the callback.
   *
   *  @param state - State to register callback for.
   *  @param cb    - The callback.
   *  @param cbarg - Argument passed to the callback without any interpretation.
   */
  void
  CTCLStateMonitor::Register(std::string state, Callback cb, void* cbarg)
  {
    Tcl_MutexLock(m_lock);
    CStateMonitor::Register(state, cb, cbarg);
    Tcl_MutexUnlock(m_lock);
  }
  /**
   * unregister
   *    Remove a callback registration.
   *
   * @param state - The state for which the callback will be removed.
   */
  void
  CTCLStateMonitor::unregister(std::string state)
  {
    Tcl_MutexLock(m_lock);
    CStateMonitor::unregister(state);
    Tcl_MutexUnlock(m_lock);
  }
  /**
   * initialState
   *    Callback for the initial state -  protect the base method class via
   *    the mutex.
   *
   * @param state - The state that we have entered without knowing anything about
   *                the prior state.
   */
void
CTCLStateMonitor::initialState(std::string state)
{
    Tcl_MutexLock(m_lock);
    CStateMonitor::initialState(state);
    Tcl_MutexUnlock(m_lock);
}
/**
 * transition
 *     Callback for transtions with known prior states.
 *     again wrap in the mutex.
 *
 * @param newState - New state we are entering (the base class maintains memory
 *                  of the prior state in a protected member variable)
 */
void
CTCLStateMonitor::transition(std::string newState)
{
    Tcl_MutexLock(m_lock);
    CStateMonitor::transition(newState);
    Tcl_MutexUnlock(m_lock);
}

/*-----------------------------------------------------------------------------
 * Implementation of the CStateMonitorCommand class.
 */

/**
 * constructor:
 *     * Register the command
 *     * Initialize the data structures.
 *
 * @param interp - TCL intepreter on which the command will be registered.
 * @param name   - Name of the command (e.g. ::statemonitor::statemonitor).
 */
CTCLStateMonitorCommand::CTCLStateMonitorCommand(
    CTCLInterpreter& interp, std::string name
) : CTCLObjectProcessor(interp, name, true),
    m_monitorThread(0), m_myThread(Tcl_GetCurrentThread()), m_pStateMonitor(0)
{
}
/**
 * destructor-- should never be called -- since we don't have the mechanics
 *              to kill the thread.
 */
CTCLStateMonitorCommand::~CTCLStateMonitorCommand()
{
    throw std::string(
        "CTCLStateMonitorCommand -destructor was invoked and that's illegal"
    );
}
/**
 * operator()
 *    Subcommand dispatcher:
 *    * Ensure there's at least a subcommand
 *    * Extract it from the command line.
 *    * If it's valid dispatch to one of start, Register and unregister.
 *    * If it's invalid declare a script error.
 *     
 *  @param interp - the interpreter running the command.
 *  @param objv   - The encapsulated objects that make up the command words.
 *  @return int
 *  @retval TCL_OK - successful completion, result depends on the subcommand.
 *  @retval TCL_ERROR - Errot, the error description is put in the command result.
 */
int
CTCLStateMonitorCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    // Throw and catch string exceptions for errors.  The string will become
    // the result string.
    try {
        requireAtLeast(objv, 2, "The command must have a subcommand");
        bindAll(interp, objv);
        std::string subCommand = objv[1];
        
        if (subCommand == "start") {
            start(interp, objv);
        } else if (subCommand == "register") {
            Register(interp, objv);
        } else if (subCommand == "unregister") {
            unregister(interp, objv);
        } else {
            throw std::string("Invalid subcommand");
        }
        
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    return TCL_OK;                        // If we got here
}
/**
 * start
 *    Start the state monitor thread
 *    * There must be exactly 4 parameter.
 *    * Extract the two URI's from the command parameter.
 *    * Create the state monitor object.
 *    * Start the state monitor thread.
 *     
 *  @param interp - the interpreter running the command.
 *  @param objv   - The encapsulated objects that make up the command words.
  */
void
CTCLStateMonitorCommand::start(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 4, "Incorrect number of command parameters");
    std::string reqUri   = objv[2];
    std::string stateUri = objv[3];
    try {
        m_pStateMonitor = new CTCLStateMonitor(reqUri, stateUri);
        int stat            = Tcl_CreateThread(
            &m_monitorThread, eventLoopThread, this, TCL_THREAD_STACK_DEFAULT,
            TCL_THREAD_NOFLAGS
        );
        if (stat != TCL_OK) {
            throw std::string("Unable to start state monitor trhead");
        }
    } catch(std::exception& e) {
        throw std::string(e.what());              // Turn into a string except.
    }
}
/**
 * Register
 *    Register a script to be called when the specified state is entered.
 *    We're really going to specify that our callback method gets called and
 *    we'll pass sufficient information to allow that to be marshalled into a
 *    an event sent to this thread with sufficient information to run the
 *    specified script.  This rigmarole is required because of the thread model
 *    Tcl supports and the fact that the event loop poller is running in a different
 *    thread than our interpreter.
 *     
 *  @param interp - the interpreter running the command.
 *  @param objv   - The encapsulated objects that make up the command words.
 *  @return int
 *  @retval TCL_OK - successful completion, result depends on the subcommand.
 *  @retval TCL_ERROR - Errot, the error description is put in the command result.
 */
void
CTCLStateMonitorCommand::Register(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 4, "Incorrect number of command parameters");
    std::string state     = objv[2];
    state                 = toUpper(state);
    std::string script    = objv[3];
    
    // do the book keeping needed to manage the callback object storage.
    
    if(m_allocationMap.find(state) != m_allocationMap.end()) {
        delete m_allocationMap[state];
    }
    m_allocationMap[state] = new std::string(script);
    
    // register the callback with the state monitor.
    
    m_pStateMonitor->Register(state, callback, this);
}
/**
 * unregister
 *    Unregister a callback.  Note that this also means killing off the
 *    cb info for it.  Note that when we are called, it is possible there's a
 *    event queued for this exact callback.  Therefore that event ensures
 *    the callback information is valid and the same before
 *    actuall invoking the callback.
 *     
 *  @param interp - the interpreter running the command.
 *  @param objv   - The encapsulated objects that make up the command words.
 */
void
CTCLStateMonitorCommand::unregister(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3, "Incorrect number of command line parameters");
    std::string state = objv[2];
    state             = toUpper(state);
    m_pStateMonitor->unregister(state);
    
    // No more events can be queued and if one is queued it can't be
    // delivered until we get back in the event loop so this is atomic
    // relative to callback dispatch from here on in:
    
    std::map<std::string, std::string*>::iterator p = m_allocationMap.find(state);
    if (p != m_allocationMap.end()) {
        delete m_allocationMap[state];              // release string storage
        m_allocationMap.erase(p);                   // Kill off the map entry.
    }
}

/**
 *  Utility methods
 */

/**
 * scriptCaller
 *    This is the ultimate event handler for constructing and dispatching
 *    the script.  At this point the event has been cooked into the pieces
 *    we need to construct and run the final script.  The assumption is that
 *    the script will be run at global level on the interpreter in which this
 *    object was registered.
 *
 *  @param prior  - The state we are leaving (empty string if initial)
 *  @param state  - The state that was entered.
 *  @param script - The user supplied script prefix.
 *
 *  @note -errors are reported via the Tcl_BackgroundError function.
 * 
 */
void
CTCLStateMonitorCommand::scriptCaller(std::string prior, std::string state, std::string script)
{
    CTCLInterpreter* pInterp = getInterpreter();
    CTCLObject      command;
    command.Bind(*pInterp);
    
    // Remember that += builds list elements so quotig is automagic.
    
    command        += script;
    command        += prior;
    command        += state;
    
    try {
        command();
    } catch (CTCLException& e) {
        pInterp->setResult(e.ReasonText());
        Tcl_BackgroundError(pInterp->getInterpreter());
    }
    
    
}
/**
 * eventRelay
 *    This is dispatched to by the interpreter's event loop.  The state monitor's
 *    event loop is causing this to happen by queueing an event to this thread.
 *    The structure of this event is a CTCLStateMonitorCommand::event where
 *    The event itself is deallocate by the event loop but the s_pScript,
 *    s_prior and s_state elements are deallocated by us.
 *
 *  @param pEvPtr - Pointer to the CTCLStateMonitor::event struct actually.
 *  @param flags  - Tcl_ServiceEvent flags.
 *  
 */
int
CTCLStateMonitorCommand::eventRelay(Tcl_Event* pEvPtr, int flags)
{
    event* pEvent = reinterpret_cast<event*>(pEvPtr);
    std::string script(pEvent->s_pScript);
    std::string prior(pEvent->s_prior);
    std::string state(pEvent->s_state);
    
    Tcl_Free(pEvent->s_pScript);
    Tcl_Free(pEvent->s_prior);
    Tcl_Free(pEvent->s_state);
    
    // see comments in unregister for why this if block is there.
    
    if (pEvent->s_pCommand->canDispatch(state, script)) {
    
        pEvent->s_pCommand->scriptCaller(prior, state, script);
    }
    
    return 1;                          // Release pEvptr
}
/**
 * callback
 *    This method is invoked as a callback from the state monitor
 *    in the state monitor's thread.  It must create and marshall a
 *    CTCLStateMonitorCommand::event structure and queue it as an event
 *    for the CTCLStateMonitorCommand's interpreter's thread.
 *
 * @param pMonitor - Pointer to the state monitor.
 * @param prior    - name of the prior state (empty if called from initialState)
 * @param state    - Current new state.
 * @param cbParam  - Actually a pointer to the CTCLStateMonitorCommand
 *
 * @note  We are called with the CTCLStateMonitor's mutex held by our thread.
 *        That, and the order of operations in unregister ensure that the
 *        command object's data will be stable.
 * @note  Since we are a member (though static) of CTCLStateMonitorCommand
 *        if we have a pointer to one of those objects we can access it's
 *        private data.
 */
void
CTCLStateMonitorCommand::callback(
    CStateMonitor* pMonitor, std::string prior, std::string state, void* cbParam
)
{
  prior = toUpper(prior);
  state = toUpper(state);

    CTCLStateMonitorCommand* pCommand =
        reinterpret_cast<CTCLStateMonitorCommand*>(cbParam);
    
    // Create the event struct:
    
    event* pEvent  = reinterpret_cast<event*>(Tcl_Alloc(sizeof(event)));
    pEvent->s_event.proc = eventRelay;
    pEvent->s_event.nextPtr = NULL;
    pEvent->s_pCommand = pCommand;
    
    pEvent->s_pScript =  Tcl_Alloc(pCommand->m_allocationMap[state]->size() + 1);
    strcpy(pEvent->s_pScript, pCommand->m_allocationMap[state]->c_str());
    
    pEvent->s_prior   =  Tcl_Alloc(prior.size() + 1);
    strcpy(pEvent->s_prior, prior.c_str());
    
    pEvent->s_state   = Tcl_Alloc(state.size() + 1);
    strcpy(pEvent->s_state, state.c_str());
    
    // Now queue the event:
    
    Tcl_ThreadQueueEvent(
        pCommand->m_myThread, reinterpret_cast<Tcl_Event*>(pEvent),
        TCL_QUEUE_TAIL
    );
    Tcl_ThreadAlert(pCommand->m_myThread);
}
/**
 * eventLoopThread
 *   The entry point for the event loop thread.
 *   *  Establish object context.
 *   *  Locate the StateMonitor
 *   *  Invoke the run method.
 *
 * @param cd   - Actually a pointer to the command object associated with this
 *               monitor.
 */
void
CTCLStateMonitorCommand::eventLoopThread(ClientData cb)
{
    CTCLStateMonitorCommand* pCommand =
        reinterpret_cast<CTCLStateMonitorCommand*>(cb);
    CTCLStateMonitor* pMonitor = pCommand->m_pStateMonitor;
    pMonitor->run();
}
/**
 * canDispatch
 *   Determines if the dispatch entry for a state has changed or been deleted.
 *
 * @param state  - State to check
 * @param script - The script we think is for that state.
 *
 * @return boolean
 * @retval true The dispatch info is consistent with the inputs.
 * @retval false The dispatch info is inconsistent with the inputs.
 */
bool 
CTCLStateMonitorCommand::canDispatch(std::string state, std::string script)
{
  // Is there a dispatch entry for the state:

  if (m_allocationMap.find(state) != m_allocationMap.end()) {
    return *m_allocationMap[state] == script;
  }
  return false;
}
/**
 * toUpper
 *     return the uppercased version of an std::string.  This uses
 *     std::transform as per the recipe posted at
 *     http://stackoverflow.com/questions/735204/convert-a-string-in-c-to-upper-case
 *     by Pierre and Johanes Schaub.  The idea is that tranform applies a function
 *     to each item in a range of a container defined by begin/end iterators.
 *     The transformation is actually in place but since the parameter is
 *     passed by value that does not matter.
 *
 *  @param in - The input string.
 *  @return std::string - the transformed string.
 */
std::string
CTCLStateMonitorCommand::toUpper(std::string in)
{
    std::transform(in.begin(), in.end(), in.begin(), ::toupper);
    return in;                 // Which has been transformed above.
}

/*-----------------------------------------------------------------------------
* Package initialization
*/

/**
 * statemanager_Init
 *
 * C binding function that allows the shared library we live in to be
 * a Tcl package.
 *
 * @param interp - Tcl interpreter (Unwrapped).
 * @return int   - The usual Tcl status.
 */
extern "C" {
    int Statemonitor_Init(Tcl_Interp* pInterp)
    {
        // Register the statemanager namespace.
        
        Tcl_CreateNamespace(pInterp, "::statemanager", NULL, NULL);
        
        // Provide the statemanager package
        
        Tcl_PkgProvide(pInterp, "statemanager", "1.0");
        
        // register all of the commands the state manager package has.
        
        CTCLInterpreter* p = new CTCLInterpreter(pInterp);
        new CTCLStateMonitorCommand(*p, "::statemanager::statemonitor");
        
        return TCL_OK;
    }
}


/*----------------------------------------------------------------------------*/
// Bowing to the needs of the Tcl++ Application framework.

void *gpTCLApplication(0);
