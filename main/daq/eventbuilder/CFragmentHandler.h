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
#ifndef CFRAGMENTHANDLER_H
#define CFRAGMENTHANDLER_H

#include <config.h>
#include <fragment.h>
#include <string>
#include <queue>
#include <map>
#include <set>
#include <list>
#include <time.h>
#include <tcl.h>
#include <deque>

#include <cstdint>

#include <limits>

class COutputThread;
class CSortThread;
// Forward definitions:

using namespace ufmt;
namespace EVB = ufmt::EVB;

namespace ufmt::EVB {
  struct _Header;
  typedef struct _FragmentHeader FragmentHeader, *pFragmentHeader;

  struct _FlatFragment;
  typedef struct _FlatFragment FlatFragment, *pFlatFragment;
  
  struct _Fragment;
  typedef struct _Fragment Fragment, *pFragment;
};


typedef std::deque<std::pair<time_t, EVB::pFragment>> EvbFragments;
typedef std::vector<std::pair<time_t, EVB::pFragment>> EvbFragmentsV;

/**
 * @class CFragmentHandler
 *
 *   Singleton class which handles and dispatches fragments.
 *   The main provisions are for:
 *   - A way to add fragments to the input queues.
 *   - A set of observers that can be handed built events when they are
 *     produced.
 *   - A mechanism to force a 'flush-build'  A flush build is one which builds events
 *     until all queues are empty (think end of run or barriers).
 *   - Mechanisms to set the time tolerance of the build and the time window over which events
 *     are accumulated until a build is done.
 *
 * @note There is an assumption that the timestamps will not roll-over
 *       as they are 64 bits wide and even at 100Mhz that provides
 *       for over 50centuries of continuous operation without rollover.
 *       This puts a burden on event sources with narrower widths to 
 *       maintain the upper bits of the timestamp.u
 */
class CFragmentHandler
{
public:
  
private:
  
  
  // Private data types:

  typedef struct _SourceQueue {
    std::uint64_t                                        s_newestTimestamp;
    std::uint64_t                                        s_lastPoppedTimestamp;
    std::uint64_t                                        s_bytesInQ; 
    std::uint64_t                                        s_bytesDeQd;
    std::uint64_t                                        s_totalBytesQd;
    std::uint64_t                                        s_lastTimestamp;
    EvbFragments                                         s_queue;
    std::string                                          s_qid;
    bool                                                 s_xoffed;
    void reset() {
        s_newestTimestamp = 0;
//        s_lastPoppedTimestamp = std::numeric_limits<std::uint64_t>::max();
        s_lastPoppedTimestamp = UINT64_MAX;
        s_bytesInQ = 0;
        s_bytesDeQd  = 0;
        s_totalBytesQd = 0;
        s_lastTimestamp = 0;
        s_xoffed        = false;
    }
    _SourceQueue() : s_qid("")  {  
      reset();
    }
    void setId(const char* qid) {s_qid = qid;}
    

  } SourceQueue, *pSourceQueue;

  typedef std::map<std::uint32_t, SourceQueue> Sources, *pSources;
  typedef std::pair<std::uint32_t, SourceQueue> SourceElement, *pSourceElement;
  typedef std::pair<const std::uint32_t, SourceQueue> SourceElementV;
  typedef struct _BarrierSummary {
    std::vector<std::pair<std::uint32_t, std::uint32_t> > s_typesPresent;
    std::vector<std::uint32_t>                        s_missingSources;
  } BarrierSummary, *pBarrierSummary;
  
  // public data types:
public:
    typedef struct _QueueStatistics {
        std::uint32_t   s_queueId;
        std::uint32_t   s_queueDepth;
        std::uint64_t   s_oldestElement;
        size_t     s_queuedBytes;               // Currently queued bytes
        size_t     s_dequeuedBytes;
        size_t     s_totalQueuedBytes;          // bytes queued since last reset.
        
    } QueueStatistics, *pQueueStatistics;
    
    typedef struct _InputStatistics {
        std::uint64_t s_oldestFragment;
        std::uint64_t s_newestFragment;
        std::uint32_t s_totalQueuedFragments;
        std::uint64_t s_inflight;
        
        std::vector<QueueStatistics> s_queueStats;
        
    } InputStatistics, *pInputStatistics;
  
public:

  // Observer base class:

  class Observer {
  public:
    virtual ~Observer() {}	// So we can chain destructors.
    
    
    
  public:
    virtual void operator()(const EvbFragments& event) = 0; // Passed built event gather maps.
  };

  // Observer for data late conditions:

  class DataLateObserver {
  public:
    DataLateObserver() {}
    virtual ~DataLateObserver() {} // Support destructor chaining.

  public:
    virtual void operator()(const ::EVB::Fragment& fragment,  std::uint64_t newest) = 0;
  };

  // Observer for successful barrier

  class BarrierObserver {
  public:
    BarrierObserver() {}
    virtual ~BarrierObserver() {}
  public:
    virtual void operator()(const std::vector<std::pair<std::uint32_t, std::uint32_t> >& barrierTypes) = 0;
  };

  class PartialBarrierObserver {
  public:
    PartialBarrierObserver() {}
    virtual ~PartialBarrierObserver() {}
  public:
    virtual void operator()(const std::vector<std::pair<std::uint32_t, std::uint32_t> >& barrierTypes, 
			    const std::vector<std::uint32_t>& missingSources) = 0;
  };
  
  class DuplicateTimestampObserver {
    public:
        DuplicateTimestampObserver() {}
        virtual ~DuplicateTimestampObserver() {}
    public:
        virtual void operator()(std::uint32_t sourceId, std::uint64_t timestamp) = 0;
  };
  
  class FlowControlObserver {
    public:
        virtual void Xon() = 0;
        virtual void Xon(std::string qid) = 0;
        virtual void Xoff() = 0;
        virtual void Xoff(std::string qid) = 0;
  };
  
  class NonMonotonicTimestampObserver {
    public:
        virtual void operator()(
            unsigned sourceid, std::uint64_t priorTimestamp, std::uint64_t thisTimestamp
        ) = 0;
  };


  // Queue statistics accumulator:
  
private:
    class QueueStatGetter {
    private:
      std::uint32_t                     m_nTotalFragments;
      std::vector<QueueStatistics> m_Stats;
    public:
      QueueStatGetter();
      void operator()(SourceElementV& source);
      std::uint32_t totalFragments();
      std::vector<QueueStatistics> queueStats();
    };

  

private:
  static CFragmentHandler* m_pInstance;	     //< The unique instance of this class.
private:
  std::uint64_t                     m_nOldest;              //!< Oldest fragment seen in terms of ticks.
  std::uint64_t                     m_nNewest;              //!< Newest fragment seen in terms of ticks.
  std::uint64_t                     m_nMostRecentlyPopped;    //!< Most recently popped fragment in ticks.

  time_t                       m_nBuildWindow;
  time_t                       m_nNow;
  time_t                       m_nOldestReceived;
  time_t                       m_nMostRecentlyEmptied;
  time_t                       m_nStartupTimeout;   //!< N seconds to wait before flushing (dflt=2)


  std::uint32_t                     m_nFragmentsLastPeriod; //!< # fragments in last flush check interval.

  
  std::list<DataLateObserver*>               m_DataLateObservers;
  std::list<BarrierObserver*>                m_goodBarrierObservers;
  std::list<PartialBarrierObserver*>         m_partialBarrierObservers;
  std::list<DuplicateTimestampObserver*>     m_duplicateTimestampObservers;
  std::list<FlowControlObserver*>            m_flowControlObservers;
  std::list<NonMonotonicTimestampObserver*>  m_nonMonotonicTsObservers;

  Sources                      m_FragmentQueues;
  bool                         m_fBarrierPending;      //< True if at least one queue has a barrier event.
  std::set<std::uint32_t>           m_liveSources;	       //< sources that are live.
  std::map<std::string, std::list<std::uint32_t> > m_socketSources; //< Each socket name has a list of source ids.
  std::map<std::string, std::list<std::uint32_t> > m_deadSockets;   //< same as above but for dead sockets.
  
  Tcl_TimerToken               m_timer;
  
  size_t                       m_nXonLimit;
  size_t                       m_nXoffLimit;
  size_t                       m_nPerQXonLimit;
  size_t                       m_nPerQXoffLimit;
  bool                         m_fXoffed;
  size_t                       m_nTotalFragmentSize;

  
  COutputThread&               m_outputThread;
  CSortThread&                 m_sorter;

  // Canonicals/creationals. Note that since this is a singleton, construction
  // is private.

private:
  CFragmentHandler();
  ~CFragmentHandler();		// No need to be virtual since you can't derive this.

  // These are just plain illegal:

  CFragmentHandler(const CFragmentHandler&);
  CFragmentHandler& operator=(const CFragmentHandler&);
  int operator==(const CFragmentHandler&) const;
  int operator!=(const CFragmentHandler&) const;

  // The only public creational is getInstance:

public:
  static CFragmentHandler* getInstance();

  // here are the operations we advertised:

public:
  void addFragments(size_t nSize, const EVB::FlatFragment* pFragments);

  void setBuildWindow(time_t windowWidth);
  time_t getBuildWindow() const;

  void setStartupTimeout(time_t duration);
  time_t getStartupTimeout() const;
  
  void setXoffThreshold(size_t nBytes);
  void setXonThreshold(size_t nBytes);

  void setPerQXoffThreshold(size_t nSize);
  void setPerQXonThreshold(size_t nSize);
  
  
  // Observer management:
  
  COutputThread* getOutputThread() {
    return &m_outputThread;
  }
  CSortThread* getSortThread() {
    return &m_sorter;
  }
public:

  void addObserver(Observer* pObserver);
  void removeObserver(Observer* pObserver);

  void addDataLateObserver(DataLateObserver* pObserver);
  void removeDataLateObserver(DataLateObserver* pObserver);

  void addBarrierObserver(BarrierObserver* pObserver);
  void removeBarrierObserver(BarrierObserver* pObserver);

  void addPartialBarrierObserver(PartialBarrierObserver* pObserver);
  void removePartialBarrierObserver(PartialBarrierObserver* pObserver);

  
  void addDuplicateTimestampObserver(DuplicateTimestampObserver* pObserver);
  void removeDuplicateTimestampObserver(DuplicateTimestampObserver* pObserver);
  
  
  void addFlowControlObserver(FlowControlObserver* pObserver);
  void removeFlowControlObserver(FlowControlObserver* pObserver);
  
  void addNonMonotonicTimestampObserver(
    NonMonotonicTimestampObserver* pObserver
  );
  void removeNonMonotonicTimestampobserver(
    NonMonotonicTimestampObserver* pObserver
  );
  
  
  // queue management.

  void flush();
  
  // Get/set state of the queues etc.

  InputStatistics getStatistics();
  void createSourceQueue(std::string socketName, std::uint32_t id);  
  void markSourceFailed(std::uint32_t id);
  void markSocketFailed(std::string sockName);
  void reviveSocket(std::string sockName);
  void resetTimestamps();
  void clearQueues();
  void observe(EvbFragments& event); // pass built events on down the line.
  
  void abortBarrierProcessing();     // If barriers are inprogress declare incomplete now.

  // utility methods:

private:
  void flushQueues(bool completely=false);
  std::pair<time_t, ::EVB::pFragment>* popOldest();
  void   dataLate(const ::EVB::Fragment& fragment);		    // Data late handler.
  void   addFragment(const EVB::FlatFragment* pFragment);
  size_t totalFragmentSize(const EVB::FragmentHeader* pHeader);
  bool   queuesEmpty();
  bool   noEmptyQueue();

  BarrierSummary generateBarrier(
      EvbFragments& outputList
  );
  void generateMalformedBarrier(
    EvbFragments& outputList
  );
  //   void generateCompleteBarrier(std::vector<EVB::pFragment>& ouptputList); 156
  
  void goodBarrier(EvbFragments& outputList);
  void partialBarrier(
      std::vector<std::pair<std::uint32_t, std::uint32_t> >& types, 
      std::vector<std::uint32_t>& missingSources
  );
  void observeGoodBarrier(
    std::vector<std::pair<std::uint32_t, std::uint32_t> >& types
  );
  void observeDuplicateTimestamp(std::uint32_t sourceId, std::uint64_t timestamp);
  void observeOutOfOrderInput(unsigned sourceId, std::uint64_t prior, std::uint64_t bad);
  void Xoff();
  void Xon();
  
  void findOldest();
  size_t countPresentBarriers() const;


  SourceQueue& getSourceQueue(std::string sockName, std::uint32_t id);
  void XoffQueue(SourceQueue& q);
  void XonQueue(SourceQueue&  q);

  void checkBarrier(bool complete);
  time_t oldestBarrier();

  size_t inFlightFragmentCount();
  void checkXoff();
  void checkXon();
  
  uint64_t findStampMark();
  uint64_t oldestStamp(EvbFragments& q);


  void DequeueUntilStamp( 
      EvbFragments& result,
      EvbFragments& q,
      uint64_t timestamp
  );
  void DequeueUntilAbsTime(
    EvbFragments& result,
    EvbFragments& q,
    time_t time
  );

  void updateQueueStatistics(
    SourceQueue& queue,
    EvbFragments& justDequeued
  );

  void insertFragment(time_t clockTime, EVB::pFragment pFrag, SourceQueue& dest);
  void handleDequeuedFragments(
    Sources::iterator& p, EvbFragments& partialSort,
    std::deque<EvbFragments*>* pFrags,
    std::list<std::pair<SourceQueue*, EvbFragments*>>& statcopy
  );
  // Static private methods:

  static void IdlePoll(ClientData obj);
  
};


#endif
