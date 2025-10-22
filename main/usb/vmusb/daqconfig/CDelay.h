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


#ifndef __CDELAY_H
#define __CDELAY_H

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
namespace XXUSB {
  class CConfigurableObject;
}
#endif
/*!
  The CDelay class is a module that inserts a delay into the stack execution.
  The delay is provided in 200-ns units.
    
  Configuration parameter is:

\verbatim
Parameter      Value type              value meaning
-value         integer [0-255]         The number of . This must be provided.
                                        (well it actually defaults to 0).


*/

class CDelay : public CReadoutHardware
{
private:
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject*    m_pConfiguration;
#else
  CReadoutModule*    m_pConfiguration;
#endif
public:
  // Class canonicals:

  CDelay();
  virtual ~CDelay();
#ifdef MVLC_GENERATE
private:
#endif
  CDelay(const CDelay& CDelay);
  CDelay& operator=(const CDelay& rhs);
public:

private:
  int operator==(const CDelay& rhs);
  int operator!=(const CDelay& rhs);


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
  unsigned int getIntegerParameter(std::string name);


};

#ifdef MVLC_GENERATOR
/**
 * @class CDelayCmmand
 *     Derivation of UserCommand to create an instace of CDelay bound into a module.
 */
class CDelayCommand : public DeviceCommand {
public:
  CDelayCommand(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~CDelayCommand();
protected:
  CReadoutModule* createDevice(std::string name);
};

#endif


#endif
