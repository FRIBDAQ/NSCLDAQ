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

/** @file:  ReadChanPar.h
 *  @brief: Provides the pixie16::readchanpar command
 */
#ifndef READCHANPAR_H
#define READCHANPAR_H
#include "CTclCommand.h"
#include <string>

/**
 * @class CReadChanPar
 *    Provides a command that wraps Pixie16ReadSglChanPar to read a single
 *    DSP parameter from the a module.  The module requires three additional
 *    parameters in order:
 *    -  The module number.
 *    -  The channel number.
 *    -  The Name of the parameter to read e.g. "TRIGGER_RISETIME"
 */


class CReadChanPar : public CTclCommand
{
public:
    CReadChanPar(Tcl_Interp* pInterp);
    virtual ~CReadChanPar();
    
    virtual int operator()(std::vector<Tcl_Obj*>& objv);
private:
    std::string apiMsg(int index, int chan, int status);
};

#endif