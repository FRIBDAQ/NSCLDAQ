/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     Aaron Chester
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef FDDATASOURCE_H
#define FDDATASOURCE_H

/** 
 * @file  FdDataSource.h
 * @brief Data source of undifferentiated ring items from a file descriptor 
 */

#include "DataSource.h"

/**
 * @class FdDataSource
 * @brief A class taking the file descriptor as a data source. Most commonly 
 * used to construct a data source from stdin.
 */

class FdDataSource : public DataSource
{
private:
    int m_fd; //!< File descrpitor data source.

public:
    /**
     * @brief Constructor. 
     * @param pFactory Pointer to the factory used to get items.
     * @param fd File descriptor open on the data source. The caller owns this,
     *   we don't close it on destruction.
     */
    FdDataSource(ufmt::RingItemFactoryBase* pFactory, int fd);
    /** @brief Destructor. */
    virtual ~FdDataSource();
    /** 
     * @brief Get a ring item from the soruce. Implementation of the mandatory 
     * interface from the base class.
     * @return Pointer to the next ring item from the stream.
     * @retval nullptr If none.
     */
    virtual ufmt::CRingItem* getItem();
};

#endif
