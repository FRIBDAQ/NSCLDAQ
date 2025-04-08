/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321

@author Ron Fox <fox at frib dot msu dot edu>
@brief Header for the driver that generates marker objects.
*/


#ifndef MVLC__CMARKER_H
#define MVLC__CMARKER_H


#include "CReadoutHardware.h"

#include <stdint.h>
#include <string>
#include <vector>
#include "DeviceCommand.h"


// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;
namespace XXUSB {
  class CConfigurableObject;
}

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
  XXUSB::CConfigurableObject*    m_pConfiguration;
public:
  // Class canonicals:

  CMarker();
  virtual ~CMarker();

  // Disallowed canonicals.
private:
  CMarker(const CMarker& CMarker);
  CMarker& operator=(const CMarker& rhs);
  int operator==(const CMarker& rhs);
  int operator!=(const CMarker& rhs);


public:
  virtual void onAttach(XXUSB::CConfigurableObject& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);

private:
  unsigned int getIntegerParameter(std::string name);


};

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
