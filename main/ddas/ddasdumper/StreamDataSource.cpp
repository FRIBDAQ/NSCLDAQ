/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  StreamDataSource.cpp
 *  @brief:  Implement the stream data source
 */
#include "StreamDataSource.h"
#include <RingItemFactoryBase.h>

using namespace ufmt;

/**
 * constructor
 *    @param pFactory - factory for ring items.
 *    @param str     - references stream from which to get ring items.
 */

StreamDataSource::StreamDataSource(RingItemFactoryBase* pFactory, std::istream& str) :
    DataSource(pFactory), m_str(str)
{}

/**
 * destructor
 */
StreamDataSource::~StreamDataSource() {}

/**
 * getItem
 *  @return CRingItem*  - pointer to the next ring item from the stream.  null if none.
 */
CRingItem*
StreamDataSource::getItem()
{
    return m_pFactory->getRingItem(m_str);
}
