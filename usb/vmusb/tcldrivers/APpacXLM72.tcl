#===================================================================
# class APpacXLM72
#===================================================================

package provide ppacxlm72 1.0
package require xlm72 
package require vmusb
package require Itcl 

##
# Low-level tcl driver for communicating with an XLM72V running
# the PPAC readout firmware, used in the S800.
#

itcl::class APpacXLM72 {
	inherit AXLM72              ;#< Base class

  ## @brief Constructor
  #
  # Sets up the addresses of the srama, sramb, fpga, vme, and base
  # based on the slot number.
  #
  # @param sl the slot in which the XLM72V is located
	constructor {sl} {
		AXLM72::constructor $sl
	} {}

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
  # @param sa   the number of samples
  #
	public method WriteSamples {ctlr sa} {Write $ctlr fpga 4 $sa}

  ## @brief Set the period
  # 
  # @warning Bus ownership must have already been obtained for X
  #          bus (0x10000)
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param pe   the period (units?)
  #
	public method WritePeriod {ctlr pe} {Write $ctlr fpga 12 $pe}

  ## @brief Set the delay
  # 
  # @warning Bus ownership must have already been obtained for X
  #          bus (0x10000)
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param de   the delay (units?)
  #
 	public method WriteDelay {ctlr de} {Write $ctlr fpga 16 $de}

  ## @brief Set the width
  #
  # @warning Bus ownership must have already been obtained for X
  #          bus (0x10000)
  #
  # @param ctlr a cvmusb::CVMUSB object
  # @param wi   the delay (units?)
  #
	public method WriteWidth {ctlr wi} {Write $ctlr fpga 20 $wi}

  ## @brief Set the shift
  # 
  # @warning Bus ownership must have already been obtained for X
  #          bus (0x10000)
  #
  # @param sh   the shift (units?)
  # @param ctlr a cvmusb::CVMUSB object
  #
	public method WriteShift {ctlr sh} {Write $ctlr fpga 24 $sh}

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
 	public method WriteThresholds {ctlr th}

  ## @brief Reset the data to read in an event
  #
  # @warning Bus ownership must have already been obtained for SRAMA
  #          bus (0x00001)
  #
  # @param ctlr a cvmusb::CVMUSB object
  #
 	public method Clear {ctlr } {Write $ctlr srama 0 0}


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
	public method Init {ctlr filename array}


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
	public method sReadAll {aStack}


  ## @brief Add a clear command to the stack
  # 
  # This simply writes 0 to the srama[0] address.
  #
  # @warning Bus ownership must have already been obtained for SRAMA
  #          bus (0x00001)
  #
  # @param aStack the stack to append functionality to
  # 
  public method sClear {aStack}

}
# END OF THE APpacXLM72 Class

################################################################################
# ------------------ UTILITY METHOD IMPLEMENTATIONS ---------------------------#




# Write all 256 threshold values
#
itcl::body APpacXLM72::WriteThresholds {ctlr th} {
# if the th list contains less than 256 values, pad it with 1023 (10 bit max)
	if {[llength $th] < 256} {
		for {set i 0} {$i < 256-[llength $th]} {incr i} {lappend th 1023}
	}
# now write thresholds to RAM block of FPGA
	for {set i 0} {$i < 64} {incr i} {
		Write $ctlr fpga 40 $i; # set RAM address
		Write $ctlr fpga 44 [lindex $th $i]; # set connector 0 threshold register
		Write $ctlr fpga 48 [lindex $th [expr $i+64]];  # set connector 1 thresh reg
		Write $ctlr fpga 52 [lindex $th [expr $i+128]]; # set connector 2 thresh reg
		Write $ctlr fpga 56 [lindex $th [expr $i+192]]; # set connector 3 thresh reg
		Write $ctlr fpga 60 1; # toggle WE of RAM (write RAM)
		Write $ctlr fpga 60 0; # toggle back
		Write $ctlr fpga 64 1; # enable RAM address for read
		Write $ctlr fpga 72 0; # read RAM into registers
		Write $ctlr fpga 64 0; # disable RAM address for read

    # Read back what we wrote to ensure it succeeded
		for {set c 0} {$c < 4} {incr c} {
			set check [Read $ctlr fpga [expr 44+$c*4]]
			if {$check != [lindex $th [expr $i+$c*64]]} {
        set errmsg "Failed to set threshold in XLM72V of [$this GetVariable self]: "
        append errmsg "$check vs [lindex $th [expr $i+$c*64]]" 
			  error $errmsg 
			}
		} ;# end of read back verification

	} ;# end of write loop

}


# ------------------ HIGH-LEVEL METHOD IMPLEMENTATIONS --------------------------#

# This method assumes filename points to an "old" type Tcl file defining parameters
# in an array called "aname"
itcl::body APpacXLM72::Init {ctlr filename aname} {
	source $filename

  # Check that the array exists that we are depending on
  if {![array exists $aname]} {
    error "APpacXLM72::Init : $filename did not define the $aname array"
  }

  # if here, there exists an $aname array. We want to check that period, 
  # delay, width, and shift all exist as names in the array.
  set names [array names $aname]

 
	AccessBus $ctlr 0x10001

  # Write the samples, period, delay, width, shift, and threshold values
  #  if and only if the $aname array provides a value. Otherwise throw an
  #  error.

  #  if {"samples" in $names} {
  #  	WriteSamples [lindex [array get $aname samples] 1]
  #  } else {
  #    error "APpacXLM72::Init : $aname does not contain element \"samples\""
  #  }

  if {"period" in $names} {
    WritePeriod $ctlr [lindex [array get $aname period] 1]
  } else {
    error "APpacXLM72::Init : $aname does not contain element \"period\""
  }

  if {"delay" in $names} {
    WriteDelay $ctlr [lindex [array get $aname delay] 1]
  } else {
    error "APpacXLM72::Init : $aname does not contain element \"delay\""
  }

  
  if {"width" in $names} {
    WriteWidth $ctlr [lindex [array get $aname width] 1]
  } else {
    error "APpacXLM72::Init : $aname does not contain element \"width\""
  }


  if {"shift" in $names} {
    WriteShift $ctlr [lindex [array get $aname shift] 1]
  } else {
    error "APpacXLM72::Init : $aname does not contain element \"shift\""
  }

  # Convert the threshold array to a list and then write it to the device
	for {set i 0} {$i < 256} {incr i} {

    set threshName [format thresholds%.3d $i]

    if {$threshName in $names} {
		  lappend th [lindex [array get $aname ] 1]
    } else {
      error "APpacXLM72::Init : $aname does not contain element \"shift\""
    }
	}
	WriteThresholds $ctlr $th

	Clear $ctlr
	ReleaseBus $ctlr
}



# ----------------- STACK BUILDING METHODS IMPLEMENTATIONS --------------------------#


#
itcl::body APpacXLM72::sReadAll {aStack} {
	sAccessBus $aStack 0x1   ;# Access the SRAMA bus

# Special NBLT read mode where address 0 of SRAMA contains the length of the subsequent block transfer
	sReadNBLT $aStack srama 0 0xffc srama 4; # mask 0x1ffc is for 8191 bytes max converted to 32 bit words (last 2 bits are 0)

  sClear $aStack
	sReleaseBus $aStack
}





#
# Clear first memory slot of SRAMA which contains the number of bytes to read
itcl::body APpacXLM72::sClear {aStack} {
	sWrite $aStack srama 0 0
}
