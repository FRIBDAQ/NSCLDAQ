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
 * @file DDASRootEvent.h
 * @brief Defines a class to encapsulate the information in a built DDAS event.
 */

#ifndef DDASROOTEVENT_H
#define DDASROOTEVENT_H

#include <TObject.h>
#include <vector>

class DDASRootHit;

/**
 * @addtogroup libddasrootformat libddasrootformat.so
 * @brief DDAS data format for ROOT I/O e.g. produced by the ddasdumper
 * program.
 * @{
 */

/**
 * @class DDASRootEvent
 * @brief Encapsulates a built DDAS event with added capabilities for writing
 * to ROOT files.
 * @details
 * Any data that was written to disk downstream of the NSCLDAQ event builder
 * will have a "built" structure. What that means is that the body of the
 * physics event item will contain data from more than one DDAS hit.
 * The DDASRootEvent class represents this type of data. It provides access to
 * the events that make it up through the DDASRootHit objects it owns and then
 * also provides some useful methods for getting data from the event as a
 * whole.
 *
 * The class is, as the name suggests, suitable for ROOT I/O. It inherits from
 * ROOT's TObject and contains a `ClassDef()` macro which adds some reflection
 * capability, allows for schema evolution and _in theory_ offers some
 * performance benefit.
 *
 * @todo (ASC 4/2/24): Write tests for this class to validate its methods.
 */
class DDASRootEvent : public TObject {
private:
  std::vector<DDASRootHit *> m_data; //!< Extensible array of hit objects.

public:
  /** @brief Default constructor. */
  DDASRootEvent();
  /** @brief Destructor. */
  ~DDASRootEvent();

  /**
   * @brief Copy constructor.
   * @param obj References the DDASRootEvent to copy-construct from.
   */
  DDASRootEvent(const DDASRootEvent &obj);
  /**
   * @brief Assignment operator.
   * @param obj Reference to the object to assign (rhs).
   * @return Reference to the assigned object (lhs).
   */
  DDASRootEvent &operator=(const DDASRootEvent &obj);
  /**
   * @brief Access internal, extensible array of channel data.
   * @return Vector of dynamically allocated DDASRootHits.
   */
  std::vector<DDASRootHit *> &GetData() { return m_data; }
  /**
   * @brief Return the number of hits in this event.
   * @return The number of hits in the event (size of the event vector).
   */
  UInt_t GetNHits() const { return m_data.size(); }
  /**
   * @brief Append channel data to event.
   * @param channel Pointer to a DDASRootHit object to append.
   */
  void AddChannelData(DDASRootHit *channel);
  /**
   * @brief Get timestamp of first channel datum.
   * @return Timestamp of the first element in the data vector.
   */
  Double_t GetFirstTime() const;
  /**
   * @brief  Get timestamp of last channel datum.
   * @return Timestamp of the last element in the data vector.
   */
  Double_t GetLastTime() const;
  /** @brief Get time difference between first and last channel data */
  Double_t GetTimeWidth() const;
  /** @brief Clear data vector and reset the event. */
  void Reset();

  // Tell ROOT we're implementing the class:

  ClassDef(DDASRootEvent, 1);
};

/** @} */

#endif
