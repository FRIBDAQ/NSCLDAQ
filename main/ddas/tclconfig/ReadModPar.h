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

/** @file:  ReadModPar.h
 *  @brief: Read a module parameter
 */
#ifndef READMODPAR_H
#define READMODPAR_H

#include "CTclCommand.h"
#include <string>

/**
 * @class CReadModPar
 *    This is a jacked for Pixie16ReadSglModPar.  The command needs two
 *    additional parameters in order:
 *    -  The module number.
 *    -  Name of the parameter to read e.g. MODULE_CSRA
 *    The result of a successful command is the value read (integer).
 *    On failure the result is a textual error message.
 */
class CReadModPar : public CTclCommand
{
public:
    CReadModPar(Tcl_Interp* pInterp);
    virtual ~CReadModPar();
    
    virtual int operator()(std::vector<Tcl_Obj*>& objv);
private:
    std::string apiMessage(int module, int status);
};


#endif