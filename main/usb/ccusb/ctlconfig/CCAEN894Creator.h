/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef CCAEN894CREATOR_H
#define CCAEN894CREATOR_H

#include "CModuleCreator.h"

#include <CControlHardware.h>

#include <memory>

/**
 * @file CCAEN894Creator
 * @brief Factory creational class for CAEN C894 CAMAC discriminator.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

/**
 * @class CCAEN894Creator
 *
 *   Extensible factories rely on registered creators to match an object type
 *   and generate an instance of that class.  This class is such a creational
 *   for the CModuleFactory which generates a C894 object to control a 
 *   CAEN C894 discriminator.
 */
class CCAEN894Creator : public CModuleCreator
{
public:
  virtual CControlHardware* operator()();
};

#endif
