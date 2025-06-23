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

@file VMUSBListComand.h
@author Ron Fox <fox at frib dot msu dot edu>
@brief Header for Tcl  encapsulation of CVMUSBReadoutList
*/
#ifndef MVLC_VMUSBListCommand_H
#define MVLC_VMUSBListCommand_H
#include <TCLObjectProcessor.h>
#include <tuple>           // Common parameer decoders.

class CVMUSBReadoutList;

/**
 *  @class VMUSBListCommand
 * 
 * Provides an encpasulation of a VMUSBReadoutList.
 * The list is owned externally to this classs
 * (e.g. by MVLCGenerator).  This is intended to
 * support Tcl translator drivers.  We define
 * This defines the 'mvlclist' command.  This command
 * is an ensemble that defins the following subcommands
 * that closely follow methods of the CVMUSBReadoutList
 * Simpler all around to do it this way than to try toget
 * swig to do it which was unsatisfying for VMUSBReadout:
 * 
 * - addWrite32 address amod datum
 * - addWrite16 address amod datum
 * - addRead32 address amod
 * - addRead16 address amod
 * - addBlockRead32 baseAddress amod numxfers
 * - addFifoRead32  address amod numxfers
 * - addBlockCountRead16 address mask amod
 * - addBlockCountRead32 address mask amod
 * - addMaskedCountBlockRead32 address amod
 * - addMaskedCountFifoRead32 addresss amod
 * - addDelay clocks
 * - addMarker value   note this is a 32bit marker.
 * - addLoopUntil32 address amod mask value
 * - addLoopUntil16 address amod mask value
 * 
 * In addition, the ommand defines the CVMUSBReadoutList
 * namespace with the following variables containing
 * address modifiers.
 *   - a32UserData
 *   - a32UserProgram
 *   - a32UserBlock
 *   - a32PrivData
 *   - a32PrivProgram
 *   - a32PrivBlock
 *   - a16User
 *   - a16Priv
 *   - a24UserData
 *   - a24UserProgram
 *   - a24UserBlock
 *   - a24PrivData
 *   - a24PrivProgram
 *   - a24PrivBlock
 * 
 * This allows address modifiers like $VMUSBReadoutList::a32UserData to be useed.
 */
class VMUSBListCommand : public CTCLObjectProcessor {
private:
    CVMUSBReadoutList* m_pList;         // what we encapsulste.

    // Canonicals:
public:
    VMUSBListCommand(CTCLInterpreter& interp);
    virtual ~VMUSBListCommand();

    // Forbidden canonicals
private:
    VMUSBListCommand(const VMUSBListCommand&);
    VMUSBListCommand& operator=(VMUSBListCommand);
    int operator==(const VMUSBListCommand&) const;
    int operator!=(const VMUSBListCommand&) const;

    // list management
public:
    void setList(CVMUSBReadoutList& list);
    CVMUSBReadoutList* getList();
    void clearList();

    // virtual overrides:

    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    // utilities (inclding subcommand handlers).
private:
    void addWrite32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void addWrite16(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void addBlockWrite32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    void addRead32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void addRead16(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    void addBlockRead32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void addFifoRead32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    void addBlockCountRead16(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void addBlockCountRead32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    void addMaskedCountBlockRead32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void addMaskedCountFifoRead32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    void addDelay(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void addMarker(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    void addLoopUntil32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void addLoopUntil16(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    // other utilities:
private:
    void throwIfNoList();
    void createAmodVars();
    void setAmodVar(const char* name, uint8_t value);
    uint8_t decodeAmod(CTCLObject& val);
    std::tuple<uint32_t, uint8_t, uint32_t> getWriteParams(std::vector<CTCLObject>& objv);
    std::tuple<uint32_t, uint8_t> getReadParams(std::vector<CTCLObject>& objv);
    std::vector<uint32_t>         listToWriteBlock(CTCLObject& obj);
    std::tuple<uint32_t, uint8_t, size_t> getBlockReadParams(std::vector<CTCLObject>& objv);
    std::tuple<uint32_t, uint32_t, uint8_t> getBlockCountReadParams(std::vector<CTCLObject>& objv);
    std::tuple<uint32_t, uint8_t, uint32_t, uint32_t> getLoopUntilParams(std::vector<CTCLObject>& objv);
};

#endif