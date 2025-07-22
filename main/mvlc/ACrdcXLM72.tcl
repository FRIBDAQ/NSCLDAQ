#===================================================================
##
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#      NSCL DAQ Development Group 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
# @file   ACrdcXLM72.tcl
# @author Daniel Bazin and Jeromy Tompkins
# @note   This is a modified version of the original ACrdcXLM72 class
#         that was written by Daniel Bazin. It has been updated to use
#         the cvmusb::CVMUSB and cvmusbreadoutlist::CVMUSBReadout methods
#         and to have a format that is more in line with other NSCLDAQ 
#         code.



package provide crdcxlm72 1.0

package require xlm72
package require Itcl
package require Utils


## @class ACrdcXLM72
#
# A class to support a JTech XLM72V device running firmware to readout the CRDC
# like the S800. This aims at supporting the firmware file s800crdc1v.bit. It
# is a low-level driver that handles the basic communication with the device
# and does not implement the CReadoutHardware or CControlHardware interfaces.
# It is, however, usable to construct such drivers.
#
#   To make this a bit more tclish...we handle what was done in an initialization array
# as options:
#   -filename - firmware file.
#   -samples - number of samples.
#   -period 
#   -delay
#   -width
#   -shift
#   -thresholds - list of thresholds e.g. ...config -thresholds [list ....]; # 256 thresholds.
#   -firmware  - the firmware file.
#
itcl::class ACrdcXLM72 {
	inherit AXLM72

  # Configurable options - a future version might provide reasonable defaults.
  # For now, however they initialize to --unset-- requiring the user to configure them.

  public variable filename --unset--
  public variable samples  --unset--
  public variable period   --unset--
  public variable delay    --unset--
  public variable width    --unset--
  public variable shift    --unset--
  public variable thresholds --unset-- ;#  Should be a list of 256 values.
  

  ## Constructor
  #
  # @param sl the slot in which the AXLM72V resides
  #
	constructor {de sl} {
		AXLM72::constructor $de $sl
	} { }

  ## Wrapper function to make this a readout class:
  #
  public method Initialize {controller}
  public method addReadoutList {stack}
  public method onEndRun {controller}

  ############################################################
  ############################################################
  # Convenience Utility functions 
  #
  # These are building blocks that do not acquire any busses
  # and should be surrounded by AccessBus and ReleaseBus 
  # when they are called.
  #-----------------------------------------------------------


  ## @brief Set the number of samples?
  #
  # @warning Bus ownership must have already been obtained for X
  #          bus (0x10000)
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param sa   the number of samples (must be in range [0,511])
  #
  # @throws error if argument is out of range
	public method WriteSamples {sa} 
	public method ReadSamples {} {
    error "ACrdcXLM72::ReadSamples not supported for mvlcgenerate"
    
  }

  ## @brief Set the period
  # 
  # @warning Bus ownership must have already been obtained for X
  #          bus (0x10000)
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param pe   the period (units?) (must be in range [0,3])
  #
  # @throws error if argument is out of range
	public method WritePeriod {pe}
	public method ReadPeriod {} {
    error "ACrdcXLM72::ReadPeriod not supported in mvlcgenerate"
    
  }

  ## @brief Set the delay
  # 
  # @warning Bus ownership must have already been obtained for X
  #          bus (0x10000)
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param de   the delay (units?) (must be in range [0,15])
  #
  # @throws error if argument is out of range
	public method WriteDelay {de} 
	public method ReadDelay {} {
    error "ACrdcXLM72::ReadDelay not supported for mvlcgenerate"

  }

  ## @brief Set the width
  #
  # @warning Bus ownership must have already been obtained for X
  #          bus (0x10000)
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param wi   the delay (units?) (must be in range [0,63])
  #
  # @throws error if argument is out of range
	public method WriteWidth {wi}
	public method ReadWidth {} {
    error "ACrdcXLM72::ReadWidth not supported for mvlcgenerate"
    
  }

  ## @brief Set the shift
  # 
  # @warning Bus ownership must have already been obtained for X
  #          bus (0x10000)
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param sh   the shift (units?) (must be in range [0,255])
  #
  # @throws error if argument is out of range
	public method WriteShift {sh}
	public method ReadShift {} { 
    error "ACrdcXLM72::ReadShift is not supported for mvlcgenerate"
  }

  ## @brief Set threshold values
  #
  # There are 256 threshold registers to be set
  # and this expects that if not all of them are 
  # provided, then the remainder are to be set to 
  # 1023 (i.e. 10-bit maximum). The writing of the 
  # bits proceeds in 64 steps, writing a threshold to
  # each of the 4 register banks at a time. If the values
  # that are written are not read back as the same
  # then an error is thrown. 
  #
  # @warning Bus ownership must have already been obtained for X
  #          bus (0x10000)
  #
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param th   the list of threshold values
  #
  # Exceptions:
  # Return code = 1 when any of 256 errors fail to write
	public method WriteThresholds {th}

  ## @brief Reset the data to read in an event
  #
  # @warning Bus ownership must have already been obtained for SRAMA
  #          bus (0x00001)
  #
  # @param ctlr a cvmusb::CVMUSB object
  #
	public method Clear {} {Write srama 0 0}

  ############################################################
  ############################################################
  # High level interactive methods 
  #
  # These call the utility functions and handle
  # acquiring and releasing the internal busses.
  #-----------------------------------------------------------


  ## Initialize the module from a script
  #
  # The initialization proceeds by evaluating a script
  # that must define an array whose keys are: period,
  # delay, width, shift, threshold.0, threshold.1, threshold.2, ...
  # threshold.255. 
  #
  # @param ctlr     a cvmusb::CVMUSB object
  # @param filename a tcl script containing initialization info
  # @param array    the name of the array contains the initialization
  #                 info
  #
  # Exceptional returns:
  #  if after sourcing the script the array named $array doesnot
  #  exist or does not contain all of the information required, 
  #  an error occurs and returns with code=1
	public method Init {}


  ############################################################
  ############################################################
  # Stack building methods 
  #-----------------------------------------------------------

  ## Add a readout procedure to the stack
  # 
  # During a readout procedure, a dynamic block transfer is
  # executed where the number of transfers is read from 
  # srama[0] with a mask of 0xffc and then data is read
  # from srama[4].
  # 
  # @param aStack  the stack to append functionality to
  #
	public method sReadAll {stack}

} ;# End of class definition


###############################################################################
#
# Implementations


itcl::body ACrdcXLM72::WriteSamples {sa} {
  if {![Utils::isInRange 0 511 $sa]} {
    set msg "ACrdcXLM72::WriteSamples Argument out of range. "
    append msg {Must be in range [0,511].}
    return -code error $msg
  }
  set res [Write fpga 4 $sa]
#  after 25 
  return $res
}

itcl::body ACrdcXLM72::WritePeriod {pe} {
  if {![Utils::isInRange 0 3 $pe]} {
    set msg "ACrdcXLM72::WritePeriod Argument out of range. "
    append msg {Must be in range [0,3].}
    return -code error $msg
  }

  set res [Write fpga 12 $pe]
#  after 25
  return $res
}

itcl::body ACrdcXLM72::WriteDelay {de} {

  if {![Utils::isInRange 0 15 $de]} {
    set msg "ACrdcXLM72::WriteDelay Argument out of range. "
    append msg {Must be in range [0,15].}
    return -code error $msg
  }

  set res [Write fpga 16 $de]
#  after 25 
  return $res
}

itcl::body ACrdcXLM72::WriteWidth {wi} {
  
  if {![Utils::isInRange 0 63 $wi]} {
    set msg "ACrdcXLM72::WriteWidth Argument out of range. "
    append msg {Must be in range [0,63].}
    return -code error $msg
  }

  set res [Write fpga 20 $wi]
#  after 25 
  return $res
}


itcl::body ACrdcXLM72::WriteShift {sh} {

  if {![Utils::isInRange 0 255 $sh]} {
    set msg "ACrdcXLM72::WriteShift Argument out of range. "
    append msg {Must be in range [0,255].}
    return -code error $msg
  }

  set res [Write fpga 24 $sh]
#  after 25 
  return $res
}



itcl::body ACrdcXLM72::WriteThresholds {th} {
# if the th list contains less than 256 values, pad it with 1023 (10 bit max)
	if {[llength $th] < 256} {
		for {set i 0} {$i < 256-[llength $th]} {incr i} {lappend th 1023}
	}
# now write thresholds to RAM block of FPGA
	for {set i 0} {$i < 64} {incr i} {
		Write fpga 40 $i; # set RAM address
		Write fpga 44 [lindex $th $i]; # set connector 0 threshold register
		Write fpga 48 [lindex $th [expr $i+64]]; # set connector 1 threshold register
		Write fpga 52 [lindex $th [expr $i+128]]; # set connector 2 threshold register
		Write fpga 56 [lindex $th [expr $i+192]]; # set connector 3 threshold register
		Write fpga 60 1; # toggle WE of RAM (write RAM)
		Write fpga 60 0; # toggle back
		Write fpga 64 1; # enable RAM address for read
		Write fpga 72 0; # read RAM into registers
		Write fpga 64 0; # disable RAM address for read
		for {set c 0} {$c < 4} {incr c} {
			set check [Read fpga [expr 44+$c*4]]
			if {$check != [lindex $th [expr $i+$c*64]]} {
				return -code error "Failed to set threshold in XLM72V of [$this GetVariable self]: $check vs [lindex $th [expr $i+$c*64]]"
			}
		}
	}
}

# This method assumes filename points to an "old" type Tcl file defining parameters
# in an array called "aname"
#  Note an incompatibility
#  We use itcl options (configure/cget to)
#
itcl::body ACrdcXLM72::Init {} {\
  if {$filename eq "--unset--"} {
    error "The FPGA firmware file has not been configured via the -filename option"
  }
  AccessBus 0x10001
	Configure $filename
  SetFPGABoot 0x10000
  BootFPGA

	

  # get a list of the names that exist in the array
  

  
  if {$samples ne "--unset--"} {
    WriteSamples $samples
  } else {
    ReleaseBus
    return -code error "ACrdcXLM72::Init -samples must have been configured and was not"
  }

  if {$period ne "--unset--"} {
    WritePeriod $period
  } else {
    ReleaseBus
    return -code error "ACrdcXLM72::Init  -period was never configured"
  }

  if {$delay ne "--unset--"} {
    WriteDelay $delay
  } else {
    ReleaseBus
    return -code error "ACrdcXLM72::Init  -delay was never configured"
  }

  if {$width ne "--unset"} {
    WriteWidth $width
  } else {
    ReleaseBus
    return -code error "ACrdcXLM72::Init -width was never configured"
  }

  if {$shift ne "--unset--"} {
    WriteShift $shift
  } else {
    ReleaseBus 
    return -code error "ACrdcXLM72::Init -shift was never configured"
  }

  if {$thresholds eq "--unset--"} {
    return -code error "ACrdcXLM72::Init -thresholds were never configured!"
  }
  
	
	WriteThresholds $thresholds


	Clear 
	ReleaseBus 
#	Write vme 0x20000 0x824; # Clear mail register
}

itcl::body ACrdcXLM72::sReadAll {stack} {
	sAccessBus $stack 0x1
# Special NBLT read mode where address 0 of SRAMA contains the length of the subsequent block transfer
#	sReadNBLT $stack srama 0 0xffc srama 4; # mask 0x1ffc is for 8191 bytes max converted to 32 bit words (last 2 bits are 0)
# Clear first memory slot of SRAMA which contains the number of bytes to read
#	sWrite $stack srama 0 0
	sReadNBLT $stack vme 0x20824 0xffc srama 4; # mask 0x1ffc is for 8191 bytes max converted to 32 bit words (last 2 bits are 0)
#	sWrite $stack vme 0x20000 0x824; # Clear mail register
	sReleaseBus $stack
}

## The wrappers:

itcl::body ACrdcXLM72::Initialize {controller} {
  setController $controller
  Init
}

itcl::body ACrdcXLM72::addReadoutList {stack} {
  sReadAll $stack
}

itcl::body ACrdcXLM72::onEndRun {controller} {
  setController $controller

  # if a class end run function is added call that here.
}