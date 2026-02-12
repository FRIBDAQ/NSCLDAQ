/*
  This software is Copyright by the Board of Trustees of Michigan
  State University (c) Copyright 2017.

  You may use this software under the terms of the GNU public license
  (GPL).  The terms of this license are described at:

  http://www.gnu.org/licenses/gpl.txt

  Authors:
      Ron Fox
      Jeromy Tompkins
      Aaron Chester
      FRIB
      Michigan State University
      East Lansing, MI 48824-1321
*/

/**
 * @file  RootFileDataSink.h
 * @brief Defines a class for writing DDAS data to a ROOT file.
 */

#ifndef ROOTFILEDATASINK_H
#define ROOTFILEDATASINK_H

#include <stdlib.h>

class TTree;
class TFile;
namespace ddasfmt {
class DDASHitUnpacker;
}
class DDASRootEvent; // Holds the decoded event for output.
namespace ufmt {
class RingItemFactoryBase;
class CRingItem;
} // namespace ufmt

/**
 * @class RootFileDataSink
 * @brief A ROOT file sink for DDAS data.
 * @note  The `put()` method is not intended to be used by this class but is
 * part of the mandatory interface of the CDataSink base class. If it's used,
 * a warning will be output to stderr. The data will then be treated as a raw
 * ring item, turned into a CRingItem and `putItem()` will be called from then
 * on. The behavior in this case is likely undefined.
 */

class RootFileDataSink {
private:
  ufmt::RingItemFactoryBase *m_pFactory; //!< Turns bodies into ring items.
  ddasfmt::DDASHitUnpacker *m_pUnpacker; //!< Unpacker for our hits.
  DDASRootEvent *m_pEvent;               //!< The ROOT-ized event to write.
  TTree *m_pTree;       //!< Tree in the output file we write to.
  TFile *m_pFile;       //!< The output ROOT file.
  bool m_warnedPutUsed; //!< Warning flag to call the right put.

public:
  /**
   * @brief Constructor.
   * @param pFactory  Factory for creating ring items from hit bodies.
   * @param fileName  ROOT file to open.
   * @param treeName  Name of the tree to create in the root file. The tree
   *   name defaults to "DDASRootHit" if not provided.
   * @throw All exceptions back to the caller.
   */
  RootFileDataSink(ufmt::RingItemFactoryBase *pFactory, const char *fileName,
                   const char *treeName = "ddas");
  /** @brief Destructor. */
  virtual ~RootFileDataSink();

public:
  /**
   * @brief Put a ring item to file.
   * @param item Reference to a ring item object.
   * @throw std::length_error If the event size is different than expected.
   */
  virtual void putItem(const ufmt::CRingItem &item);
  /**
   * @brief Called to put arbitrary data to the file.
   * @param pData  Pointer to the data.
   * @param nBytes Number of bytes of data to put; actually ignored.
   */
  virtual void put(const void *pData, size_t nBytes);
};

#endif
