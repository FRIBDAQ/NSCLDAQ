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

/** @file:  Inventory.cpp
 *  @brief: Implement the pixie16::inventory command.
 */
#include "Inventory.h"
#include <Configuration.h>
#include <config.h>
#include <config_pixie16api.h>


#include <sstream>
static const char* apiErrors[3] = {
    "Success",
    "Invalid module number",
    "Failed to read I2C Serial EEPROM"
};

/**
 * constructor
 *   @param interp - interpreter we register the pixie16::inventory  command.
 *   @param config - References the configuration from which we get slots.
 */
CInventory::CInventory(Tcl_Interp* pInterp, DAQ::DDAS::Configuration& config) :
    CTclCommand(pInterp, "pixie16::inventory"),
    m_config(config)
{}

/**
 * destructor, currently empty.
 */
CInventory::~CInventory() {}

/**
 * operator()
 *    Process the command:
 *    - Require no parameters.
 *    - Create a list object with which to provide the result.
 *    - Loop over all modules in the configuration calling
 *       Pixie16ReadModuleInfo to get information.
 *    - Build the list of outputs.
 *
 * @param argv - the command line parameters
 * @return int - TCL_OK On success with the result set to the inventory
 * @retval       TCL_ERROR on failure with an english error message.
 */
int
CInventory::operator()(std::vector<Tcl_Obj*>& argv)
{
    Tcl_Obj* result = Tcl_NewListObj(0, nullptr);   // Empty list.
    std::vector<Tcl_Obj*> listElements;
    Tcl_IncrRefCount(result);
    int index = 0;
    try {
        requireExactly(argv, 1);
        auto slots = m_config.getSlotMap();
        for (index =0; index < slots.size(); index++) {
            unsigned short rev, bits, mhz;
            unsigned int ser;
            int status = Pixie16ReadModuleInfo(index, &rev, &ser, &bits, &mhz);
            if (status < 0) throw -status;
            listElements.push_back(
                describeModule(slots[index], rev, ser, bits, mhz)
            );
        }

    }
    catch (std::string msg) {
        setResult(msg.c_str());
        freeObjects(result, listElements);
        return TCL_ERROR;
    }
    catch (int code) {
        std::string msg = apiError(index, code);
        setResult(msg.c_str());
        freeObjects(result, listElements);
        return TCL_ERROR;
    }
    // Now build the result list:
    
    Tcl_SetListObj(result, listElements.size(), listElements.data());
    
    setResult(result);
    freeObjects(result, listElements);
    return TCL_OK;
}

/////////////////////////////////////////////////////////////////////
// Private methods.

static void lappend(Tcl_Interp* pInterp, Tcl_Obj* list, Tcl_Obj* el)
{
    Tcl_IncrRefCount(el);
    Tcl_ListObjAppendElement(pInterp, list, el);
    Tcl_DecrRefCount(el);
    
}

/**
 * describeModule
 *   Creates the object that describes a module.
 *   The object will have had it's reference count incremented.
 *
 * @param slot - module slot number.
 * @param rev  - module revision number.
 * @param ser  - Module serial number.
 * @param bits - Module bits
 * @param mhz  - Module sampling MHz.
 * @return Tcl_Obj* the description. object will have an incremened refcount.
 *
 */
Tcl_Obj*
CInventory::describeModule(
   int slot, int rev, int ser, int bits, int mhz 
)
{
    Tcl_Obj* slotObj = Tcl_NewIntObj(slot);
    Tcl_Obj* revObj  = Tcl_NewIntObj(rev);
    Tcl_Obj* serObj  = Tcl_NewIntObj(ser);
    Tcl_Obj* bitsObj = Tcl_NewIntObj(bits);
    Tcl_Obj* mhzObj  = Tcl_NewIntObj(mhz);
    
    Tcl_Obj* result  = Tcl_NewListObj(0, nullptr);
    Tcl_IncrRefCount(result);
    Tcl_Interp* pInterp = getInterpreter();
    
    lappend(pInterp, result, slotObj);
    lappend(pInterp, result, revObj);
    lappend(pInterp, result, serObj);
    lappend(pInterp, result, bitsObj);
    lappend(pInterp, result, mhzObj);
    
    return result;
}
/**
 * apiError
 *   Format an API error for the ReadModuleInfo failure:
 * @param index - module index that failed.
 * @param code  - negative of the error code.
 * @return std::string - message
 */
std::string
CInventory::apiError(int index, int code)
{
    std::stringstream s;
    s << "Failed callling Pixie16ReadModuleInfor for module index: "
      << index << ": " << apiErrors[code];
    std::string msg = s.str();
    return msg;
}
/**
 * freeObjects
 *    Just a convenience method to decrement the reference count
 *    of the list and all its elements so far.
 *  @param list - the list object.
 *  @param elements -the list elements so far.
 */
void
CInventory::freeObjects(Tcl_Obj* list, std::vector<Tcl_Obj*>& elements)
{
    for (int i = 0; i < elements.size(); i++) {
        Tcl_DecrRefCount(elements[i]);
    }
    Tcl_DecrRefCount(list);
}

