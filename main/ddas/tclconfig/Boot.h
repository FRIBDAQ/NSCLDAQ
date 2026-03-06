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

/** @file  Boot.h
 *  @brief Provides pixie16::boot to boot a single module.
 */

#ifndef BOOT_H
#define BOOT_H

#include <string>

#include "CTclCommand.h"

// Forward definitions:

namespace DAQ {
namespace DDAS {
class Configuration;
}
} // namespace DAQ

/**
 * @class CBoot
 *    This class provides the pixie16::boot command which accepts
 *    a two parameters, the module *index* of the module to boot.
 *    If the user wants to boot by slot number, they must inventory
 *    the modules and look through the inventory to figure out which
 *    index corresponds to the desired slot. Note that the boot pattern
 *    will only be 0x7f which boots everything.
 * @note As of 6 March 2026, the boot feature is not fully functional. If the
 *    module is running a managed firmware set (XIA standard firmware directory
 *    structure), then the boot process will work. If the user overrides the
 *    system firmware set, the reboot process will fail with an "invalid
 *    firmware" error despite initially booting properly. This issue has been
 *    reported to XIA and is currently under investigation.
 */
class CBoot : public CTclCommand {
private:
  DAQ::DDAS::Configuration &m_config;

public:
  CBoot(Tcl_Interp *pInterp, DAQ::DDAS::Configuration &config);
  virtual ~CBoot();

  virtual int operator()(std::vector<Tcl_Obj *> &objv);

private:
  int getHardwareType(int index);
  void bootModule(int index, int type);
};
#endif
