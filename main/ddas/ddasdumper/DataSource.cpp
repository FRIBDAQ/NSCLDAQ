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
 * @file  DataSource.cpp
 * @brief Implementation of the non pure virtual methods of DataSource.
 */

#include "DataSource.h"

#include <RingItemFactoryBase.h>

using namespace ufmt;

/**
 * @details
 * Just saves the factory pointer - note that the format factory selector
 * maintains ownership of the factory.
 */
DataSource::DataSource(RingItemFactoryBase *pFactory) : m_pFactory(pFactory) {}

/**
 * @details
 * Pretty simple: Set a new factory e.g., if the format changes. Note that
 * since we do not own the factory we do not delete the old one.
 */
void DataSource::setFactory(RingItemFactoryBase *pFactory) {
  m_pFactory = pFactory;
}
