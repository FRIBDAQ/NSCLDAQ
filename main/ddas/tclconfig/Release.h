/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  Release.h
 *  @brief: Releases access to the address space mapped to the modules.
 */
#ifndef RELEASE_H
#define RELEASE_H
#include "CTclCommand.h"

#include <string>

// Forward definitions

namespace DAQ { namespace DDAS {class Configuration; }}
/**
 * @class CRelease
 *    Provides the pixie16::release command that releases
 *    access to the modules in the configuration.
 */
class CRelease : public CTclCommand
{
private:
    DAQ::DDAS::Configuration& m_config;
public:
    CRelease(Tcl_Interp* pInterp, DAQ::DDAS::Configuration& config);
    virtual ~CRelease();
    
    virtual int operator()(std::vector<Tcl_Obj*>& objv);
private:
    std::string apiError(int index, int slot, int status);
    
};


#endif