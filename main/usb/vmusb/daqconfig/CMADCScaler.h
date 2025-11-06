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
 * @file CMADCScaler.h 
 * @brief Support the Mesytec MADC32 internal counter as a scaler in VMUSB and MVLC
 * @note MVLC_GENERATOR will be defined when compiling for mvlcgenerate.
 */
#ifndef __CMADCSCALER_H
#define __CMADCSCALER_h


#include <CReadoutHardware.h>
#include <stdint.h>
#include <string>
#include <vector>
#ifdef MVLC_GENERATOR
#include <DeviceCommand.h>
#endif

// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

#ifdef MVLC_GENERATE
namespace XXUSB {
  class CConfigurableObject;
}
#endif

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
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject*     m_pConfiguration;
#else
  CReadoutModule*     m_pConfiguration;
#endif
public:
  CMADCScaler();
  
  virtual ~CMADCScaler();

#ifdef MVLC_GENERATOR
private:
#endif
  CMADCScaler(const CMADCScaler& rhs);
  CMADCScaler& operator=(const CMADCScaler& rhs);
private:
  int operator==(CMADCScaler& rhs) const;
  int operator!=(CMADCScaler& rhs) const;

  // The interface for CReadoutHardware:

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

};

#ifdef MVLC_GENERATOR
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

#endif
