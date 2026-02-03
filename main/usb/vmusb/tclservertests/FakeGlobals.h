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

#ifndef __GLOBALS_H
#define __GLOBALS_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


class CConfiguration;
class CVMUSB;
class TclServer;
class CTCLInterpreter;
class Tcl_ThreadId;
class Tcl_Event;


/*!
  This namespace defines global variables.  We've tried to keep this to minimum.
  Here's what we define/need
  - pConfig : CConfigurtation*           Will hold the daq configuration 
                                         (adcs and scalers).
  - configurationFilename : std::string  Holds the daq configuration filename
  - controlConfigFilename : std::string  Holds the controllable object configuration 
                                         filename.
  - pUSBController        : CVMUSB*      Points to the VMUSB controller object.
*/

namespace Globals {
  CConfiguration* pConfig = 0;
  std::string     configurationFilename;
  std::string     controlConfigFilename;
  CVMUSB*         pUSBController = 0;
  bool            running;
  TclServer*      pTclServer = 0;
  unsigned        scalerPeriod;
  size_t          usbBufferSize;
  unsigned        sourceId;
  char*           pTimestampExtractor;
  int*            mainThreadId = 0;
  CTCLInterpreter*       pMainInterpreter = 0;
  uint32_t  deviceSelectorValue =0; 
};



#endif
