#===================================================================
##
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2015.
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
# @note   This is a modified version of the original AGD16XLM72 class
#         that was written by Daniel Bazin. It has been updated to use
#         the cvmusb::CVMUSB and cvmusbreadoutlist::CVMUSBReadout methods
#         and to have a format that is more in line with other NSCLDAQ 
#         code.
# @note - Rewritten by R. Fox to work as a mvlcgenerate translator module.

package provide gd16xlm72 1.0

package require Itcl
package require xlm72
package require snit
#
# Note rather than using that really awful aname thingy, we use configuration
# options:
#    -delay - list of 16 delay values, one per channel.
#    -width - list of 16 width values, one per channel.
#    -bypass - bypass value.
#    -inspect - inspection value.
#    -filename   - firmware file path.
#
itcl::class AGD16XLM72 {
	inherit AXLM72

  # configurable options, see above:
  # initializing them to --unset-- requires the user to configure them.
  # in the future, reasonable default values might be stuffed in instead.

  public variable delay "--unset--"
  public variable width "--unset--"
  public variable bypass "--unset--"
  public variable inspect "--unset--"
  public variable filename "--unset--"

	constructor {de sl} {
		AXLM72::constructor $de $sl
	} {}
  #  Note the reads are only done in the slow controls env and the
  #  controller is set.  Have to think abit about if this is threadsafe!!
  #  I think it is - sort of - because the slow controls module encapsulates
  # its own instance of this, _however_   if data taking is active, when
  # slow controls is happening, the bus access might get conflicted in the
  # mvlc case since the stack is autonomously managing access while the
  # Slow controls is too.  Not very sure about how to handle that.

	public method WriteDelayWidth {ch de wi}
	public method ReadDelayWidth {ch}

	public method WriteBypass {by} 
        public method ReadBypass {}

	public method WriteInspect {in}
	public method ReadInspect {}

  public method ReadFirmware {}

	public method Init {filename}
}

itcl::body AGD16XLM72::WriteDelayWidth {ch de wi} {
  set offset [expr $ch*4] 
  return [Write fpga $offset [expr $de+($wi<<8)]]
}

itcl::body AGD16XLM72::ReadDelayWidth {ch} {
  set offset [expr $ch*4] 

  set retValue [Read fpga $offset]


  # parse the results
  set delay [ expr 0xff & $retValue ]
  set width [ expr ($retValue>>8) & 0xff ]
  
  set result [list $delay $width]

  return $result
}

itcl::body AGD16XLM72::WriteBypass {by} {
  return [Write fpga 68 $by]
}

itcl::body AGD16XLM72::ReadBypass {} {
  return [Read fpga 68]
  
}

itcl::body AGD16XLM72::WriteInspect {in} {
  return [Write fpga 72 $in]
}

itcl::body AGD16XLM72::ReadInspect {} {
  return [Read fpga 72]
  
}

itcl::body AGD16XLM72::ReadFirmware {} {
  set fw [Read fpga 0]
  return $fw
}

###
#  @param filename - a file that is sourced in
#    this gives the caller a chance to configure the
#    object
itcl::body AGD16XLM72::Init {} {
  if {![file exists $filename]} {
    set msg "AGD16XLM72::Init initialization error. "
    append msg "FIrmware file ($filename) does not exist."
    return -code error $msg
  } 

  

  #  Check that all options have been initialized:

  if {$delay eq  "--unset--"} {
    return -code error "[info class]::Init - the -delay option has not been configured."
  }
  if {[llength $delay] != 16} {
    return -code error "[info class]::Init - the -delay option must be configured with exactly 16 delays"
  }

  if {$width eq "--unset--"} {
    return -code error "[info class]::Init - the -width option has not been configured."
  }
  if {[llength $width] != 16} {
    return -code error "[info class]::Init - the -width option must be configured with exactly 16 widths"
  }
  
  if {$bypass eq "--unset--"} {
    return -code error "[info class]::Init - the -bypass option has not been configured"
  }
  if {$inspect eq "--unset--"} {
    return -code error "[info class]::Init - the -inspect option has not been configured."
  }

	AccessBus 0x10000

  # Load the firmware:

  Configure $filename
  SetFPGABoot 0x10000
  BootFPGA


	for {set i 1} {$i <= 16} {incr i} {
    set delay [lindex $delay $i] 
    set width [lindex $width $i]
		WriteDelayWidth $i $delay $width
	}
	WriteBypass $bypass
	WriteInspect $inspect
	ReleaseBus
}


###############################################################################
###############################################################################
###############################################################################

## \brief Slow controls module for GD16XLM72
#
#  The AGD16XLM72Control module is used as the server-side component for 
#  talking to a GD16XLM72 via the slow controls server. It is intended to be
#  communicated with by the XLM72GateDelayGUI but can be used with any
#  application that speaks the proper protocol. It merely is a wrapper around 
#  an AGD16XLM72 driver for which it translates requests into actual low-level
#  driver calls.
snit::type AGD16XLM72Control {

  option -slot -default 0

  variable driver {}

  constructor args {
    $self configurelist $args
    set driver [AGD16XLM72 #auto {} $options(-slot)];  
  }


  method Initialize driverPtr {
    $self Update $driverPtr
  }


  method Update driverPtr {
    set ctlr $driverPtr;     # No need to do fancy stuff 
  }

  method SetInspect value {
    $driver AccessBus 0x10000
    if {[catch {$driver WriteInspect $value} msg]} {
      $driver ReleaseBus
      return "ERROR Failure while writing inspect : $msg"
    }

    $driver ReleaseBus
    return OK
  }

  method SetBypass value {
    $driver AccessBus 0x10000
    if {[catch {$driver WriteBypass $value} msg]} {
      $driver ReleaseBus
      return "ERROR Failure while writing bypass : $msg"
    }

    $driver ReleaseBus
    return OK
  }

  method SetDelayWidth {param value} {
    set channel [$self ExtractChannelNumber $param]
    if {[catch {$self DecodeDelayAndWidth $value} values]} {
      return $values
    }

    set delay [lindex $values 0]
    set width [lindex $values 1]

    $driver AccessBus 0x10000
    if {[catch {$driver WriteDelayWidth $channel $delay $width} msg]} {
      $driver ReleaseBus
      return "ERROR Failed while writing delay width to the module. Msg=\"$msg\""
    }

    $driver ReleaseBus
    return OK
  }

  method DecodeDelayAndWidth value {
    # extract the delay and width 
    set pattern {^delay(\d+)width(\d+)$}
    set matches [regexp -inline $pattern $value]

    if {[llength $matches] == 0} {
      return -code error "ERROR Unable to parse delay and width values from value=\"$value\""
    }

    return [lreplace $matches 0 0]
  }

  method ExtractChannelNumber param {
    set pattern {^delaywidth(\d+)$}
    set channel [lindex [regexp -inline $pattern $param] 1]
    # make sure that the channel is not left padded with zeroes, because that
    # might end up causing the number to be treated as an octal number
    return [string trimleft $channel 0]
  }

  method IsDelayWidth {param} {
    return [expr {[$self ExtractChannelNumber $param] ne {}}]
  }

  method Set {ctlr param value} {
    # convert the ctlr to something usable
    set ctlr [::VMUSBDriverSupport::convertVmUSB $ctlr];   # This works in either case.

    $driver SetController $ctlr
  
    switch $param {
      inspect {
        return [$self SetInspect $value]
      }
      bypass  {
        return [$self SetBypass $value]
      }
      default {
        if {[$self IsDelayWidth $param]} {
          return [$self SetDelayWidth $param $value]
        } else {
          return "ERROR Parameter value \"$param\" is not supported for Set operation."
        }
      }
    } ;# end of switch

  } ;# end of Set


  method GetFirmware {} {
    $driver AccessBus 0x10000
    if {[catch {$driver ReadFirmware} msg]} {
      $driver ReleaseBus
      return "ERROR Failure while reading firmware signature : $msg"
    }
    $driver ReleaseBus
    return $msg
  }

  method GetInspect {} {
    $driver AccessBus 0x10000
    if {[catch {$driver ReadInspect} msg]} {
      $driver ReleaseBus
      return "ERROR Failure while reading inspect register : $msg"
    }
    $driver ReleaseBus
    return $msg
  }

  method GetBypass {} {
   
    $driver AccessBus 0x10000
    if {[catch {$driver ReadBypass} msg]} {
      $driver ReleaseBus
      return "ERROR Failure while reading bypass register : $msg"
    }
   
    $driver ReleaseBus
    return $msg
  }

  method GetDelayWidth param {
    set channel [$self ExtractChannelNumber $param]

    $driver AccessBus 0x10000
    if {[catch {$driver ReadDelayWidth $channel} msg]} {
      $driver ReleaseBus
      return "ERROR Failed while reading channel $channel delay/width from the module. Msg=\"$msg\""
    }

    $driver ReleaseBus
    return $msg
  }

  method Get {ctlr param} {

    set ctlr [::VMUSBDriverSupport::convertVmUSB $ctlr]
    $driver SetController $ctlr

    switch $param {
      fwsignature { return [$self GetFirmware]}
      inspect     { return [$self GetInspect]}
      bypass      { return [$self GetBypass]}
      default {
        if {[$self IsDelayWidth $param]} {
          return [$self GetDelayWidth $param]
        } else {
          return "ERROR Parameter value \"$param\" is not supported for Get operation."
        }
      }
    } ; # end of switch
  } ;# end of Get


  method addMonitorList aList {}

  method processMonitorList data {
    return 0
  }

}
