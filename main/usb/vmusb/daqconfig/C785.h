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

#ifndef __C785_H
#define __C785_H

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

/*!
   The V785 is an ADC module that will be read out during physics triggers.
   We will run the module in multievent singles mode.  The module will be set
   up so that it will BERR when read dry.  The readout list will be a 
   block transfer from the module of 32 full sized events.  This is highly
   tuned to the needs of the LLNL neutron imaging system..where events are
   essentially singles.

   Configuration parameters are:

   \verbatim
   Parameter               Value type          Value meaning.
   -base                   integer             Base address of the module.
   -geo                    integer             Geographical address programmed into the module
   -thresholds             integer[32]         32 threshold values that manage the 0 supression
   -smallthresholds        bool                True if 'small' thresholds used.
   -ipl                    integer             Interrupt priority level
   -vector                 integer             Interrupt vector.
   -highwater              integer             Number of events needed to fire interrupt.
   -fastclear              integer             Fast clear interval values are in units of
                                               1/32 of a usec.
   -supressrange           bool                true to supress out of range data.
   -supresseunderthreshold bool                true to supress under threshold
   -supresseoverflow       bool                true to supress overflow
   -timescale              int                 The full scale range in ns.  Note there is
                                               a granularity of 35ps to the range.
                                               this must be an integer between 140 and 1200.
                                               Defaults to 600ns.
   \endverbatim

   These will also have reasonable defaults programmed into them.. see
   the implementation of onAttach for more information about the defaults.

*/
class C785 : public CReadoutHardware
{
  // Exported data types
public:
  typedef enum _Position {
    leftmost,
    middle,
    rightmost
  } Position;
private:
  CReadoutModule*    m_pConfiguration;
public:
  // Class canonicals:

  C785();
  C785(const C785& rhs);
  virtual ~C785();
  C785& operator=(const C785& rhs);
private:
  int operator==(const C785& rhs) const;
  int operator!=(const C785& rhs) const;

  // operations specific to a C785 object:

public:
  void addToChain(CVMUSB& controller,
		  uint32_t mcstAddress,
		  Position where);

  // overridable : operations on constructed objectgs:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;


  // utilities:
private:
  unsigned int  getIntegerParameter(std::string name);
  bool getBoolParameter(std::string name);
  void getThresholds(std::vector<uint16_t>& thresholds);
  int  getModuleType(CVMUSB& controller, uint32_t base);
};


#endif
