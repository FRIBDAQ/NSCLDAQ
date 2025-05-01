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
#ifndef MVLC_C3820_H
#define MVLC_C3820_H

#include "CReadoutHardware.h"
#include "DeviceCommand.h"
#include <stdint.h>
#include <string>

// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

namespace XXUSB {
  class CConfigurableObject;
}

/*!
   The C3820 is a scaler module that will be read out during scaler events.
   We will run the module in 32 bit latch mode.  In our case, the latch
   impetus will be provided from the VME by the readout code.
   The only configuration parameter is
\verbatim
    Parameter Value Type       Value meaning
    -base     integer          Module base address.
    -timestamp bool            If true (not default), the module only reads the extended width ch0 and 16
                               making it usable for an event timestamp module.
    -inputmode enum            Defines the use of the inputs.  
    -outputmode enum           Defines theuse of the outputs.
\endverbatim
*/
class C3820 : public CReadoutHardware
{
private:
  XXUSB::CConfigurableObject*   m_pConfiguration;
public:
  C3820();
  virtual ~C3820();

private:
  C3820(const C3820& rhs);
  C3820& operator=(const C3820& rhs);
  int operator==(const C3820& rhs) const;
  int operator!=(const C3820& rhs) const;

public:
  virtual void onAttach(XXUSB::CConfigurableObject& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
private:
  uint32_t getBase() const;
};


/**
 * @class C3820Command
 *     Derives from DeviceCommand to create ReadoutModules that wrap a C3820 device driver.
 */
class C3820Command : public DeviceCommand {
public:
  C3820Command(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~C3820Command();
protected:
  virtual CReadoutModule* createDevice(std::string name);
  
};
#endif
