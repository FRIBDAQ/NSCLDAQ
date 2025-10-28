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
 * @file CMADC32.h 
 * @brief Header for support for mesytec MADC32 in VMUSB and MVLC.
 * @note When compiled for mvlcgenerator MVLC_GENERATOR is defined.
 * 
 */

#ifndef __CMADC32_H
#define __CMADC32_h

#include "CMesytecBase.h"
#ifdef MVLC_GENERATOR
#include <DeviceCommand.h>
#endif
#include <stdint.h>
#include <string>
#include <vector>


// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;
#ifdef MVLC_GENERATOR
namespace XXUSB {
  class CConfigurableOjbect;
}
#endif


/*!
   The MADC32 is a 32 channel ADC module produced by Mesytec.
   This module will be used in single event mode.
   The following configuration parameters can be sued to tailor
   the module:

\verbatim
   Name                 Value type          Description
   -base                integer             Base address of the module in VME space.
   -id                  integer [0-255]     Module id (part of the module header).
   -ipl                 integer [0-7]       Interrupt priority level 0 means disabled.
   -vector              integer [0-255]     Interrupt vector.
   -timestamp           bool  (false)       If true enables the module timestamp.
   -gatemode            enum (separate,common)  Determines if the bank gates are
                                            independent or common.
   -holddelays          int[2]              Delay between trigger and gate for each bank.
   -holdwidths          int[2]              Lengths of generated gates.
   -gategenerator       on, off, gdg1, gdg2 Enable gate generator (hold stuff)
   -inputrange          enum (4v,8v,10v)    ADC input range.
   -ecltermination      bool                Enable termination of the ECL inputs.
   -ecltming            bool                Enables ECL timestamp inputs
                                            (oscillator and reset).
   -nimtiming           bool                Enables NIM input for timestamp inputs
                                            (oscillator & rset).
   -timingsource        enum (vme,external)  Determines where timestamp source is.
   -timingdivisor       int [0-15]          Divisor (2^n) of timestamp clock
   -thresholds          int[32] [0-4095]    Threshold settings (0 means unused).
   -multievent          bool (false)        Enable/disablen multi-event mode.
   -irqthreshold        integer 0           # Events before interrupt.
   -resolution          enum (8k)           2k 4k 4khires 8k 8khires ..
                                            possible ADC resolution values.

\endverbatim
*/
class CMADC32 : public CMesytecBase
{

private:
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject*     m_pConfiguration;
#else
  CReadoutModule*     m_pConfiguration;
#endif
public:
  CMADC32();
  
  virtual ~CMADC32();
#ifdef MVLC_GENERATOR
private:
#endif
  CMADC32(const CMADC32& rhs);
  CMADC32& operator=(const CMADC32& rhs);
private:
  int operator==(CMADC32& rhs) const;
  int operator!=(CMADC32& rhs) const;

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

  // The following functions are used by the madcchain module.
  //
  void setChainAddresses(CVMUSB& controller,
			 CMesytecBase::ChainPosition position,
			 uint32_t      cbltBase,
			 uint32_t      mcastBase);

  void initCBLTReadout(CVMUSB& controller, uint32_t cbltAddress, int wordsPermodule);
  // Utilities:

  int resolutionValue(std::string selector); // Resolution string to register value.
  int computeUseGGregister(int gdgEnables, std::string gatemode);
};

#ifdef MVLC_GENERATOR
/**
 * @class CMADC32Command - 
 * generating command for an MADC32 instance.Address
 * 
 */
class CMADC32Command : public DeviceCommand {
public:
  CMADC32Command(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~CMADC32Command();
protected:
  CReadoutModule* createDevice(std::string name);
};

#endif


#endif
