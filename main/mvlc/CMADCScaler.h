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
#ifndef MVLC_CMADCSCALER_H
#define MVLC_CMADCSCALER_h


#include "CReadoutHardware.h"
#include "DeviceCommand.h"
#include <stdint.h>
#include <string>
#include <vector>

// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;
namespace XXUSB {
  class CConfigurableObject;
}
/*!
  This module provides support for a 'scaler' that reads two of the time
  counters in an MADC32 module.  The counters read are the
  daq_time_lo/hi counter which provides the time the adc is dead, and the
  time_0/time_1 registers.  Initialization will zero these time counters.

  Using these values provides a dead-time information for the system.

Configuration parameters:

\verbatim
   -base     integer   - Base address of the module.
\endverbatim


*/
class CMADCScaler : public CReadoutHardware
{
private:
  XXUSB::CConfigurableObject*     m_pConfiguration;
public:
  CMADCScaler();
  
  virtual ~CMADCScaler();
  
private:
  CMADCScaler(const CMADCScaler& rhs);
  CMADCScaler& operator=(const CMADCScaler& rhs);
  int operator==(CMADCScaler& rhs) const;
  int operator!=(CMADCScaler& rhs) const;

  // The interface for CReadoutHardware:

public:
  virtual void onAttach(XXUSB::CConfigurableObject& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);


};

/**
 *  @class CMADCScalerCommand 
 *    DeviceCommand that crate CMADCSCaler objects.
 */
class CMADCScalerCommand : public DeviceCommand {
public:
  CMADCScalerCommand(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~CMADCScalerCommand();

protected:
  CReadoutModule* createDevice(std::string name);
};
#endif
