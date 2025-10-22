#ifndef __C3804_H
#define __C3804_H
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

/**
 * @file C3804.h
 * @brief header for supporting SIS 3804 scaler in MVLC an VMUSB
 * @note when compiling for mvlcgenerate, MVLC_GENERATOR is defined.
 */
#include "CReadoutHardware.h"
#include <string>
#include <stdint.h>
#ifdef MVLC_GENERATOR
#include "DeviceCommand.h"
#endif

// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

#ifdef MVLC_GENERATOR
namespace XXUSB {
  class CConfigurableObject;
}
#endif

/*!
  The C3804 class represents the SIS C3804 scaler module.  This module will be readout
during scaler events.  We will run the module as a software latched scaler, by
preceding a read with a clock shadow register. operation.  The Front panel will be run in
input mode 1, that is
\verbatim:
Input1:    Disable all counters.
Input2:    Clear all channels.
Input3:    Externally clock shadow registers.
Input4:    External test pulse (presumably counts all channels?

\verbatim.

The following configuration parameters are supported:

\endverbatim

Option       Value type       Default    Meaning
-base        integer          0          Base address of the module.
-refpulser   bool             false      enable reference pulser to channel 0 if true.
-disables    integer          0          Mask of channels in which to disable counting.
-autoclear   bool             true       Read and clear rather than allowing scalers to
                                         continuously count.

\endverbatim


*/


class C3804 : public CReadoutHardware
{
private:
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject* m_pConfiguration;
#else
  CReadoutModule*   m_pConfiguration;
#endif
public:
  C3804();
  virtual ~C3804();
#ifdef MVLC_GENMERATOR
private:
#endif  
  C3804(const C3804& rhs);
  C3804& operator=(const C3804& rhs);
public:
private:
  int operator==(const C3804& rhs) const;
  int operator!=(const C3804& rhs) const;


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
  uint32_t   getIntegerParameter(std::string name) const;
  bool       getBoolParameter(std::string name)    const;

  // 32 bit transfers that throw if there's an error:

  void       checkRead(CVMUSB& controller, uint32_t address, uint32_t& value);
  void       checkWrite(CVMUSB& controller, uint32_t address, uint32_t value);
};   

#ifdef MVLC_GENERATOR

/**
 *  @class SIS3804Command 
 *   DeviceCommand Deriviation to creat modules wrapping C3804 drivers
 */

 class SIS3804Command : public DeviceCommand {
public:
  SIS3804Command(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~SIS3804Command();

protected:
  CReadoutModule* createDevice(std::string name);
 };
#endif
#endif
