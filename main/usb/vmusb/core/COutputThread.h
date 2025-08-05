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

#ifndef __COUTPUTTHREAD_H
#define __COUTPUTTHREAD_H


#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __THREAD_H
#include <Thread.h>
#ifndef __THREAD_H
#define __THREAD_H
#endif
#endif

// Forward definitions:

struct DataBuffer;
typedef struct _StringsBuffer StringsBuffer;

class  CRingBuffer;
class CSystemControl;

/*!
    This class bridges the gap between the buffer format of the
    VM-USB and the NSCL Buffer format.  We will not bother to
    reformat the data within the buffer, just change the
    header word into an NSCL style buffer.  The data
    we are using will come from a buffer queue.  Elements
    of the buffer queue will have the following format:
\verbatim
    +--------------------------------+
    |  Buffer size                   | 
    +--------------------------------+
    |  Buffer Type                   |
    +--------------------------------+
    | Time stamp                     |
    +--------------------------------+
    |   Buffer contents as described |
    | in section 4.6 of the VM USB   |
    | manual                         |
    +- - - - - - -- - - - - - - - - -+

\endverbatim
    
    -#  Buffer size is gotten from the read from the VM_USB
        and indicates the number of bytes of data
        in the body and its header word (does not include either itself nor
	the buffer type.
    -#  The buffer type is one of the following:
        - 1   Run starting... in this case there will be no VM-USB body.
              and the buffersize will be size of Spectrodaq buffer desired.
        - 2   VM-USB data  note that run end is determined by seeing
              the LB bit in the VM-USB header.
    -# We will generate only the following sorts of NSCL data buffers:
        - 11  Begin run.
        - 12  End Run
        - 1   Physics data
        - 2   Scaler Data.
    -# Time stamp is result of time(2) when the buffer was received from
       the VMUSB.

 \note  This class is a separate thread of execution.
 \note  A global variable: gFilledBuffers is a CBufferQueue that contains
        the data shown above and is used to receive raw data buffers from
	the readout thread.
  \note There is no need to start/stop thread each run.   Once a run is over,
        this thread will simply block on the buffer queue until the next run
        emits the begin run buffer.
  \note The global variable bufferMultiplier determines the size of the output buffer size.
        The output buffersize is bufferMultiplier*26*1024+32  
        the 26*1024 represent the size of a single VM-USB buffer (note that we are assuming
        mixed mode and event spanning on.
*/

class COutputThread  : public Thread
{
public:
	typedef struct _Counters {
		size_t s_triggers;
		size_t s_acceptedTriggers;
		size_t s_bytes;
	} Counters;
	typedef struct _Statistics {
		Counters s_cumulative;
		Counters s_perRun;
	} Statistics;
private:
   // Private data type:
   
   typedef uint64_t (*TimestampExtractor)(void*);
   typedef void     (*StateChangeCallback)();
   
  // Thread local data:
private:
  // These are fetched from the CRun state at start of run.

  uint32_t    m_runNumber;	// Run number;
  std::string m_title;          // Run title
	Statistics  m_statistics;

  // other data:
private:
  int         m_elapsedSeconds;	   /* Seconds into the run. */
  timespec    m_startTimestamp;    //!< Run start time.
  timespec    m_lastStampedBuffer; //!< Seconds into run of last stamped buffer.
  size_t      m_nOutputBufferSize;       //!< size of output buffer in bytes.
                                   //!< determined at the start of a run.
  uint8_t*    m_pBuffer;	   //!< Pointer to the current buffer.
  uint8_t*    m_pCursor;           //!< Where next event goes in buffer.
  size_t      m_nWordsInBuffer;    //!< Number of words already in the buffer.
  std::string m_ringName;           //!< Name of destination ringbuffer.
  CRingBuffer* m_pRing;		    //!< The actual ring in which we put data.
  uint64_t    m_nEventsSeen;        //!< Events processed so far for the physics trigger item.
  unsigned    m_nBuffersBeforeEventCount; //!< Buffers to go before an event count item.
  TimestampExtractor m_pEvtTimestampExtractor;
  TimestampExtractor m_pSclrTimestampExtractor;
  StateChangeCallback m_pBeginRunCallback;
  CSystemControl& m_systemControl;
#ifdef VMUSB_END_OF_EVENT_WORKAROUND
  // We need to keep track of the last header pointer.
  // Since the buffer can be re-allocated we use a buffer offset
    ssize_t    m_LastHeader;        // Signed because 0 is a valid last header.
#endif


  // Constuctors and other canonicals.

public:
  COutputThread(std::string ring, CSystemControl& sysControl);
  virtual ~COutputThread();
private:
  COutputThread(const COutputThread& rhs);
  COutputThread& operator=(const COutputThread& rhs);
  int operator==(const COutputThread& rhs) const;
  int operator!=(const COutputThread& rhs) const;
public:

  virtual void run();		/* Adapt between nextgen and spectrodaq threading model. */
	const Statistics& getStatistics() const;

  // Thread operations are all non-public in fact.. don't want to call them4
  // from outside this class.. only from within the thread.. This includes the
  // thread entry point.

protected:

  virtual int operator()(); //!< Entry point.
	
private:

  DataBuffer& getBuffer();		      // 
  void freeBuffer(DataBuffer&  buffer);	      // 
  void processBuffer(DataBuffer& buffer);     // 
  void processEvents(DataBuffer& buffer);     //
  void processStrings(DataBuffer& buffer, StringsBuffer& strings);

  uint8_t* newOutputBuffer();                //
  void startRun(DataBuffer& buffer); //  
  void endRun(DataBuffer& buffer);   //
  void pauseRun(DataBuffer& buffer);   //  Bug #5882
  void resumeRun(DataBuffer& buffer);  //  Bug #5882
  void event(void* pData);      //
  void scaler(void* pData);	//
  void sendToTclServer(uint16_t* pEvent);
  void attachRing();
  void outputTriggerCount(uint32_t runOffset);
  void getTimestampExtractor();

  bool hasOptionalHeader();

  void scheduleApplicationExit(int status);
};


#endif
