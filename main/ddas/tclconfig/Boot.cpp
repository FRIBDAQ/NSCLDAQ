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
#ifdef DEBUG
#include <iostream> // Needed for debugging output
#endif
#include <sstream>

#include <CXIAException.h>
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
  int index;
  Tcl_Interp *pInterp = getInterpreter();
  try {
    requireExactly(objv, 2);
    index = getInteger(objv[1]);

    auto slots = m_config.getSlotMap();
    if ((index < 0) || (index >= slots.size())) {
      throw std::string("Module index is invalid");
    }
    auto slot = slots[index];

    int hwtype = getHardwareType(index); // throws on error.
    bootModule(index, slot, hwtype);
  } catch (std::string msg) {
    setResult(msg.c_str());
    return TCL_ERROR;
  } catch (CXIAException &e) {
    setResult(e.ReasonText());
    return TCL_ERROR;
  }
  return TCL_OK;
}

//////////////////////////////////////////////////////////
// Private utilities.

/**
 * getHardwareType
 *   Get the computed hardware type.  This is an NSCL specific
 *   value that combines the properties of the module into a single
 *   integer that can be used to lookup stuff like the firmware files
 *   appropriate to the module.
 *   -  Use PixieGetModuleInfo  to get the module information.
 *   -  Ask the hardware registry to compute the hardware type.
 * @param index - module number.
 * @return int  - Hardware type of the module.
 * @throw CXIAException - if the API call to get the module info fails.
 * @throw std::string - if we can't figure out a valid hardware type.
 */
int CBoot::getHardwareType(int index) {
  module_config cfg;
  int rv = PixieGetModuleInfo(index, &cfg);
  if (rv < 0) {
    std::stringstream msg;
    msg << "Failed to get module info for module " << index;
    throw CXIAException(msg.str(), "PixieGetModuleInfo()", rv);
  }
  // Module type must be known:
  auto type = DAQ::DDAS::HardwareRegistry::computeHardwareType(
      cfg.revision, cfg.adc_sampling_frequency, cfg.adc_bit_resolution);
  if (type == DAQ::DDAS::HardwareRegistry::Unknown) {
    throw std::string("Module hardware type is unknown");
  }

  return type;
}

/**
 * bootModule
 *   Given we know the hardware type of a module, fetch the firmware
 *   files needed and try to boot the module. Since the system must be running
 *   (booted) prior to running the pixieserver, we just ask the moudule what FW
 *   its currently running.
 *
 * @param index - Index of module to boot.
 * @param type  - hardware type of module.
 * @throw CXIAException - if any of the API calls fail.
 */
void CBoot::bootModule(int index, int slot, int type) {
  std::string settingsFile = m_config.getModuleSettingsFilePath(index);
  int rv;

// Debugging output can throw on its own!
#ifdef DEBUG
  module_config cfg;
  rv = PixieGetModuleInfo(index, &cfg);
  if (rv < 0) {
    std::stringstream msg;
    msg << "Failed to get module info for module " << index;
    throw CXIAException(msg.str(), "PixieGetModuleInfo()", rv);
  }

  std::cout << "Booting module " << index << " (slot " << slot << ")"
            << " with hardware type " << type << std::endl;
  std::cout << "  System file: " << cfg.fw_device_file[0] << std::endl;
  std::cout << "  FIPPI file: " << cfg.fw_device_file[1] << std::endl;
  std::cout << "  DSP file: " << cfg.fw_device_file[2] << std::endl;
  std::cout << "  Variable file: " << cfg.fw_device_file[3] << std::endl;
  std::cout << "  Settings file: " << settingsFile << std::endl;
#endif

  rv = Pixie16RebootModule(settingsFile.c_str(), index, 0x7f);
  if (rv < 0) {
    std::stringstream msg;
    msg << "Failed to boot module " << index;
    throw CXIAException(msg.str(), "Pixie16RebootModule()", rv);
  }
}