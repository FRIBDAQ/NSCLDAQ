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
#ifndef FDDATASOURCE_H
#define FDDATASOURCE_H
/** @file:  FdDataSource.cpp
 *  @brief: Data source of undifferentiated ring items from a file descriptor
 *  
 */
#include "DataSource.h"

class FdDataSource : public DataSource
{
private:
    int m_fd;                         // File descrpitor data source.
public:
    FdDataSource(ufmt::RingItemFactoryBase* pFactory, int fd);
    virtual ~FdDataSource();
    virtual ufmt::CRingItem* getItem();
};

#endif
