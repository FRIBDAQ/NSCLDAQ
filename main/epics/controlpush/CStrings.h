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



#ifndef CSTRINGS_H
#define CSTRINGS_H

#include <string>


/*!
  This class provides static member functions that implement string utilities.
*/

class CStrings
{
public:
  static std::string EscapeString(const char* in, 
			     const char* charset, 
				  const char* how) ;
};

#endif
