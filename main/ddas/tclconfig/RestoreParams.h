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

/** @file:  RestoreParams
 *  @brief: provides a pixie16::restore
 */

#ifndef RESTOREPARAMS_H
#define RESTOREPARAMS_H
#include "CTclCommand.h"
#include <string>

/**
 * @class CRestoreParams
 *    Implements the pixie16::restore command which loads a .set file
 *    into the modules.
 */
class CRestoreParams : public CTclCommand
{
 public:
    CRestoreParams(Tcl_Interp* pInterp);
    virtual ~CRestoreParams();
    
    int operator()(std::vector<Tcl_Obj*>& objv);
    
private:
    std::string apiMessage(const char* pFilename, int status);
};

#endif