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

/** @file:  InitSystem.h
 *  @brief: Implement pixie16::init
 */
#ifndef INITSYSTEM_H
#define INITSYSTEM_H

#include "CTclCommand.h"

// Forward definitions.
namespace DAQ {
    namespace DDAS {
        class Configuration;
    }
}


/**
 * The init system command takes the configuration passed to it at construction
 * time and initializes access to the Pixie16 system.
 */
class CInitSystem : public CTclCommand
{
private:
    DAQ::DDAS::Configuration& m_config;                // References the config.
public:
    CInitSystem(Tcl_Interp* pInterp, DAQ::DDAS::Configuration& config);
    virtual ~CInitSystem();
    
    virtual int operator()(std::vector<Tcl_Obj*>& argv);
};

#endif