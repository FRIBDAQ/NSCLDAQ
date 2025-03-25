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

/** @file:  WriteChanPar.h
 *  @brief: Tcl wrapper for Pixie16WriteSglChanPar
 */
#ifndef WRITECHANPAR_H
#define WRITECHANPAR_H
#include "CTclCommand.h"
#include <string>
/**
 * @class CWriteChanPar
 *    This class wraps the Tcl command pixie16::writechanpar around
 *    the XIA api Pixie16WriteSglChanPar.
 *    The command takes the following parameters in order:
 *    -  module number.
 *    -  channel number
 *    - parameter name
 *    - parameter value (double).
 */
class CWriteChanPar : public CTclCommand
{
public:
    CWriteChanPar(Tcl_Interp* pInterp);
    virtual ~CWriteChanPar();
    
    virtual int operator()(std::vector<Tcl_Obj*>& objv);
private:
    std::string apiMessage(int mod, int chan, int status);
};

#endif