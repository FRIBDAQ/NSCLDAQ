
#include <COneShotHandler.h>
#include <COneShotException.h>
#include <ErrnoException.h>
#include <limits>
#include <sstream>
#include <iostream>


namespace DAQ {

static uint32_t defaultRunNumber = std::numeric_limits<uint32_t>::max();


/*!
 * \brief Constructor
 *
 * \param ntrans    number of begin run type items to observe
 * \param beginType the type of item to consider a begin run
 * \param endType   the type of item to consider an end
 * \param types     the types to keep counters for
 */
COneShotHandler::COneShotHandler(int ntrans, uint32_t beginType,
                                 uint32_t endType, const std::vector<uint32_t>& types)
  : m_nExpectedSources(ntrans),
  m_stateCounts(),
  m_cachedRunNo(defaultRunNumber),
  m_complete(false),
  m_beginType(beginType),
  m_endType(endType)
{
    for (auto type : types) {
        m_stateCounts[type] = 0;
    }
}

/*!
 * \brief Specify how many begin runs should be expected
 *
 * \param transitions   the number of begin run items to expect
 */
void COneShotHandler::setExpectedTransitions(int transitions)
{
    m_nExpectedSources = transitions;
}

/**! Process a new type and run number from a state change
*
* Check for error case:
* - the run number changes when in the middle of the run.
*
* @param type       type of data item
* @param runNumber  a run number
* 
* @throws CErrnoException when run number changes unexpectedly
*/
void COneShotHandler::update(uint32_t type, uint32_t runNumber)
{

  // if we have already reached our limit, throw
  if (m_complete) {
    throw COneShotException("COneShotHandler::update(uint32_t,uint32_t)",
                            "Unexpected, extra state change item");
  }

  if (validType(type)) {
    updateState(type, runNumber);
  }
}

/*!
 * \brief The guts of the logic for this class
 *
 * \param type  type of data item
 * \param run   a run number
 *
 * If type is a begin run type and it is the first observed, the counters
 * are cleared and the run number is cached. If no begin run items have arrived
 * and a type arrives different from a begin run item, nothin is done. If
 * a begin run has arrived, the counter for the type is incremented.
 *
 * \throws COneShotException if the run number passed in differs from
 *                           the cached run number
 * \throws COneShotException if type is a begin run type and it is one
 *                           more begin run than was expected
 */
void COneShotHandler::updateState(uint32_t type, uint32_t run)
{
  // Check that the run number hasn't changed unexpectedly
  if (run != m_cachedRunNo && m_cachedRunNo != defaultRunNumber) {
    throw COneShotException("COneShotHandler::updateState(uint32_t, uint32_t)",
        "More begin runs detected than expected");
  }

  // Only do something if we understand the state change
  if (type==m_beginType) {
    if (waitingForBegin()) {
      // Handle the first begin run specially
      initialize(run);

      ++m_stateCounts[type];

    } else if (getCount(m_beginType) >= m_nExpectedSources) {
      // Handle if there are too many BEGIN_RUNS 
      std::ostringstream errmsg;
      errmsg << "Too many begin runs observed. Expecting only " 
        << m_nExpectedSources;
      throw COneShotException("COneShotHandler::updateState(uint32_t, uint32_t)",
          errmsg.str());

    }
  } else {
    if (! waitingForBegin()) { 
      ++m_stateCounts[type]; 
    }
  }

  m_complete = (getCount(m_endType)==m_nExpectedSources);
}

void COneShotHandler::initialize(uint32_t runNumber)
{
  // Set the run number
  m_cachedRunNo = runNumber;
  
  // Reset the counters
  clearCounts();

}


/**! Check if the run has been completed */
bool COneShotHandler::complete() const
{
  return m_complete;
}

void COneShotHandler::reset()
{
  clearCounts();
  m_complete = false;
}

bool COneShotHandler::waitingForBegin() const
{
  return (getCount(m_beginType)==0);
}

/**! Get the number of state change items already seen
*
* @param key the type of the state change item
* @return the number of valid state changes of type key observed 
*/
uint32_t COneShotHandler::getCount(uint32_t key) const
{
  uint32_t count=0;
  try {
    std::map<uint32_t,uint32_t>::const_iterator it;

    it = m_stateCounts.find(key);
    count = it->second;

  } catch (std::exception&) {}

  return count;
}


/**! Verify that type corresponds to a type of interest
*
* @param type the ring item type
* @return true if type is a type of interest
*/
bool COneShotHandler::validType(uint32_t type) const
{
  std::map<uint32_t,uint32_t>::const_iterator it, itend;
  it = m_stateCounts.begin();
  itend = m_stateCounts.end();
  
  // Check that this is in fact one of the 4 supported transitions
  bool foundMatch=false;
  while (it!=itend && !foundMatch) {
    foundMatch = (type == it->first);
    ++it;   
  }

  return foundMatch;
}

/**! Reset the counters */
void COneShotHandler::clearCounts()
{
  std::map<uint32_t,uint32_t>::iterator it, itend;
  it = m_stateCounts.begin();
  itend = m_stateCounts.end();
  while (it!=itend) {
    it->second = 0; 
    ++it;
  }
}


} // end DAQ
