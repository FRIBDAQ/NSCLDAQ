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

#ifndef __CVMUSBCREATOR_H
#define __CVMUSBCREATOR_H

/**
 * @file CVMUSBCreator.h
 * @brief Define a creator for the VMUSB remote list processor.
 */

#ifndef __CMODULECREATOR_H
#include <CModuleCreator.h>
#endif

#include <CControlHardware.h>
#include <memory>

/**
 * Concrete CModuleCreator that creates a CVMUSBModule
 */
class CVMUSBCreator : public CModuleCreator
{
public:
  virtual std::unique_ptr<CControlHardware> operator()();
};

#endif
