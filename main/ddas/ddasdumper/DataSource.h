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

#ifndef DATASOURCE_H
#define DATASOURCE_H

/**
 * @file  DataSource.h
 * @brief Works with factories to provide a data source for undifferentiated
 * ring items.
 */

namespace ufmt {
class CRingItem;
class RingItemFactoryBase;
} // namespace ufmt

/**
 * @class DataSource
 * @brief Abstract base class for DDAS data sources.
 * @details
 * Pure abstract data source which uses a factory's ring item getters to
 * provide ring items from a data source. Since the factory provides this,
 * we'll need concrete classes:
 * - FdDataSource: give data from a file descriptor.
 * - StreamDataSource: give data from a stream.
 * @note Neither of these data sources supports reading directly from a ring
 * buffer, as the format library is unaware of those NSCLDAQ classes. To read
 * data from a ringbuffer you can create a file descriptor data source and
 * read data from stdin i.e. `ringselector | ddasdumper -`.
 */

class DataSource {
protected:
  ufmt::RingItemFactoryBase *m_pFactory; //!< Ptr to our ring item factory.

public:
  /**
   * @brief Constructor.
   * @param pFactory Pointer to concrete ring item factory.
   */
  DataSource(ufmt::RingItemFactoryBase *pFactory);
  /** @brief Destructor. */
  virtual ~DataSource() = default;
  /**
   * @brief Pure-virtual method to access a ring item from the data source.
   * Must be implemented in derived classes.
   * @return Pointer to the next ring item from the source.
   */
  virtual ufmt::CRingItem *getItem() = 0;
  /** @brief Set a new factory. */
  void setFactory(ufmt::RingItemFactoryBase *pFactory);
};

#endif
