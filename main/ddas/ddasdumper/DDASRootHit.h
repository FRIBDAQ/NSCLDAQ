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
 * @file  DDASRootHit.h
 * @brief Extends the DDASHit class for ROOT I/O.
 */

#ifndef DDASROOTHIT_H
#define DDASROOTHIT_H

#include <DDASHit.h>
#include <TObject.h>

/**
 * @todo (ASC 3/26/24): Enforce some consistent namespace business here:
 * 1. Does this class belong in some namespace or not?
 * 2. What namespace(s) is/are used for DDAS code?
 * 3. What purpose(s) do they serve?
 * For example, DDASRootHit probably should be under some daq::ddas::
 * namespace.
 */

/**
 * @addtogroup libddasrootformat libddasrootformat.so
 * @brief DDAS data format for ROOT I/O e.g. produced by the ddasdumper
 * program.
 * @{
 */

/**
 * @class DDASRootHit
 * @brief Encapsulation of a generic DDAS hit with added capabilities for
 * writing to ROOT files.
 * @details
 * The DDASRootHit class is intended to encapsulate the information that is
 * emitted by the Pixie-16 digitizer for a single hit from a single channel.
 * It is generic because it can store data for the 100 MSPS, 250 MSPS, and
 * 500 MSPS Pixie-16 digitizers used at the lab. In general all of these
 * contain the same set of information, however, the meaning of the CFD data
 * is different for each. The DDASRootHit class abstracts these differences
 * away from the user.
 *
 * This is a very, very simple class. It inherits from ddasfmt::DDASHit and
 * ROOT's TObject class and adds a ROOT `ClassDef()` macro. Everything else,
 * including copy construction, assignment, etc. is handled "for free" by the
 * base classes. Because the class inherits from ddasfmt::DDASHit, we can
 * trivially use the ddasfmt::DDASHitUnpacker to unpack the event data
 * directly into the ddasfmt::DDASHit members.
 *
 * @note In previous versions of NSCLDAQ (prior to 12.1), this class is called
 * ddaschannel and is part of the libddaschannel library. The class is now
 * called DDASRootHit to make more clear its relationship to the DDASHit class,
 * which it is derived from. The library which contains the ROOT I/O classes
 * is now called libddasrootformat to better reflect its purpose.
 *
 * @note No namespace because we are going to refactor everything anyway...
 */

class DDASRootHit : public ddasfmt::DDASHit, public TObject {
public:
  /** @brief Default constructor. */
  DDASRootHit() {};
  /** @brief Default destructor. */
  ~DDASRootHit() {};

  // Tell ROOT we're implementing the class:

  ClassDef(DDASRootHit, 1);
};

/** @} */

#endif
