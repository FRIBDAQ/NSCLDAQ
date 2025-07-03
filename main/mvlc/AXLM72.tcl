#
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
# @file   AXLM72.tcl
# @author Jeromy Tompkins
# @note   This is a heavily modified version of the original AXLM72 class
#         that was written by Daniel Bazin. It has been gutted to use
#         the cvmusb::CVMUSB and cvmusbreadoutlist::CVMUSBReadout methods
#         and to have a format that is more in line with other NSCLDAQ 
#         code.




package provide xlm72 1.0

package require Itcl
package require VMUSBDriverSupport 


##
# @class AXLM72
#
# This is the IncrTcl base class for all XLM72* modules. It provides
# a large number of convenience functions that derived classes can use
# in their drivers. The XLM72* family of devices are general purpose
# logic modules that have no real function independent of the firmware
# loaded onto them. For this reason, derived classed typically handle
# how to interact with devices that are running a specific firmware.
#
itcl::class AXLM72 {
	private variable self
	protected variable device 

	private variable slot   ;#< slot of the device
	private variable base   ;#< base address 
	private variable vme    ;#< address offset to vme interface
	private variable fpga   ;#< address offset to fpga 
	private variable dsp    ;#< address offset to dsp
	private variable srama  ;#< address offset to srama
	private variable sramb  ;#< address offset to sramb
	

  ##
  # Constructor
  #
  # Sets up all the addresses of devices within the XLM. These 
  # are all with respect to the base address computed from the slot
  # passed in as an argument
  # @param de - ignored.
  # @param sl slot in which the XLM resides.
  #
	constructor {de sl} {
    set device "mvlc";      # This is the 'controller command' on mvlcgenerate.
		set self [string trimleft $this :]
		set slot  $sl
		set base  [expr $sl<<27]
		set vme   [expr $base+0x800000]
		set dsp   [expr $base+0x600000]
		set fpga  [expr $base+0x400000]
		set sramb [expr $base+0x200000]
		set srama [expr $base+0x000000]
	}
	
	destructor {}
	
# interactive functions
  ## A method for dereferencing a tcl name
  #
  # @param v the name of a tcl variable 
  # @returns the associated value
  public method setController {controller} {set device $controller}
	public method GetVariable {v} {set $v}

  public method SetController {ctlr} {set device $ctlr}
  public method GetController {} {return $device}

  ## 
  # Read
  #
  # Performs a 32-bit read using the A32 unpriviledged access address modifier (0x09).
  # We allow this because it may be used in a slow controls server:
  #
  # @param dev     base address of the XLM72
  # @param address offset to add to the dev argument
  # 
  # @returns the value read from the module 
  public method Read {dev address} {
      
      return [$device vmeRead32 [expr [set $dev]+$address] 0x09]
  }

  ##
  # Write
  #
  # Performs a 32-bit write using the A32 unpriviledged access address modifier (0x09)
  #
  # @param dev     base address of the XLM72
  # @param address offset to add to the dev argument
  # @param data    the integer value to write 
  # 
  # @return int
  # @retval  0 - success
  # @retval -1 - USB write failed
  # @retval -2 - USB read failed
  # @retval -3 - VME Bus error
  public method Write {dev address data} { 
      return [$device vmeWrite32 [expr [set $dev]+$address] 0x09 [expr $data]]
  }

  ##
  # ReadSBLT
  #
  # Immediately performs a block read using an unprivileged A32 BLT address 
  # modifier (0x0b) as a single shot operation. The BLT will begin at the 
  # address formed by adding together the dev and address arguments.
  # 
  # @param dev      base address of the XLM72
  # @param address  address offset begin BLT read from
  # @param words    number of transfers to include in the BLT
  #
  public method ReadSBLT {dev address words}

  #we are going to see if we can get away with not implementing this.
#	public method ReadFBLT {dev address blocks} {i
#      return [$device ReadFBLT32 [expr [set $dev]+$address] $blocks 0]
#  }

  ## 
  # ReadNBLT
  #
  # Immediately performs a variable block read. The command sets up the 
  # operation as a stack and then executes it using the executeList 
  # method. The module first reads a value from a single address using
  # an A32 unprivileged data access and applies a mask to it. The result
  # of the masked value specifies the number of transfers in the
  # BLT. The address of each operation is provided as a base address and
  # an offset (i.e. address to read transfer count = ndev + nadd )
  #
  # @param ndev     base address of the module where the transfer count will be read from
  # @param nadd     offset address for transfer count
  # @param dev      base address of the module targeted by the BLT
  # @param address  offset address where to start the BLT
  # 
  # @return a Tcl List
  # @retval the data returned from the entire operation parsed into 32-bit words
  # 
  public method ReadNBLT {ndev nadd mask dev address} 

  ##
  # AccessBus
  #
  # Convenience function for gaining access of an internal bus. This is 
  # two single shot operations. The first requests the bus of interest
  # and the second inhibits the FPGA and DSP from gaining mastership.
  # The valid values for the code are any bitwise of bus A (0x1), bus B (0x2), 
  # bus X (0x1000), and bus D (0x2000).
  # 
  # @param code   the code for the internal address 
  # 
  public method AccessBus {code} {
      Write vme [expr 0xc] 1
      Write vme 0 $code; 
      #set aBusOwner [Read vme [expr 0x10000]]
      #set bBusOwner [Read vme [expr 0x10004]]
      #set xBusOwner [Read vme [expr 0x10008]]

      $device delay 5000;    # Just delay 5ms and hope for the best.
      
#      after 100
  }

  ##
  # ReleaseBus
  # 
  # Convenience function for releasing ownership of the internal busses. 
  # This is similar to the AccessBus method but writes 0 to both the bus
  # request and bus inhibit addresses.
  # 
  # 
  public method ReleaseBus {} {
      Write vme 0 0; 
      Write vme [expr 0xc] 0
  }

  ##
  # BootFPGA
  #
  # Convenience function for booting the device. The operation only boots
  # the FPGA. This is accomplished by first writing 1's to the reset bits 
  # of the FPGA and DSP and then writing a 1 to the DSP and a 0 to the FPGA. 
  # This causes the FPGA to boot and the DSP remains in reset mode (i.e. its 
  # reset bit is still set).  
  #
  # @warning Bus ownership should have already been obtained
  #
  #
  public method BootFPGA {}

  ## 
  # SetFPGABoot
  # 
  # A convenience method for writing to the FPGA boot source register.
  # Valid boot sources are 
  # - sector 0 flash (code = 0x0) 
  # - sector 1 flash (code = 0x1)
  # - sector 2 flash (code = 0x2) 
  # - sector 3 flash (code = 0x3) 
  # - sram A (code = 0x10000)
  # .
  # 
  # @warning Bus ownership should have already been obtained
  #
  # @param source  the integer code for the boot source
  #
  public method SetFPGABoot {source} {
    Write vme 8 $source
  }
  
  ## 
  # Configure 
  # 
  # Loads firmware into sramA and then boots the device from sramA
  #
  # @param filename  name of firmware file (.bit) to load
  #
  public method Configure {filename}


  ##
  # ExecuteLongStack
  #
  # Converts a tcl list of raw stack commands into a 
  # cvmusbreadoutlist::CVMUSBReadoutList object that gets 
  # subsequently executed. Before returning the value data it is 
  # parsed using the VMUSBDriverSupport::convertBytesListToTclList
  # because the executeList command returns a vector of bytes.
  # 
  # @param stack   a Tcl list contain raw stack commands
  #
  # @return a Tcl list
  # @retval the data from the stack execution converted to 32-bit integers 
  public method ExecuteLongStack {stack}

# stack functions

  ##
  # sRead
  # 
  # Adds an A32/D32 unpriviledged read (amod = 0x09) to the stack
  # 
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
  # @param dev   base address of the module to read from
  # @param add   offset address to target within the mode at dev
  #
  # No return value.
  #
  public method sRead {stack dev add} {$stack addRead32 [expr [set $dev]+$add] 9}
  
  ##
  # sWrite
  #
  # Adds an A32/D32 unpriviliged read (amod = 0x09) to the stack
  # 
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
  # @param dev   base address of the module to read from
  # @param add   offset address to target within the mode at dev
  #
  # No return value.
  # 
  public method sWrite {stack dev add data} {$stack addWrite32  [expr [set $dev]+$add] 9 $data}

  ## 
  # sReadSBLT
  #
  # Same as the ReadSBLT command except that the commands are added to the 
  # stack passed in and rather than being executed immediately
  #
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
  # @param dev   base address of the module to read from
  # @param add   offset address to start the BLT from
  # @param words number of transfers in the BLT
  #
  # No return value. 
  #
  public method sReadSBLT {stack dev add words}


  # We are going to see if we can get away without this.
#	public method sReadFBLT {stack dev add blocks} {
#   $device sReadFBLT32 $stack [expr [set $dev]+$add] $blocks 0
# }


  ##
  # sReadNBLT
  #
  # Same as the ReadNBLT except that the commands are added to the 
  # stack passed in and rather than being executed immediately
  #
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
  # @param ndev  base address of the module to read count from
  # @param nadd  offset from ndev to read count from
  # @param mask  mask applied to count
  # @param dev   base address of the module to read from
  # @param add   offset address to start the BLT from
  #
  # No return value. 
  #
  public method sReadNBLT {stack ndev nadd mask dev add}

  ##
  # sAccessBus
  # 
  # Same as the AccessBus except the commands are added to the stack
  # passed in and rather than being executed immediately. See AccessBus proc for the
  # valid bus request codes.
  # 
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList object
  # @param code  code specifying which busses to request
  # 
  # No return value
  #
  public method sAccessBus {stack code} {
    sWrite $stack vme 0xc 1
    sWrite $stack vme 0 $code
  }

  ##
  # sReleaseBus
  #
  # Same as the ReleaseBus command except that the commands are added to
  # the stack passed in rather than being executed immediately.
  #
  # @param stack a cvmusbreadoutlist::CVMUSBReadoutList
  #
  # No return value.
  #
  public method sReleaseBus {stack} {
    sWrite $stack vme 0 0 
    sWrite $stack vme 0xc 0
  }
}



###############################################################################
###############################################################################
#
# Implementations
#
#
#
itcl::body AXLM72::ReadSBLT {dev address words} {
  error "AXLM72::ReadSBLT is not supported under mvlcgenerate"

 }



##
#
itcl::body AXLM72::ReadNBLT {ndev nadd mask dev address} {
  error "AXLM72::ReadNBLT is not supported under mvlcgenerate"
  
}


##
#
itcl::body AXLM72::BootFPGA {} {
  set RESETFPGA 1
  set RESETDSP  2
  set resetAll    [expr $RESETFPGA|$RESETDSP]
  set releaseFPGA [expr $RESETDSP]

  # do the writes
	Write vme 4 $resetAll     ;# Set resets for both FPGA & DSP
	Write vme 4 $releaseFPGA  ;# Keep DSP in reset, Release FPGA

  # Leave some time to boot FPGA
	after 500
}


##
#
# This method sets up a Tcl list of raw stack commands and then converts it
# to a valid cvmusbreadoutlist::CVMUSBReadoutList object
# to execute.
#   Loads the FPGA bits into the SRAMA
#   and boots it fromt he SRAMA

itcl::body AXLM72::Configure {filename} {
	set file [open $filename r]
	fconfigure $file -translation binary
	set config [read $file]
	close $file
	set size [file size $filename]
	binary scan $config c$size bytes
# Detect first 0xff in file
	set index 0
	while {[lindex $bytes $index] != -1} {incr index}

# Determine number of 256 bytes blocks
	set nblocks [expr int(($size-$index+1)/64)+1]

# Stack is made of 16 bits words
	#set stacksize [expr $nblocks*128+8]
# Write stack header
#	lappend stack [expr ($stacksize-1)]
#  lappend stack 0
	#lappend stack [expr ((0x4000<<16) | 0x80b) ]; # multi-block BLT write
	#lappend stack $nblocks; 
	#lappend stack [expr $srama ];

  set dest $srama;   # load to SRAM A:
  AccessBus [expr 0x1];   # Get control over the bus to SRAM:

# Translate and write configuration in SRAMA
	for {set i $index} {$i < $size} {incr i} {
		set byte [lindex $bytes $i]
		set blow [expr $byte&0xf]
		set bhigh [expr ($byte&0xf0)>>4]
		set llow [expr (($blow&0x7)<<2) + (($blow&0x8)<<7)]
		set lhigh [expr (($bhigh&0x7)<<2) + (($bhigh&0x8)<<7)]
		set value [expr (($lhigh<<16) | $llow) ];   # 16 bits to write (I think).
    $device vmeWrite16 $dest $::CVMUSBReadout::a32UserData $value
    incr dest 2;        #  Point to next word.
	}
# Execute long stack
	
  #flush stdout
  #ExecuteLongStack $stack

  # let it rest for a 100 ms before computing. This should allow it to 
  # complete anything it needs to do
	$device delay [expr {100*5000}];  # 100ms delay.
	ReleaseBus;               # FPGA needs to get at SRAMA to load.

# Now boot the FPGA from SRAMA

	SetFPGABoot [expr 0x10000]
	BootFPGA

}

##
#
itcl::body AXLM72::ExecuteLongStack {stack} {
  error "AXLM72::ExecuteLongStack is not supported in mvlcgenerate"
  
}

##
#
itcl::body AXLM72::sReadSBLT {stack dev add words} {
  $stack addBlockRead32 [expr [set $dev]+$add] 0x0b $words
}

##
#
itcl::body AXLM72::sReadNBLT {stack ndev nadd mask dev add} {
  set naddr [expr [set $ndev]+$nadd]
  set addr  [expr [set $dev]+$add]
  $stack addBlockCountRead32 $naddr $mask 0x09 
  $stack addMaskedCountBlockRead32 $addr 0x0b 
}




