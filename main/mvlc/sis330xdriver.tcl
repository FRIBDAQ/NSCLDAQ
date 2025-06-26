#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

lappend auto_path [file normalize [file join [file dirname [info script]] .. .. lib]]

package require snit
package require VMUSBDriverSupport
package require cvmusb
package require cvmusbreadoutlist

package provide SIS330XDriver 1.0

##
# @file sis3340xdriver.tcl 
#
#   Provides a Tcl/Snit VM-USB device driver for the 
#   SIS330x Flash adc module.  The options are closely patterned
#   after the methdos of the sis3300.cpp class which implements
#   SBS support for this module.
#
#
#  OPTIONS:
#     -base            - Module base address in VME
#     -clocksource     - Module clock source.. See clocksourceValues bellow for the valid list.
#     -startdelay      - Boolean.  If true the start delay is enabled and:
#     -startdelayticks - this is the number of samples in the start delay.
#     -stopdelay       - Boolean. IF true, the stop dela is enabled and:
#     -stopdelayticks  - this is the number of sample in the stop delay.
#     -stoptrigger     - The module stop is the trigger.
#     -gatemode        - Module is or is not in gate mode (start stop from gate input fall/rise)
#     -lemostartstop   - Start/Stop come from lemo inputs.
#     -p2startstop     - Backplane signals on the p2 connector provide start/stop.
#     -hirarandomclock - If true, HiRA random clock mode is enabled.
#     -randomclock     - If true front panel provides non-periodic clock (not as random as
#                        HiRA's clock).
#     -samplesize      - Size of sample buffers.
#     -wrap            - Buffers are  in wrap mode (normally used if start/stop is longer than
#                        samplesize.
#     -thresholdslt    - Thresholds represent a  level _below_ which the conversion must fall.
#     -thresholds      - Threshold values for all 8 channels.
#     -groupsread      - list of flags indicating which groups of ADCs will be read (the module
#                        is organized into 4 groups of 2 ADCs each).
#  PUBLIC METHODS:
#     Initialize     -  Initialize the module in accordance with the current configuration.
#     addReadoutList -  Adds elements to the readout list that read the module.   

snit::type sis330xDriver {
    option -base            -default 0              -configuremethod configureInt
    option -clocksource     -default 100Mhz         -configuremethod configureClockSource
    option -startdelay      -default off            -configuremethod configureBool
    option -startdelayticks -default   0            -configuremethod configureInt
    option -stopdelay       -default off            -configuremethod configureBool
    option -stoptrigger     -default off            -configuremethod configureBool
    option -stopdelayticks  -default   0            -configuremethod configureInt
    option -gatemode        -default off            -configuremethod configureBool
    option -lemostartstop   -default on             -configuremethod configureBool
    option -p2startstop     -default off            -configuremethod configureBool
    option -hirarandomclock -default off            -configuremethod configureBool
    option -randomclock     -default off            -configuremethod configureBool
    option -triggeronstop   -default off            -configuremethod configureBool
    option -samplesize      -default 128K           -configuremethod configureSampleSize
    option -wrap            -default off            -configuremethod configureBool
    option -thresholdslt    -default false          -configuremethod configureBool
    option -thresholds      -default [list 0x3fff 0x3fff 0x3fff 0x3fff 0x3fff 0x3fff 0x3fff 0x3fff] -configuremethod configureThresholds
    option -groupsread      -default [list true true true true] -configuremethod configureGroupsRead


    # Enumeration specifications:
    #
    # Indices are valid enum values and values are the corresponding selectors:
    #
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

    variable EDIREndEventMask 0x1ffff
    variable EDIRWrapFlag 0x80000

    # HiRA firmware is a pre-requisite to using the HiRA
    # indpendent random clock mode.

    variable HIRAFWMAJOR  0x13
    variable HIRAFWMINOR  0x05


    #-----------------------------------------------------------------------
    #
    # constructor:
    #
    # Provides a chance to configure and create in one step:
    #
    constructor args {
	$self configurelist $args
    }
    #-----------------------------------------------------------------------
    #
    # Public methods that implement the driver interface:
    #

    # Initialize:
    #
    #  Initialize the device to the configuration defined by the configuration
    #  options.
    #
    # @param vmusb - VM-USB controller object handle.  This must be converted to
    #                a cvmusb object.
    #
    method Initialize vmusb  {
	set controller [::VMUSBDriverSupport::convertVmUSB $vmusb]
	set base       $options(-base)

	# Reset as a single shot operation then 

	$controller vmeWrite32 [expr $base +  $Reset] $setupAmod 0

	# Build a list of operations to complete the init:

	set list       [::cvmusbreadoutlist::CVMUSBReadoutList]

	# Figure out and set the  initial CSR.

	set csrValue   [expr $CRUserOutputOff | \
			    $CRLedOff        | \
			    $CREnableTriggerOutput | \
			    $CRNormalTriggerOutput]

	if {$options(-stoptrigger)} {
	    set csrValue [expr $csrValue | $CRTriggerOnArmedAndStarted]
	} else {
	    set csrValue [expr $csrValue | $CRTriggerOnArmed]
	}
	$list addWrite32 [expr $base + $CR] $setupAmod $csrValue
	puts "CR Value written as [format %08x $csrValue]"

	# Turn off all bits in the ACQ register:

	$list addWrite32 [expr $base + $DAQControl] $setupAmod 0xffff0000
	$list addDelay 20

	# Figure out what needs turning on in the acquisition register (having
	# first turned everything off) ( and program related parameters if necessary)

	set acqValue 0

	# start delay also implies programing the start delay value:

	if {$options(-startdelay)} {
	    set acqValue [expr $acqValue | $DAQStartDelayOn]
	    $list addWrite32 [expr $base + $ExternStartDelay] $setupAmod $options(-startdelayticks)
	}
	# stop delay also implies programming the stop delay value:

	if {$options(-stopdelay)} {
	    set acqValue [expr $acqValue | $DAQStopDelayOn]
	    $list addWrite32 [expr $base + $ExternStopDelay] $setupAmod $options(-stopdelayticks)
	}
	if {$options(-gatemode)} {
	    set acqValue [expr $acqValue | $DAQEnableGateMode]
	}
	if {$options(-randomclock)} {
	    set acqValue [expr $acqValue | $DAQEnableRandomClock]
	}
	if {$options(-lemostartstop)} {
	    set acqValue [expr $acqValue | $DAQEnableLemoStartStop]
	}
	if {$options(-p2startstop)} { 
	    set acqValue [expr $acqValue | $DAQEnableP2StartStop]
	}
	if {$options(-hirarandomclock)} {
	    set acqValue [expr $acqValue | $DAQEnableHiRARCM | $DAQEnableRandomClock]
	}
 
	set acqValue [expr $acqValue | ($clockSourceValues($options(-clocksource)) << $DAQClockSetShiftCount)]
	
	$list addWrite32 [expr $base + $DAQControl] $setupAmod $acqValue
	puts "ACQ Value [format %04x $acqValue]"
	

	# Configure the global event configuration register.

	set evtConfig [expr $sampleSizeValues($options(-samplesize)) << $ECFGPageSizeShiftCount]
	if {$options(-wrap)} {
	    set evtConfig [expr $evtConfig | $ECFGWrapMask]
	}
	if {$options(-randomclock)} {
	    set evtConfig [expr $evtConfig | $ECFGRandomClock]
	}
	puts "Evt config register [format %08x $evtConfig]" 
	$list addWrite32 [expr $base + $CommonInfo + $EventConfiguration] $setupAmod $evtConfig

	
	# Channel thresholds:

	puts "Setting channel enables"

	for {set chan 0} {$chan < 8} {incr chan 2} {
	    set even [lindex $options(-thresholds) $chan]
	    if {$options(-thresholdslt)} {
		set even [expr $even | $THRLt]
	    }
	    set odd [lindex $options(-thresholds) [expr $chan + 1]]
	    if {$options(-thresholdslt)} {
		set odd [expr $odd | $THRLt]
	    }
	    set registerValue [expr ($odd << $THRChannelShift) | $even]
	    puts "Writing threshold [format "%08x to %08x" $registerValue [expr $base +  $EventBases([expr $chan/2]) + $TriggerThreshold]]"

	    $list addWrite32 \
		[expr $base +  $EventBases([expr $chan/2]) + $TriggerThreshold] \
		$setupAmod $registerValue
	}
	puts "Done setting channel thresholds"

	# Disarm both banks first:

	$list addWrite32 [expr $base + $DAQControl] $setupAmod \
	    [expr $DAQSampleBank1Off | $DAQSampleBank2Off]


	
	# Execute the list:
	
	puts "Executing list"

	$controller executeList $list 1000
#	$inputData destroy
	$list -delete
	puts "Returning from init"

	# Dump the values of some key registers:

	set cr [$vmusb vmeRead32 [expr $base + $CR] $setupAmod]
	puts "Control register [format %08x $cr]"

	set daqctl [$vmusb vmeRead32 [expr $base + $DAQControl] $setupAmod]
	puts "DAQ control register  [format %08x $daqctl]"


	set evconfig [$vmusb vmeRead32 [expr $base + $EventInfo1] $setupAmod]
	puts "Event config: [format %08x $evconfig]"

	# Do some one shot stuff at the end of the bulk initialization.
	#  - Clear the data
	#  - arm bank 1
	#  - If the configuration justifies, start sampling.

	$self _SetArm $controller 1 on

	if {!$options(-gatemode)} {
	    puts "Turning on sampling now!"
	    $self _StartSampling $controller
	}
	set daqctl [$vmusb vmeRead32 [expr $base + $DAQControl] $setupAmod]
	puts "DAQ control register(after arming)  [format %08x $daqctl]"	
    }
    ##
    # addReadoutList 
    #   Add elements to the readout list that responds to the trigger for the stack
    #   an instance of this class belongs to.
    #
    #   The resulting data structure is:
    #    +----------------------+
    #    | group mask           | 1's for each group read.
    #    +----------------------+
    #    | group longword count | (for lowest ordered bit in the group mask).
    #    +----------------------+
    #    | Samples for group... |
    #    \                      \ See SIS manual for the format here.
    #    /                      /
    #    +---------------------+
    #      ...
    #
    #
    # @param list - Swig handle to a cvmusbreaoutlist::VMUSBReadoutList object.
    #               This must be converted to a swig object.
    #
    #
    method addReadoutList list {
	set base $options(-base); # using this frequently so...
	set list [VMUSBDriverSupport::convertVmUSBReadoutList $list]

	# Stop the adc sample clock:

	$list addWrite32 [expr $base + $VMEStop] $setupAmod 1
	$list addWrite32 [expr $base + $DAQControl] $setupAmod $DAQSampleBank1Off

	# Prefix the output with a mask of the groups we'll read:

	set mask 0 
	$list addMarker 0xfadc
	foreach bit [list 1 2 4 8] enable $options(-groupsread) {
	    if {$enable} {
		set mask [expr $mask | $bit]
	    }
	}
	$list addMarker $mask
	$list addDelay  1

	# Read each enabled group using the address register to provide a count.

	foreach enable $options(-groupsread) \
	    evInfoBase [list $EventInfo1 $EventInfo2 $EventInfo3 $EventInfo4] \
	    evBuffer [list $Bank1Group1 $Bank1Group2 $Bank1Group3 $Bank1Group4] {
		if {$enable} {
		    puts "Adding maske count read from \
[format %08x [expr $base + $evInfoBase + $EventDirectory1]] mask : [format %08x $sampleMaskValues($options(-samplesize))]" 
		    $list addBlockCountRead32 \
			[expr $base + $evInfoBase + $EventDirectory1] \
			[expr $sampleMaskValues($options(-samplesize))] $setupAmod; # read count..
		    
		    $list addMaskedCountBlockRead32 [expr $base + $evBuffer] $blockXfer; # the read itself.
		    $list addMarker 0xaaaa
		    $list addDelay   1
		}
	    }

	#  Clear daq, re-arm and start sampling.
	#

	$list addWrite32 [expr $base + $DAQControl] $setupAmod $DAQSampleBank1On
	$list addWrite32 [expr $base + $VMEStart]   $setupAmod 1


    }

    method onEndRun vmusb {

    }

    #-----------------------------------------------------------------------
    #
    # Private utilties:


    ##
    #  method _Clear controller
    #
    #  Clears the data in the module
    #
    # @param controller - VMUSB controller object.
    #
    method _Clear controller {
	$controller vmeWrite32 [expr $options(-base) +$DAQControl] \
	    $setupAmod $DAQSampleBank1On
    }
    ##
    # method _SetArm controller which state
    #
    # Arms or disarms an adc bank:
    #
    # @param controller - VMUSB controller object.
    # @param which      - Which bank to arm/disarm (1 or 2).
    # @param state      - What to do: true - arm, false disarm
    #
    method _SetArm {controller which state} {
	array set arms    [list 1 $DAQSampleBank1On  2 $DAQSampleBank2On]
	array set disarms [list 1 $DAQSampleBank1Off 2 $DAQSampleBank2Off]

	set addr [expr $options(-base) + $DAQControl]
	if {$state} {
	    set value $arms($which)
	} else {
	    set value $disarms($which)
	}
	puts "Arm/disarm: [format "%08x @ %08x" $value $addr]"
	$controller vmeWrite32 $addr $setupAmod $value
    }
    #
    # _StartSampling controller
    #
    # Start the ADC Sampling.
    #
    # @param controller - VM-USB controller object.
    #
    method _StartSampling {controller} {
	$controller vmeWrite32 [expr $options(-base) + $VMEStart] \
	    $setupAmod 1;	# Key register could write anything.
    }


    ##

    #-----------------------------------------------------------------------
    #
    # Configuration setters:

    ##
    # configureClockSource:
    #
    #  Configure the clock source selector.  Array indices of clockSourceValues
    #  are the set of valid value strings.
    #
    # @param option - name of the option to set.
    # @param value  - Proposed new value.
    #
    method configureClockSource {option value} {
	::VMUSBDriverSupport::validEnum $value [array names clockSourceValues]
	set options($option) $value
    }
    ##
    # configureBool:
    #
    # Common function to validate and configure a boolean parameter.
    #
    # @param option - name of the option to set.
    # @param value  - Proposed new value.
    #
    method configureBool {option value} {
	::VMUSBDriverSupport::validBool $value
	set options($option) $value
    }
    ##
    # configureInt
    #   
    # Common code to configure an integer that has no limits
    #
    # @param option - name of the option to set.
    # @param value  - Proposed new value.
    #
    method configureInt {option value} {
	::VMUSBDriverSupport::validInt $value
	set options($option) $value
    }
    ##
    # configureSampleSize:
    #   Configure the number of samples of data to take.
    #   This is an enumerated parameter whose legal values are the
    #   array indices of sampleSizeValues
    #
    # @param option - name of the option to set.
    # @param value  - Proposed new value.
    #
    method configureSampleSize {option value} {
	::VMUSBDriverSupport::validEnum $value [array names sampleSizeValues]
	set options($option) $value
    }

    ##
    # configureThresholds:
    #   configure the channel threshold values.  These are an 8 element list of values.
    #   in the range 0 - 3fff (SIS 3301 limits are used).
    #
    # @param option - name of the option to set.
    # @param value  - Proposed new value.
    #
    method configureThresholds {option value} {
	::VMUSBDriverSupport::validIntList $value 8 8 0 0x3ff
	set options($option) $value
    }
				     
    ##
    #  configureGroupsRead:
    #    Configures the set of groups to read.  This is a 4 element list of bools
    # 
    # @param option - name of the option to set.
    # @param value  - Proposed new value.
    #
    method configureGroupsRead {option value} {
	::VMUSBDriverSupport::validBoolList $value 4 4
	set options($option) $value
    }
			    
}
