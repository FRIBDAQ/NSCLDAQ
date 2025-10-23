/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Genie Jhang
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CMDPP32PADC_H
#define __CMDPP32PADC_H

#ifndef __CMDPP_H
#include "CMDPP.h"
#endif

/*!
   The MDPP-32 is a 32 channel fast high resolution time and amplitude digitizer module produced by Mesytec.
   The following configuration parameters can be sued to tailor
   the module:

\verbatim
   Name                 Value type          Description
   -base                integer             Base address of the module in VME space.
   -id                  integer [0-255]     Module id (part of the module header).
   -ipl                 integer [0-7]       Interrupt priority level 0 means disabled.
   -vector              integer [0-255]     Interrupt vector.
   -irqdatathreshold    integer [0-32256]   Threshold of the number of 32bit words in FIFO to transfer
   -irqeventthreshold   integer [0-32256]   Threshold of the number of events in FIFO to transfer
   -irqsource           enum (event,data)   Which IRQ threshold to be applied
   -maxtransfer         integer [0-irqth]   The maximum amount of data being transferred at once. See Doc.
   -datalenformat       integer [0-4]       Data length format. See Doc.
   -multievent          integer             Multi event register. See Doc.
   -marktype            enum (eventcounter,timestamp,extended-timestamp)
   -tdcresolution       integer [0-5]       25ns/2^(10-value)
   -outputformat        integer [0-2]       0:Time(T) and Peak Amplitude(A), 1:A, 2:T
   -signalwidth         int[8] [2-2000]     FWHM in unit of 12.5 ns
   -threshold           int[32] [1-65535]   Threshold to start measuring. Calculated as value/0xFFFF percentage.
   -printregisters      bool                Print out all the register values on screen.
\endverbatim
*/

class CMDPP32PADC : public CMesytecBase
{
public:
  typedef std::map<std::string, uint16_t> EnumMap;

private:
  CReadoutModule* m_pConfiguration;

public:
  CMDPP32PADC();
  CMDPP32PADC(const CMDPP32PADC& rhs);
  virtual ~CMDPP32PADC();

private:
  CMDPP32PADC& operator=(const CMDPP32PADC& rhs); // assignment not allowed.
  int operator==(const CMDPP32PADC& rhs) const;	  // Comparison for == and != not suported.
  int operator!=(const CMDPP32PADC& rhs) const;


public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual void onEndRun(CVMUSB& controller);
  virtual CReadoutHardware* clone() const; 

public:
  void setChainAddresses(CVMUSB& controller,
                         CMesytecBase::ChainPosition position,
                         uint32_t      cbltBase,
                         uint32_t      mcastBase);

  void initCBLTReadout(CVMUSB& controller,
                       uint32_t cbltAddress,
                       int wordsPermodule);


private:
  void printRegisters(CVMUSB& controller);
};

#endif
