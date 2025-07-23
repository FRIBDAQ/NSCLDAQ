#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file sis330x.tcl
# @brief  generic driver for the sis 3300.  The sis330xDriver.tcl file seems to
#         only work properly for HIRA applications.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide sis330x  1.0

package require snit
package require VMUSBDriverSupport
# These are not needed for the mvlc translator:
#

if {[array names ::env MVLC_TRANSLATOR] eq ""} {
    
    package require cvmusb
    package require cvmusbreadoutlist
}
##
#  Provides a snit driver for {set the sis 3300/1 FADC.  This driver is fully
#  generic.  It provides three pseudo modes:
#   *   Start mode - The front panel provides a start and data are buffered until
#       Either the sample count is reached or an external stop is received.
#       This is essentially start/stop mode as documented on page 45 of the manual
#       where start comes from the front panel and stop may or may not come from
#       the front panel.  Wrap is turned off for this mode.
#   *   Stop mode - the digitizer is started from the VME and stopped by the
#       front panel. Note that in this mode, the buffer wrap bit is in place
#       to prevent digitization from stopping whenthe buffer page is full.
#   *   Gate mode - This is gate mode as defined on page 45,46 of the manual.
#
#
#  Full option set:
#
#  |   Option          |   default |   Description/values                     |
#  |  -base            | 0         | Module base address - gets validated     |
#
#  |  -clock           | 100Mhz    | Clock source type: can be one of         |
#  |                               | 100Mhz, 50Mhz, 25Mhz, 12.5Mhz, 6.25Mhz   |
#  |                               |  3.125Mhz, external, random, hira        |
#
#  |  -mode            | start     | The adc psuedo mode: {start, stop, gate} |
#
#  |  -startdelay      | disabled  | if an integer, the number of ticks  the start is delayed |
#  |                               | Only useful in start and gate mode       |
#
#  | -stopdelay        | disabled  | Stop delay useful in start, stop and gate mode |
#
#  | -samples          | 128K      | Number of samples the buffer can hold    |
#  |                               | 128K, 16K, 4K, 2K, 1K, 512, 256 or 128   |
#
#  | -thresholdslt     | 8*false   | array of 8 booleans true means thresholds |
#  |                               | are made when the trace goes below the threshold value|
#
#  | -thresholds       |8*0x3fff   | Array of 8 integer threshold values.      |
#
#  | -minlttime        | 4*0       | Number of clocks for which the threshold   |
#  |                               | made in LT mode to create a trigger        |
#
#  | -mingttime       | 4*0        | Number of clocks for which the trigger must be made |
#  |                               | in GT mode to create a trigger            |
#
#  | -trigwidth       | 4*0        | Trigger width in clocks for threshold based triggers |
# 
#
#  | -groupenables    | 4*true     | array of 4 bools that specify which gruops are read |
#  | -header          | 0xfadc     | 16 bit marker at the front of the data from a group. |
#  | -trailer         | 0xffff    | 16 bit marker at the end of group data   |

snit::type sis330x {
    option -base            -default 0
    option -clock           -default 100Mhz  -validatemethod _validClock
    option -mode            -default start   -validatemethod _validMode
    option -startdelay      -default disabled -validatemethod _validDisableOrInt
    option -stopdelay       -default disabled -validatemethod _validDisableOrInt
    option -samples         -default 128K     -validatemethod _validPageSize
    option -thresholdslt    -default [lrepeat 8 false]   -validatemethod _validThresholdsLt
    option -thresholds      -default [lrepeat 8 0x3fff]  -validatemethod _validThresholds
    option -minletime       -default [lrepeat 4 0]       -validatemethod _validMNP
    option -mingttime       -default [lrepeat 4 0]       -validatemethod _validMNP
    option -trigwidth       -default [lrepeat 4 0]       -validatemethod _validMNP       
    option -groupenables    -default [lrepeat 4 true]    -validatemethod _validGroupEnables
    option -header          -default 0xfadcfadc              
    option -trailer         -default 0xffffffff              
    
    
    
    
    #  The address modifiers we'll use:
    
    variable setupAmod     0x09;                 # Single actions.
    variable blockXferAmod 0x0b;                 # Block transfer address modifier.

    #-------------------------------------------------------------------------
    #
    #  SIS330x hardware definitions e.g. register offsets and bits in registers.


    # Register offsets:   Comments are from the table in 3.1 of the SIS 
    #                     FADC manual.
    #    Control register page: 

    variable CR                                0x00; # Control register (JK Flipflops).
    
    # Bits we need to route the trigger to the output:
    
    variable CREnableTriggerOutput             0x04
    variable CREnableInternalTriggerRouting    0x40;     #Internal trigger routed to STOP
    
    variable Firmware                          0x04; # module Id/firmware.
    variable InterruptConfig		       0x08; # Interrupt configuration register.
    variable InterruptControl		       0x0c; # Interrupt contro register.
    variable DAQControl                        0x10; # Data acquisition control register (JK)

        # What the clock source values mean in terms of Acquisition control register bits:
    
    variable clockSourceAcqRegister -array [list     \
        100Mhz      0                                \
        50Mhz       0x1000                          \
        25Mhz       0x2000                          \
        12.5Mhz     0x3000                          \
        6.25Mhz     0x4000                          \
        3.125Mhz    0x5000                          \
        external    0x6000                          \
        random      0x6800                          \
        hira        0x6008                          \
    ]
    #  What the mode values mean in terms of Acquisition control register bits:
    
    variable modeAcqRegister -array [list           \
        start   0x100                               \
        stop    0x100                               \
        gate    0x500                               \
    ]  
    
    #  The following bits enable the start delay and stop delay
    #  They are also relevant to the acquisition control register:
    
    variable DAQControlEnableStartDelay     0x40
    variable DAQControlEnableStopDelay      0x80

    variable ExternStartDelay                  0x14; # External start delay register.
    variable ExternStopDelay                   0x18; # External stop delay register.
    variable Predivider                        0x1c; # Timestamp predivider register.
    variable Reset                             0x20; # General reset.
    variable VMEStart                          0x30; # VME Start sampling.
    variable VMEStop                           0x34; # VME Stop sampling.
    variable StartAutoBankSwitch               0x40; # Start auto memory bank switch.
    variable StopAutoBankSwitch   	       0x44; # Stop auto memory bank switch.
    variable ClearBank1Full                    0x48; # Clear bank 1 memory full
    variable ClearBank2Full                    0x4c; # Clear bank 2 memory full.

    #    Time stamp directories (each is 0x1000 bytes long).

    variable TimestampDir1                     0x1000; # Event timestamp directory bank 1
    variable TimestampDir2                     0x2000; # Event timestamp directory bank 2

    

    #   Each ADC group (2 adcs/bank) has an event information register page
    #   for that group.  The structur of that page is common so this
    #   section supplies the offsets to each adc bank information page
    #   and the offsets to each register within the page
    #   Therefore referencing means base + information page base + register offset
    #   In addition there's a common group:
    #
    
    

    variable CommonInfo                       0x00100000; # Common group register base.
    variable EventInfo1                       0x00200000; # Event information for group 1.
    variable EventInfo2                       0x00280000; # Event information for group 2.
    variable EventInfo3                       0x00300000; # Event information for group 3
    variable EventInfo4                       0x00380000; # Event information for group 4

    variable EventConfiguration               0x00000000; # Event configuration register (all ADCS)
    
    #
    #  What the -samples value means for bits in the event configuration register:
    #
    variable samplesEvtConfigReg -array [list       \
        128K    0                                   \
        16K     1                                   \
        4K      2                                   \
        2K      3                                   \
        1K      4                                   \
        512     5                                   \
        256     6                                   \
        128     7                                   \
    ]
    
    variable longwordsPerSample -array [list        \
        128K        [expr {128*1024}]               \
        16K         [expr {16*1024}]                \
        4K          [expr {4*1024}]                 \
        2K          [expr {2*1024}]                 \
        1K          1024                            \
        512         512                             \
        256         256                             \
        128         128                             \
    ]  
    #   Wrap bit:
    
    variable WrapEnableEvtConfigReg         8
    
    #  In HiRA clock mode we need to set the external random clock mode.
    
    variable  HiRaRandomClockEvtConfigReg   0x800
    
    variable TriggerThreshold                 0x00000004; # Trigger threshold register (all ADCS)
    
    #  Threshold registers are essentially  two 16 bit registers packed into
    #  a single 32 bit word.  The low order word is the odd adc (numbered from 0)
    #  the high order word is the even adc.  (Note that the SIS manual nubers from
    #  1 just to confuse us).
    
    variable ShiftTriggerThreshReg                  16;          # Shift this much for high bits.
    variable LETriggerThreshReg                 0x8000;          # Bit for LE selection.
    
    variable Bank1AddressCounter              0x00000008; # Bank 1 address counter Really an item count
    variable Bank2AddressCounter              0x0000000c; # Bank 2 address counter.
    variable Bank1EventCounter                0x00000010; # Bank 1 event count register.
    variable Bank2EventCounter                0x00000014; # Bank2 event count register
    variable SampleValue                      0x00000018; # Actual sample value.
    variable TriggerClearCounter              0x0000001c; # Trigger flag clear counter register.
    variable ClockPredivider                  0x00000020; # GRP 3 only Sampling clock predivider.
    variable SampleCount                      0x00000024; # Number of samples
    variable TriggerSetup                     0x00000028; # Trigger setup register (all ADCS)
    
    #  TriggerSetup as M, N, P and enable bits for those bits.
    
    variable mShiftTriggerSetupReg            0
    variable nShiftTriggerSetupReg            8
    variable pShiftTriggerSetupReg            16
    variable mnEnableTriggerSetupReg           0x01000000
    variable pEnableTriggerSetupReg           0x10000000
    
    variable MaxEvents                        0x0000002c; # Max no of events register (all ADCS).
    
    variable EventDirectory1                  0x00001000; # Event directory bank 1.
    variable EventDirectory2		      0x00002000; # Event directory bank 2.

    # Event memory buffers.  Each event memory is 0x80000 bytes long:

    variable Bank1Group1                      0x00400000; # Bank 1 adc 1/2
    variable Bank1Group2                      0x00480000; # Bank 1 adc 3/4
    variable Bank1Group3                      0x00500000; # Bank 1 adc 5/6
    variable Bank1Group4                      0x00580000; # Bank 1 adc 7/8.

    


    #
    #  The bit field defs etc. are basically cut and pasted from sis3300.h and
    #  mechanically converted.
    #

    # Bits in the control register: Each control has a set/unset bit (J/K flip
    # flop control).

    variable CRLedOn                            1
    variable CRUserOutputOn                     2
    variable CREnableTriggerOutput              4
    variable CRInvertTriggerOutput       0x000010
    variable CRTriggerOnArmedAndStarted  0x000020
    variable CRLedOff                    0x010000
    variable CRUserOutputOff             0x020000
    variable CREnableUserOutput          0x040000
    variable CRNormalTriggerOutput       0x100000
    variable CRTriggerOnArmed            0x200000

    # Bits in the status register:

    variable SRLedStatus                    1
    variable SRUserOutputState              2
    variable SRTriggerOutputState           4 
    variable SRTriggerIsInverted     0x000010
    variable SRTriggerCondition      0x000020; # 1: armed and started
    variable SRUserInputCondition    0x010000
    variable SRP2_TEST_IN            0x020000
    variable SRP2_RESET_IN           0x040000
    variable SRP2_SAMPLE_IN          0X080000


    # Bits in the data acquisition control register:
    #
    variable DAQSampleBank1On        0x00000001
    variable DAQSampleBank2On        0x00000002
    variable DAQEnableHiRARCM        0x00000008
    variable DAQAutostartOn          0x00000010
    variable DAQMultiEventOn         0x00000020
    variable DAQStopDelayOn          0x00000080
    variable DAQStartDelayOn         0x00000040
    variable DAQEnableLemoStartStop  0x00000100
    variable DAQEnableP2StartStop    0x00000200
    variable DAQEnableGateMode       0x00000400
    variable DAQEnableRandomClock    0x00000800
    variable DAQClockSetMask         0x00007000
    variable DAQDisableHiRARCM       0x00080000
    variable DAQClockSetShiftCount   12
    variable DAQSampleBank1Off       0x00010000
    variable DAQBusyStatus           0x00010000
    variable DAQSampleBank2Off       0x00020000
    variable DAQAutostartOff         0x00100000
    variable DAQMultiEventOff        0x00200000
    variable DAQStopDelayOff         0x00800000
    variable DAQStartDelayOff        0x00400000
    variable DAQDisableLemoStartStop 0x01000000
    variable DAQDisableP2StartStop   0x02000000
    variable DAQDisableGateMode      0x04000000
    variable DAQDisableRandomClock   0x08000000
    variable DAQClockClearMask       0x70000000
    variable DAQCLockClearShiftCount 28


    # Bits and fields in the event configuration register.
    #

    variable ECFGPageSizeMask       7
    variable ECFGPageSizeShiftCount 0
    variable ECFGWrapMask           8
    variable ECFGWrapShiftCount     3
    variable ECFGRandomClock        [expr {1 << 11}]

    # Bits and fields in the threshold register.
    variable THRLt                 0x8000
    variable THRChannelShift     16

    # Bits and fields in the event directory longs:
    #

    variable EDIREndEventMask 0x00001ffff
    variable EDIRWrapFlag     0x000080000
    variable EDIRTriggers     0xff000000

    # HiRA firmware is a pre-requisite to using the HiRA
    # indpendent random clock mode.

    variable HIRAFWMAJOR  0x13
    variable HIRAFWMINOR  0x05

    variable isMVLC;      # True if running in mvlcgenerator:
    variable EventConfigShadow;   # Needed for MVCL since we can't read it.
    ##
    # constructor
    #   Just processes the options:
    #
    constructor args {
        $self configurelist $args
        set isMVLC [expr {[array names ::env MVLC_TRANSLATOR] eq "MVLC_TRANSLATOR"}]
        if {$isMVLC} {
            set EventConfigShadow  0x00001000
        }
    }
    
    #--------------------------------------------------------------------------
    #  Methods that implement the driver interface:
    #
 
    ##
    # Initialize
    #   Called prior to the start of a run to setup the module.
    #
    #  @param vmusb   - Controller object handle must convert to an object.
    #                   in mvlcgenerate this is just the base command of the
    #                   command ensemble.
    #
    method Initialize vmusb {
        #
        #  We must have at lesat one group enabled else it's an error:
        #
        set enablesOk false
        foreach e $options(-groupenables) {
            if {$e} {set enablesOk true}
        }
        if {!$enablesOk} {
            error "SIS330x driver the module at $options(-base) must have at least one group enabled."
        }
        
        #
        #  Get the controller as an object and pull the base out of the options.
        #
        set controller [::VMUSBDriverSupport::convertVmUSB $vmusb]
        set base $options(-base)
        
        #  Ensure this is actually a 3300 or 3301:
        #  Only in VMUSB env:

        if {!$isMVLC} {
            set fw [$controller vmeRead32 [expr {$base + $Firmware}] $setupAmod]
            set model [expr {($fw >> 16) & 0xffff}]
            puts [format "Found model %x at -base 0x%08x"  $model $base]
            if {($model != 0x3300) && ($model != 0x3301)} {
                error "There is no SIS 3300/1 module at $base"
            }

            # output the firmware version.

            set minor [expr {$fw & 0xff}]
            set major [expr {($fw >> 8) & 0xff}]
            puts [format "Firmware is version %d.%d" $major $minor]
        }
    $controller vmeWrite32 [expr {$base + $Reset}] $setupAmod 0;  # Reset the module.
    if {!$is} {
        after 500 
    } else {
        controller delay [expr {50*1000}];     # Maybe 50ms is actually long enough?
        set EventConfigShadow  0x00001000;     # reset value.
    }
	

        # Figure out the value of the acquisition control register.  This is a
        # function -mode,  -stopdelay, -startdelay, -clock and program the register:
        
        set acqCsr 0
        set acqCsr [expr {$acqCsr | $clockSourceAcqRegister($options(-clock))}]
        set acqCsr [expr {$acqCsr | $modeAcqRegister($options(-mode))}]
        
        
        
        if {$options(-startdelay) ne "disabled"} {
            set acqCsr [expr {$acqCsr | $DAQStartDelayOn}]
            $controller vmeWrite32 \
                [expr $base + $ExternStartDelay] $setupAmod $options(-startdelay)
        }
        if {$options(-stopdelay) ne "disabled"} {
            set acqCsr [expr {$acqCsr | $DAQStopDelayOn}]
            $controller vmeWrite32 \
                [expr {$base + $ExternStopDelay}] $setupAmod $options(-stopdelay)
        }
        # enable sampling into bank 1:
        # set acqCsr [expr {$acqCsr | $DAQSampleBank1On}]

        $controller vmeWrite32 [expr {$base + $DAQControl}] $setupAmod $acqCsr
        
        # set the number of samples in the page Size field of the Event
        # configuration register. We only support the same sampling size
        # for all groups so this is programmed in the $CommonInfo + $EventConfiguration
        # register.
        
        $self _writeEvtConfigReg $controller $samplesEvtConfigReg($options(-samples))
        
        # If the clock mode is HiRA we need to set the External clock random
        # mode in that register as well

	if {$options(-clock) eq "hira"} {
	    set evtConfigReg [$self _readEvtConfigReg $controller ]
	    $self _writeEvtConfigReg $controller [expr {$evtConfigReg | $HiRaRandomClockEvtConfigReg} ]
        }
        # In stop mode we'll let the memory wrap
        
        if {$options(-mode) eq "stop"} {
            set evtConfigReg [$self _readEvtConfigReg $controller]
            set evtConfigReg  [expr {$evtConfigReg | $WrapEnableEvtConfigReg}]
            $self _writeEvtConfigReg $controller $evtConfigReg
        }

        #  Thresholds Foreach threshold register we need to compute
        #  the value from both the thresholds and the lt flag.
        #  for the thresholds associated with that group and progream the
        #  register.
        set groups [list $EventInfo1 $EventInfo2 $EventInfo3 $EventInfo4]
        foreach                                                           \
            {lowthr hithr} $options(-thresholds)                          \
            {lowlt hilt} $options(-thresholdslt)                          \
            group $groups {

            set addr [expr {$base + $group + $TriggerThreshold}]
            set lowPart [$self _constructThreshold $lowthr $lowlt]
            set hiPart  [$self _constructThreshold $hithr $hilt]
            
            set value [expr {($lowPart << $ShiftTriggerThreshReg) | $hiPart}]
            $controller vmeWrite32 $addr $setupAmod $value
        }

        # Based on the -minletime, the -mingttime and -trigwidth parameters,
        # setup each group's trigger setup register:
        
        foreach m $options(-minletime) n $options(-mingttime) \
            p $options(-trigwidth) group $groups {
            
            set addr [expr {$base + $group + $TriggerSetup}]
            
            # Build up the m,n,p parts of the register:
            
            set reg [expr \
                {$m | ($n << $nShiftTriggerSetupReg) |
                ($p << $pShiftTriggerSetupReg)}         \
            ]
            # Set enable bits as needed:
            
            if {($m | $n) > 0} {
                set reg [expr {$reg | $mnEnableTriggerSetupReg}]
            }
            if {$p > 0} {
                set reg [expr {$reg | $pEnableTriggerSetupReg}]
            }
            #  Now we can write the register:
            
            $controller vmeWrite32 $addr $setupAmod $reg

        }

        #  The other options all have to do with setting up the readout list.
        
        # We want to allow the internal thresholds to make a trigger.
        # to do this requires routing the internal trigger to the
        # output.  This is done in the CR by enabling the trigger output.
        # We're not going to enable internal trigger routing because we want the
        # user to be able to use the internal trigger as a term in some
        # grander trigger logic...and besides the user might want to route
        # the internal trigger to START not STOP.
        
        $controller vmeWrite32 \
            [expr {$base + $CR}] $setupAmod $CREnableTriggerOutput
        
        # Enable sampling into bank 1 According to pg 48 of the manual this must
        # be done last:

        $controller vmeWrite32 \
            [expr {$base + $DAQControl}] $setupAmod $DAQSampleBank1On

        # In start mode and gated mode, the inputs will start sampling:
        #  -  Start mode, the start input will start sampling and stop or
        #     event size will stop it.
        #  -  Gate mode, the falling edge of start begins sampling and the
        #     rising edge (or event size) stops it.
        #  - Stop mode VME starts sampling and the Stop input will stop it.
        #    we have the wrap bit set so there's not going to be a stop
        #    on event size.
        # 
        
        if {$options(-mode) eq "stop"} {
            $controller vmeWrite32 [expr {$base + $VMEStart}] $setupAmod 1
        }
    }
    ##
    # addReadoutList
    #    Add to the readout list for the stack this module is in.
    #    We're going to create output data that looks like this:
    #
    #    
    #    +--------------------------------------+
    #    | Mask of group enables                |
    #    +--------------------------------------+
    #    | DAQ control register                 |
    #    +--------------------------------------+
    #    | options(-header)                     |   \
    #    +--------------------------------------+    |
    #    | Group Trigger Event directory        |    |
    #    +--------------------------------------+    |
    #    |  Size of data from group             |    > For each enabled group.
    #    +--------------------------------------+    | from low set bit to high
    #    | Data from group                      |    |
    #    +--------------------------------------+    |
    #    |  options(-trailer)                   |    |
    #    +--------------------------------------+    /
    #          ..............
    #    
    # @param vmusbList  - Handle to the vmusb readout list that must
    #                     be converted to an object.
    #
    method addReadoutList vmusbList {
        
        # Pull out the base address from the -base option and turn the list handle
        # into an object/command ensemble.
        
        set base $options(-base)
        set list [VMUSBDriverSupport::convertVmUSBReadoutList $vmusbList]
        
        #
        # Regardless we need a 16 bit marker with bits set for each enabled
        # group.  We know as well that at least one bit is set because
        # the initialization tosses an error if not -- although that does not
        # really matter for us.
        
        set enablesMask 0
        set enableBit   1
        foreach enable $options(-groupenables) {
            if {$enable} {
                set enablesMask [expr {$enablesMask | $enableBit}]
            }
            set enableBit [expr {$enableBit << 1}]
        }
        $list addMarker $enablesMask
	    $list addRead32 [expr {$base + $DAQControl}] $setupAmod;                         # DAQ control register.


        #  Now we need to read each bit.  Note that if we are in
        #  start or stop mode we're going to just do a fixed block read
        #  of the number of samples requested.  For stop mode the user
        #  needs to use the trigger directory word to sort out where
        #  the waveform starts.
        #  For gate mode, we're going to do a masked block read to get only
        #  the number of words in the gate time.
        #
        
        foreach                                                               \
            enable $options(-groupenables)                                    \
            group [list $EventInfo1 $EventInfo2 $EventInfo3 $EventInfo4]      \
            buffer [list $Bank1Group1 $Bank1Group2 $Bank1Group3 $Bank1Group4] \
        {
            if {$enable} {
                $list addMarker $options(-header)
                $list addRead32 [expr $base + $group + $EventDirectory1] $setupAmod
                
                # How we do the readout depends on the mode:
                
                if {$options(-mode) eq "gate"} {
                    $self _addGatedRead $list $base $group $buffer
                } else {
                    $self _addFullRead $list $base $group $buffer
                }
                $list addMarker $options(-trailer)
            }
        }
        # Prepare the digitizer for the next event:
        
        $list addWrite32 [expr $base + $ClearBank1Full] $setupAmod 1;   # May not need this clear.
        $list addWrite32 [expr $base + $DAQControl] $setupAmod 1;   # Enable clock -> bank 1
        
        # If in stop mode we also need to issue a vme start.
        
        if {$options(-mode) eq "stop"} {
            $list addWrite32 [expr {$base + $VMEStart}] $setupAmod 1
        }
        
    }
    ##
    # onEndRun
    #   Invoked at the end of a run to perform shutdown functions.
    #
    #  @param vmusb  - VMUSB handle that must be converted to an object.
    #
    method onEndRun vmusb {
        
    }
    #--------------------------------------------------------------------------
    # Configuration validations methods.
    #
    
    ##
    # _enumElement
    #   Throws an error if a value is not a member of a  list of elements
    #
    # @param value - value to check
    # @param list  - list of legal values
    proc _enumElement {value list} {
        if {$value ni $list} {
            error "Value $value must be an element of {[join $list {, }]}"    
        }
    }
    ##
    # _validClock
    #
    #  Validate a clock value.
    #
    # @param optname   - name of the option being validated for.
    # @param value     - proposed value.
    #
    method _validClock {optname value} {
        _enumElement $value [array names clockSourceAcqRegister]
    }
    ##
    # _validMode
    #   Validate the digitizer acquisition mode:
    #
    # @param optname - name of the option being set (-mode).
    # @param value   - proposed new value.
    #
    method _validMode {optname value} {
        _enumElement $value [array names modeAcqRegister]
    }
    ##
    # _validEnableDisable
    # Validate an option that can be either 'enable' or 'disable'.
    #
    method _validEnableDisable {optname value} {
        _enumElement $value [list enabled disable]
    }
    ##
    # _validDisableOrInt
    #    Validate entries that can be either 'disable' or an integer
    #    16 bit value.  (e.g. -startdelay).
    # @param optval - option  name.
    # @param value  - Option value.
    #
    method _validDisableOrInt {optval value} {
        if {$value eq "disabled"} return
        if {![string is integer -strict $value]} {
            error "$value for $optval must be an integer."
        }
        if {($value < 0) || ($value > 0xffff)} {
            error "$value for $optval must be in the range 0-[expr 0xffff]"
        }
    }
    ##
    # _validPageSize
    #    Validate the page size.
    # @param optname - Name of the option being validated.
    # @param value   - Propopsed new value.
    #
    method _validPageSize {optnam value} {
        _enumElement $value [array names samplesEvtConfigReg]
    }
    ##
    # _validRangeList
    #   Ensure all elements of a list are integers in the specified
    #   range
    # @param value   - List of values to chaekc.
    # @param low     - Lowest allowed value
    # @param hi      - Highest allowed value
    #
    proc _validRangeList {value low hi} {
        foreach v $value {
            if {![string is integer -strict $v]} {
                error "$v in value list $value must be an integer."
            }
            if {($v < $low) || ($v > $hi) } {
                error "$v in value list $value must be in the range \[$low, $high \]"
            }
        }
    }
    ##
    # _validThresholdsLt
    #   Check for the valid values for -thresholdslt
    #
    # @param optname - name of option being validated.
    # @param optval  - value validating.
    #
    method _validThresholdsLt {optname optval} {
	if {[llength $optval] != 8} {
	    error "$optname value must be an 8 element list of booleans"
	}
	foreach value $optval {
	    if {![string is boolean $value]} {
		error "$optname value must be 8 booleans but $optval contains $value"
	    }
	}

    }
    
    ##
    # _validThresholds
    #    validate the thresholds array:
    #    -   Must be exactly 8 elements.
    #    -   Values mustbe integers < 0x4000
    #
    # @param optname - name of option being configured.
    # @param value   - Proposed value.
    #
    method _validThresholds {optname value} {
        if {[llength $value] != 8} {
            error "Threshold values ($optname) must be specified an 8 element list."
        }
        _validRangeList $value 0 0x3fff
    }
    
    ##
    # _validMNP
    #    Validates the m,n, p values (see 4.23 of the manual register x00028).
    #    These can be four element lists integers in the range 0-15.
    # 
    # @param optname -name of the option being checked.
    # @param value  - proposed new value.
    #
    method _validMNP {optname value} {
        if {[llength $value] != 4} {
            error "$optname values must be specified as four elemet lists. Was $value"
        }
        _validRangeList $value 0 0xf
    }

    ##
    # _validGroupEnables
    #    Validates the -groupenables -- must be a four element list  of bools.
    #
    # @param optname name of option (-groupenables)
    # @param proposed new value
    method _validGroupEnables {optname value} {
	if {[llength $value] != 4} {
	    error "$optname value must be a four element list of booleans"
	}
	foreach element $value {
	    if {![string is boolean $element]} {
		error "$optname value must be a list of 4 booleans but $value contains $element"
	    }
	}
    }
    #--------------------------------------------------------------------------
    # Private methods that interact with the module:
    
    
    ##
    # _readEvtConfigReg
    #   Returns the common event configuration register.
    #
    # @oaram   c  - the objectized controller.
    # @return uint32_t
    #
    method _readEvtConfigReg {c} {

	#
	#  Notethat the common eventconfiguration register is write-only.  Since we are not
	#  writing different values for each group, we'll read the group 0 evtconfigt.

        if {$isMVLC} {
            return $EventConfigShadow;    # can't do a real read.
        } else {
            return [$c vmeRead32 \
                [expr $options(-base)+ $EventInfo1 + $EventConfiguration] $setupAmod \
            ]
        }
    }
    ##
    # _writeEvtConfigReg
    #   Write a new value to the common event configuration register.
    #
    # @param c   - objectized controller.
    # @param value - value to write to the register.
    #
    method _writeEvtConfigReg {c value} {
        set EventCOnfigShadow $value;       # Used by MVLC since reads are not possible.
        $c vmeWrite32 \
            [expr {$options(-base) + $CommonInfo + $EventConfiguration}] $setupAmod $value

    }
    ##
    # _constructThreshold
    #
    #   Construct the threshold bits for half a group's threshold registers:
    #
    # @param thresh    - The threshold value.
    # @param le        - true if LE false if GT
    # @return integer  - The correct 16 bits for half a threshold register.
    # @note  These bits may still require shifting to position them properly
    #        in the register.
    
    method  _constructThreshold {thresh le} {
        return [expr {$thresh | ($le ? $LETriggerThreshReg : 0)}]
    }
    ##
    # _samplesToLongs
    #    Translate the samples keyword into a number of samples
    #
    # @param key  - Keywords e.g. 128K
    # @return integer - number of 32 bit elements to read.
    #
    method _samplesToLongs {key} {
        
        return $longwordsPerSample($key)
    }
    ##
    # _addFullRead
    #
    #   Adds a read for bank1 that extends from the beginning of the page
    #   to the end.  The pagesize is determined by options(-samples)
    #   We'll prefix the data read with the number of longs we're reading.
    #
    #
    #   @param list   - list command ensemble to which we're adding commands.
    #   @param base   - module base address.
    #   @param group  - Base of the group info for the group being read.
    #   @param buffer - Page from which to read.
    #
    method _addFullRead {list base group buffer} {
        set size [$self _samplesToLongs $options(-samples)]

        # Insert the size as a little endian longword:
        

        $list addMarker [expr {$size & 0xffff}];                                         # size read.
        $list addMarker [expr {$size >> 16}]
        
        # Add the block read:
        
        $list addBlockRead32 [expr {$base + $buffer}] $blockXferAmod $size
        
    }
    ##
    # _addGatedRead
    #    Adds a read for gate mode.  In this mode, sampling is started by
    #    the falling edge of IN1 and stopped on he rising edge.  In this case,
    #    the number of samples may well be less than the options(-sample) valu.
    #     We do a masked block read using the trigger event directory to
    #     determine the number of longs to read.
    #
    # @param list   - Command ensemble for the list we're adding to.
    # @param base    - Base addfresss of the digitizer.
    # @param group   - Base address of the group information block.
    # @param buffer  - Pointer to the SRAM in which the data are.
    #
    method _addGatedRead {list base group buffer}  {
	$list addBlockCountRead32 \
            [expr {$base + $group + $EventDirectory1}] $EDIREndEventMask \
            $setupAmod
        $list addMaskedCountBlockRead32 [expr {$base + $buffer}] $blockXferAmod
    }
}
    
