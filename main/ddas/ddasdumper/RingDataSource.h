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
#ifndef RINGDATASOURCE_H
#define RINGDATASOURCE_H

/** @file:  RingDataSource.h
 *  @brief: Provide ring items from a ringbuffer.
 */

#include "DataSource.h"

class CRingBuffer;

class RingDataSource : public DataSource
{
private:
    CRingBuffer& m_ring;
public:
    RingDataSource(ufmt::RingItemFactoryBase* pFact, CRingBuffer& ring);
    virtual ~RingDataSource();
    virtual ufmt::CRingItem* getItem();
};

#endif
