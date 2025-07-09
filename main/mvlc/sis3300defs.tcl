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
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file sis3300defs.tcl
# @brief Common definitions of the sis3300
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide sis3300defs 1.0

    # Enumeration specifications:
    #
    # Indices are valid enum values and values are the corresponding selectors:
    #
    
    variable modeAcqRegister -array [list           \
        start   0x100                               \
        stop    0x100                               \
        gate    0x500                               \
    ]  
    
    variable clockSourceValues -array {
	100Mhz        0
	50Mhz         1
	25Mhz         2
	12.5Mhz       3
	6.25Mhz       4
	3.125Mhz      5
	FrontPanel    6
	P2Connector   7
    }
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
    #  Difference here is that the bits are already shifted into
    #  position.
     
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
        p2          0x7000                          \
    ]
    
    variable sampleSizeValues -array {
	128K               0
	16K                1
	4K                 2
	2K                 3
	1K                 4
	512                5
	256                6
	128                7
    }
    
    variable sampleMaskValues -array {
	128K              0x1ffff
	16K               0x03fff
	4K                0x00fff
	2K                0x007ff
	1K                0x003ff
	512               0x001ff
	256               0x000ff
	128               0x0007f
    }

    #
    #  We use two address modifiers:
    #  setupamod - A32D32 - supervisory data access.
    #  blockread - A32D32 Block transfer supervisory data  mode.

    variable setupAmod 0x09
    variable blockXfer 0x0b

    #-------------------------------------------------------------------------
    #
    #  SIS330x hardware definitions e.g. register offsets and bits in registers.


    # Register offsets:   Comments are from the table in 3.1 of the SIS 
    #                     FADC manual.
    #    Control register page: 

    variable  HiRaRandomClockEvtConfigReg   0x800
    variable CR                                0x00; # Control register (JK Flipflops).
    variable Firmware                          0x04; # module Id/firmware.
    variable InterruptConfig		       0x08; # Interrupt configuration register.
    variable InterruptControl		       0x0c; # Interrupt contro register.
    variable DAQControl                        0x10; # Data acquisition control register (JK)
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
    variable EventBases -array {			 
	0 $EventInfo1
	1 $EventInfo2
	2 $EventInfo3
	3 $EventInfo4
    };				                          # Lookup table of bases.

    variable EventConfiguration               0x00000000; # Event configuratino register (all ADCS)
    variable TriggerThreshold                 0x00000004; # Trigger threshold register (all ADCS)
    variable Bank1AddressCounter              0x00000008; # Bank 1 address counter Really an item count
    variable Bank2AddressCounter              0x0000000c; # Bank 2 address counter.
    variable Bank1EventCounter                0x00000010; # Bank 1 event count register.
    variable Bank2EventCounter                0x00000014; # Bank2 event count register
    variable SampleValue                      0x00000018; # Actual sample value.
    variable TriggerClearCounter              0x0000001c; # Trigger flag clear counter register.
    variable ClockPredivider                  0x00000020; # GRP 3 only Sampling clock predivider.
    variable SampleCount                      0x00000024; # Number of samples
    variable TriggerSetup                     0x00000028; # Trigger setup register (all ADCS)
    variable MaxEvents                        0x0000002c; # Max no of events register (all ADCS).
    
    variable EventDirectory1                  0x00001000; # Event directory bank 1.
    variable EventDirectory2		      0x00002000; # Event directory bank 2.

    # Event memory buffers.  Each event memory is 0x80000 bytes long:

    variable Bank1Group1                      0x00400000; # Bank 1 adc 1/2
    variable Bank1Group2                      0x00480000; # Bank 1 adc 3/4
    variable Bank1Group3                      0x00500000; # Bank 1 adc 5/6
    variable Bank1Group4                      0x00580000; # Bank 1 adc 7/8.

    variable mShiftTriggerSetupReg            0
    variable nShiftTriggerSetupReg            8
    variable pShiftTriggerSetupReg            16
    variable mnEnableTriggerSetupReg           0x01000000
    variable pEnableTriggerSetupReg           0x10000000


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
    variable THRChannelShift         16

    # Bits and fields in the event directory longs:
    #

    variable EDIREndEventMask 0x1ffff
    variable EDIRWrapFlag 0x80000

    # HiRA firmware is a pre-requisite to using the HiRA
    # indpendent random clock mode.

    variable HIRAFWMAJOR  0x13
    variable HIRAFWMINOR  0x05