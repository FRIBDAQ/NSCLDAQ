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

#ifndef _TCL_H
#include <tcl.h>
#ifndef _TCL_H
#define _TCL_H
#endif
#endif


class CConfiguration;
class CVMUSB;
class TclServer;
class CTCLInterpreter;
class CTheApplication;

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
  extern CConfiguration* pConfig;
  extern std::string     configurationFilename;
  extern std::string     controlConfigFilename;
  extern CVMUSB*         pUSBController;
  extern bool            running;
  extern TclServer*      pTclServer;
  extern unsigned        scalerPeriod;
  extern size_t          usbBufferSize;
  extern unsigned        sourceId;
  extern char*           pTimestampExtractor;
  extern Tcl_ThreadId           mainThreadId;
  extern CTCLInterpreter*       pMainInterpreter;
  extern CTheApplication* pApplication;
  extern uint32_t         deviceSelectorValue; // for issue #422
};

#endif
