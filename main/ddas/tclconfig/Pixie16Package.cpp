/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
             Aaron Chester
             NSCL/FRIB
             Michigan State University
             East Lansing, MI 48824-1321
*/

/** @file  Pixie16Package.cpp
 *  @brief Package initialization file for the pixie16 package.
 */

/**
 * @todo (ASC 3/26/25): Entire package needs to replace hardcoded API error
 * messages with API calls to get error messages from return codes.
 * See Boot.cpp for initial progress.
 */

/**
 *  This file is loaded when
 *  package require pixie16 is executed.  It must:
 *  - provide the package so pkg_mkIndex does the right thing.
 *  - check the existence of the cfgPixie16.txt file in the current
 *    directory without which we can't function.
 *  - Create a configuration object (if modevtlen is required for this
 *    we'll need to require it as well above).
 *  - register the package commands.
 */

#include <unistd.h>

#include <string>

#include <tcl.h>

#include <Configuration.h>
#include <ConfigurationParser.h>

#include "Boot.h"
#include "CAdjustOffsets.h"
#include "InitSystem.h"
#include "Inventory.h"
#include "IsActive.h"
#include "ReadChanPar.h"
#include "ReadModPar.h"
#include "Release.h"
#include "RestoreParams.h"
#include "SaveParams.h"
#include "WriteChanPar.h"
#include "WriteModPar.h"

DAQ::DDAS::Configuration crateConfiguration;

static const char *pkgVersion = "1.1";              // Tcl package version.
static const char *configFile = "cfgPixie16.txt";   // Crate config
static const char *modEvtLenFile = "modevtlen.txt"; // Event lengths.

/**
 * setErrnoResult
 *    Set an interpreter result that consists of a leading text string
 *    followed by an message describing the errno value on entry
 *
 *  @param pInterp - pointer to the interpreter.
 *  @param message - Pointer to the leading text string.
 */
static void setErrnoResult(Tcl_Interp *pInterp, const char *message) {
  // Build the full message string:

  int e = Tcl_GetErrno();
  std::string fullMessage(message);
  fullMessage += Tcl_ErrnoMsg(e);

  // Make a string object, with the error text in it and
  // use Tcl_SetObjResult to set the result:

  Tcl_Obj *pResult = Tcl_NewStringObj(fullMessage.c_str(), -1);
  Tcl_IncrRefCount(pResult);
  Tcl_SetObjResult(pInterp, pResult);
  Tcl_DecrRefCount(pResult); // Frees pResult if possible.
}

/**
 * Pixie_Init
 *    Package initialization.  Executed when the package is loaded.
 *  - provide the package so pkg_mkIndex does the right thing.
 *  - check the existence of the cfgPixie16.txt file in the current
 *    directory without which we can't function.
 *  - Create a configuration object (if modevtlen is required for this
 *    we'll need to require it as well above).
 *  - register the package commands.
 *
 *  @param pInterp - Interpreter loading the package and on which
 *                   the commands will be registered.
 *  @return int   - TCL_OK on proper load and TCL_ERROR with result
 *                  set to a human readable error text if not.
 *
 */
extern "C" {
int Pixie_Init(Tcl_Interp *pInterp) {
  // Make our package known to the interpreter.

  int status = Tcl_PkgProvide(pInterp, "pixie16", pkgVersion);
  if (status != TCL_OK)
    return status;

  // Check for the CfgPixie16.txt file in the current directory.

  status = access(configFile, R_OK);
  if (status < 0) {
    setErrnoResult(pInterp,
                   "Unable to read cfgPixie16.txt in current directory ");
    return TCL_ERROR;
  }

  status = access(modEvtLenFile, R_OK);
  if (status < 0) {
    setErrnoResult(pInterp,
                   "Unable to read modevtlen.txt in current directory ");
    return TCL_ERROR;
  }

  DAQ::DDAS::Configuration *pCfg;
  const char *fwFile = getenv("FIRMWARE_FILE");
  if (fwFile) {
    crateConfiguration = *(
        DAQ::DDAS::Configuration::generate(fwFile, configFile, modEvtLenFile));
  } else {
    crateConfiguration = *(
        DAQ::DDAS::Configuration::generateManagedFW(configFile, modEvtLenFile));
  }

#ifdef DEBUG
  std::cout << "Configuration: \n";
  crateConfiguration.print(std::cout);
  std::cout << std::endl;
  std::cout.flush();
#endif
  // All the commands register in the pixie16 namespace:

  Tcl_CreateNamespace(pInterp, "pixie16", nullptr, nullptr);

  // Register the commands:

  new CInitSystem(pInterp, crateConfiguration);
  new CInventory(pInterp, crateConfiguration);
  new CRelease(pInterp, crateConfiguration);
  new CBoot(pInterp, crateConfiguration);
  new CReadChanPar(pInterp);
  new CReadModPar(pInterp);
  new CWriteChanPar(pInterp);
  new CWriteModPar(pInterp);
  new CSaveParams(pInterp);
  new CRestoreParams(pInterp);
  new CIsActive(pInterp, crateConfiguration);
  new CAdjustOffsets(pInterp);
  return TCL_OK;
}
}
