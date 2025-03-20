/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  AdjustOffsets.h
 *  @brief: Provides the pixie16::adjustOffsts command.
 */
#ifndef ADJUSTOFFSETS_H
#define ADJUSTOFFSETS_H
#include "CTclCommand.h"
#include <string>

/**
 * @class CAdjustOffsets
 *      Encasulates the Pixie16AdjustOffsets API call.
 *      This takes a single parameter; the module number to
 *      adjust.
 */
class CAdjustOffsets : public CTclCommand {
public:
    CAdjustOffsets(Tcl_Interp* pInterp);
    virtual ~CAdjustOffsets();
    
    virtual int operator()(std::vector<Tcl_Obj*>& objv);
private:
    std::string apiMsg(int index,  int status);
};


#endif