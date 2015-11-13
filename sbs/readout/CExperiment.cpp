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


#include "CExperiment.h"
#include <CRingBuffer.h>
#include <DataFormat.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingItem.h>
#include <CRingTextItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CDataFormatItem.h>

#include <RunState.h>
#include <StateException.h>
#include <string.h>
#include <CCompoundEventSegment.h>
#include <CScalerBank.h>
#include <CTriggerLoop.h>
#include <CDocumentedPacketManager.h>
#include <CVariableBuffers.h>

#include <TCLApplication.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CBusy.h>
#include <vector>
#include <string>
#include <fragment.h>
#include <os.h>
#include <CCondition.h>
#include <CMutex.h>


using namespace std;


//
// This struct is used to the event used to schedule an end run with the interpreter.
//
typedef struct _EndRunEvent {
  Tcl_Event     tclEvent;
  bool         pause;
  CExperiment* pExperiment;	// Pointer to the experiment object.
  CMutex*      pLock;            // Lock for condition variable.
  CConditionVariable* pCondVar;      // Condition variable.

} EndRunEvent, *pEndRunEvent;

typedef struct _TriggerFailEvent {
    Tcl_Event    tcl_Event;
    char*        message;
} TriggerFailEvent, *pTriggerFailEvent;

extern CTCLApplication* gpTCLApplication; // We need this to get the tcl interp.

///////////////////////////////////////////////////////////////////////////////////////////

/*!
   Construction. 
   - Initialize all the pointers and things to zero.
   - If necessary create/format the ring.
   - Create the ring buffer object in which data will be placed.
   - Save the initial event buffer size.
*/
CExperiment::CExperiment(string ringName,
			 size_t eventBufferSize) :
  m_pRing(0),
  m_pRunState(0),
  m_pScalers(0),
  m_pReadout(0),
  m_pBusy(0),
  m_pEventTrigger(0),
  m_pScalerTrigger(0),
  m_pTriggerLoop(0),
  m_nDataBufferSize(eventBufferSize),
  m_nDefaultSourceId(0)
{
  m_pRing = CRingBuffer::createAndProduce(ringName);
  m_pRunState = RunState::getInstance();

}

/*!
   Destruction goes back around deleting all this stuff.  In C++, a delete ona null
   pointer is a no-op and therefore not a problem.
*/
CExperiment::~CExperiment()
{
  delete m_pRing;
  delete m_pScalers;
  delete m_pReadout;
#if 0				// These types/classes are not yet defined :-(
  delete m_pBusy;
  delete m_pEventTrigger;
  delete m_pScalerTrigger;
#endif

}
/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Set the default data source id.
 *
 * @param sourceId - new default.
 */
void
CExperiment::setDefaultSourceId(unsigned sourceId)
{
    m_nDefaultSourceId = sourceId;
    m_nSourceId        = sourceId;
}
/*!
  Sets a new size for the event buffer.  This must be at least big enough to hold a
  single event.

*/
void
CExperiment::setBufferSize(size_t newSize)
{
  m_nDataBufferSize = newSize;
}
/*!
   Returns the size of the data buffer.
*/
size_t
CExperiment::getBufferSize() const
{
  return m_nDataBufferSize;
}

///////////////////////////////////////////////////////////////////////////////////////

/*!
  Start a new run. 
  - This is illegal if a run is active.
  - Writes a begin to the ring.
  - This creates and starts a new trigger thread, which can dispatch triggers to us.

  \param resume - true if this is actually a resume run.
*/
void
CExperiment::Start(bool resume)
{

  // The run must be in the correct state:

  if (resume && (m_pRunState->m_state != RunState::paused)) {
    throw CStateException(m_pRunState->stateName().c_str(),
			  RunState::stateName(RunState::paused).c_str(),
			  "Starting data taking");
  }
  if (!resume && (m_pRunState->m_state != RunState::inactive)) {
    throw CStateException(m_pRunState->stateName().c_str(),
			  RunState::stateName(RunState::inactive).c_str(),
			  "Starting data taking");
  }




  // Can only start it if the triggers have been established:
  
  if (m_pEventTrigger && m_pScalerTrigger) {

 
    //create the trigger loop object

    if (!m_pTriggerLoop) {
      m_pTriggerLoop = new CTriggerLoop(*this);
    }
    // Initialize/clear the hardware:
    
    if (m_pReadout) {
      m_pReadout->initialize();
      m_pReadout->clear();
    }
    if (m_pScalers) {
      m_pScalers->initialize();
      m_pScalers->clear();
    }

    // If not resuming we'll also output a ring format item so that the
    // user knows the structure of the ring:
    //
    
    CDataFormatItem format;
    format.commitToRing(*m_pRing);
    
    // Begin run zeroes the previous scaler time, and ring buffer item sequence.
    // 
    //
    time_t stamp = time(&stamp);	// Absolute timestamp for this 'event'.
    uint64_t msTime = getTimeMs();	// Current time in ms.
  
    if (m_pRunState->m_state != RunState::paused) {
      m_nRunStartStamp  = msTime;
      m_nEventsEmitted  = 0;
      m_nLastScalerTime = msTime;
      m_nPausedmSeconds  = 0;
      
    }
    if (resume) {
      m_nPausedmSeconds += (msTime - m_nLastScalerTime);
      m_nLastScalerTime = msTime;
    }

    uint32_t elapsedTime = (msTime - m_nRunStartStamp - m_nPausedmSeconds)/1000;

    CRingStateChangeItem item(NULL_TIMESTAMP, m_nSourceId, BARRIER_START,
        resume ? RESUME_RUN : BEGIN_RUN,  m_pRunState->m_runNumber,
        elapsedTime, stamp,
        std::string(m_pRunState->m_pTitle).substr(0, TITLE_MAXSIZE));
    item.commitToRing(*m_pRing);

    // Now invoke either onBegin or onResume in the event segment.

    if (m_pReadout) {
      if (resume) {
        m_pReadout->onResume();
      } else {
        m_pReadout->onBegin();
      }
    }
    
    DocumentPackets();		// output a documentation packet.
    
    // emit a physics event count item that indicates no events have been sent yet
    // if this is a start run:
    
    if (!resume) {
    CRingPhysicsEventCountItem count;
    count.commitToRing(*m_pRing);
    }
    
    
    m_pTriggerLoop->start();
    if (m_pBusy) {
      m_pBusy->GoClear();
    }
    
    // The run is now active if looked at by the outside world:
    
    m_pRunState->m_state = RunState::active;
    
  }


}
/*!
  End an existing run.  
  - This is illegal if the run is halted already.
  - This marks the event trigger thread for exit and 
    joins with it to ensure that it does actually exit.
  - Writes an end run record to the ring.

  \param pause - true if this is actually a pause run.
*/
void
CExperiment::Stop(bool pause)
{
  if (pause && (m_pRunState->m_state != RunState::active)) {
    throw CStateException(m_pRunState->stateName().c_str(),
			  RunState::stateName(RunState::active).c_str(),
			  "Stopping data taking");
  }
  if (!pause && (m_pRunState->m_state == RunState::inactive)) {
    string validStates;
    validStates  = RunState::stateName(RunState::active);
    validStates += ", ";
    validStates += RunState::stateName(RunState::paused);

    throw CStateException(m_pRunState->stateName().c_str(),
			  validStates.c_str(), 
			  "Stopping data taking");
  }
  // Schedule the trigger thread to stop.

  if (m_pTriggerLoop) {
    if (m_pRunState->m_state == RunState::active) {
      m_pTriggerLoop->stop(pause); // run is active
    }
    else {
      syncEndRun(false);
      
    }

  }
  // Everything else is scheduled by Tcl events from the trigger loop.

}
/*
** Called (indirectly) by the Tcl interpreter event dispatcher
** as a result of the trigger loops scheduling the end of run action.
*/
void
CExperiment::syncEndRun(bool pause)
{

  readScalers();		// last scaler read.

  // Disable the hardware:

  if (m_pScalers) {
    m_pScalers->disable();
  }
  if (m_pReadout) {
    m_pReadout->disable();
    if (pause) {
      m_pReadout->onPause();
    } else {
      m_pReadout->onEnd();
    }
  }

    
  // 

  // Create the run state item and commit it to the ring.

  time_t now;
  time(&now);
  uint64_t msTime = getTimeMs();
  uint32_t endOffset = (msTime - m_nRunStartStamp - m_nPausedmSeconds)/1000;

  uint16_t        itemType;
  RunState::State finalState;
  if (pause) {

    itemType   = PAUSE_RUN;
    finalState = RunState::paused;
  }
  else {
    itemType   = END_RUN;
    finalState = RunState::inactive;
  }

  CRingStateChangeItem item(NULL_TIMESTAMP, m_nSourceId, BARRIER_END,
                            itemType, 
			    m_pRunState->m_runNumber,
			    endOffset,
			    now,
			    std::string(m_pRunState->m_pTitle).substr(0, TITLE_MAXSIZE));
  item.commitToRing(*m_pRing);


  // The run is in the appropriate inactive state if looked at by the outside world

  m_pRunState->m_state = finalState;
}

//////////////////////////////////////////////////////////////////////////////////////

/*!
   Append an event segment to the event readout list
   - If the root event segment does not yet exist, create it.
   \param pSegment - Pointer to the new segment to install.  Note this could in turn
                     be a compound event segment.
*/
void
CExperiment::AddEventSegment(CEventSegment* pSegment)
{
  if (! m_pReadout) {
    m_pReadout = new CCompoundEventSegment;
  }
  m_pReadout->AddEventSegment(pSegment);
}
/*!
  Remove an existing event segment from the readout.  Note that if the event 
  segment is actually a compound, its members are also removed.
  The removed event segment is not deleted. Storage management is up to the
  higher levels of the world.
  \param pSegment - Pointer to the segment to remove.  It is not an error to remove
                   a segment that is not in the readout.. that's just a no-op.
*/
void
CExperiment::RemoveEventSegment(CEventSegment* pSegment)
{
  // If the root event segment does not exist, there's nothing to delete:

  if (m_pReadout) {
    m_pReadout->DeleteEventSegment(pSegment);
  }
}
/*!
   Add a scaler module to the end of the root scaler bank.  
   - If the root bank does not yet exist it will be created.
   - the module added could itself be a scaler bank.
   
   \param pScalerModule - pointer to the module to add.
*/
void
CExperiment::AddScalerModule(CScaler* pScalerModule)
{
  // if the root scaler module has not yet been created create it:

  if (!m_pScalers) {
    m_pScalers = new CScalerBank;
  }
  // Add the new module:

  m_pScalers->AddScalerModule(pScalerModule);
}
/*!
   Remove a scaler module from the scaler hierarchy.  
   - If the module is not in the hierarhcy this is a no-op.
   - If the module is a composite, all of the elements it contains
     will also be removed.
   - The removed module (and any children) won't be deleted.  Storage
     management is the job of higher level code.
*/
void
CExperiment::RemoveScalerModule(CScaler* pScalerModule)
{
  // remove the module from the root level scaler bank, if it exists.

  if (m_pScalers) {
    m_pScalers->DeleteScaler(pScalerModule);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////

/*!
   Establishes the trigger module. The trigger module is consulted by the
   trigger thread to know when to invoke ReadEvent.
   \param pEventTrigger - pointer to the event trigger.
*/
void
CExperiment::EstablishTrigger(CEventTrigger* pEventTrigger) 
{
  m_pEventTrigger = pEventTrigger;
}

/*!
  Establishes the scaler trigger module.  This is periodically consulted to 
  know when it's time for the trigger thread to invoke TriggerScalerReadout
  \param pScalerTrigger - pointer to the scaler trigger object.
*/
void
CExperiment::setScalerTrigger(CEventTrigger* pScalerTrigger)
{
  m_pScalerTrigger = pScalerTrigger;
}

/*!

  Establishes the busy module. The busy module represents hardware that 
  describes to the outside world when the system is unable to react to any triggers.

  \param pBusyModule

*/
void
CExperiment::EstablishBusy(CBusy* pBusyModule)
{
  m_pBusy = pBusyModule;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/*!
   Reads an event. If the root event segment exists it is asked to read its
   data into the a physics ring item.
   The resulting event is placed in the ring buffer.

*/
void
CExperiment::ReadEvent()
{

  

  // If the root event segment exists, read it into the data buffer
  // and put the resulting event in the ring buffer:
  //
  if (m_pReadout) {
    m_pReadout->keep();
    m_needHeader = false;
    m_nEventTimestamp = 0;
    m_nSourceId  = m_nDefaultSourceId;
    
    CRingItem item(PHYSICS_EVENT, m_nDataBufferSize);
    uint16_t* pBuffer = reinterpret_cast<uint16_t*>(item.getBodyPointer());
    size_t nWords = m_pReadout->read(pBuffer +2 , m_nDataBufferSize);
    if (m_pReadout->getAcceptState() == CEventSegment::Keep) {
      *(reinterpret_cast<uint32_t*>(pBuffer)) = nWords +2;
      item.setBodyCursor(pBuffer + nWords+2);
      item.updateSize();
      if (m_needHeader) {
        item.setBodyHeader(m_nEventTimestamp, m_nSourceId, 0);
      }
      item.commitToRing(*m_pRing);
      m_nEventsEmitted++;
    }
    m_pReadout->clear();	// do any post event clears.

  }
  if (m_pBusy) {
    m_pBusy->GoClear();
  }
  // TODO: Need to keep track of trigger counts and from time to time emit a 
  //      trigger count item.
}

/*
 * Read the scalers.
 */
void
CExperiment::readScalers()
{
  time_t           now     = time(&now);
  uint64_t         msTime  = getTimeMs();
  uint32_t         startTime = 
    (m_nLastScalerTime  - m_nRunStartStamp - m_nPausedmSeconds);
  uint32_t         endTime   =
    (msTime - m_nRunStartStamp - m_nPausedmSeconds);

  m_nLastScalerTime = msTime;

  // can only do scaler readout if we have a root scaler bank:

  if (m_pScalers) {
    
    vector<uint32_t> scalers = m_pScalers->read();
    uint64_t timestamp = m_pScalers->timestamp();
    int      srcid     = m_pScalers->sourceId();
    if (srcid == -1) srcid = m_nSourceId;
    
    m_pScalers->clear();	// Clear scalers after read.

    CRingScalerItem  item(timestamp, srcid, BARRIER_NOTBARRIER,
                          startTime,
			  endTime,
			  now,
			  scalers,
                          1000);  // ms time.
    item.commitToRing(*m_pRing);
			  
  }
  // Regardless, let's emit a physics event count stamp:

  CRingPhysicsEventCountItem item(m_nEventsEmitted,
				  endTime,
				  now);
  item.commitToRing(*m_pRing);
}

/*!
  Reads scaler data.  If the root scaler bank exists, it is asked to read its
  data and return a vector of uint32_t.  That vector is used as the scalers in a scaler
  event that is submitted to the ring buffer.

*/

void CExperiment::TriggerScalerReadout()
{


  readScalers();

  // For now documented variables are tied to this trigger too:

  ScheduleRunVariableDump();

}

/*!
   Dumps a documentation event that describes the documented packets to the ring
   buffer.  This usually happens as the run becomes active again.
   Implicit inputs are the documented packet manager which can produce the information we need.


*/
void
CExperiment::DocumentPackets()
{
  CDocumentedPacketManager* pMgr = CDocumentedPacketManager::getInstance();
  vector<string> packetDefs = pMgr->Format();

  // only emit a record if there are documented packets:

  if (packetDefs.size()) {
    time_t           now     = time(&now);
    uint64_t         msTime  = getTimeMs();
    uint32_t         offset = (msTime - m_nRunStartStamp - m_nPausedmSeconds)/1000;
    CRingTextItem item(PACKET_TYPES, NULL_TIMESTAMP, m_nSourceId, BARRIER_NOTBARRIER,
                       packetDefs,
                       offset,
                       static_cast<uint32_t>(now));
    item.commitToRing(*m_pRing);
  }
}

/*!
  Schedules a dump of the run variables.  Since this is invoked from the trigger thread,
  but references Tcl variables in the Tcl interpreter thread.  This is scheduled via
  a ThreadRequest that blocks us until the operation completes in the Tcl thread.
  Blocking also serializes access to the ring buffer.

*/
void
CExperiment::ScheduleRunVariableDump()
{
  CVariableBuffers* pVars = CVariableBuffers::getInstance();
  uint64_t timeOffset = m_nRunStartStamp + m_nPausedmSeconds;
  pVars->triggerRunVariableBuffer(m_pRing, timeOffset);
  pVars->triggerStateVariableBuffer(m_pRing, timeOffset);
}


/*!
  Return the current event trigger:
*/
CEventTrigger*
CExperiment::getEventTrigger()
{
  return m_pEventTrigger;
}
/*!
  Returnthe current scaler trigger:
*/
CEventTrigger*
CExperiment::getScalerTrigger()
{
  return m_pScalerTrigger;
}
/*!
   Schedule the end run buffer read out:
   @param pause  - true if this is a pause run.
*/
void
CExperiment::ScheduleEndRunBuffer(bool pause)
{

  pEndRunEvent pEvent = reinterpret_cast<pEndRunEvent>(Tcl_Alloc(sizeof(EndRunEvent)));
  pEvent->tclEvent.proc = CExperiment::HandleEndRunEvent;
  pEvent->pause         = pause;
  pEvent->pExperiment   = this;
  pEvent->pLock         = new CMutex;
  pEvent->pCondVar      = new CConditionVariable;
  
  pEvent->pLock->lock();                // Required to wait on condition.

  CTCLInterpreter* pInterp = gpTCLApplication->getInterpreter();
  Tcl_ThreadId     thread  = gpTCLApplication->getThread();

  Tcl_ThreadQueueEvent(thread, 
		       reinterpret_cast<Tcl_Event*>(pEvent),
		       TCL_QUEUE_TAIL);
  
  // The handler is going to
  // 1. acquire the mutex (that will synch us with the wait start)
  // 2. do its stuff
  // 3. Free the event, but not the mutex and condition variable.
  // 4. Signal the condition variable.
  
  CMutex* pMutex               = pEvent->pLock;    // Tcl Event storage might
  CConditionVariable *pCondVar = pEvent->pCondVar; // get re-used.
  
  pEvent->pCondVar->wait(*(pMutex));

  // Now the run is ended so we can release the mutex and destroy all the bits.
  
  pMutex->unlock();
  delete(pMutex);
  delete(pCondVar);

}
/*!
  Handle the end run event by getting object context and 
 calling syncEndRun
*/
int CExperiment::HandleEndRunEvent(Tcl_Event* evPtr, int flags)
{
  pEndRunEvent pEvent = reinterpret_cast<pEndRunEvent>(evPtr);
  pEvent->pLock->lock();                 // Wait for signaller to condwait.
  CExperiment* pExperiment  = pEvent->pExperiment;
  pExperiment->syncEndRun(pEvent->pause);
  pEvent->pCondVar->signal();             // Tell signaller we're done.
  pEvent->pLock->unlock();                // And this releases it to run.
  return 1;
}

/**
 * Return the current time of day in ms from the Realtime clock.
 * @return uint64_t  
 * @retval ms since the epoch.
 */
uint64_t 
CExperiment::getTimeMs()
{
  timespec currentTime;
  uint64_t msTime;
  clock_gettime(CLOCK_REALTIME, &currentTime); // We can assume no errors (see the  manpage).

  msTime = currentTime.tv_sec;
  msTime = msTime * 1000;	// seconds ->ms.
  msTime += currentTime.tv_nsec/(1000*1000); // ns-> ms.

  return msTime;

}
/**
 * setTimestamp
 *
 * Set the timestamp for the current event and flag that it needs a bodyheader.
 *
 * @param stamp - the timestamp.
 */
void
CExperiment::setTimestamp(uint64_t stamp)
{
    m_nEventTimestamp = stamp;
    m_needHeader = true;
}
/**
 * setSourceId
 *
 * Set the current event's source id and flag that it needs a body heder.
 *
 * @param id
 */
void
CExperiment::setSourceId(uint32_t id)
{
    m_nSourceId  = id;
    m_needHeader = true;
}
/**
 * getSourceId
 *   return the current evnt source id.
 * @return uint32_t
 */
uint32_t
CExperiment::getSourceId()
{
  return m_nSourceId;
}
/**
 * triggerFail
 *   Called by the trigger thread if it caught an exceptino that caused the
 *   trigger loop to exit.  In that case we schedule an event for the main thread
 *   (HandleTriggerLoopError).  The event will attempt to invoke the
 *   command ::onTriggerFail passing the message we got from the trigger loop.
 *   That command can do whatever it wants (I recommend at least ending the run).
 *
 *   @param msg - A text message from the trigger loop that describes the
 *                error.
 */
void
CExperiment::triggerFail(std::string msg)
{
    // Create and fill in  the event struct:
 
    pTriggerFailEvent pEvent =
        reinterpret_cast<pTriggerFailEvent>(Tcl_Alloc(sizeof(TriggerFailEvent)));
    
    pEvent->tcl_Event.proc = HandleTriggerLoopError;
    pEvent->message = Tcl_Alloc(msg.size() + 1);
    strcpy(pEvent->message, msg.c_str());
    
    //  figure out which thread is our target and schedule the event.
    
    Tcl_ThreadId threadId = gpTCLApplication->getThread();
    Tcl_ThreadQueueEvent(threadId, reinterpret_cast<Tcl_Event*>(pEvent), TCL_QUEUE_TAIL);
    
    
}
/**
 * HandleTriggerLoopError
 *   Runs in the main thread.  Receives events that report errors in the trigger
 *   thread.  Here's what we try to do:
 *   *  First try to call ::onTriggerFail passing it the message.
 *   *  If that fails invoke bgerror again, passing it the message.
 *   *  Finally release storage associated with the message and return 1
 *      indicating the event dispatcher can release the event.
 *   @param pEvent - actually a pointer to a TriggerFailEvent struct.
 *   @param flags  - event flags.
 *   @return int - (1) - indicates the event storage can be disposed of.
 *   
 */
int
CExperiment::HandleTriggerLoopError(Tcl_Event* pEvent, int flags)
{
    // Obtain the message and delete it's storage so we won't forget:
    
    pTriggerFailEvent pTfEvent = reinterpret_cast<pTriggerFailEvent>(pEvent);
    std::string msg(pTfEvent->message);
    Tcl_Free(pTfEvent->message);
    pTfEvent->message = 0;                    // Ensure it's never referenced.
    
    // Give ::onTriggerFail a try:
    
    CTCLInterpreter* pInterp = gpTCLApplication->getInterpreter();
    CTCLObject       command = createCommand(pInterp, "::onTriggerFail", msg);
    int              stat    = Tcl_GlobalEvalObj(
        pInterp->getInterpreter(), command.getObject()
    );
    
    // If that failed fall back on bgerror:
    
    if (stat != TCL_OK) {
        command = createCommand(pInterp, "bgerror", msg);
        Tcl_GlobalEvalObj(
            pInterp->getInterpreter(), command.getObject()
        );
    }
    // Return such that the event can be released:
    
    return 1;
    
}
/**
 * createCommand
 *  Create a CTCLObject that is a verb an a parameter.
 *
 *  @param pInterp - pointer to the Tcl encapsulated interpreter.
 *  @param verb    - command verb.
 *  @param parameter - Command parameter.
 */
CTCLObject
CExperiment::createCommand(
    CTCLInterpreter* pInterp, const char* verb, std::string parameter
)
{
    CTCLObject command;
    command.Bind(pInterp);
    command += verb;
    command += parameter;
    
    return command;
}
