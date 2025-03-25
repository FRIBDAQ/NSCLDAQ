/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013-2018
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
# @file   CSortThread.cpp
# @brief  Implementation of thread to sort together extents of fragment queues
# @author <fox@nscl.msu.edu>
*/

#include "CSortThread.h"
#include "COutputThread.h"
#include <map>

// Timestamp comparison for sorted merge:

static bool
TsCompare(std::pair<time_t, EVB::pFragment>& q1, std::pair<time_t, EVB::pFragment>& q2)
{
  return q1.second->s_header.s_timestamp < q2.second->s_header.s_timestamp ;
}


/**
 * construction
 *   Just need to salt away the fragment handler:
 */
CSortThread::CSortThread() : Thread(*(new std::string("SortThread"))),
m_pHandler(0), m_nQueuedFrags(0)
{}

/**
 * destruction
 *    Clear the buffer queue.  The assumption is that no data is coming in
 *    in other threads, so once it's empty we're done.
 */
CSortThread::~CSortThread()
{
    clearBufferQueue();

}

/**
 * run
 *   Thread entry point
 *   Logic is pretty simple:
 *   - get a set of fragments from the buffer queue
 *   - merge those fragments into a single fragment list
 *   - Queue that single fragment list to the output thread
 *   - Release the storage taken by the fragments and their input lists.
 *   - Rinse and repeat.
 */
void
CSortThread::run()
{
   
    while (1) {
        Fragments* newData = dequeueFragments();
        if (!m_pHandler)
            m_pHandler = CFragmentHandler::getInstance(); // We know frag handler construction is done.
        FragmentList* mergedFrags = new FragmentList;  // Deleted by output thread.
        merge(*mergedFrags, *newData);
        
        
        //m_pHandler->observe(*mergedFrags);
        
        COutputThread* pOutput = m_pHandler->getOutputThread();
        releaseFragments(*newData); 
        m_nQueuedFrags -= mergedFrags->size();
        pOutput->queueFragments(mergedFrags);

    }
}
/**
 * queueFragments
 *    Called by the fragment handler thread to queue fragments for processing
 *    by the sort thread
 *
 * @param frags - References a dynamically allocated Fragments object that
 *     will be processed in this thread.
 */
void
CSortThread::queueFragments(Fragments& frags)
{
    for (int i = 0; i < frags.size(); i++) {
      m_nQueuedFrags += frags[i]->size();
    }
    m_fragmentQueue.queue(&frags);
  
}
/**
 * dequeueFragments
 *    Returns the least recently queued fragment.  Blocks if needed.
 *  @return Fragments*
 */
CSortThread::Fragments*
CSortThread::dequeueFragments()
{
    return m_fragmentQueue.get();
}
/**
 * merge
 *    Given a vector of sorted lists, merges them into a single sorted vector.
 *    Uses the minheap merge algorithm.  Note that lists will be empty when we're
 *    done.
 *
 *  @param[out] result - list into which the fragments will be merged
 *                      (could be empty).
 *  @param[in] lists - vector of fragment lists to merge
 */
void
CSortThread::merge(FragmentList& result, Fragments& lists)
{
  // If there's one list, we only need to append::
  
  if (lists.size() == 1) {
    merge(result, *(lists[0]));
    return;
  }
  // If there's two lists we have the fast(?) two way merge:
  
  if (lists.size() == 2) {
    merge(result, *(lists[0]), *(lists[1]));
    return;
  }
  
  // Build the minheap for the merge.  Note that the timestamp is a minimum
  // at the front of each queue.  We make the minheap a map keyed on the
  // timestamp that contains a pointer to the dequeue from which the
  // timestamp came.

  
  std::multimap<uint64_t, FragmentList*> minheap;  // dup timestamps are possible.
  for (int i =0; i < lists.size(); i++) {
    uint64_t ts = lists[i]->front().second->s_header.s_timestamp;
    minheap.emplace(std::make_pair(ts, lists[i]));
  }
  // Once the map size is 2 we can drop through to the special 2-way merge
  // which in turn drops into the append when it's down to one list:
  
  while(minheap.size() > 2) {
    auto q = minheap.begin();
    result.push_back(q->second->front());
    q->second->pop_front();
    minheap.erase(q);
    
    // If there are more elements in q.second - put its timestamp into the minheap:
    
    if (!q->second->empty()) {
      uint64_t ts = q->second->front().second->s_header.s_timestamp;
      minheap.emplace(std::make_pair(ts, q->second));
    }
  }
  if (minheap.size() == 2) {         // last two:
    merge(result, *(minheap.begin()->second), *(minheap.rbegin()->second));
    return;
    
  } else if (minheap.size() == 1) {   // Don't think this is possible.
    merge(result, *(minheap.begin()->second));  
  }

}
/**
 * merge -two way
 *    Merge data from two lists of fragments.  This can be done more efficiently
 *    than the minheap by direct comparison of frons of queues.
 *    When we're down to the last queue we just do the one-way 'merge'.
 *
 * @param result -reference to the output list.
 * @param list1 - Reference to the first list oif fragments.
 * @param list2 - References the second list of fragments.
 */
void
CSortThread::merge(FragmentList& result, FragmentList& list1, FragmentList& list2)
{
  auto f1 = list1.front();
  auto f2 = list2.front();
  while(1) {
    
    if (f1.second->s_header.s_timestamp < f2.second->s_header.s_timestamp) {  // f1 goes.
      result.push_back(f1);
      list1.pop_front();
      f1 = list1.front();
      if (list1.empty()) {          // Only lis2 left.
        merge(result, list2);
        return;
      }
    } else {             // F2 goes:
      
      result.push_back(f2);
      list2.pop_front();
      f2 = list2.front();
      if (list2.empty()) {             // only list1 left.
        merge(result, list1);
        return;
      }
    }
  }
}
/**
 * merge - 1-way.
 *   Just append the remaining fragments to the result. queue.
 *
 *   @result - the resulting sorted list.
 *   @list   - the remaining fragmnets.
 */
void
CSortThread::merge(FragmentList& result, FragmentList& list)
{
  result.insert(result.end(), list.begin(), list.end());
}
/**
 * clearBufferQueue
 *    Empties out the buffer queue and
 *    destroys all data in it.
 */
void
CSortThread::clearBufferQueue()
{
    Fragments* pFrags;
    while(m_fragmentQueue.getnow(pFrags)) {
        releaseFragments(*pFrags);
    }
}
/**
 * releaseFragments
 *    Gets rid of all fragment lists and then the fragments object itself.
 *    Note that by now these are empty.
 *
 * @parm frags - fragments to get rid of.
 */
void
CSortThread::releaseFragments(Fragments& frags)
{
    //Note the elements of the fragments dequeue are, themselves new'd into being.
    // and must be deleted.  See Issue #231
    // The elements of each dequeue are pointers that are passed on to the output thread
    // and deleted there to avoid copying.
    for (auto p : frags) {
      delete p;
    }
    delete &frags;
}
