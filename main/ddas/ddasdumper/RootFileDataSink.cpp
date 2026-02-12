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
      NSCL
      Michigan State University
      East Lansing, MI 48824-1321
*/

/**
 * @file  RootFileDataSink.cpp
 * @brief Implement the DDAS ROOT file sink.
 */

#include "RootFileDataSink.h"

#include <cstdio>

#include <TFile.h>
#include <TROOT.h>
#include <TTree.h>

#include <CPhysicsEventItem.h>
#include <CRingItem.h>
#include <DDASHit.h>
#include <DDASHitUnpacker.h>
#include <RingItemFactoryBase.h>
#include <fragment.h>

#include "DDASRootEvent.h"
#include "DDASRootHit.h"

using namespace ufmt;
using namespace ddasfmt;

static const Int_t BUFFERSIZE(1024 * 1024); // 1 MB

/**
 * @details
 * We're going to make this sink so it can be used in other programs. That
 * implies preserving ROOT's concept of a current working directory across
 * our operation.
 */
RootFileDataSink::RootFileDataSink(RingItemFactoryBase *pFactory,
                                   const char *fileName, const char *treeName)
    : m_pFactory(pFactory), m_pUnpacker(new DDASHitUnpacker),
      m_pEvent(new DDASRootEvent), m_pTree(nullptr), m_pFile(nullptr),
      m_warnedPutUsed(false) {
  const char *oldDir = gDirectory->GetPath();
  gDirectory->Cd("/"); // Have to start somewhere

  try {
    m_pFile = new TFile(fileName, "RECREATE"); // Default directory.
    m_pTree = new TTree(treeName, treeName);
    m_pTree->Branch("rawevents", m_pEvent, BUFFERSIZE);
    gDirectory->Cd(oldDir); // Restore the directory.

  } catch (...) {
    delete m_pUnpacker;
    delete m_pEvent;
    delete m_pTree;
    delete m_pFile;
    gDirectory->Cd(oldDir); // Back to original directory.
    throw;                  // Propagate the error.
  }
}

/**
 * @details
 * Flush the stuff to file and delete all the dynamic components which we own.
 * @note The factory is owned by the caller and is the caller's responsibility.
 */
RootFileDataSink::~RootFileDataSink() {
  m_pFile->Write();
  delete m_pUnpacker;
  delete m_pEvent;
  delete m_pTree;
  delete m_pFile; // Deleting the object saves and closes the file.
}

/**
 * @details
 * The ring item is assumed to consist of a set of fragments. Each fragment
 * contains a hit. The hits are decoded and added to the tree event. Once
 * that's done we can fill the tree and delete any dynamic storage we got.
 */
void RootFileDataSink::putItem(const CRingItem &item) {
  try {
    const uint32_t *pBody =
        reinterpret_cast<const uint32_t *>(item.getBodyPointer());
    uint32_t eventBytes = item.getBodySize();
    uint32_t eventWords = eventBytes / sizeof(uint32_t);

    uint32_t processedWords = 0; // 32-bit words processed.

    // Body pointer currenty points to the size of the entire event.
    pBody++;          // Points at first event builder fragment.
    processedWords++; // Count the first word.

    m_pEvent->Reset(); // Free dynamic hits from last event.

    // Process each fragment in the item:

    const uint32_t *pPrevBody = pBody;
    while (processedWords < eventWords) {

      // The first five 32-bit words of the fragment make up the
      // fragment header. Skip them:

      pBody += sizeof(EVB::FragmentHeader) / sizeof(uint32_t);

      // Use the factory to make a ring item out of the fragment
      // and get a pointer to its body:

      const RingItem *pFrag = reinterpret_cast<const RingItem *>(pBody);
      std::unique_ptr<CRingItem> pUndiff(m_pFactory->makeRingItem(pFrag));
      std::unique_ptr<CPhysicsEventItem> pPhysics(
          m_pFactory->makePhysicsEventItem(*pUndiff));
      const uint32_t *pFragBody =
          reinterpret_cast<const uint32_t *>(pPhysics->getBodyPointer());
      uint32_t fragmentWords = pPhysics->size() / sizeof(uint32_t);

      // Since the hit is passed by reference to the unpacker, we can
      // take advantage of the polymorphism to upcast our DDASRootHit
      // to a DDASHit and unpack the data right into it.

      DDASRootHit *pHit = new DDASRootHit;
      m_pUnpacker->unpack(pFragBody, pFragBody + fragmentWords, *pHit);

      // As of 3/28/24, ROOT 6.30.04 cannot handle I/O of shared_ptrs.
      // DDASRootEvent is responsible for managing and cleaning up
      // the hit data after we add it to the event:

      m_pEvent->AddChannelData(pHit);
      pBody += fragmentWords; // Point to next fragment.

      // Increment the counters:

      processedWords += (pBody - pPrevBody);
      pPrevBody = pBody;
    }

    if (m_pTree->Fill() < 0) {
      std::cerr << "Error filling output TTree!" << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "RootFileDataSink::putItem caught and unexpected "
              << "exception while unpacking: " << e.what() << std::endl;
    std::cerr << "Processing will continue with the next fragment\n";
  }
}

/**
 * @details
 * We really don't know how to do this so:
 * - First time we're called we'll emit a warning that users shouldn't really
 *   do this.
 * - We'll treat the data pointer as a pointer to a raw ring item, turn it
 *   into a CRingItem and call putItem.
 */
void RootFileDataSink::put(const void *pData, size_t nBytes) {
  if (!m_warnedPutUsed) {
    m_warnedPutUsed = true;
    std::string msg(
        "***WARNING*** RootFileDataSink::put was called. You should use "
        "putItem to translate and put ring items containing DDAS hits "
        "that potentially have fits. We'll treat this as an attempt to "
        "output a raw ring item. If that's not the case this can fail "
        "spectacularly. YOU HAVE BEEN WARNED: be sure your code is right!");
    std::cerr << msg << std::endl;
  }

  const RingItem *pRawItem = reinterpret_cast<const RingItem *>(pData);
  std::unique_ptr<CRingItem> pItem(m_pFactory->makeRingItem(pRawItem));
  putItem(*pItem.get());
}
