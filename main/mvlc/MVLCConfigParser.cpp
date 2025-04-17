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

/**
 * constructor:
 */
MVLCConfigParser::MVLCConfigParser(const std::string& script):
    TCLConfigParser(script) {}

/**
 *  Destructor.
 */
MVLCConfigParser::~MVLCConfigParser() {}

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
}