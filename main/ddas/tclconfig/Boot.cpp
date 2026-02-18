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

/** @file Boot.cpp
 *  @brief Implement the boot command.
 */

#include "Boot.h"

#include <cstring>
#include <sstream>

#include <Configuration.h>
#include <HardwareRegistry.h>
#include <config.h>
#include <config_pixie16api.h>

/**
 * constructor
 *  @param pInterp - pointer to the interpreter our command
 *  @param config  - references the module configuration.
 */
CBoot::CBoot(Tcl_Interp *pInterp, DAQ::DDAS::Configuration &config)
    : CTclCommand(pInterp, "pixie16::boot"), m_config(config) {}

/**
 * destructor
 *    null for now.
 */
CBoot::~CBoot() {}

/**
 * operator()
 *   - Validate the parameter, and parameter type.
 *   - Figure out the module hardware type.
 *   - Fetch the names of the various files we need to boot.
 *   - Boot the module.
 * @param objv - the command line words.
 * @return int - TCL_OK for success, TCL_ERROR for failure.
 * @note -there's only a result if errors are throw and then it's
 *        a textual error message.
 */
int CBoot::operator()(std::vector<Tcl_Obj *> &objv) {
  int index, slot;
  const char *pDoing;
  Tcl_Interp *pInterp = getInterpreter();
  try {
    requireExactly(objv, 2);
    index = getInteger(objv[1]);

    auto slots = m_config.getSlotMap();
    if ((index < 0) || (index >= slots.size())) {
      throw std::string("Module index is invalid");
    }
    slot = slots[index];

    pDoing = "getting module hardware type";
    int hwtype = getHardwareType(index); // throws on error.

    pDoing = "booting module";
    bootModule(index, hwtype);
  } catch (std::string msg) {
    setResult(msg.c_str());
    return TCL_ERROR;
  } catch (int status) {
    std::string msg = apiMsg(index, slot, status, pDoing);
    setResult(msg.c_str());
    return TCL_ERROR;
  }
  return TCL_OK;
}
//////////////////////////////////////////////////////////
// Private utilities.

/**
 * apiMsg
 *    Returns a string appropriate to an API error status.
 * @param index -module index.
 * @param slot  - corresponding module slot.
 * @param status - API status value.
 * @param doing  -  String describing what failed.
 * @return std::string - the error message.
 */
std::string CBoot::apiMsg(int index, int slot, int status, const char *doing) {
  char xiaErrMsg[1024];
  PixieGetReturnCodeText(status, xiaErrMsg, 1024);

  std::stringstream s;
  s << "Error " << doing << " module number " << index << " (slot: " << slot
    << "): " << xiaErrMsg;

  std::string result = s.str();

  return result;
}
/**
 * getHardwareType
 *   Get the computed hardware type.  This is an NSCL specific
 *   value that combines the properties of the module into a single
 *   integer that can be used to lookup stuff like the firmware files
 *   appropriate to the module.
 *   -  Use ReadModuleInfo to get the module information.
 *   -  Ask the hardware registry to compute the hardware type.
 * @param index - module number.
 * @return int  - Hardware type of the module.
 * @throw int   - status code of failing calls to ReadModuleInfo.
 * @throw std::string - if we can't figure out a valid hardware type.
 * @todo (ASC 3/27/25): If the module is in a bad state, does
 *   `PixieGetModuleInfo()` still retrieve the info properly? Do we need to
 *   maintain FW and HW maps of the modules independent of XIA's management?
 */
int CBoot::getHardwareType(int index) {
  module_config cfg;
  int rv = PixieGetModuleInfo(index, &cfg);
  if (rv < 0)
    throw rv;

  unsigned short rev = cfg.revision;
  unsigned short msps = cfg.adc_sampling_frequency;
  unsigned short bits = cfg.adc_bit_resolution;

  // Module type must be known:
  auto type = DAQ::DDAS::HardwareRegistry::computeHardwareType(rev, msps, bits);
  if (type == DAQ::DDAS::HardwareRegistry::Unknown) {
    throw "Module hardware type is unknown";
  }

  return type;
}

/**
 * bootModule
 *   Given we know the hardware type of a module, fetch the firmware
 *   files needed and try to boot the module.
 *
 * @param index - Index of module to boot.
 * @param type  - hardware type of module.
 * @throw int   - The status from BootModule.
 *
 * @todo (ASC 3/27/25): If the module is in a bad state, does
 *   `PixieGetModuleInfo()` still retrieve the info properly? Do we need to
 *   maintain FW and HW maps of the modules independent of XIA's management?
 */
void CBoot::bootModule(int index, int type) {
  char sysFile[PIXIE16_API_MOD_CONFIG_MAX_STRING];
  char fippiFile[PIXIE16_API_MOD_CONFIG_MAX_STRING];
  char dspFile[PIXIE16_API_MOD_CONFIG_MAX_STRING];
  char varFile[PIXIE16_API_MOD_CONFIG_MAX_STRING];
  std::string settingsFile;

  module_config cfg;
  int rv = PixieGetModuleInfo(index, &cfg);
  if (rv < 0)
    throw rv;

  strcpy(sysFile, cfg.fw_device_file[0]);
  strcpy(fippiFile, cfg.fw_device_file[1]);
  strcpy(dspFile, cfg.fw_device_file[2]);
  strcpy(varFile, cfg.fw_device_file[3]);
  settingsFile = m_config.getModuleSettingsFilePath(index);

  rv = Pixie16BootModule(sysFile, fippiFile, nullptr, dspFile,
                         settingsFile.c_str(), varFile, index, 0x7f);
  if (rv < 0)
    throw rv;
}
