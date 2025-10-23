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

#ifndef __CMDPP16QDC_H
#define __CMDPP16QDC_H

#ifndef __CMDPP_H
#include "CMDPP.h"
#endif

/*!
   The MDPP-16 is a 32 channel fast high resolution time and amplitude digitizer module produced by Mesytec.
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
   -outputformat        integer [0-3]       0:Time(T) and long integral(L), 1:L, 2:T, 3:LT and short integral
   -signalwidth         int[8] [0-1023]     FWHM in ns
   -inputamplitude      int[8] [0-65535]    0 to peak voltage in mV. Maximum value is the jumper range value.
   -jumperrange         int[8] [0-65535]    Range printed on jumper top.
   -qdcjumper           bool[8]             If QDC jumper is used.
   -intlong             int[8] [2-506]      Long integration time. Multiple of 12.5 ns.
   -intshort            int[8] [1-127]      Short integration time. Multiple of 12.5 ns.
                                            This should be smaller than intlong.
   -threshold           int[16] [1-65535]   Threshold to start measuring. Calculated as value/0xFFFF percentage.
   -resettime           int[8] [0-1023]     When OF/UF, input preamp and digital section is resetted.
   -gaincorrectionlong  enum (div4,mult4,none) Either divide by 4 or multiply by 4 to the integral value.
   -gaincorrectionshort enum (div4,mult4,none) Either divide by 4 or multiply by 4 to the integral value.
   -printregisters      bool                Print out all the register values on screen.
\endverbatim

   Comment by Genie:
     - MDPP-16 QDC firmware has tf_gain_correction at 0x612C while MDPP-16 doesn't have one listed in the doc.
     - MDPP-16 QDC chain methods are implemented, but chain mode is not supported as of 05/24/22.
*/

class CMDPP16QDC : public CMesytecBase
{
public:
  typedef std::map<std::string, uint16_t> EnumMap;

private:
  CReadoutModule* m_pConfiguration;

public:
  CMDPP16QDC();
  CMDPP16QDC(const CMDPP16QDC& rhs);
  virtual ~CMDPP16QDC();

private:
  CMDPP16QDC& operator=(const CMDPP16QDC& rhs); // assignment not allowed.
  int operator==(const CMDPP16QDC& rhs) const;	  // Comparison for == and != not suported.
  int operator!=(const CMDPP16QDC& rhs) const;


public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual void onEndRun(CVMUSB& controller);
  virtual CReadoutHardware* clone() const; 

public:
  static EnumMap gainCorrectionMap();

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
