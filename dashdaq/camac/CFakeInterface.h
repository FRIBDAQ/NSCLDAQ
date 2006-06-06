/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CFAKEINTERFACE_H
#define __CFAKEINTERFACE_H

#ifndef __CCAMACINTERFACE_H
#include <CCAMACInterface.h>
#endif

class CFakeInterface : public CCAMACInterface
{
public:
  CFakeInterface();
  virtual ~CFakeInterface();

public:
  virtual size_t lastCrate() const;
};

#endif
