/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
      Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CMQDC32RdoHdwr_H
#define __CMQDC32RdoHdwr_h

#include <CReadoutHardware.h>
#ifdef MVLC_GENERATOR
#include "DeviceCommand.h"
#endif

#include <stdint.h>
#include <string>
#include <vector>
#include <CMQDC32StackBuilder.h>
#include <CMesytecBase.h>

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
   The MQDC32 is a 32 channel QDC module produced by Mesytec.
   This module can be used in single and multievent mode, though it is most sensible
   to use it in single event mode with VMUSBReadout.
   The following configuration parameters can be sued to tailor
   the module:

\verbatim
   Name                 Value type          Description
   -base                integer             Base address of the module in VME space.
   -id                  integer [0-255]     Module id (part of the module header).
   -ipl                 integer [0-7]       Interrupt priority level 0 means disabled.
   -vector              integer [0-255]     Interrupt vector.
   -irqthreshold        integer 0           # Events before interrupt.
   -multievent          bool (false)        Enable/disablen multi-event mode.
   -bankoffsets         int [2]             Shifts adc output by +-1000 ch for each bank
   -gatemode            enum (separate,common)  Determines if the bank gates are
                                            independent or common.
   -exptrigdelays       int[2]              Delay between trigger and gate for each bank.
   -gatelimits          int[2]              Lengths of generated gates.
   -inputcoupling0      enum  (AC,DC)       Determines electrical coupling for bank 0 inputs  
   -inputcoupling1      enum  (AC,DC)       Determines electrical coupling for bank 1 inputs  
   -pulser              bool (false)        Enables/disables pulser
   -pulseramp           int [0-255]         Amplitude of pulser for useramplitdue mode
   -ecltermination      bool                Enable termination of the ECL inputs.
   -ecltming            bool                Enables ECL timestamp inputs
                                            (oscillator and reset).
   -nimtiming           bool                Enables NIM input for timestamp inputs
                                            (oscillator & rset).
   -timestamp           bool  (false)       If true enables the extended timestamp.
   -timingsource        enum (vme,external)  Determines where timestamp source is.
   -timingdivisor       int [0-15]          Divisor (2^n) of timestamp clock
   -syncmode            enum                Determines how the tstamp resets
   -multlowerlimits     int[2] [0-0x3f]     Specifies lower multiplicility limits 
   -multupperlimits     int[2] [0-0x3f]     Specifies upper multiplicility limits 
   -thresholds          int[32] [0-4095]    Threshold settings (0 means unused).
   -usethresholds       bool                Determines whether to use thresholds

   
\endverbatim
@note If compiled for the mvlcgenerator MVLC_GENERATOR will be a defined preprocessor symbol.
*/
class CMQDC32RdoHdwr : public CMesytecBase
{
private:
  MQDC32::CMQDC32StackBuilder     m_logic;
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject*     m_pConfig;
#else
  CReadoutModule*     m_pConfig;
#endif
public:
  CMQDC32RdoHdwr();
  virtual ~CMQDC32RdoHdwr();
#ifdef MVLC_GENERATOR
private:
#endif
  CMQDC32RdoHdwr(const CMQDC32RdoHdwr& rhs);
  CMQDC32RdoHdwr& operator=(const CMQDC32RdoHdwr& rhs);
private:
  int operator==(CMQDC32RdoHdwr& rhs) const;
  int operator!=(CMQDC32RdoHdwr& rhs) const;

public:
  // The interface for CReadoutHardware:

#ifdef MVLC_GENERATOR
  virtual void onAttach(XXUSB::CConfigurableObject& configuration);
#else
  virtual void onAttach(CReadoutModule& configuration);  
#endif
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual void onEndRun(CVMUSB& ctlr);
#ifndef MVLC_GENERATOR  
  virtual CReadoutHardware* clone() const;
#endif

  // The following functions are used by the madcchain module.
  //
  virtual void setChainAddresses(CVMUSB& controller,
                           			 ChainPosition position,
                            	   uint32_t      cbltBase,
                  			         uint32_t      mcastBase);

  virtual void initCBLTReadout(CVMUSB& controller, 
                               uint32_t cbltAddress, 
                               int wordsPermodule);
  // Utilities:

private:
  // Utility methods to deal with the logic associated with either an individual
  // or group of options.  These all add vme operations to a readout list that 
  // are later executed.  For the most part, they are just chunks of the logic 
  // associated with the Initialize method.
  void configureModuleID(CVMUSBReadoutList& list);
  void configureThresholds(CVMUSBReadoutList& list);
  void configureMarkerType(CVMUSBReadoutList& list);
  void configureMemoryBankSeparation(CVMUSBReadoutList& list);
  void configureGates(CVMUSBReadoutList& list);
  void configureBankOffsets(CVMUSBReadoutList& list);
  void configureTestPulser(CVMUSBReadoutList& list);
  void configureInputCoupling(CVMUSBReadoutList& list);
  void configureTimeDivisor(CVMUSBReadoutList& list);
  void configureECLTermination(CVMUSBReadoutList& list);
  void configureECLInputs(CVMUSBReadoutList& list);
  void configureNIMInputs(CVMUSBReadoutList& list);
  void configureNIMBusy(CVMUSBReadoutList& list);
  void configureTimeBaseSource(CVMUSBReadoutList& list);
  void configureIrq(CVMUSBReadoutList& list);
  void configureMultiEventMode(CVMUSBReadoutList& list);
  void configureMultiplicity(CVMUSBReadoutList& list);
  void configureCounterReset(CVMUSBReadoutList& list);

  uint32_t getBase();

};

#ifdef MVLC_GENERATOR

// for devoceinstance creation in the configuration:


/**
 *  @class MqdcCommand
 * 
 *    Derive from DevicCommand to create CMQDCRdoHardware wrapped in CReadoutModule object.
 */
class MqdcCommand : public DeviceCommand {
public:
  MqdcCommand(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~MqdcCommand();

protected:
  CReadoutModule* createDevice(std::string name);
};

#endif


#endif
