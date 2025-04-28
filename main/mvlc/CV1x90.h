#ifndef MVLC_CV1X90_H
#define MVLC_CV1X90_H

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

#include "CReadoutHardware.h"
#include "DeviceCommand.h"
#include <stdint.h>
#include <string>
#include <vector>
#include <map>

// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;
namespace XXUSB {
  class CConfigurableObject;
}


/*!
  This class supports the CAEN V1190 and V1290 multihit TDC.  The two modules
  are essentially the same board from the software point of view.
  The difference is the set of resolutions and number of channels supported by the
  module.  This comes into play in defining the size of some of the lists that
  are arguments to some configuratino parameters.  There are really four cases
  we will need to deal with:
  - CAEN V1190A - 128 channels
  - CAEN V1190B -  64 channels
  - CAEN V1290A -  32 channels
  - CAEN V1290N -  16 channels.

  The validators we use will accept 16 through 128 channel lists, when we actually
  initialize a module, we'll do further validation to ensure that the module channel
  counts actually match the required list sizes.

  In any event, the module is pretty complex.  It supports continuous storage
  mode, where hits are just clocked relative to some base time that can be reset under
  program or external control (bunch reset time), or trigger matching mode, where
  hits are organized into events that consist of hits that occured within some time window
  of a trigger input.

  This software only supports trigger matching mode, as it's the most useful mode
  for the experiments this software tends to be used for.  Here are the configuration
  parameters we support:

\verbatim
Parameter Name      Value Type     Value Meaning
-base               integer        Base address of the module. Default: 0
-vsn                (0..31)        Virtual slot number.  CAEN calls this a GEO address,
                                   however our assumption is that the module does not actually
                                   do geographical addressing.  In the event it does, you
				   should program this to the actual, physical slot of the
				   module..and the base addressing system will still be used.
                                   The -vsn tags the data from a module so that you know
				   which data comes from where.
				   Default: 0
-ipl                0..7           The interrupt priority level at which you want the module
                                   to interrupt on if there are events.  This should only be
                                   nonzero if you put the module in a list that is interrupt
                                   triggered, and you want the TDC to trigger the list.
				   Default: 0
-vector             0..255         The interrupt vector.  See -ipl above.  When used,
                                   this should match the vector number that will be used to
                                   trigger the VM-USB readout.
				   Default: 0
-termination       {none, switch,  Describes the termination of the module.  'none' means
                    on}            the module is unterminated, 'switch means' that the switch
                                   on the module board controls termination, while 'on'
                                   means the module will be terminated.
                                   DEFAULT: on
-tagtime            bool           If enabled, the trigger time will be included in the events.
                                   DEFAULT: enabled
-highwatermark      0..65535       Determines the number of events that must be buffered in the
                                   TDC for an interrupt to fire (if IPL > 0). 
                                   DEFAULT:  1 which means that
                                   an interrupt will fire whenever there is any data in the
				   module.  This is ignored if -ipl = 0.
-ecloutput         {ready, full,   Defines the meaning of the ecl programmable output pin.
                    almostfull, error} DEFAULT: ready
-window             1..4095        The width of the trigger matching window in 25 ns units.
                                   This is the effective range of the TDC.  See also
				   -offset, -extramargin, and -rejectmargin
                                   DEFAULT: 40 => 1000ns.  
-offset             -2048..40      Specifies where time the trigger matching window starts
                                   relative to the trigger time.  See the manual 2.4.1
                                   for additional constraints that are not enforced by this
                                   software.    The units of the offset are 25ns.
				   Note that the actual window start time will jitter by
				   +/-25ns, so you should fan a trigger signal into a channel
				   and compare hit times to that trigger time for best
				   results.
				   DEFAULT -40 => -1usec.
-extramargin        0..4095        The extra search margin for hits.  This is the addtional
                                   time during which the module will search for hits that
                                   are within the window before declaring the event.  
				   This is needed because hits are searched for in the
				   module's L1 buffer. Contention may prevent matching hits
				   from being written to the L1 buffer for some time after
				   they have actually occured.  See 2.4.1 in the manual.
				   The units of this value are also 25ns.
				   DEFAULT: 8 => 200ns.
-rejectmargin      0..4095         The reject margin.  This is also in 25ns units.  While
                                   the module is in continuous storage, it maintains a reject
                                   counter that flushes hits from the buffer when the
                                   trigger window is not active.  The module will only
				   retain hits for the width of the trigger 
				   window + offset + reject marjin before throwing them away
                                   if there is no trigger.  This ensures the TDC buffer does
                                   not overflow and that the search for matching hits on the
				   trigger is rapid.  
				   DEFAULT: 4 => 100ns.
-triggerrelative   bool            If enabled, the trigger time is subtracted from all the
                                   hits.  Note again that the trigger time is 
				   only precise to 25ns. Precise timing relative to the trigger
				   can only be done by subtracting a digitized trigger time
                                   from the hits.
				   DEFAULT: enabled
-edgedetect       {pair, leading,  Sets the module edge detect mode.   Figure 2.2 provides
                   trailing, both}  documentation about what this means, however here goes:
                                   'pair' provides the width of a pulse in a 
                                   channel, 'leading' provides a hit time at the leading
                                   edge of a pulse, 'trailing' provides a hit time at the
                                   trailing edge of an input pulse, and 'both' provides
				   the time of both the leading and trailing edges of a pulse
				   DEFAULT: leading
-edgeresolution  {800ps, 200ps,    Selects the resolution for the leading/trailing resolution.
                  100ps, 25ps}     It is an error to use 25ps if the module is not a V1290
                                   DEFAULT: 100ps
-leresoluton     {100ps, 200ps,    In leading, trailing and both mode, sets the resolution
                  400ps, 800ps,    with which the leading edge is detected.  
		  1.6ns, 3.12ns,   DEFAULTS: 100ps.
		  6.25ns, 12.5ns} 
		                   
-widthresolution {100ps, 200ps,    Sets the resolution with which a pulse width is
                  400ps, 800ps,    measured in pair mode. 
		  1.6ns, 3.2ns,    DEFAULT: 100ps
		  6.25ns 12.5ns
		  400ns, 800ns}
-deadtime        {5ns, 10ns, 30ns, Sets the double hit resolution.
                  100ns}           DEFAULT: 5ns.
-encapsulatechip  bool             If true, the data from a chip is encapsulated as shown
                                   in figures 6.2/6.4 by a TDC Chip header/trailer.
				   DEFAULT: enabled
-maxhits        {0, 1, 2, 4, 8,    Specifies the maximum number of hits for each TDC chip
                 16, 32, 64, 128,  in an event.  Note that a TDC chip has 32 channels.
		 infinite}         DEFAULT: infinite
-errormark       bool              If true, when an error is detected, an error mark item
                                   is placed in the output buffer. Figure 6.5 describes the
                                   format of this item.
				   DEFAULT: enabled
-errorbypass     bool              If enabled, a TDC that reports an error will not be
                                   read out for that event.
                                   DEFAULT: enabled.
-globaloffset   int[2]             Provides the global offset/vernier offset.
                                   DEFAULT: {0 0}
-channeladjusts see description    Arbitary number of elements that can adjust
                                   the value of individual channels by adding a positive
                                   offset to them.  The value of this is a list of two element
                                   lists where each element consist of a channel number and
                                   its offset. e.g. {{10 6} {32 5}}  sets the channel
                                   offsets for channel 10 to 6 and for channel 32 to 5. 

-model                             TDC Model number can be one of "v1190" or "v1290"
                                   defaults to "v1290".  Needed for the mvlc because we can't
                                   do interactive initialization.
\endverbatim
The following configuration parameters are simply hunted for by SpecTcl and not actually
used in the configuration of the module:

\verbatim
  -refchannel    int               Sets the reference channel.  The value of this channel
                                   is subtracted from all other channels to produce
                                   high precision times.  Defaults to 0.
  -depth         int               Sets the number of hits to retain for each channel
                                   for the purposes of histogramming.
                                   Defaults to 16.
  -channelcount  int               Sets the number of channels in the
                                   TDC.. should be 16, 32, 64, or 128
                                   we'll accept any int however.

\endverbatim

*/
class CV1x90 : public CReadoutHardware
{
  // Static data: these are maps that help us easily map enumerated values to 
  // register values:
public:
  typedef std::map<std::string, uint16_t> EnumMap;
private:
  static EnumMap    m_edgeMap;
  static EnumMap    m_edgeResolutionMap;
  static EnumMap    m_leResolutionMap;
  static EnumMap    m_widthResolutionMap;
  static EnumMap    m_deadtimeMap;
  static EnumMap    m_maxhitsMap;

private:
  XXUSB::CConfigurableObject*   m_pConfiguration;
  int               m_Model;	// Module e.g. 1190, or 1290.
  char              m_Suffix;   // Module suffix, e.g. A,B,N.

  int               m_nChipCount;    // Number of TDC chips in the module.
  int               m_nChannelCount; // Number of channels in the module

  // Class canonicals:
public:
  CV1x90();
  virtual ~CV1x90();
  
private:
  CV1x90(const CV1x90& rhs);
  CV1x90& operator=(const CV1x90& rhs);
  int operator==(const CV1x90& rhs) const;
  int operator!=(const CV1x90& rhs) const;
public:


  // Implementation member funbctuions

public:
  virtual void onAttach(XXUSB::CConfigurableObject& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);

  
  // Utilities:

private:
  void createConfiguration();	// Helps prep for porting to nscldaq-10.0.
  void moduleType(CVMUSB& controller,
		  uint32_t base);
  void computeCounts();		// Create m_nChipCount, m_nChannel count.
  static bool isValidChannelAdjustList(std::string name, std::string value, void* arg);

  void WaitMicro(CVMUSB& controller, uint32_t base);
  void WriteMicro(CVMUSB&   controller,
		  uint32_t  base,
		  uint16_t  opCode,
		  uint16_t* data,
		  size_t    dataWords);
  std::vector<std::pair<int, int> >  getChannelAdjustments();
  static bool validateChannelOffsets(std::string name, std::string value, void* arg);
  void validateModule(CVMUSB& controller, uint32_t base);
 

  // Static functions that create the various enumeration maps:
  // probably need to be public to be called in initializers.
public:
  static EnumMap edgeMap();
  static EnumMap edgeResolutionMap();
  static EnumMap leResolutionMap();
  static EnumMap widthResolutionMap();
  static EnumMap deadtimeMap();
  static EnumMap maxhitsMap();
};

                                   


#endif
