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

/** @file:  CTclCommand.h
 *  @brief: Base class for Tcl commands.
 */
#ifndef CTCLCOMMAND_H
#define CTCLCOMMAND_H
#include <tcl.h>
#include <string>
#include <vector>
/**
 * @class CTclCommand
 * This class is a light weight replacement for CTCLObjectCommand.
 * in this project we don't have the libtcl++ library available but
 * this makes object oriented command handlers somewhat simpler.
 */
class CTclCommand {
private:
    Tcl_Interp* m_pInterp;
    std::string m_cmdName;
public:
    CTclCommand(Tcl_Interp* pInterp, const char* cmdName);
    virtual ~CTclCommand();
    
    
    // Subclasses must implement this method.  It gets control
    // when the actual command is executed.
    
    virtual int operator()(std::vector<Tcl_Obj*>& argv) = 0;

    // services for concrete subclasses.
protected:
    Tcl_Interp* getInterpreter() { return m_pInterp; }
    void requireExactly(std::vector<Tcl_Obj*>& objv, int nParams);
    void requireAtMost(std::vector<Tcl_Obj*>& objv, int max);
    void setResult(const char* result);
    void setResult(Tcl_Obj* result);
    void setResult(double value);
    void setResult(int value);
    
    int  getInteger(Tcl_Obj* intObj);
    double getDouble(Tcl_Obj* dblObj);
    
private:
    static int commandRelay(
        ClientData pObject, Tcl_Interp* pInterp,
        int objc, Tcl_Obj* const objv[]
    );
    
};

#endif