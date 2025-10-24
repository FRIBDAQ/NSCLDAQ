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

#ifndef _CV1495SC_H
#define _CV1495SC_H


#include <CReadoutHardware.h>
#include <stdint.h>
#include <string>
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
 * CV1495sc is a scaler module with 128 channels that will be typically read out
 * during scaler events.  It is very much like a V830 except that
 * * It has a header on the MEB format.
 * * It has 128 channels worth of inputs.
 *
 * The following are the configuration options
 *
 \verbatim
   Parameter      Type      Default    meaning
   -base          unsigned  0          Module base address, Must match the module settings.
   -exttrig       bool      false      External trigger enabled.
   -inttrig       bool      false      Internal trigger enabled, See -dwelltime
   -vmetrigger    bool      true       Readout will be preceeded by a swtrigger.
   -inputlevel    enum      nim        Input voltage levels.  {nim, ttl}
   -autoreset     bool      true       External channels are reset on a trigger.
   -g1_mode       enum      inhibit    Purpose of the G1 input: {inhibit, reset, test}
   -includetime   bool      false      If true the time tag word is included in the event.
   -bank1enables  unsigned  0xffffffff Bits are 1 for each channel enabled in bankn
   -bank2enables  unsigned  0xffffffff
   -bank3enables  unsigned  0xffffffff
   -bank4enables  unsigned  0xffffffff
   -dwelltime     unsigned  2000000    Microseconds between internal triggers if they are enabled.

\endverbatim

*/

class CV1495sc : public CReadoutHardware
{
private:
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject*   m_pConfiguration;
#else
  CReadoutModule*   m_pConfiguration;
#endif
public:
  CV1495sc();
  virtual ~CV1495sc();

#ifdef MVLC_GENERATOR
private:
#endif
  CV1495sc(const CV1495sc& rhs);
  CV1495sc& operator=(const CV1495sc& rhs);
private:
  int operator==(const CV1495sc& rhs) const;
  int operator!=(const CV1495sc& rhs) const;

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
  uint32_t mapEnum(std::string value, 
		   const char** stringArray, const uint32_t* valueArray);
  size_t countBits(uint32_t mask);
};

#ifdef MVLC_GENERATOR

/**
 * @class V1495Command
 *    Deriviation from DeviceCommand to instantiate CV1495sc modues from scrip.
 * 
 */
class V1495Command: public DeviceCommand {
public:
  V1495Command(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~V1495Command();

protected:
  CReadoutModule* createDevice(std::string name);
};
#endif


#endif
