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
#ifndef DATASOURCE_H
#define DATASOURCE_H

/** @file  DataSource.h
 *  @brief Works with factories to provide a data source for undifferntiaed ring items.
 * @note Abstract base class for FdDataSource, StreamDataSource and RingDataSource
 */

namespace ufmt {
    class CRingItem;
    class RingItemFactoryBase;
}
    
/**
 * @class DataSource
 *    Pure abstract data source which uses a factory's ring item getters to
 *    provide ring item from a data source.  Since the factory provides this,
 *    we'll need concrete classes:
 *    - FdDataSource - give data from a file descriptor.
 *    - StreamDataSource -give data from a stream.
 *    - RingDataSource -give data from a ringbuffer.
 */
class DataSource {
protected:
    ufmt::RingItemFactoryBase* m_pFactory;
public:
    DataSource(ufmt::RingItemFactoryBase* pFactory);
    virtual ~DataSource();
    virtual ufmt::CRingItem* getItem() = 0;
    void setFactory(ufmt::RingItemFactoryBase* pFactory);
};


#endif
