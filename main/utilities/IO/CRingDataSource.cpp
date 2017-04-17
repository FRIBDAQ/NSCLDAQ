/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

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
#include "CRingDataSource.h"

#include <CRingBuffer.h>
#include <CAllButPredicate.h>
#include <URL.h>
#include <CRemoteAccess.h>
#include <CTimeout.h>

#include <iostream>

using std::vector;
using std::string;


namespace DAQ {


///////////////////////////////////////////////////////////////////////////////////////
//
// Constructors and canonicals:
//

/*!
  Create a ring data source.
  \param url     - URL describing the ring we shoud connect to.
  \param sample  - List of item types that should be sampled.
  \param exclude - List of item types that should not be accepted from the ring.
*/
CRingDataSource::CRingDataSource(const URL &url, vector<uint16_t> sample, vector<uint16_t> exclude) :
  CDataSource(),
  m_pRing(0), 
//  m_pPredicate(0),
  m_url(*(new URL(url)))
{
  openRing();
//  makePredicate(sample, exclude);
}

/*!
  Destructor:
*/
CRingDataSource::~CRingDataSource()
{
  delete m_pRing;
//  delete m_pPredicate;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Required interfaces:


size_t CRingDataSource::availableData() {
    return m_pRing->availableData();
}

size_t CRingDataSource::peek(char *pBuffer, size_t nBytes)
{
    return m_pRing->peek(pBuffer, nBytes);
}

void CRingDataSource::ignore(size_t nBytes)
{
    m_pRing->skip(nBytes);
}

size_t CRingDataSource::tell() const
{
    return 0;
}


size_t CRingDataSource::timedRead(char* pBuffer, size_t nBytes, const CTimeout& timeout)
{
    using namespace std::chrono;
    auto nSeconds = duration_cast<seconds>(timeout.getRemainingTime());
    return m_pRing->get(pBuffer, nBytes, nSeconds.count());
}


CRingBuffer& CRingDataSource::getRing()
{
    return *m_pRing;
}

const CRingBuffer& CRingDataSource::getRing() const
{
    return *m_pRing;
}


void CRingDataSource::setPredicate(std::vector<uint16_t> &sample,
                                   std::vector<uint16_t> &exclude)
{
//    auto pOldPredicate = m_pPredicate;
//    m_pPredicate = nullptr;

//    try {
//        makePredicate(sample, exclude);
//        delete pOldPredicate;
//    } catch (...) {
//        if (m_pPredicate) delete m_pPredicate;

//        m_pPredicate = pOldPredicate;
//    }


}

///////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities:
//

/*
** Opens the ring and fills in m_pRing from it.
*/
void
CRingDataSource::openRing()
{
  string uri = string(m_url);
  m_pRing    = CRingAccess::daqConsumeFrom(uri);
}
/*
** Make the predicate from the lists of sampled and excluded item types:
*/
void
CRingDataSource::makePredicate(vector<uint16_t> sample, vector<uint16_t> exclude)
{
//  m_pPredicate = new CAllButPredicate;
//  for (int i=0; i < exclude.size(); i++) {
//    m_pPredicate->addExceptionType(exclude[i]);
//  }
//  for (int i=0; i < sample.size(); i++) {
//    m_pPredicate->addExceptionType(sample[i], true);
//  }
}


} // end DAQ
