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

/** @file:   Inventory.h
 *  @brief:  Defines the processor for the pixie16::inventory command.
 */
#ifndef INVENTORY_H
#define INVENTORY_H

#include "CTclCommand.h"

namespace DAQ { namespace DDAS {class Configuration;}}

/**
 * @class CInventory
 *    This class provides the pixie16::inventory command
 *    it returns information about the moduels that are int the
 *    crate.  The returned value is a Tcl list of lists.  The list item
 *    index is the module number.  The list subelements will be:
 *    {slot revision serial bits Mhz}.
 */

class CInventory : public CTclCommand {
private:
    DAQ::DDAS::Configuration& m_config;
public:
    CInventory(Tcl_Interp* pInterp, DAQ::DDAS::Configuration& config);
    virtual ~CInventory();
    
    virtual int operator()(std::vector<Tcl_Obj*>& argv);
private:
    Tcl_Obj* describeModule(
        int slot, int rev, int ser, int bits, int mhz
    );
    std::string apiError(int index, int code);
    void freeObjects(Tcl_Obj* list, std::vector<Tcl_Obj*>& elements);
};

#endif