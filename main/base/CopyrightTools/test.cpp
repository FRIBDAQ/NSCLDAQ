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

static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";
#include <config.h>
#include "CopyrightNotice.h"
#include <iostream>

using namespace std;

int
main(int argc, char** argv)
{
  // Should produce a coypright notice..

  CopyrightNotice::Notice(cout, argv[0],
			  "1.0", "2004");
  // Should produce an author credit:

  CopyrightNotice::AuthorCredit(cout, argv[0],
				"Ron Fox", 
				"Jay Kusler", NULL);
}
