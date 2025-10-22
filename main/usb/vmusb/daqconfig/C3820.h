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
#ifndef __C3820_H
#define __C3820_H

#include <CReadoutHardware.h>
#include <stdint.h>
#ifdef MVLC_GENERATOR
#include <DeviceCommand.h>
#endif

// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;
#ifdef MVLC_CONFIG
namespace XXUSB {
  class CConfigurableObject;
}
#endif
/*!
   The C3820 is a scaler module that will be read out during scaler events.
   We will run the module in 32 bit latch mode.  In our case, the latch
   impetus will be provided from the VME by the readout code.
   The only configuration parameter is
\verbatim
    Parameter Value Type       Value meaning
    -base     integer          Module base address.
\endverbatim
*/
class C3820 : public CReadoutHardware
{
private:
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject*   m_pConfiguration;
#else
  CReadoutModule*   m_pConfiguration;
#endif
public:
  C3820();
  virtual ~C3820();
#ifdef MVLC_GENERATOR
private:
#endif
  C3820(const C3820& rhs);
  C3820& operator=(const C3820& rhs);
public:
private:
  int operator==(const C3820& rhs) const;
  int operator!=(const C3820& rhs) const;

public:
#ifdef MVLC_GENERATOR
  virtual void onAttach(XXUSB::CConfigurableObject& configuration);
#else
  virtual void onAttach(CReadoutModule& configuration);
#endif
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
#ifndef MVLC_GENERATOR
  virtual CReadoutHardware* clone() const;
#endif
private:
  uint32_t getBase() const;
};


#ifdef MVLC_GENERATOR

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

#endif
