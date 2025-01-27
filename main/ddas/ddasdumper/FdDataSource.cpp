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

/** 
 * @file FdDataSource.cpp
 * @brief Implementation of the file descriptor data source.
 */

#include "FdDataSource.h"

#include <RingItemFactoryBase.h>

using namespace ufmt;

FdDataSource::FdDataSource(RingItemFactoryBase* pFactory, int fd) :
    DataSource(pFactory), m_fd(fd)
{}

FdDataSource::~FdDataSource() {}

CRingItem*
FdDataSource::getItem()
{
    return m_pFactory->getRingItem(m_fd);
}
