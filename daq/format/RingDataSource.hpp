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

#include <CRingBuffer.h>
#include <CAllButPredicate.h>
#include <URL.h>
#include <CRemoteAccess.h>
#include <CRingItem.h>

namespace NSCLDAQ 
{

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
  template<class T>
    RingDataSource<T>::RingDataSource(URL& url, 
                                      std::vector<uint16_t> sample, 
                                      std::vector<uint16_t> exclude) :
      m_pRing(0), 
      m_pPredicate(0),
      m_url(*(new URL(url)))
  {
    openRing();
    makePredicate(sample, exclude);
  }

  /*!
Destructor:
*/
  template<class T>
    RingDataSource<T>::~RingDataSource()
    {
      delete m_pRing;
      delete m_pPredicate;
    }

  ///////////////////////////////////////////////////////////////////////////////////////////////
  //
  //  Required interfaces:

  /*!
    Returns the next qualified item from the ring.  The predicate selects the
    appropriate item.
    \return CRingItem*
    \retval pointer to the next item that is returned from the ring. This is dynamically
    allocated and it's up to the caller to delete it.
    */
  template<class T> 
    T* RingDataSource<T>::getItem()
    {
      return nullptr;
    }

  ///////////////////////////////////////////////////////////////////////////////////////////
  //
  // Utilities:
  //

  /*
   ** Opens the ring and fills in m_pRing from it.
   */
  template<class T> 
    void RingDataSource<T>::openRing()
    {
      std::string uri = std::string(m_url);
      m_pRing    = CRingAccess::daqConsumeFrom(uri);
    }
  /*
   ** Make the predicate from the lists of sampled and excluded item types:
   */
  template<class T> 
    void RingDataSource<T>::makePredicate(std::vector<uint16_t> sample, 
                                          std::vector<uint16_t> exclude)
    {
      m_pPredicate = new CAllButPredicate;
      for (int i=0; i < exclude.size(); i++) {
        m_pPredicate->addExceptionType(exclude[i]);
      }
      for (int i=0; i < sample.size(); i++) {
        m_pPredicate->addExceptionType(sample[i], true);
      }
    }

  // specialization of getItem for CRingItems.
  template<> 
    CRingItem* RingDataSource<CRingItem>::getItem()
    {
      CRingItem* pItem = CRingItem::getFromRing(*m_pRing, *m_pPredicate);
      return pItem;
    }

} // end of NSCLDAQ namespace
