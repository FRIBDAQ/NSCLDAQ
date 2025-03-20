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

/** @file:  WriteModPar.h
 *  @brief: Wraps the Pixie16WriteSglModPar function in the pixie16::writemodpar cmd.
 */
#ifndef WRITEMODPAR_H
#define WRITEMODPAR_H
#include <CTclCommand.h>
#include <string>
/**
 * @class CWriteModPar
 *    Provides a command wrapper for the Pixie16WriteSgnModPar method.
 *    The command requires the following additional parameters:
 *    -  Module number
 *    - Parameter name (e.g. MOD_CSRA).
 *    - Parameter value - an integer.
 */
class CWriteModPar : public CTclCommand
{
public:
    CWriteModPar(Tcl_Interp* pInterp);
    virtual ~CWriteModPar();
    
    virtual int operator()(std::vector<Tcl_Obj*>& objv);
    
private:
    std::string apiMessage(int module, int status);
};

#endif