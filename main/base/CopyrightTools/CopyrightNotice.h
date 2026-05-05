/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014-2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef COPYRIGHTNOTICE_H
#define COPYRIGHTNOTICE_H

/// Requires include of config.h by parent.

#include <ostream>


/*!
  Generate simple copyright and authorship notices.
  Copyright notices are intended for interactive output.
  Authorship notices are intended to acknowledge and ego boost.
 */  
class CopyrightNotice
{
public:
  static void  Notice(std::ostream& out,  const char* program, 
		      const char* version,  const char* year);
  static void  AuthorCredit(std::ostream& out,const  char* program, ...);

}; 

#endif
