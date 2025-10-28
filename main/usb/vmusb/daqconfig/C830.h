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
 * @file C830.h
 * @brief Definition of VMUSB and MVLC device support for the V830 scaler.
 * @note when compiled for mvlcgenerator  MVLC_GENERATOR is defined.
 */
#ifndef __C830_H
#define __C830_H

#include <CReadoutHardware.h>
#include <stdint.h>
#include <string>
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
   The C830 supports the CAEN V830 scaler module.  This module will be created via a command like:


\verbatim
v830 create name base-address
\endverbatim

    Once created it can be configured via:

\verbatim
v830 config name option value ?...?

where the options/values are:

Name        Type/range      Default       Purpose
-channels   integer         0xffffffff    Selectively enable channels.
-dwelltime  integer         0             If in periodic trigger mode, the time between triggers
-header     bool            false         If true data will include a header.
-trigger    random |        vme           Specifies how the module triggers an event. random
            periodic |                    means the external latch signal latches the scalers
            vme                           to an event, periodic means the scaler is triggered every
                                          'dwelltime' * 400ns.  vme means the readout list triggers
                                          the scaler.
-wide      bool             true          true  - Scalers are 32 bit value, false, 26 bit values.
                                          note that 26 bit wide scaler readout is tagged in the
                                          top bits with the channel number.
-header    bool              false        If true scaler data includes a header as described in
                                          fig 3.1 of the manual.
-autoreset bool              true         If true, the latch operation also clears the counters.
-geo       integer (0-0x1f)  0            The geograhpical address of the module (from PAUX or
                                          as programmed, see below).
-setgeo    bool              false        If true, the geographical address is programmed,
                                          this can only be done for modules without the PAUX
                                          connector...otherwise, the geo address comes from
                                          the PAUX connector.
-ipl       integer 0-7       0            If non zero enables the module to interrupt when
                                          the highwater mark is reached.
-vector    integer 0-255     0xdd         Status/id value to interrupt on.
-highwatermark integer (0-0xffff) 1       How many events need to be in the module to interrupt.

\endverbatim
*/
class C830 : public CReadoutHardware
{
private:
  typedef enum _TriggerSource {
    random,
    periodic,
    vme
  } TriggerSource;
private:
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject*    m_pConfiguration;
#else  
  CReadoutModule*    m_pConfiguration;
#endif

public:
  C830();
  virtual ~C830();
#ifdef MVLC_GENERATOR
private:                      // These are not impolementyed in mvlc
#endif
  C830(const C830& rhs);
  C830& operator=(const C830& rhs);
public:
private:
  int operator==(const C830& rhs) const;
  int operator!=(const C830& rhs) const;
public:

  // Operations:

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
  uint32_t getIntegerParameter(std::string name) const;
  bool     getBooleanParameter(std::string name) const;
  TriggerSource getTriggerSource() const;


};

#ifdef MVLC_GENERATOR
/**
 * @class V830Command 
 *    Derivation from DeviceCommand to build modules that wrap C830 drivers.
 */
class V830Command : public DeviceCommand {
public:
  V830Command(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~V830Command();

protected:
  CReadoutModule* createDevice(std::string name);
};
#endif

#endif
