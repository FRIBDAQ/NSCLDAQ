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

#ifndef _CV1729_H
#define _CV1729_H

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
#ifdef MVLC_GENERATOR
namespace XXUSB
{
  class CConfigurableObject;
} // namespace XXUSB

#endif


/**
 *  Support the CAEN V1729A flash adc module.  This is a waveform digitizer module:
 *  4 channels with 2520 usable points per trigger.
 *  Sampling speeds can run up to 2Ghz into an analog memory but with the 5MHz adc
 *  and low onboard clock speeds the board has several issues to be concerned with:
 *  # At startup 150usec of dead time are required for the board's clock to stabilize.
 *  # After trigger 650usec are required before the board can be read.  If the ADC is triggering
 *    readout via it's interrupt this is unimportant.  If, however the VM-USB triggers readout
 *    it may be necessary to block the readout stack prior to proceeding to read this module.
 *    as the VM-USB can delay at most 256usec prior to readout a delay parameter allows further
 *    delay elements to be inserted in the readout list prior to executing the readout of the
 *    module
 *  \note the 1729 has a completely different register set and cannot use this class.

 * 
 * Here are the configuration parameters supported by this module:
 *\verbatim
 *   Parameter        Default           Usage
 *   -base            0                Module base address.
 *   -irqenable       false            Turns on end of digitization interrupts.
 *   -threshold       4095             Threshold for internal trigger.
 *   -pretrigger      10240            Pretrigger time in samples.
 *   -posttrigger     64               Post trigger time in samples.
 *   -triggersource   external         Specifies trigger source: his can also be 'internal', or 'both'
 *   -triggeredge     rising           Specifies which edge of the trigger is used can be falling too.
 *   -triggermask     off              Specifies whether the trigger is masked via the EXT_EN_TRIGGER
 *                                     if this is 'on' EXT_EN_TRIGGER is required to enable triggers
 *                                     to fire.
 *   -triggerchannels 0xf              Only used if internal triggers are allowed.  Set one bit for each
 *                                     channel from which triggers are allowed.
 *  -poststoplatency  4                Sets the post trigger latency register 
 *  -postlatencypretrig 1              Sets the value of the post latency pretrigger register.
 *  -samplingfreq     2ghz             Sampling frequency, can also be 1Ghz.
 *  -delay            0                uSec to delay prior to executing the readout.
 *                                     this can be set to zero if the modle interrupt is 
 *                                     used to trigger the data acquisition else it should be set
 *                                     to on the order of 700usec depending on triggering parameters.
 *                                     650usec from end of the signal are required to transfer data.
 *                                     the remaining time depends on the time of the computer trigger
 *                                     relative to the time of the signal end.
 *
 *
 *\endverbatim
 */
class CV1729 : public CReadoutHardware
{
  // Exported data types

private:
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject*    m_pConfiguration;
#else
  CReadoutModule*    m_pConfiguration;
#endif
public:
  // Class canonicals:

  CV1729();
  
  virtual ~CV1729();
#ifdef MVLC_GENERATOR
private:
#endif
  CV1729(const CV1729& rhs);
  CV1729& operator=(const CV1729& rhs);
private:
  int operator==(const CV1729& rhs) const;
  int operator!=(const CV1729& rhs) const;


  // overridable : operations on constructed objectgs:

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

  // utilities:
private:
  unsigned enumToValue(const char*     pValue,
		       const char**    ppLegalValues,
		       const unsigned* pMap);


};

#ifdef MVLC_GENERATOR
/**
 *  @class V1729aCommand
 *     Derived from DeviceCommand to provide the v1729a command to create/config those drivers.
 */
class V1729aCommand : public DeviceCommand {
public:
  V1729aCommand(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~V1729aCommand();

protected:
  CReadoutModule* createDevice(std::string name);
};

#endif


#endif
