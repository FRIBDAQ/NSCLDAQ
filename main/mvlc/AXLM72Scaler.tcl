#===================================================================
# class AXLM72Scaler
# D. Bazin, Dec. 2009
# These functions are based on the FPGA configuration named ech32x24
# which contains 32 scalers of 24 bit depth each, as well as a trigger
# output selectable from the bits set in the trigger variable
#===================================================================

package provide scalerxlm72 1.0

package require xlm72
package require Itcl
package require snit 
package require VMUSBDriverSupport 


## @brief Low-level driver for interactions with the XLM72 running the ech32x24.bit
# firmware
#
# This device is mostly just a 32-bit latching scaler but has the feature
# that channel inputs can be redirected as a trigger output. The trigger 
# output is the result of OR'ing all of the input signals of channels
# whose trigger bits are set. 
#
# The scaler can be enabled/disabled and atomically cleared (aka reset).
#
#
#
itcl::class AXLM72Scaler {
	inherit AXLM72
	
  private variable trigger

	constructor {de sl} {
		AXLM72::constructor $de $sl
	} {
		# This is for a SpartanXL version of the module (24 bits scalers)
		for {set i 0} {$i < 32} {incr i} {
			set trigger($i) 0
		}
  }

  ##########################################################
  ##########################################################
  #
  # Interactive functions
  #

  ## @brief Read the firmware signature 
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @returns integer
  # @retval the firmware signature 
	public method GetFirmware {}
  
  ## @brief Atomically clear all scalers
  #
  # This is achieved by writing a 1 and then a 0
  # to the lowest address in the fpga. 
  # 
  # @param ctlr a cvmusb::CVMUSB object
  # 
  # @return int
  # @retval  0 - success
  # @retval -1 - failure during either of the two writes
  # 
	public method Reset {}

  ## @brief Latch the scaler values into SRAMA 
  # 
  # @param ctlr a cvmusb::CVMUSB object
  # 
  # @return int
  # @retval  0 - success
  # @retval -1 - failure during either of the two writes
  # 
	public method Latch {}

  
  ###########################################################
  ###########################################################
  # Trigger register manipulators

  ## @brief Write the value of trigger($bit) to the trigger reg 
  #
  # As a result of history, this relies on the state of the
  # trigger array to determine what to write. This ultimately
  # calls SetTriggerBits after properly setting the requested
  # bit.
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param bit  the bit to change
  #
  # @return int
  # @retval  0 - success
  # @retval -1 - failed during write
	public method SetTrigger {bit}

  ## @brief Set a specific value for a certain trigger bit
  #
  # This first sets the value of the trigger($bit)
  # and then calls SetTrigger.
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param bit  the bit to change
  # 
  # @return int
  # @retval  0 - success
  # @retval -1 - failure during write
	public method SetTriggerBit {bit value}

  ## @brief Set a specific bit in the trigger register
  # 
  # As opposed to the previous two methods, this replaces the
  # entire trigger register value rather than manipulating 
  # specific bits. It is therefore a lower level method but
  # it enables one to write all bits at once.
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param bitset  the bitset to write
  # 
  # @return int
  # @retval  0 - success
  # @retval -1 - failure during write
  public method SetTriggerBits {bitset}

  ## @brief Read the trigger register
  # 
  # @param ctlr a cvmusb::CVMUSB object
  # 
  # @return int
  # @retval the value of the trigger register
	public method ReadTrigger {}
 
  ## @brief Enable or disable the scaler channels from counting 
  # 
  # Write the value of the enable register
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param onoff boolean value determing whether scalers are enabled 
  # 
  # @return int
  # @retval  0 - success
  # @retval -1 - failure during write
	public method SetEnable {onoff}

  ## @brief Read whether scalers are enabled or disabled
  # 
  # Read the value of the enable register
  # 
  # @param ctlr a cvmusb::CVMUSB object
  #
  # @return int
  # @retval 0 - scalers are disabled
  # @retval 1 - scalers are enabled
	public method ReadEnable {}

  ## @brief Read all of the scaler channels 
  #  
  # A readout cycle produced by this method begins with
  # the latching of the scaler values into SRAMA. Then 
  # a block read (A32/D32) is executed to read the first
  # 32 integers from the SRAMA memory.
  #
  # @param ctlr a cvmusb::CVMUSB object
  # 
  # @return list of all 32 scaler values beginning with ch.0
	public method ReadAll {}

  #########################################################
  #########################################################
  # Stack manipulation functions
  #
  # These will be a bit more terse because they do exactly the
  # same thing as their interactive forms
  #--------------------------------------------------------

  ## @brief Add a write to stack that sets enable register to 1 
  #
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
	public method sEnable  {stack}
  
  ## @brief Add a write to stack that sets enable register to 0
  #
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
	public method sDisable {stack}
  
  ## @brief Add a write to the stack that latches the scaler values
  #
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
	public method sLatch   {stack}
 
  ## @brief Add a read out cycle to the stack (latch, delay, blt)
  # 
  # This differs slightly from the interactive form ReadAll
  # because it adds a 4us execution delay into the stack.
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
	public method sReadAll {stack}
  
  ## @brief Add a write to the stack to clear the scalers
  #
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
	public method sReset   {stack}

  
} 
# End of AXLM72Scaler class

# Interactive functions implementation
itcl::body AXLM72Scaler::GetFirmware {} {
	AccessBus 0x10000
	set value [Read fpga 0]
	ReleaseBus
  set fw [format 0x%8x $value]
	return $fw 
}

##
# Reset 
# 
# Clears the scalers by toggling a bit
itcl::body AXLM72Scaler::Reset {} {
	AccessBus 0x10000
	set st0 [Write fpga 0 1] 
  set st1 [Write fpga 0 0]
	ReleaseBus

  set status [expr $st0==0 && $st1==0]
  if {$status == 0 } {
    return -1 
  } else {
    return 0
  }
}

##
# Latch Scaler values
#
# Causes the current scaler values to be written 
# into SRAMA for the next readout cycle.
#
itcl::body AXLM72Scaler::Latch {} {
	AccessBus 0x10000
	set st0 [Write fpga 4 1]; # set latch bit
	set st1 [Write fpga 4 0]; # reset it
	ReleaseBus; # once bus is released, FPGA takes over and writes scalers to SRAMA

  set status [expr $st0==0 && $st1==0]
  if {$status == 0 } {
    return -1 
  } else {
    return 0
  }
}

## 
# SetTriggerBit
#
# Updates the shadow state of the trigger register
# and then writes the value to the module.
#
itcl::body AXLM72Scaler::SetTriggerBit {bit value} {
	set trigger($bit) $value
	return [SetTrigger $bit]
}

##
# Based on the shadow state of the trigger register
# which is contained in the trigger array, the bit
# specified is either turned on or off.
#
itcl::body AXLM72Scaler::SetTrigger {bit} {
	AccessBus 0x10000

  # read the trigger register and then manipulate the
  # desired bit
	set value [Read fpga 0xc]
	if {$trigger($bit)} {
		set value [expr $value|(1<<$bit)]
	} else {
		set value [expr $value&(~(1<<$bit))]
	}
	ReleaseBus

  # write the new value back to the register
  return [SetTriggerBits $value]
}

##
# Quick way to write value to the entire trigger register 
#
itcl::body AXLM72Scaler::SetTriggerBits {bitset} {
	AccessBus 0x10000
	set status [Write fpga 0x0c $bitset]
	ReleaseBus
  return $status
}

# 
itcl::body AXLM72Scaler::ReadTrigger {} {
	AccessBus 0x10000
	set value [Read fpga 0xc]
	ReleaseBus
	return $value 
}

##
# Set enable register
#
#
itcl::body AXLM72Scaler::SetEnable {onoff} {
	AccessBus 0x10000
	set status [Write fpga 8 $onoff]
	ReleaseBus
  return $status
}

##
# Read the enable register
#
# Not entirely sure what the purpose of this is.
#
itcl::body AXLM72Scaler::ReadEnable {} {
	AccessBus 0x10000
  set reg [Read fpga 8]
	set enable [expr $reg&1]
	ReleaseBus
  return $enable
}

##
# ReadAll scaler values
#
# Latches the current scaler values and then
# performs a block transfer read from the SRAMA
# address. There are 32 transfers for the 32
# channels.
#
itcl::body AXLM72Scaler::ReadAll {} {

	Latch
	AccessBus 1
	set data [ReadSBLT srama 0 32]
	ReleaseBus
  
  return $data
}

# Stack functions implementation
itcl::body AXLM72Scaler::sEnable {stack} {
	sAccessBus $stack 0x10000
	sWrite $stack fpga 8 1
	sReleaseBus $stack
}

itcl::body AXLM72Scaler::sDisable {stack} {
	sAccessBus $stack 0x10000
	sWrite $stack fpga 8 0
	sReleaseBus $stack
}

itcl::body AXLM72Scaler::sLatch {stack} {
	sAccessBus $stack 0x10000
	sWrite $stack fpga 4 1
	sWrite $stack fpga 4 0
	sReleaseBus $stack
}

itcl::body AXLM72Scaler::sReadAll {stack} {
	sLatch $stack
	$stack addDelay 20
	sAccessBus $stack 1
	sReadSBLT $stack srama 0 32
	sReleaseBus $stack
}

itcl::body AXLM72Scaler::sReset {stack} {
	sAccessBus $stack 0x10000
	sWrite $stack fpga 0 1
  sWrite $stack fpga 0 0
	sReleaseBus $stack
}


#############################################################
############################################################
#
# AXLM72ScalerControl 
#

## Plug in for the slow controls server. 
#
# This is the companion class of the XLM72SclrGUICtlr class. 
# When properly configured, the slow controls server will 
# direct requests to AXLM72Scaler driver that it owns. It is 
# actually a very thin class that maintains no state. It is mostly
# just a relay. 
#
# The AXLM72ScalerControl class is a slow controls driver and
# implements the Get, Set, Update, addMonitorList, processMonitorList
# interface. At the moment it only implements the Get and Set 
# methods.
#

snit::type AXLM72ScalerControl {
  option -slot -default 0   ;#< the slot in which the XLM72 resides

  variable driver ""        ;#< an instance of the AXLM72Scaler driver

  ## 
  # Construct the instance
  # 
  # It is here that we instantiate a driver 
  #
  constructor args {
    $self configurelist $args
    set driver [AXLM72Scaler #auto {} $options(-slot)]
  }

  # The CControlHardware interface

  ## 
  # Initialize
  #
  # Handle initialization routines that are needed by the
  # device. This will execute the script defined by the -initscript option.
  # This script is also executed if the init command is called without any
  # alternate control configuration file. 
  #
  # \param driverPtr a pointer to a VMUSB controller
  #
  # \return OK always. 
  method Initialize driverPtr {
    $self Update $driverPtr

  }

  ## 
  # Update
  #
  # When an update command is called, the script located by the -updatescript
  # is executed in the global namespace.
  #
  # \param driverPtr a pointer to a VMUSB controller
  #
  # \return OK always. 
  method Update driverPtr {
    set ctlr [::VMUSBDriverSupport::convertVmUSB $driverPtr]
  }

  ##
  # Response to a Set command
  # 
  # The AXLM72ScalerControl handles 3 parameters:
  #  trigger%d (where %d is an integer for the desired channel)
  #  enable
  #  reset  
  # All other parameters will result in an error
  #  
  # \return transaction result
  #
  # Exceptional return when parameter is not understood  
  method Set {ctlr parameter value} {
    # convert the ctlr to something usable
    set ctlr [::VMUSBDriverSupport::convertVmUSB $ctlr]
  
    $driver SetController $ctlr
    # if the value is a list, take the first argument
    set key $parameter
    if {[scan $parameter {trigger%d} subkey] == 1} {
      set key trigger
      # we are already done with retrieving the subkey
    }

    # switch yard for relaying the different responses to appropriate handlers
    switch $key {
      enable  {return [$self SetEnable $value]}
      reset   {return [$self SetReset]} 
      trigger {return [$self SetTrigger $subkey $value]}
      default {error "AXLM72ScalerControl::Set does not support '$key' as a valid parameter"}
    }

  }

  ######################
  ## Set Handlers

  method SetEnable {value} {
    return [$driver SetEnable $value]
  }

  method SetReset {} {
    return [$driver Reset]
  }

  method SetTrigger {ch value} {
    return [$driver SetTriggerBit $ch $value]
    
  }

  ##
  # Respond to a Get command
  # 
  # Given a valid parameter name, this calls the appropriate handler.
  # Five separate parameters are supported by this:
  #  # enable 
  #  # alltriggers
  #  # firmware
  #  # runstate
  #  # allscalers
  # Any parameter name other than these will result in an exceptional return
  #
  # @param ctlr       a cvmusb::CVMUSB object
  # @param parameter  name of parameter requested
  #
  # @return result of request
  method Get {ctlr parameter} {
    # convert swig pointer into something useful
    set ctlr [::VMUSBDriverSupport::convertVmUSB $ctlr]
    
    $driver SetController $ctlr
    # relay parameters to appropriate handlers
    switch $parameter {
      enable {return [$self GetEnable]}
      alltriggers {return [$self GetAllTriggers]}
      firmware {return [$self GetFirmware]}
      runstate {return [$self GetRunState]}
      allscalers {return [$self GetAllScalers]}
      default {error "$parameter is not supported in XLMScalerControl::Get"}
    }

  }
 
  #######################
  ## Get Handlers 

  method GetEnable {} {
    return [$driver ReadEnable] 
  }

  method GetAllTriggers {} {
    set data [$driver ReadTrigger] 
    return $data
  }

  method GetFirmware {} {
    set fw [$driver GetFirmware]
    return $fw 
  }

  method GetRunState {} {
    set state [runstate]
    return $state 
  }

  method GetAllScalers {} {
    set data [$driver ReadAll] 

    return $data
  }

  ## 
  # addMonitorList
  # 
  #
  # NO-OP
  #
  # \param aList list to a VMUSB controller 
  method addMonitorList aList {

  }

  ## 
  # processMonitorList
  #
  # NOOP becase we didn't define a monitor list
  #
  # \param data a tcl list of data bytes remaining to be processed 
  # \return number of bytes processed
  method processMonitorList {data} {
    return 0
  }


  

}

###############################################################
###############################################################
#
# AXLM72ScalerReadout
#

## @class AXLM72ScalerReadout
#
# @brief A prepackaged readout driver based on the AXLM72Scaler driver. It 
#        provides a basic readout module for any XLM72 running the ech32x24.bit 
#        firmware.
#
#
snit::type AXLM72ScalerReadout {
  option -slot          -default 1     -configuremethod configureInt
  option -incremental   -default off   -configuremethod configureBool
  option -loadfirmware  -default true  -configuremethod configureBool
  option -firmware      -default ""    -configuremethod configurePath

  variable driver       "" 

  ## @brief Constructor
  # 
  # @param args   a list of option value pairs
  constructor args {
    variable driver

    $self configurelist $args
    set driver [AXLM72Scaler #auto $options(-slot)]
  }

  ## @brief Initialize the device
  #
  # The logic of the initialize is pretty simple. It is provided below.
  # 
  # \begincode
  method Initialize {driverPtr} {
    set ctlr [VMUSBDriverSupport::convertVmUSB $driverPtr]

    $driver SetController $ctlr

    if {$options(-slot) == 1} {
        set msg "AXLM72ScalerReadout::Initialize : "
        append msg " Slot has not been specified for device. Be sure to configure the -slot option!"
        error $msg
    }

    # if the user wants to load the firmware, then do it
    # otherwise kick and yell
    if {$options(-loadfirmware)} {
      set fwPath $options(-firmware)
      if {[file exists $fwPath]} {
        $driver Configure $fwPath
      } else {
        set msg "AXLM72ScalerReadout::Initialize : "
        append msg " Cannot load firmware because firmware file (\"$fwPath\") does not exist!"
        error $msg
      }
    }

    # Enable and reset the module
    $driver SetEnable 1
    $driver Reset
  }
  # \endcode


  ## @brief Add a readout cycle to the stack
  #
  # Adds a latch, BLT from SRAMA, and then potentially a 
  # reset to the stack. 
  #
  # @param stack  a cvmusbreadoutlist::CVMUSBReadoutList object
  method addReadoutList {stack} {
      set aList [VMUSBDriverSupport::convertVmUSBReadoutList $stack] 
  
      # add the SRAMA readout cycle
      $driver sReadAll $aList

      # clear after the read if desired
      if {$options(-incremental)} {
        $driver sReset $aList
      }
  }

  ## @brief End of run procedures
  # 
  # @param driverPtr  a pointer to a CVMUSB object.
  #
  method onEndRun {driverPtr} {
    # convert to something usable
    set ctlr [VMUSBDriverSupport::convertVmUSB $driverPtr]
    $driver SetEnable 0
  }


  # ----------- CONFIGURATION METHODS --------------

  ## @brief Validates whether the value is an integer
  #
  # @param option name of option
  # @param value  value to assign to the option
  method configureInt {option value} {
    ::VMUSBDriverSupport::validInt $value
    set options($option) $value
  }

  ## @brief Validates whether the value is boolean
  #
  # @param option name of option
  # @param value  value to assign to the option
  method configureBool {option value} {
    ::VMUSBDriverSupport::validBool $value
    set options($option) $value
  }
  
  ## @brief Sets the option without question
  #
  # @param option name of option
  # @param value  value to assign to the option
  method configurePath {option value} {
    set options($option) $value
  }
}
