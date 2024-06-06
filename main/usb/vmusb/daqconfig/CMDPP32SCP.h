/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2024.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Genie Jhang
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CMDPP32SCP_H
#define __CMDPP32SCP_H

#ifndef __CMDPP_H
#include "CMDPP.h"
#endif

Const(TFIntDiff)            0x6110;
Const(PZ0)                  0x6112;
Const(PZ1)                  0x6114;
Const(PZ2)                  0x6116;
Const(PZ3)                  0x6118;
Const(Gain)                 0x611a;
Const(ShapingTime)          0x6124;
Const(BLR)                  0x6126;
Const(SignalRiseTime)       0x612a;


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
   -adcresolution       enum (4k,8k,16k,32k,64k)
   -outputformat        integer [0-2]       0:Standard - Time and amplitude, 1:Amplitude only, 2:Time only
   -tfintdiff           int[8] [1-127]      TF integration/differentiation time in 12.5 ns unit.
   -pz                  int[32] [64-65535]  Signal decay time in 12.5 ns unit. Infinite=65535. Not defined [64001-65534].
   -gain                int[8] [100-25000]  Gain. 100 means gain 1. 25000 means gain 250.
   -threshold           int[32] [0-64000]   Threshold to start measuring. 64000 corresponds to the full range.
   -shapingtime         int[8] [8-2000]     Shaping time in 12.5 ns unit. 8 = 100 ns. 2000 = 25 us.
   -blr                 int[8] [0-2]        0: off, 1: int time = 4 shaping time, 2: int time = 8 shaping time
   -signalrisetime      int[8] [0-127]      Signal rise time in 12.5 ns unit.
   -resettime           int[8] [16-1023]    When OF/UF, input preamp and digital section is reset.
   -printregisters      bool                Print out all the register values on screen.
\endverbatim

   Comment by Genie:
     - MDPP-32 SCP chain methods are implemented, but chain mode is not supported as of 05/24/22.
*/

class CMDPP32SCP : public CMesytecBase
{
public:
  typedef std::map<std::string, uint16_t> EnumMap;

private:
  CReadoutModule* m_pConfiguration;

public:
  CMDPP32SCP();
  CMDPP32SCP(const CMDPP32SCP& rhs);
  virtual ~CMDPP32SCP();

private:
  CMDPP32SCP& operator=(const CMDPP32SCP& rhs); // assignment not allowed.
  int operator==(const CMDPP32SCP& rhs) const;	  // Comparison for == and != not suported.
  int operator!=(const CMDPP32SCP& rhs) const;


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
