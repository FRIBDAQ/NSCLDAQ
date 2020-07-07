#ifndef CCREATOR_H
#define CCREATOR_H

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

#include <string>

/*!
   The abstract extensible factor pattern is widely enough used by
me to be worth a templated implementation.  This class provides
the base class for the creators of these factories.
See the commens in CExtensibleFactory.h for information about how
to instantiate this pattern.
*/

template <class T>
class CCreator
{
public:
  virtual T* operator()(void* userData) = 0;
  virtual std::string describe() const = 0;
};


#endif
