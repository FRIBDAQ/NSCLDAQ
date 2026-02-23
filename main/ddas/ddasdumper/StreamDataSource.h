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
 * @file  StreamDataSource.h
 * @brief Defines a class that gets ring items from a stream.
 */

#ifndef STREAMDATASOURCE_H
#define STREAMDATASOURCE_H

#include "DataSource.h"

#include <istream>

/**
 * @class StreamDataSource
 * @brief A class taking the input stream as a data source. Most commonly
 * used to construct a data source from an NSCLDAQ event file on disk.
 */

class StreamDataSource : public DataSource {
private:
  std::istream &m_str; //!< Stream name to read ring items from.
public:
  /**
   * @brief Constructor.
   * @param pFactory Pointer to the ring item factory.
   * @param str References the stream from which to get ring items.
   */
  StreamDataSource(ufmt::RingItemFactoryBase *pFactory, std::istream &str);
  /** @brief Destructor. */
  virtual ~StreamDataSource();
  /**
   * @brief Get a ring item from the soruce. Implementation of the mandatory
   * interface from the base class.
   * @return Pointer to the next ring item from the stream.
   * @retval nullptr If none.
   */
  virtual ufmt::CRingItem *getItem();
};

#endif
