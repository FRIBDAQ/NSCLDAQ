/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321

@file VMUSBComand.h
@author Ron Fox <fox at frib dot msu dot edu>
@brief Header for Tcl  encapsulation of CVMUSB
*/
#ifndef MVLC_VMUSBCOMMAND_H
#define MVLC_VMUSBCOMMAND_H
#include <TCLObjectProcessor.h>
#include <stdint.h>
#include <utility>
#include <tuple>

class CVMUSB;

/**
 * This class implements a wrapper for a VMUSB controller memorizer.
 * it is a command ensemble that is registered as:
 * "mvlc"
 * 
 * The wrapper must have its controller set prior to being used.  Thisis normally done
 * by methods in MVLCGenerate which will define this command ensemble
 * and keep ownership of this command object.
 * 
 * The command is an ensemble with subcommands that closely match the methods of the CVMUSB class:
 * 
 * -  vmewrite32 address amodifier data
 * -  vmewrite16 addresss amodifier data
 * -  delay ms
 * -  loopuntil32 address amod mask value
 * -  loopuntil16 address amod mask value
 * 
 * @note to maintainerse  - if methods are addded to CVMUSB, you need to consider if they should
 * be added as subcommands to this command ensemble.
 * 
 */
class VMUSBCommand : public CTCLObjectProcessor {
    // Object data:
private:
    CVMUSB*   m_pController;

    // Canonicals:
public:
    VMUSBCommand(CTCLInterpreter& interp);
    virtual ~VMUSBCommand();

    // Forbidden canonicals:
private:
    VMUSBCommand(const VMUSBCommand&);
    VMUSBCommand& operator=(const VMUSBCommand&);
    int operator==(const VMUSBCommand&) const;
    int operator!=(const VMUSBCommand&) const;

public:
    // controller management:

    void setController(CVMUSB& controller);
    void clearController();
    CVMUSB* getController();

    // Command processing:

public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    // Utilities:

private:
    void vmewrite32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void vmewrite16(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void delay(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void loopuntil32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void loopuntil16(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    void throwIfNoController();

    // Common parameter checker/getters.

    std::tuple<uint32_t, uint8_t, uint32_t> getWriteParameters(std::vector<CTCLObject>& ovjv);
    std::tuple<uint32_t, uint8_t, uint32_t, uint32_t> getLoopParameters(std::vector<CTCLObject>& objv);    
    uint8_t decodeAmod(CTCLObject& val);

};
#endif