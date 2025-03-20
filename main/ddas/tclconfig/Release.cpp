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

/** @file:  Release.cpp
 *  @brief:  Implement the pixe16::release command.
 */
#include "Release.h"
#include <Configuration.h>
#include <config.h>
#include <config_pixie16api.h>
#include <sstream>

static const char* apiMessages[3] = {
    "Success",
    "Invalid Pixie16 module number",
    "Failed to close Pixie16 module"
};

/**
 * constructor
 *   Base class construction registers the command and
 *   we squirrel away the configuration so we know which
 *   module numbers exist and need to be released.
 */
CRelease::CRelease(Tcl_Interp* pInterp, DAQ::DDAS::Configuration& config) :
    CTclCommand(pInterp, "pixie16::release"),
    m_config(config)
{
        
}
/**
 * destructor
 *  null for now.
 */
CRelease::~CRelease()
{}

/**
 * operator()
 *    Called in response to the command.
 *  @param objv - the command words
 *  @return int - TCL_OK on success TCL_ERROR on failure with an
 *                error message in the result.
 */
int
CRelease::operator()(std::vector<Tcl_Obj*>& objv)
{
    int index;
    int slot;
    try {
        requireExactly(objv, 1);      // No extra parameters.
        auto slots = m_config.getSlotMap();
        for (index = 0; index < slots.size(); index++) {
            slot = slots[index];
            int status = Pixie16ExitSystem(index);
            if (status) throw -status;
        }
    }
    catch (std::string msg) {
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    catch (int code) {
        std::string msg = apiError(index, slot, code);
        setResult(msg.c_str());
        return TCL_ERROR;
    }
    return TCL_OK;
}
//////////////////////////////////////////////////////////////
// Private utilities.

/**
 * apiError
 *   Format an API error message for ExitSystem.
 *   @param index - module index number
 *   @param slot  - module slot number.
 *   @param status - error code.
 *   @return std::string - the message
 */
std::string
CRelease::apiError(int index, int slot, int status)
{
    std::stringstream s;
    s << "Error calling Pixie16ExitSystem for module number " << index
      << " (slot " << slot << "): " << apiMessages[status];
    std::string result = s.str();
    return result;
}
