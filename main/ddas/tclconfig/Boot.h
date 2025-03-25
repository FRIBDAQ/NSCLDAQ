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

/** @file:  Boot.h
 *  @brief: Provides pixie16::boot to boot a single module.
 */
#ifndef BOOT_H
#define BOOT_H
#include "CTclCommand.h"
#include <string>

// Forward definitions:

namespace DAQ { namespace DDAS {class Configuration; }}

/**
 * @class CBoot
 *    This class provides the pixie16::boot command which accepts
 *    a two parameters, the module *index* of the module to boot.
 *    If the user wants to boot by slot number, they must inventory
 *    the modules and look through the inventory to figure out which
 *    index corresponds to the desired slot.  Note that the boot pattern
 *    will only be 0x7d which
 *    - Boots everything but the trigger fpga
 *    - Assumes you don't have a revA module which is the only one
 *      that has a trigger fpga.
 */
class CBoot : public CTclCommand
{
private:
    DAQ::DDAS::Configuration& m_config;
public:
    CBoot(Tcl_Interp* pInterp, DAQ::DDAS::Configuration& config);
    virtual ~CBoot();
    
    virtual int operator()(std::vector<Tcl_Obj*>& objv);
private:
    std::string apiMsg(int index, int slot, int status, const char* doing, const char* msgs[]);
    int getHardwareType(int index);
    void bootModule(int index, int type);
};
#endif