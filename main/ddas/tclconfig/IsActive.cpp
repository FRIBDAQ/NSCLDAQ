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

/** @file:  IsActive.cpp
 *  @brief: Implementation of CIsActive (pixie16::isActive command).
 */
#include "IsActive.h"
#include <Configuration.h>
#include <config.h>
#include <config_pixie16api.h>
#include <sstream>

// Note that at present, only one error status is possible, -1,
// for Invalid module number.  The result of the call to
// Pixie16CheckRunStatus gives the state.


/**
 * constructor
 *   Construct the command.
 * @param pInterp - Interpreter on which the command will be registered.
 * @param config  - Reference to the module configuration.
 */
CIsActive::CIsActive(Tcl_Interp* pInterp, DAQ::DDAS::Configuration& config) :
    CTclCommand(pInterp, "pixie16::isActive"),
    m_config(config)
{}
/**
 * destructor
 */
CIsActive::~CIsActive() {}

/**
 * operator()
 *    Executes the command.
 *  @param objv - the command words.
 *  @return int - TCL_OK On success, TCL_ERROR on failure.
 *  @note see the header comments for a description of the result
 *        on a successful return. on a TCL_ERROR, the result contains
 *        a human readable error message.
 */
int
CIsActive::operator()(std::vector<Tcl_Obj*>& objv)
{
    int module;
    int slot;
    try {
        requireAtMost(objv, 2);
        
        if (objv.size() ==1) {
            // All modules:
            
            setResult(getAllModuleStates());
            
        } else {
            // Module# is the parameter:
            
            module = getInteger(objv[1]);
            auto slots = m_config.getSlotMap();
            slot = slots[module];
            
            bool state = moduleState(module);
            Tcl_Obj* result = Tcl_NewBooleanObj(state);
            setResult(result);
        }
    }
    catch (std::string msg) {
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    catch (int status) {
        std::string msg = apiMsg(module, slot, status);
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    return TCL_OK;
}
/**
 * getAllModuleStates
 *    Iterates through the modules in the configuration creating
 *    a list of results and then an Or of those results.
 *    See the class header comments for the form of the result list
 * @return Tcl_Obj* resulting list.
 * @throw std::string if there's a failure.
 */
Tcl_Obj*
CIsActive::getAllModuleStates()
{
    auto slots = m_config.getSlotMap();
    
    Tcl_Obj* moduleStatusList = Tcl_NewListObj(0, nullptr);
    Tcl_Interp* pInterp = getInterpreter();
    
    bool orStatus = false;
    for (int module = 0; module < slots.size(); module++) {
        bool status = moduleState(module);
        Tcl_Obj* m = slotInfo(module, slots[module], status);
        Tcl_IncrRefCount(m);
        Tcl_ListObjAppendElement(pInterp, moduleStatusList, m);
        Tcl_DecrRefCount(m);
        orStatus |= status;
    }
    // At this point, the moduleStatusList contains the
    // list of status data. and orStatus conains the
    // or of status. Build up the final list:
    
    Tcl_Obj* result = Tcl_NewBooleanObj(orStatus);

    // What follows shimmers this into a list:
    
    Tcl_IncrRefCount(moduleStatusList);
    Tcl_ListObjAppendList(pInterp, result, moduleStatusList);
    Tcl_DecrRefCount(moduleStatusList);
    return result;
    
}
/**
 * moduleState
 *    Returns the status of a single module
 * @param index - module index.
 * @return bool - true if running false if not.
 * @throw std::string - if there's an error.
 */
bool
CIsActive::moduleState(int index)
{
    int status = Pixie16CheckRunStatus((unsigned short)(index));
    if (status < 0) {
        // note that the only error now is an invalid slot index:
        // so give a fake slot for now.
        
        std::string msg = apiMsg(index, -1, -status);
        throw msg;
    }
    return status > 0;
}
/**
 * slotInfo
 *   Produce a Tcl_Obj* that describes the state of one module.
 * @param index - module index.
 * @param slot  - module slot number.
 * @param state - Boolean state of the module.
 * @return Tcl_Obj* - pointer to the resulting list.
 * 
 */
Tcl_Obj*
CIsActive::slotInfo(int index, int slot, bool state)
{
    // Need three objects;
    //
    Tcl_Obj* elements[3];
    elements[0] = Tcl_NewIntObj(index);
    elements[1] = Tcl_NewIntObj(slot);
    elements[2] = Tcl_NewBooleanObj(state);
    
    for (int i =0; i < 3; i++) {
        Tcl_IncrRefCount(elements[i]);
    }
    Tcl_Obj* result = Tcl_NewListObj(3, elements);
    
    for (int i =0; i < 3; i++) {
        Tcl_DecrRefCount(elements[i]);
    }
    return result;
}
/**
 * apiMsg
 *    Return the API appropriate error message for the status slot
 *    and index.
 *  @param index - module number.
 *  @param slot  - module slot number.
 *  @param status - Status
 *  @return std::string - string error message.
 *  @note for the Pixie16CheckRunStatus function there's really only
 *        invalid module index error status (-1) so the slot is ignored
 *        as it's likely invalid.
 */
std::string
CIsActive::apiMsg(int index, int slot, int status)
{
    std::string msg;
    if (status == 1) {
        std::stringstream s;
        s << "Module index " << index << " is invalid";
        msg = s.str();
    } else {
        std::stringstream s;
        s << "Unrecognized status code " << status
        << " slot: " << slot << " index: " << index;
        std::string msg = s.str();
        
    }
    return msg;
}
