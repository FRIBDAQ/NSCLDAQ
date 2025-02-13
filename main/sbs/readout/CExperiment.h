#ifndef CEXPERIMENT_H
#define CEXPERIMENT_H
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

#include <stdint.h>
#include <string>
#include  <time.h>
#include <tcl.h>
#include <TCLObject.h>
#include <CElapsedTime.h>

#include <TCLObject.h>
#include <CElapsedTime.h>

class CTCLInterpreter;



// Forwared definitions:

class RunState;			
class CScalerBank;
class CCompoundEventSegment;
class CBusy;
class CEventTrigger;
class CTriggerLoop;
class CRingBuffer;
class CEventSegment;
class CScaler;
class CRingItem;

struct gengetopt_args_info;

/*!
  This class controls the experiment.  The experiment consists of two major
  components, scaler lists and events.  Both are organized as
  Composites which are the root of a tree of modules.

  The experiment holds the ring buffer pointer/handle and 
  takes care of moving data from local buffers into the ring buffer.
  Event data are acquired in a local buffer whose size can be set, and which
  must be big enough to hold a full event.


*/
class CExperiment // (final).
{
public:
	// public data structurs:
	
	typedef struct _Counters {
		size_t s_triggers;
		size_t s_acceptedTriggers;
		size_t s_bytes;
	} Counters;
	typedef struct _Statistics {
		Counters s_cumulative;
		Counters s_perRun;
	} Statistics;
	
  // local data:

private:
  CRingBuffer*           m_pRing;	//!< Where event data eventually goes.
  RunState*              m_pRunState;   //!< state information about the run.
  CScalerBank*           m_pScalers;    //!< The scalers to read.
  CCompoundEventSegment* m_pReadout;    //!< The event segment root.
  CBusy*                 m_pBusy;       //!< The busy module.
  CEventTrigger*         m_pEventTrigger; //!< trigger for events.
  CEventTrigger*         m_pScalerTrigger; //!< scaler trigger.
  CTriggerLoop*          m_pTriggerLoop; //!< Thread that runs the trigger.

  size_t                 m_nDataBufferSize; //!< current event buffer size.

  double                 m_nLastScalerTime; // last scaler time in ms since epoch (usually).
  uint64_t               m_nEventsEmitted;
                                             
  uint64_t                m_nEventTimestamp;
  uint32_t                m_nSourceId;
  uint32_t                m_nRunNumber;
  bool                    m_needHeader;
  uint16_t                m_nDefaultSourceId;
  bool                    m_useBarriers;
  bool                    m_fHavemore;      // If true readout has more events.
  bool                    m_fWantZeroCopy;  // Want zero copy ring items.
  bool                    m_fNeedVmeLock;
	CElapsedTime            m_runTime;
	
	Statistics             m_statistics;
  bool                   m_incrementalScalers;

  // Canonicals:

public:
  CExperiment(std::string ringName,
	      size_t      eventBufferSize = 4096, bool barriers = true);
  ~CExperiment();		// This is a final class.
private:
  CExperiment(const CExperiment&);
  CExperiment& operator=(const CExperiment&);
  int operator==(const CExperiment&) const;
  int operator!=(const CExperiment&) const;

  // Selectors needed by other classes:

public:
  CEventTrigger*      getEventTrigger();
  CEventTrigger*      getScalerTrigger();

  // Member functions:

public:
  void   enableVmeLock() {m_fNeedVmeLock = true;}
  void   disableVmeLock()  { m_fNeedVmeLock = false;}
  void   setDefaultSourceId(unsigned sourceId);
  void   setBufferSize(size_t newSize);
  size_t getBufferSize() const;
  bool   isIncremental() const;
  void   setIncremental(bool state);
  void   Start(bool resume=false);
  void   Stop(bool pause=false);
  void   AddEventSegment(CEventSegment*    pSegment);
  void   RemoveEventSegment(CEventSegment* pSegment);
  void   AddScalerModule(CScaler*    pModule);
  void   RemoveScalerModule(CScaler* pModule);

  void   EstablishTrigger(CEventTrigger* pEventTrigger);
  void   setScalerTrigger(CEventTrigger* pScalerTrigger);
  void   EstablishBusy(CBusy*            pBusyModule);
  void   ReadEvent();
  void   TriggerScalerReadout();
  void   DocumentPackets();
  void   ScheduleRunVariableDump();
  void   ScheduleEndRunBuffer(bool pause);
  CRingBuffer* getRing() {
    return m_pRing;
  }
  void setTimestamp(uint64_t stamp);
  void setSourceId(uint32_t id);
  uint32_t getSourceId();
  uint32_t getRunNumber();
  void triggerFail(std::string msg);
  void syncEndRun(bool pause);
  void haveMore() { m_fHavemore = true; }
  void setZeroCopy(bool state) {m_fWantZeroCopy = state;}
	const Statistics& getStatistics() const {return m_statistics; } 
private:
  void readScalers();
	void readEvent(CRingItem& item);
	
  static int HandleEndRunEvent(Tcl_Event* evPtr, int flags);
  static int HandleTriggerLoopError(Tcl_Event* evPtr, int flags);
  static CTCLObject createCommand(
    CTCLInterpreter* pInterp, const char* verb, std::string parameter);
	void clearCounters(Counters& c);
};

#endif
