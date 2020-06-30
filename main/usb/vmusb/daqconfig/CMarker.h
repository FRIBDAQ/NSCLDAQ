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


#ifndef CMARKER_H
#define CMARKER_H

#include "CReadoutHardware.h"
#include <stdint.h>
#include <string>
#include <vector>


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
  CReadoutModule*    m_pConfiguration;
public:
  // Class canonicals:

  CMarker();
  CMarker(const CMarker& CMarker);
  virtual ~CMarker();
  CMarker& operator=(const CMarker& rhs);

private:
  int operator==(const CMarker& rhs);
  int operator!=(const CMarker& rhs);


public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;

private:
  unsigned int getIntegerParameter(std::string name);


};


#endif
