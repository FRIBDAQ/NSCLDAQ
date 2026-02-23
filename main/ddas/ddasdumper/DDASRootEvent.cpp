/*
  This software is Copyright by the Board of Trustees of Michigan
  State University (c) Copyright 2017.

  You may use this software under the terms of the GNU public license
  (GPL).  The terms of this license are described at:

  http://www.gnu.org/licenses/gpl.txt

  Authors:
      Aaron Chester
      Jeromy Tompkins
      FRIB
      Michigan State University
      East Lansing, MI 48824-1321
*/

/**
 * @file DDASRootEvent.cpp
 * @brief Implementation of a class to encapsulate the information in a built
 * DDAS event.
 */

#include "DDASRootEvent.h"

#include "DDASRootHit.h"

DDASRootEvent::DDASRootEvent() : TObject(), m_data() {}

/**
 * @details
 * Implements a deep copy.
 */
DDASRootEvent::DDASRootEvent(const DDASRootEvent &obj)
    : TObject(obj), m_data() {
  // Create new copies of the DDASRootHit events
  for (UInt_t i = 0; i < m_data.size(); ++i) {
    m_data[i] = new DDASRootHit(*obj.m_data[i]);
  }
}

/**
 * @brief
 * Performs a deep copy of the data belonging to obj. This assignment operator
 * is simple at the expense of some safety. The entirety of the data vector is
 * deleted prior to assignment. If the initialization of the DDASRootHit
 * objects threw an exception or caused something else to happen that is bad,
 * then it would be a big problem. The possibility does exist for this to
 * happen. Coding up a safer version is just more complex, harder to
 * understand, and will be slower.
 */
DDASRootEvent &DDASRootEvent::operator=(const DDASRootEvent &obj) {
  if (this != &obj) {
    // Create new copies of the DDASRootHit events
    for (UInt_t i = 0; i < m_data.size(); ++i) {
      delete m_data[i];
    }
    m_data.resize(obj.m_data.size());
    for (UInt_t i = 0; i < m_data.size(); ++i) {
      m_data[i] = new DDASRootHit(*obj.m_data[i]);
    }
  }

  return *this;
}

/**
 * @details
 * Deletes the objects stored in m_data and clear the vector.
 */
DDASRootEvent::~DDASRootEvent() { Reset(); }

/**
 * @details
 * Appends the pointer to the internal, extensible data array. There is no
 * check that the object pointed to by the argument exists, so that it is
 * the user's responsibility to implement.
 */
void DDASRootEvent::AddChannelData(DDASRootHit *channel) {
  m_data.push_back(channel);
}

/**
 * @details
 * If data exists return timestamp of first element in the array. This should
 * be the earliest unit of data stored by this object. If no data exists,
 * returns 0.
 */
Double_t DDASRootEvent::GetFirstTime() const {
  Double_t time = 0;
  if (m_data.size() > 0) {
    time = m_data.front()->getTime();
  }

  return time;
}

/**
 * @details
 * If data exists return timestamp of last element in the array. This should
 * be the most recent unit of data stored by this object. If no data exists,
 * returns 0.
 */
Double_t DDASRootEvent::GetLastTime() const {
  Double_t time = 0;
  if (m_data.size() > 0) {
    time = m_data.back()->getTime();
  }

  return time;
}

/**
 * @details
 * Calculate and return the timestamp difference between the last and first
 * elements of the data vector. If the data vector is empty, returns 0.
 */
Double_t DDASRootEvent::GetTimeWidth() const {
  return GetLastTime() - GetFirstTime();
}

/**
 * @details
 * Deletes the DDASRootHit data objects and resets the  size of the extensible
 * data array to zero.
 */
void DDASRootEvent::Reset() {
  // Delete all of the object stored in m_data
  for (UInt_t i = 0; i < m_data.size(); ++i) {
    delete m_data[i];
  }

  // Clear the array and resize it to zero
  m_data.clear();
}
