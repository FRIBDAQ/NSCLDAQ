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
#ifndef STREAMDATASOURCE_H
#define STREAMDATASOURCE_H
/** @file:  StreamDataSource.h
 *  @brief: Defines a class that gets ring items from a stream.
 */
#include "DataSource.h"
#include <istream>

class StreamDataSource : public DataSource
{
private:
    std::istream& m_str;
public:
    StreamDataSource(ufmt::RingItemFactoryBase* pFactory, std::istream& str);
    virtual ~StreamDataSource();
    virtual ufmt::CRingItem* getItem();
};


#endif
