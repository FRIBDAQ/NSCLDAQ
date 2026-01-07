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
 * @file C785.h
 * @brief Header for the VMUSB/MVLC driver for the CAEN 32 channel analog converters.
 * @note When compiled for the MVLCGenerator the preprocessor symbo MVLC_GENERATOR is defined.
 */

#ifndef C785_H
#define C785_H


#include <CReadoutHardware.h>
#include <stdint.h>
#include <string>
#include <vector>
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
                                               defaults to 0 which, won't program the GEO [1]
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
   -inputs              ribbon | NIM          implicitly defines the number of inputs the module has
                                               ribbon - 32, NIN - 16.  Used by SpecTcl in decoding.                                           
    The MVLC generator version also adds:
   
    -type                 adc* | tdc | qdc     type of module for module dependent code.

   These will also have reasonable defaults programmed into them.. see
   the implementation of onAttach for more information about the defaults.

  NOTES:
    @note [1] - programming the GEO on a module with CERN backplane extensions (gets the GEO from the 
          P3 connector) with the MVLC is a fatal error.  If -geo is 0, the GEO register is not programmed.
          since on the other modules the reset will set GEO to zero this works fine on modules which require
          GEO Programming, however the use of -geo 0 is discouraged.
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
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject* m_pConfiguration;
#else
  CReadoutModule*    m_pConfiguration;
#endif
public:
  // Class canonicals:

  C785();
#ifdef MVLC_GENERATOR
private:                      // MVLCGenerator does not implement these
#endif
  C785(const C785& rhs);      // canonicals but VMUSB does.
  C785& operator=(const C785& rhs);
public:

  virtual ~C785();
  
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
  unsigned int  getIntegerParameter(std::string name);
  bool getBoolParameter(std::string name);
  void getThresholds(std::vector<uint16_t>& thresholds);
  int  getModuleType(CVMUSB& controller, uint32_t base);
};



#ifdef MVLC_GENERATOR
// The command to add the 'adc' command to the generator's interpreter:

class C785Command : public DeviceCommand {
public:
  C785Command(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~C785Command();

protected:
  virtual CReadoutModule* createDevice(std::string name);
};
#endif
#endif
