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

/** @file:  IsActive.h
 *  @brief: Provides the pixie16::isActive command.
 */
#ifndef ISACTIVE_H
#define ISACTIVE_H

#include "CTclCommand.h"
#include <string>

// Forward definitions.

namespace DAQ { namespace DDAS { class Configuration; }}

/**
 * @class CIsActive
 *     Returns information about which modules are actively taking data.
 *     The command, pixie16::isActive can be issued either with no
 *     parameters or with a module number parameter.  If issued with
 *     a module number the active state of that module is returned as the result
 *     as a bool. Without a parameter it will produce a list of module statuses
 *     for each module in the configuration.  This list will contain a first
 *     element which is the or of the status of all elements (e.g. a boolean that
 *     is true if any module in the configuration is active an false if none are).
 *     The remaining elements are three elements sublists that contain, in order
 *     the following elements:
 *     -  Module id. (unsigned integer)
 *     -  Slot Number (unsigned integer).
 *     -  Status (bool true if active, false if not)
 */
class CIsActive : public CTclCommand {
private:
    DAQ::DDAS::Configuration& m_config;
public:
    CIsActive(Tcl_Interp* pInterp, DAQ::DDAS::Configuration& config);
    virtual ~CIsActive();
    
    virtual int operator()(std::vector<Tcl_Obj*>& objv);
private:
    Tcl_Obj* getAllModuleStates();
    bool moduleState(int index);
    Tcl_Obj* slotInfo(int index, int slot, bool state);
    std::string apiMsg(int index, int slot, int status);
};
#endif