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
#include <tcl.h>
#include <CVMUSB.h>



using std::string;

class CConfiguration;
class CVMUSB;
class TclServer;
class CTCLInterpreter;
class CTheApplication;


namespace Globals {
  CConfiguration*    pConfig;
  string             configurationFilename;
  string             controlConfigFilename;
  CVMUSB*            pUSBController;
  bool               running;
  TclServer*         pTclServer;     
  unsigned           scalerPeriod;
  size_t             usbBufferSize;
  unsigned           sourceId = 0;
  char*              pTimestampExtractor = 0;
  Tcl_ThreadId           mainThreadId = 0;
  CTCLInterpreter*       pMainInterpreter = 0;
  CTheApplication*   pApplication;
  // Issue #422 device source register to restore.
  uint32_t           deviceSelectorValue  =CVMUSB::DeviceSourceRegister::nimO1Busy       |
			    CVMUSB::DeviceSourceRegister::nimO2VMEAS      |
			    CVMUSB::DeviceSourceRegister::dggADisabled    |
			    CVMUSB::DeviceSourceRegister::dggBDisabled;      // Default value.
}