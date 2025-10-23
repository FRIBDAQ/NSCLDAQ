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
 * @file CMarker.h
 * @brief Header for Makers in the VMUSB and MVLC
 * @note When compiling for mvlc, MVLC_GENERATOR is defined
 */
#ifndef __CMARKER_H
#define __CMARKER_H

#include <CReadoutHardware.h>
#include <stdint.h>
#include <string>
#include <vector>

#ifdef MVLC_GENERATOR
#include <DeviceCommand.h>
namespace XXUSB {
  class CConfigurableObject;
}
#endif
// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

/*!
  The CMarker class is a module that inserts a marker word into the output
  data stream.  The marker word is a 16 bit datum. You can insert a 32 bit
  marker by inserting two of these with the low order 16 bits first.

  Configuration parameter is:

\verbatim
Parameter      Value type              value meaning
-value         integer [0-0xffff]       The marker word to insert. This must be provided.
                                        (well it actually defaults to 0).


*/

class CMarker : public CReadoutHardware
{
private:
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject*    m_pConfiguration;
#else
  CReadoutModule*    m_pConfiguration;
#endif
public:
  // Class canonicals:

  CMarker();
  
  virtual ~CMarker();
#ifdef MVLC_GENERATOR
private:
#endif
  CMarker(const CMarker& CMarker);  
  CMarker& operator=(const CMarker& rhs);

private:
  int operator==(const CMarker& rhs);
  int operator!=(const CMarker& rhs);


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
 * @class CMarkerCommand
 *    Derived class from DeviceCommand that generates
 * CMarker objects.
 */
class CMarkerCommand : public DeviceCommand {
public:
  CMarkerCommand(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~CMarkerCommand();
protected:
  virtual CReadoutModule* createDevice(std::string name);
};

#endif


#endif
