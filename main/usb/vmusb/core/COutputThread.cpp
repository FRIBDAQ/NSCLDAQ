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
#include "COutputThread.h"
#include "CRunState.h"
#include "DataBuffer.h"
#include <CSystemControl.h>
#include <event.h>
#include <string>
#include <Exception.h>
#include <ErrnoException.h>
#include <Globals.h>
#include <CVMUSB.h>
#include <TclServer.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <CRingBuffer.h>

#include <CRingStateChangeItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CDataFormatItem.h>
#include <CStack.h>

#include <sys/time.h>
#include <dlfcn.h>

#include <fragment.h>

#include <iostream>
#include <iomanip>
using namespace std;


static DataBuffer* lastBuffer(0);
static const unsigned ReadoutStack(0);
static const unsigned ScalerStack(1);
static const unsigned MonitorStack(7);

static const unsigned BUFFERS_BETWEEN_EVENTCOUNTS(64);  // max buffers before an event count item.


////////////////////////////////////////////////////////////////////////
//   mytimersub - since BSD timeval is not the same as POSIX timespec:
////////////////////////////////////////////////////////////////////////
static inline void 
mytimersub(timespec* minuend, timespec* subtrahend, timespec* difference)
{
  // We'll cheat and map these to timevals, use timersub and convert back:
  // this means we're only good to a microsecond not a nanosecond _sigh_
  // if this is not good enough we'll do the subtraction manually later.

  timeval m,s,d;
  m.tv_sec   = minuend->tv_sec;
  m.tv_usec  = minuend->tv_nsec/1000;

  s.tv_sec   = subtrahend->tv_sec;
  s.tv_usec  = subtrahend->tv_nsec/1000;

  timersub(&m, &s, &d);

  difference->tv_sec  = d.tv_sec;
  difference->tv_nsec = d.tv_usec * 1000;
}

////////////////////////////////////////////////////////////////////////
///////////////////// Construction and destruction /////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Create an outupt thread. 

   @param ring - Name of the ring buffer into which data will be put.


*/
COutputThread::COutputThread(std::string ring, CSystemControl& sysControl) : 
  m_nOutputBufferSize(0),		// Don't know yet.
  m_pBuffer(0),
  m_pCursor(0),
  m_nWordsInBuffer(0),
  m_ringName(ring),
  m_pRing(0),
  m_pEvtTimestampExtractor(0),
  m_pSclrTimestampExtractor(0),
  m_pBeginRunCallback(0),
  m_systemControl(sysControl)
#ifdef VMUSB_END_OF_EVENT_WORKAROUND
  ,m_LastHeader(-1)		// Not set.
#endif
{
    memset(&m_statistics, 0, sizeof(m_statistics));
}
/*!
  Destructor...actually this is probably not called as the thread lifetime
  is the lifetime of the program.  Regardless...do things cleanly.
*/
COutputThread::~COutputThread()
{
  delete m_pRing;		// close connection to ring.
  free(m_pBuffer);		// Free the event assembly buffer.
}

////////////////////////////////////////////////////////////////////////
////////////////////// Thread entry point... ///////////////////////////
////////////////////////////////////////////////////////////////////////

/* Adapt between nextgen and spectrodaq style threading models:
 */
void
COutputThread::run()
{
  getTimestampExtractor();
  operator()();
}

/**
 * getStatistics
 *    Return the current counters:
 */
const COutputThread::Statistics&
COutputThread::getStatistics() const {
    return m_statistics;
}

/*
   Thread entry point.  After attaching to the outputring,
   this is just an infinite buffer processing loop.
*/
int
COutputThread::operator()()
{
  // Main loop is pretty simple.
  // - Acquire next filled buffer
  // - process the filled buffer
  // - add the buffer back onto the queue of free buffers
  //
  // should something bad happen...
  // - schedule an exit
  // - keep freeing buffers
  //

  bool failedState = false;
  try {
    attachRing();

    while(1) {
      
      DataBuffer& buffer(getBuffer());
      processBuffer(buffer);
      freeBuffer(buffer);
      
    }
  }
  catch (string msg) {
    cerr << "COutput thread caught a string exception: " << msg << endl;
    failedState = true;
    scheduleApplicationExit(EXIT_FAILURE);
  }
  catch (char* msg) {
    cerr << "COutput thread caught a char* exception: " << msg << endl;
    failedState = true;
    scheduleApplicationExit(EXIT_FAILURE);
  }
  catch (CException& err) {
    cerr << "COutputThread thread caught a daq exception: "
	        << err.ReasonText() << " while " << err.WasDoing() << endl;
    failedState = true;
    scheduleApplicationExit(EXIT_FAILURE);
  }
  catch (...) {
    cerr << "COutput thread caught some other exception type.\n";
    failedState = true;
    scheduleApplicationExit(EXIT_FAILURE);
  }

  // if we failed... don't try to output data.. just keep freeing buffers
  // so that we can end the application gracefully.
  if (failedState) {
    while (1) {
      DataBuffer& buffer(getBuffer());
      freeBuffer(buffer);
    }

  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////// Utility functions ///////////////////////////
/////////////////////////////////////////////////////////////////////////


/*
   Get a buffer from the event queue.  A reference to the buffer will
   be returned.

   Note that this will block if needed to wait for a buffer.
   note as well that between runs, we'll wind up blocking in here.

   @return DataBuffer&
   @retval reference to the buffer at the head of the buffer queue.

*/
DataBuffer&
COutputThread::getBuffer()
{
  DataBuffer* pBuffer = gFilledBuffers.get(); // Will block if needed.
  return *pBuffer;

}
/*
   Free a buffer that has been completely processed.
   This will return the buffer to the gFreeBuffers queue.

   @param buffer - Reference to the buffer to return to the free pool.

*/
void
COutputThread::freeBuffer(DataBuffer& buffer)
{
  gFreeBuffers.queue(&buffer);
}
/*
   Process a buffer from the reader.  At this time we are just going
   to figure out what type of buffer we have and dispatch accordingly.
   Buffers are as follows:

  -  Begin run is indicated by the type in the DataBuffer, all others look like
      data buffers.
  -  End run is indicated by the data buffer type.
  - All others are depend on the stack from which they came, and are handled
    by processEvent.

  @note begin and end buffers are generated by software not the VM-USB.

  @param buffer  - The buffer from the readout thread.
*/
void
COutputThread::processBuffer(DataBuffer& buffer)
{
  if (buffer.s_bufferType == TYPE_START) {
    startRun(buffer);
    memset(&m_statistics.s_perRun, 0, sizeof(m_statistics.s_perRun)); // clear per run statistics.
  }
  else if (buffer.s_bufferType == TYPE_STOP) {
    endRun(buffer);
  }
  // TYPE_PAUSE and TYPE_RESUME buffer added to resolve Bug #5882
  
  else if (buffer.s_bufferType == TYPE_PAUSE) {
    pauseRun(buffer);  
  }
  else if (buffer.s_bufferType == TYPE_RESUME) {
    resumeRun(buffer);
  }
  else if (buffer.s_bufferType == TYPE_EVENTS) {
    processEvents(buffer);
  }
  else if (buffer.s_bufferType == TYPE_STRINGS) {
    pStringsBuffer pBody = reinterpret_cast<pStringsBuffer>(buffer.s_rawData);
    processStrings(buffer, *pBody);
  }
  else {
    // Unexpected buffer type is really bad throw:

    char exceptionText[1000];
    sprintf(exceptionText, "Unexpected buffer type %d encounterd", buffer.s_bufferType);
    throw std::string(exceptionText);
  }
}

/**
 *  TODO: Belowe  startRun, endRun, pauseRun and resumeRun:
 *        The generation of the control ring item can be factored once the
 *        run start time has been memorized.
*/

/*
   Process a begin run pseudo buffer. I call this a psuedo buffer because
   there will not be any data from the VM_usb for this

   We must:
   - Update our concept of the run state.
   - Set the m_nOutputBufferSize accordingly.
   - Create and submit a state BEGIN_RUN State change item to the destination ring
     buffer
   
   @param buffer   - the buffer from the buffer queue.

   @throws CErrnoException if system calls fail.
   @throws std::string If CRingBufer operations fail.
*/
void
COutputThread::startRun(DataBuffer& buffer)
{

  time_t timestamp;
  
  // If there's a begin run callback call it before emitting the begin  run
  // record:
  
  if (m_pBeginRunCallback) {
    (*m_pBeginRunCallback)();
  }

  m_nOutputBufferSize = Globals::usbBufferSize;

  if (time(&timestamp) == -1) {
    throw CErrnoException("Failed to get the time in COutputThread::startRun");
  }

  // Update our concept of run state.

  CRunState* pState = CRunState::getInstance();
  m_runNumber       = pState->getRunNumber();
  m_title           = pState->getTitle();

  clock_gettime(CLOCK_REALTIME, &m_startTimestamp);
  m_lastStampedBuffer = m_startTimestamp; // Last timestamped event...that is.
  m_elapsedSeconds = 0;
  
  m_nEventsSeen    = 0;

  CDataFormatItem format;
  format.commitToRing(*m_pRing);
  
 
  CRingStateChangeItem begin(NULL_TIMESTAMP, Globals::sourceId, BARRIER_START,
                             BEGIN_RUN,
			     m_runNumber,
			     0,
			     static_cast<uint32_t>(timestamp),
			     m_title.substr(0, TITLE_MAXSIZE-1));

  begin.commitToRing(*m_pRing);
  
  // Reset the number of buffers of events between event count items:
  
  m_nBuffersBeforeEventCount = BUFFERS_BETWEEN_EVENTCOUNTS;


}
/*
  Called when an end of run has occured. The end of run is pseudo buffer
  generated by software.   All we need to do is crate an END_RUN state change
  ring item and submit it to our target ring.

  @param buffer - Data buffer from the VM-USB.
  
  @throws CErrnoException if unable to get the timestamp for the end of the run.
  @throws std::string from CRingBuffer if the ring can't be manipulated.

*/
void
COutputThread::endRun(DataBuffer& buffer)
{

  free(m_pBuffer);
  m_pBuffer = 0;

  // Determine the absolute timestamp.

  time_t stamp;
  if (time(&stamp) == -1) {
    throw CErrnoException("Failed  to get the timestamp in COutputThread::endRun");
  }
  // Determine the run relative timestamp:
  // See Issue #423 - need to factor this.
 
  timespec microtime;
  clock_gettime(CLOCK_REALTIME, &microtime);
  timespec microdiff;
  mytimersub(&microtime, &m_startTimestamp, &microdiff);
  
  CRingStateChangeItem end(NULL_TIMESTAMP, Globals::sourceId, BARRIER_END,
                           END_RUN,
			   m_runNumber,
			   microdiff.tv_sec,
			   stamp,
			   m_title);

  end.commitToRing(*m_pRing);
			   


}
/**
 * pauseRun    (Bug#5882)
 *  Pause the run.
 *    @param buf - the data buffer.
 */
void
COutputThread::pauseRun(DataBuffer& buffer)
{
  free(m_pBuffer);
  m_pBuffer = 0;

  // Determine the absolute timestamp.

  time_t stamp;
  if (time(&stamp) == -1) {
    throw CErrnoException("Failed  to get the timestamp in COutputThread::endRun");
  }
  // Determine the run relative timestamp:
  // See Issue #423 - need to factor this.
 
  timespec microtime;
  clock_gettime(CLOCK_REALTIME, &microtime);
  timespec microdiff;
  mytimersub(&microtime, &m_startTimestamp, &microdiff);
  
  CRingStateChangeItem pause(NULL_TIMESTAMP, Globals::sourceId, BARRIER_END,
                           PAUSE_RUN,
			   m_runNumber,
			   microdiff.tv_sec,
			   stamp,
			   m_title);

  pause.commitToRing(*m_pRing);  
}
/**
 * resumeRun    (Bug#5882)
 *    Resume the run.  The run is considered active across the
 *    pause.  This allows us to know the amount of time the run was paused by
 *    looking at the timestamps in both the pause and resume items
 *    It also means that the end item will be the total time between
 *    begin and end.
 *
 *  @param buffer - the data buffer.
 */
void
COutputThread::resumeRun(DataBuffer& buffer)
{
  free(m_pBuffer);
  m_pBuffer = 0;

  // Determine the absolute timestamp.

  time_t stamp;
  if (time(&stamp) == -1) {
    throw CErrnoException("Failed  to get the timestamp in COutputThread::endRun");
  }
  // Determine the run relative timestamp:
  // See Issue #423 - need to factor this.
 
  timespec microtime;
  clock_gettime(CLOCK_REALTIME, &microtime);
  timespec microdiff;
  mytimersub(&microtime, &m_startTimestamp, &microdiff);
  
  CRingStateChangeItem resume(NULL_TIMESTAMP, Globals::sourceId, BARRIER_END,
                           RESUME_RUN,
			   m_runNumber,
			   microdiff.tv_sec,
			   stamp,
			   m_title);

  resume.commitToRing(*m_pRing);
}

/**!
* Check whether the bit in the global mode register to output
* a second buffer header word is set. 
*
* \return whether the devices is set to out an extra buffer header
*/
bool COutputThread::hasOptionalHeader()
{
  uint16_t glbl_mode = Globals::pUSBController->getShadowRegisters().globalMode;

  return (glbl_mode & CVMUSB::GlobalModeRegister::doubleHeader);
}


/**
 * Process events in a buffer creating output buffers as required.
 *  - Figure out the used words in the buffer
 *  - For each event in the buffer invoke either event or scaler depending on
 *    the stack number.  Stack 1 is always a scaler event while any other stack
 *    is considered a physics event.
 *  - Stack 7 is the monitor stack and is sent to the Tcl server via a Tcl event
 *    sent to its event queue.
 *
 * @param inBuffer  -  Reference to a raw input buffer.
 */
static uint32_t  bufferNumber = 0; 
void 
COutputThread::processEvents(DataBuffer& inBuffer)
{
  uint16_t* pContents = inBuffer.s_rawData;
  int16_t  nEvents   = (*pContents) & VMUSBNEventMask;


  bufferNumber++;

  pContents++;			// Point to first event.
  ssize_t    nWords    = (inBuffer.s_bufferSize)/sizeof(uint16_t) - 1; // Remaining words read.

  // Check if the optional second header exists
  if (hasOptionalHeader()) {
    uint16_t wordsInBuffer = *pContents++;  
    if (wordsInBuffer != nWords-1) {
      // The words that are reported in the optional header do not count 
      // the first buffer header word. The number is self inclusive though.
      cerr << "VMUSB specifies " << wordsInBuffer << " in buffer, but ";
      cerr << "the number of words read is in disagreement." ;
    }
    --nWords;
  } 

  while (nWords > 0) {
    if (nEvents <= 0) {
      // Next long should be 0xffffffff buffer terminator:
      // I've seen this happen but it's not fatal...just go on to the next buffer.

      uint32_t* pNextLong = reinterpret_cast<uint32_t*>(pContents);
      if (*pNextLong != 0xffffffff) {
        cerr << "Ran out of events but did not see buffer terminator\n";
        cerr << nWords << " remaining unprocessed\n";
      }

      break;			// trusting event count vs word count(?).
    }

    // Pull the event length and stack number from the header:
    // event length is not self inclusive and is in uint16_t units.

    uint16_t header     = *pContents;
    size_t   eventLength = header & VMUSBEventLengthMask;
    uint8_t  stackNum   = (header & VMUSBStackIdMask) >> VMUSBStackIdShift;

    // Dispatch depending on the actual stack number.  The dispatch provided
    // allows for multiple event stacks (e.g. interrupt triggered stacks
    // that do different things).

    if (stackNum == ScalerStack) {
      scaler(pContents);
    }
    else if (stackNum == MonitorStack) {
      sendToTclServer(pContents);
    }
    else {
      event(pContents);
    }
    // Point at the next event and compute the remaining word and event counts.
    
    pContents += eventLength + 1; // Event count is not self inclusive.
    nWords    -= (eventLength + 1);
    nEvents--;
  }

  // I've seen the VM-USB hand me a bogus event count...but never a bogus
  // buffer word count.  This is non fatal but reported.

  if (nWords < 0) {
    cerr << "Warning used up more than the buffer  by " << (-nWords) << endl;
  }
  m_nBuffersBeforeEventCount--;
  if (m_nBuffersBeforeEventCount == 0) {
    
    // No scaler buffers maybe so figure out the time in seconds we are
    // into the run from the realtime clock...forget fractions.
    
    timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    
    outputTriggerCount(now.tv_sec - m_startTimestamp.tv_sec);   // Figure out run offset.
    m_nBuffersBeforeEventCount = BUFFERS_BETWEEN_EVENTCOUNTS;
  }
}
/**
 * stringBuffer:
 *    Process a strings buffer.  Strings buffers contain a set of null terminated
 *    strings.  They are used to carry e.g. Control data buffers.
 *
 * @param buffer - Reference to the full data buffer.
 * @param strings The body of the data buffer already cst to a reference 
 *                to a StringsBuffer.
 */
void
COutputThread::processStrings(DataBuffer& buffer, StringsBuffer& strings)
{

  // Marshall the strings into a vector as expected by the CRingTextItem c-tor.

  std::vector<std::string> stringVector; 
  const char* pSrc = strings.s_strings;
  
  for (int i=0; i < strings.s_stringCount; i++) {
    stringVector.push_back(pSrc);
    pSrc += strlen(pSrc) + 1;	// +1 for the null terminator.
  }
  // Once we have a timestamp we're ready to go.

  time_t now = time(NULL);

  // Create and commit the item to the ring.

  CRingTextItem texts(
    strings.s_ringType, NULL_TIMESTAMP, Globals::sourceId, BARRIER_NOTBARRIER,
		      stringVector,
		      m_elapsedSeconds, // best we can do for now.
		      static_cast<uint32_t>(now));
  texts.commitToRing(*m_pRing);

}


/**
 * Output a physics trigger count event item.  These are used to monitor
 * overall rates as well as to provide sampling statistics for sampling
 * consumers.
 *
 * @param runOffset - seconds intothe run at which this is being emitted.
 */
void
COutputThread::outputTriggerCount(uint32_t runOffset)
{
  time_t now = time(nullptr);
  CRingPhysicsEventCountItem item(NULL_TIMESTAMP, Globals::sourceId, BARRIER_NOTBARRIER
				  ,m_nEventsSeen, runOffset, now);
  item.commitToRing(*m_pRing);
}

/**
 * Process a scaler event:
 * - Figure out the time interval start/stop times, and the absolute time.
 * - extract the vector of scalers from the VM-USB event.
 * - Create and submit the CRingScalerItem to the ring.
 *  
 * @note We assume scaler events won't consist of more than 2048
 *       scalers (e.g. the continuation bit is not set.

 * @param pData - Pointer to scaler data.
 *
 * @throw std::string - If a scaler buffer has a continuation segment.
 * @throw CErrnoException - If we can't get the absolute timestamp.
 * @throw std::string - From CRingBuffer if unable to commit the item to the ring.
 */
void
COutputThread::scaler(void* pData)
{


  time_t timestamp;
  if (time(&timestamp) == -1) {
    throw CErrnoException("COutputThread::scaler unable to get the absolute timestamp");
  }

  // Figure out where the scalers are and fetch the event header.

  uint16_t* pHeader = reinterpret_cast<uint16_t*>(pData);
  uint16_t  header  = *pHeader;
  uint32_t* pBody   = reinterpret_cast<uint32_t*>(pHeader+1); // Pointer to the scalers.

  // See Issue #424 - for now throw an error  if there's a continuation segment:


  // Figure out how many wods/scalers there are:

  size_t        nWords   =  header & VMUSBEventLengthMask;
  size_t        nScalers =  nWords/(sizeof(uint32_t)/sizeof(uint16_t));

  // Marshall the scalers into an std::vector:

  std::vector<uint32_t> counterData;
  for (int i = 0; i < nScalers; i++) {
    counterData.push_back(*pBody++);
  }

  // The VM-USB does not timestamp scaler data for us at this time so we
  // are going to rely on the scaler period to be correct:

  uint32_t endTime = m_elapsedSeconds + Globals::scalerPeriod;

  // Output a ring count item using this time:

  outputTriggerCount(endTime);
  m_nBuffersBeforeEventCount = BUFFERS_BETWEEN_EVENTCOUNTS;

  // Create the final scaler item and submit it to the ring.

  CRingItem* pEvent;
  if (m_pSclrTimestampExtractor) {
    pEvent = new CRingScalerItem(m_pSclrTimestampExtractor(pData), 
                                 Globals::sourceId, 
                                 BARRIER_NOTBARRIER,
                                 m_elapsedSeconds, 
                                 endTime, 
                                 timestamp, 
                                 counterData,
				 1, CStack::scalerIsIncremental());
  } else {
    pEvent = new CRingScalerItem(m_elapsedSeconds, 
                                 endTime, 
                                 timestamp, 
                                 counterData,
				 CStack::scalerIsIncremental());
  }

  pEvent->commitToRing(*m_pRing);
  m_elapsedSeconds = endTime;
  delete pEvent;
}



/**
 * Create a new output buffer.
 * for now this is trivial
 *
 * @return uint8_t*
 * @retval Pointer to the output buffer.
 */
uint8_t* 
COutputThread::newOutputBuffer()
{
  return reinterpret_cast<uint8_t*>(malloc(m_nOutputBufferSize));
  
}
/**
 * Process a single event:
 * - If necessary create the event assembly buffer and initialize its
 *   cursor.
 * - Put the segment in the event assembly buffer.
 * - If there is a continuation segment we're done for now..as we'll get called again with the next
 *   segment
 * - If there is no continuation segment then we create and submit the output
 *   event to the ring and reset the cursor.
 *
 * @param pData - pointer to a VM-USB event segment.
 *
 * @throws std::string - event overflows the output buffer.
 * @throws std::string - Errors from the ring buffer classes.
 *
 * @note The data go in in native VM-USB format.  That's what the SpecTcl disassembler expects.
 */
void 
COutputThread::event(void* pData)
{
  // If necessary make an new output buffer

  if (!m_pBuffer) {
    m_pBuffer        = newOutputBuffer();
    m_pCursor        = m_pBuffer;
    m_nWordsInBuffer = 0;	  
  }

  // Initialize the pointers to event bits and pieces.

  uint16_t* pSegment = reinterpret_cast<uint16_t*>(pData);
  uint16_t  header   = *pSegment;

  // Figure out the header:
 
  size_t segmentSize = header & VMUSBEventLengthMask;
  bool   haveMore    = (header & VMUSBContinuation) != 0;
  
  // Events must currently fit in the buffer...otherwise we throw an error.

  segmentSize += 1;		// Size is not self inclusive

  if ((segmentSize + m_nWordsInBuffer) >= m_nOutputBufferSize/sizeof(uint16_t)) {
    int newSize          = 2*segmentSize*sizeof(uint16_t);
    uint8_t* pNewBuffer = reinterpret_cast<uint8_t*>(realloc(m_pBuffer, m_nOutputBufferSize+newSize));
    if (pNewBuffer) {
      m_pBuffer            = pNewBuffer;
      m_pCursor            = m_pBuffer + m_nWordsInBuffer * sizeof(uint16_t);
      m_nOutputBufferSize += newSize;
      
    } else {
      throw std::string("Failed to resize event buffer to fit an oversized segment");
    }


  }
  // Next we can copy our data to the output buffer and update the cursor
  // remembering that the size is not self inclusive:
  //
 #ifdef VMUSB_END_OF_EVENT_WORKAROUND
  uint16_t* dest = reinterpret_cast<uint16_t*>(m_pCursor);
#endif
  memcpy(m_pCursor, pData, segmentSize*sizeof(uint16_t));
  m_nWordsInBuffer += segmentSize;
  m_pCursor += segmentSize*sizeof(uint16_t); // advance the cursor

  

  // If that was the last segment submit it and reset cursors and counters.

  if (!haveMore) {			    // Ending segment:

  #ifdef VMUSB_END_OF_EVENT_WORKAROUND
    // If this is enabled, we have a trailing marker we have to delete, and
    // we need to adjust the wordcount in the segment header down by that
    // marker...and deal with the edge case that this is the only 
    // word in that segment.... in that case we need to retract the
    // header as well and remove the contiuation bit from the last
    // header:

    // remove the marker:

    segmentSize--;
    m_pCursor -= sizeof(uint16_t);
    m_nWordsInBuffer--;

    // Ajust the header properly


    // Note the header length is not self inclusive hence the -1
    header = header & (~VMUSBEventLengthMask) | (segmentSize-1);  //Sset the adjust segment size...
    *dest = header;        // Relace it in our copy-in buffer.
    if (header == 0) {
      // That left an empty segment so retract the header too...
      // if there was no last header that means we're going to emit an empty event...
      // but that means the user has an event stack with no modules so they
      // get what they deserve.

      if (m_LastHeader >= 0) {
        m_pCursor -= sizeof(uint16_t);    // Retract the empty header
        m_nWordsInBuffer--;
        
        uint16_t* pLastHeader =
	    reinterpret_cast<uint16_t*>(m_pBuffer + m_LastHeader);  // Remove the continuation bit
	uint16_t lastHeader = *pLastHeader;
        lastHeader &= ~VMUSBContinuation;  // from the previous header.
        *pLastHeader = lastHeader;
      } 
    }

  #endif

    //
    // IF we were given a timestamp extractor we create an event with full
    // body header.
    
    CRingItem* pEvent;
    if (m_pEvtTimestampExtractor) {
        pEvent = new CRingItem(
            PHYSICS_EVENT, m_pEvtTimestampExtractor(m_pBuffer), Globals::sourceId,
            BARRIER_NOTBARRIER, m_nWordsInBuffer*sizeof(uint16_t) + 100
        );        
    } else {
        pEvent = new CRingItem(
            PHYSICS_EVENT, m_nWordsInBuffer*sizeof(uint16_t) + 100
        ); // +100 really needed?
    }
    
     
    CRingItem& event(*pEvent);
    // Put the data in the event and figure out where the end pointer is.

    void* pDest = event.getBodyPointer();
    memcpy(pDest, m_pBuffer, m_nWordsInBuffer*sizeof(uint16_t));
    uint8_t* pEnd = reinterpret_cast<uint8_t*>(pDest);
    pEnd += m_nWordsInBuffer*sizeof(uint16_t); // Where the new body cursor goes.

    // If the end of event workaround is enabled,
    // We tell the ring item not to include the 16 bit marker
    // we transparently added to the readout stack to force
    // end of event to fire.

#ifndef VMUSB_END_OF_EVENT_WORKAROUND
    event.setBodyCursor(pEnd);
#else
    event.setBodyCursor(pEnd - sizeof(uint16_t));
#endif
    event.updateSize();
    event.commitToRing(*m_pRing);
    delete pEvent;
    
    m_statistics.s_perRun.s_triggers++;
    m_statistics.s_perRun.s_acceptedTriggers++;
    m_statistics.s_perRun.s_bytes += m_nWordsInBuffer*sizeof(uint16_t);
    
    m_statistics.s_cumulative.s_triggers++;
    m_statistics.s_cumulative.s_acceptedTriggers++;
    m_statistics.s_cumulative.s_bytes += m_nWordsInBuffer*sizeof(uint16_t);
    // Reset the cursor and word count in the assembly buffer:

    m_nWordsInBuffer = 0;
    m_pCursor        = m_pBuffer;
    
        
    m_nEventsSeen++;


#ifdef VMUSB_END_OF_EVENT_WORKAROUND
    m_LastHeader = -1;      // Next event does not have a prior header.
#endif
  }
#ifdef VMUSB_END_OF_EVENT_WORKAROUND
  else {
    // Save the header offset so we can adjust if the last segment only has a marker:
    // Note we save the offset because the buffer might get reallocated/moved on the
    // next segment, invalidating a pointer.
      
      m_LastHeader = reinterpret_cast<uint8_t*>(dest) - m_pBuffer;
  }
#endif
>>>>>>> 0fdd1fcf5 (Issue #294 - VMUSB firmware workarounds.)

}


/**
 * Sends a control monitoring event (one from stack 7) to the 
 * control server. These events contain periodic monitoring data.
 * It's up to the Tcl server to assemble them into a single 
 * event (if they are split across a buffer boundary).
 * 
 * @param pEvent - Pointer to the event fragment.
 */
void
COutputThread::sendToTclServer(uint16_t* pEvent)
{


  // Locate the Tcl Server and queue the event:

  TclServer* pServer = Globals::pTclServer;
  pServer->QueueBuffer(pEvent);

  
}
/**
 * Attach to the ring buffer specified in m_ringName.  If that ring does not
 * yet exist, it is created.  A pointer to the created CRingBuffer object is
 * put in m_pRing
 *
 * @throws std::string if we can't attach to the ring or creation fails.
 *                     The actual exceptions are tossed from the bowels of
 *                     the CRingBuffer class.
 */
void
COutputThread::attachRing()
{
  try {
    m_pRing = CRingBuffer::createAndProduce(m_ringName);
  } catch (CException& e) {
    std::cerr << "Failed to attach ring buffer: " << m_ringName << " "
      << e.ReasonText() << std::endl;
    std::cerr << "Note: Permission denied can mean another Readout is "
      << "attached to the ringbuffer\n";
    exit(EXIT_FAILURE);
  }

}
/**
 * getTimestampExtractor
 *    Fills in m_pTimestampExtractor if this should be non-null
 *    - If the Globals::pTimestampExtractor is non-null
 *      it is a path to a shared lib that is mapped.
 *    - If successfully mapped, the entry points getEventTimestamp() and
 *      and getScalerTimestamp() are located
 *    - If that's found the pointer to getEventTimestamp is filled in for
 *      m_pEvtTimestampExtractor and the getScalerTimestamp is filled in for
 *      m_pSclrTimestampExtractor.
 */
void
COutputThread::getTimestampExtractor()
{
    if (Globals::pTimestampExtractor) {
        void* pDllHandle = dlopen(
            Globals::pTimestampExtractor, RTLD_NOW | RTLD_NODELETE
        );
        // Load the so/dll:
        
        if (!pDllHandle) {
            std::cerr << "Failed to load timestamp extractor library: "
                << Globals::pTimestampExtractor << " "  << dlerror() << std::endl;
            exit(EXIT_FAILURE);
        }
        // Locate the function entry point:
        
        void* pEvtFunction = dlsym(pDllHandle, "getEventTimestamp");
        if (!pEvtFunction) {
            std::cerr << "Warning: Unable to locate getEventTimestamp  in "
                << Globals::pTimestampExtractor << " "
                << dlerror() << std::endl;
        } else {
            m_pEvtTimestampExtractor = reinterpret_cast<TimestampExtractor>(pEvtFunction);
        }

        void* pSclrFunction = dlsym(pDllHandle, "getScalerTimestamp");
        if (!pSclrFunction) {
            std::cerr << "Warning: Unable to locate getScalerTimestamp  in "
                << Globals::pTimestampExtractor << " "
                << dlerror() << std::endl;
        } else {
            m_pSclrTimestampExtractor = reinterpret_cast<TimestampExtractor>(pSclrFunction);
        }

        // save the entry point and close the handle (RTLD_NODELETE) keeps
        // the .so/.dll in  memory:
        if (m_pSclrTimestampExtractor==0 && m_pEvtTimestampExtractor==0) {
            std::cerr << "Fatal error: user provided library with neither"
                      << " timestamp extractor function" << std::endl;
            exit(EXIT_FAILURE);
        }
        // If there is a begin run callback register it too:
        
        void* pBegRun = dlsym(pDllHandle, "onBeginRun");
        if (pBegRun) {
            m_pBeginRunCallback = reinterpret_cast<StateChangeCallback>(pBegRun);
        }

        dlclose(pDllHandle);
        
    }
}


void COutputThread::scheduleApplicationExit(int status)
{
  m_systemControl.scheduleExit(status);
}
