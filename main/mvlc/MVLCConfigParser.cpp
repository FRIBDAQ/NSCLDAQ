/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief TCLCOnfigParser implementation.
*/
#include "MVLCConfigParser.h"
// Define the commands to register.
#include "CStack.h"
#include "CMarker.h"
#include "C785.h"
#include "C3820.h"
#include "CDelay.h"
#include "CMADC32.h"
#include "CCAENChain.h"
#include "CMADCChain.h"
#include "CMADCScaler.h"
#include "CMTDC32.h"
#include "CMQDC32RdoHdwr.h"
#include "CMDPP32QDC.h"
#include "CMDPP16QDC.h"
#include "CMDPP32SCP.h"
#include "C3804.h"
#include "C830.h"
#include "CNADC2530.h"
#include "CV977.h"
#include "CV1495sc.h"
#include "CV1729.h"
#include "CV1x90.h"
#include "CXLMTimestamp.h"
#include "CXLMFERA.h"
#include "VMUSBListCommand.h"


#include <stdexcept>



/**
 * constructor:
 */
MVLCConfigParser::MVLCConfigParser(const std::string& script):
    TCLConfigParser(script) {
}

/**
 *  Destructor.
 */
MVLCConfigParser::~MVLCConfigParser() {

}

/**
 * addExtensions
 *    Add the extensions the parser needs:
 */
void
MVLCConfigParser::addExtensions() {
    CTCLInterpreter& interp(*getInterpreter());
    addExtension(new CStackCommand(interp, *this));
    addExtension(new CMarkerCommand(interp, *this));
    addExtension(new C785Command(interp, *this));
    addExtension(new C3820Command(interp, *this));
    addExtension(new CDelayCommand(interp, *this));
    addExtension(new CMADC32Command(interp, *this));
    addExtension(new CAENChainCommand(interp, *this));
    addExtension(new CMADCChainCommand(interp, *this));
    addExtension(new CMADCScalerCommand(interp, *this));
    addExtension(new MTDCCommand(interp, *this));
    addExtension(new MqdcCommand(interp, *this));
    addExtension(new Mdpp32qdcCommand(interp, *this));
    addExtension(new CMdpp16QdcCommand(interp, *this));
    addExtension(new Mdpp32ScpCommand(interp, *this));
    addExtension(new SIS3804Command(interp, *this));
    addExtension(new V830Command(interp, *this));
    addExtension(new Nadc2530Command(interp, *this));
    addExtension(new V977Command(interp, *this));
    addExtension(new V1495Command(interp, *this));
    addExtension(new V1729aCommand(interp, *this));
    addExtension(new V1x90Command(interp, *this));
    addExtension(new XLMTSCommand(interp, *this));
    addExtension(new XLMFERACmd(interp, *this));
    addExtension(new VMUSBListCommand(interp));
}

