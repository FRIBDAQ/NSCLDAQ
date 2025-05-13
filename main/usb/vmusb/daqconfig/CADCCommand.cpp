/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <config.h>
#include "CADCCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CConfiguration.h"
#include <C785.h>
#include <CReadoutModule.h>
#include <XXUSBConfigurableObject.h>

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

using std::string;
using std::vector;

////////////////////////////////////////////////////////////////////////
/////////////////// Canonicals that are implemented ////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Construct the command and register it (base constructor does this
   by default.
   \param interp : CTCLInterpreter&
       Tcl interpreter on which the command will be registered.
   \param config : CConfiguration& config
       The configuration of ADCs that will be manipulated by this command.
   \param commandName std::string
       Name of the command to register.
*/
CADCCommand::CADCCommand(CTCLInterpreter& interp,
			 CConfiguration&  config,
			 std::string      commandName) : 
  CDeviceCommand(interp, commandName.c_str(), config),
  m_Config(config)
{
}
/*!
   Destructor is a no-op but chains to the base class which unregisters
   etc.
*/
CADCCommand::~CADCCommand()
{
}

////////////////////////////////////////////////////////////////////////
///////////////////// Command processing ///////////////////////////////
////////////////////////////////////////////////////////////////////////


/*
   Process the create subcommand:
   - ensure we have enough values on the command line.
   - ensure we have a valid adc name, and base address.
   - ensure that there is no other adc with the same name.
   - Create the new adc module
   - Add it to the configuration.
   Parameters:
     CTCLInterpreter&    interp   - Interpreter that is executing this command.
     vector<CTCLObject>& objv     - Vector of command words.
  Returns:
    int: 
       TCL_OK      - Command was successful.
       TCL_ERROR   - Command failed.
  Side effects:
     The result for the interpreter is set as follows:
     - On error this is an error message of the form ERROR: message
     - On success, this is the name of the module. allowing e.g.
       adc config [adc create adc1 0x80000000] ....
*/
int
CADCCommand::create(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  // Need to have exactly 4 elements, command 'create' name base.

  if (objv.size() < 4) {
    Usage(interp, "Not enough parameters for create subcommand", objv);
    return TCL_ERROR;
  }

  // Get the command elements and validate them:

  string name    = objv[2];
  string sBase   = objv[3];

  errno = 0;
  uint32_t base  = strtoul(sBase.c_str(), NULL, 0);
  if ((base == 0) && (errno != 0)) {
    Usage(interp, "Invalid value for base address", objv);
    return TCL_ERROR;
  }
  // Deal with buggy base addressing with the VMUSB:

  if ((base & 0xff000000) == 0) {
    Usage(
      interp, 
      "Module base addresses of the form 0x00nn0000 don't work with the VMUSB!!!", 
      objv
    );
    return TCL_ERROR;
  }

  CReadoutModule* pModule = m_Config.findAdc(name);
  if (pModule) {
    Usage(interp, "Duplicate module creation attempted", objv);
    return TCL_ERROR;
  }
  // This is a unique module so we can create it:

  C785* pAdc = new C785;
  pModule    = new CReadoutModule(name, *pAdc);
  pModule->configure("-base", sBase);
  
  // If there are sufficient parameters, try to do additional configuration:
  
  if (objv.size() > 4) {
    int status = configure(interp, pModule, objv, 5);
    if (status != TCL_OK) {
      return status;                   // Config failed.
    }
  }

  m_Config.addAdc(pModule);

  m_Config.setResult(name);
  return TCL_OK;
  
}
/*
  Return the configuration. This allows subclassed commands to function properly.
*/
CConfiguration* 
CADCCommand::getConfiguration()
{
  return &m_Config;
}

