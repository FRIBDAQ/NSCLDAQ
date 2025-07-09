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
# @file AXLM72Generic.tcl
# @brief Generic slow controls driver for XLM72.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide AXLM72Generic 1.0
package require Itcl
package require cvmusb
package require cvmusbreadoutlist
package require VMUSBDriverSupport
package require xlm72

##
# @class  AXLM72Generic
#    Generic slow controls support class for the XLM72.
#    We provide the following services:
#    *  Initialize - no-op.
#    *  Update     - no-op.
#    *  addMonitorList - no-op.
#    *  processMonitorList - no-op.
#    *  getMonitoredData  - No-op.
#    *  Set - Set a value in the XLM address space - or boot. or configure.
#    *  Get - Retrieve a value in the XLM address space.
#

itcl::class AXLM72Generic {
    inherit AXLM72
    
    private variable base
    
    ##
    # constructor
    #   Just need to call base class constructor and save the base
    #   address.
    #
    # @param dev  - Device object.
    # @param slot - Slot the XLM lives in.  Base is slot << 27.
    #
    constructor {dev slot} {

        AXLM72::constructor $dev $slot
    } {
        set base [expr $slot << 27]
    }
    #  Slow controls interface class:
    
    # No-op methods:
    
    public method Initialize  vme
    public method Update      vme
    public method addMonitorList vmeList
    public method processMonitorList data
    public method getMonitoredData {}
    
    # Methods that do something:
    
    public method Set {vme name value}
    public method Get {vme name}
}

#------------------------------------------------------------------------------
#  Class implementation.  Starting with the no-op methods:
#

##
# Initialize
#    Normally initializes the device.  The XLM needs no initialization:
#
# @param vme - VME device controller.  We'll set that in our base:
#
itcl::body AXLM72Generic::Initialize vme {

    SetController [::VMUSBDriverSupport::convertVmUSB $vme]
}

##
# Update
#    Intended for devices with write-only registers.  Updates device registers
#    from an internal shadow copy.  No-op for the XLM72.
#
# @param vme - vme controller object.
#
itcl::body AXLM72Generic::Update vme {

    SetController [::VMUSBDriverSupport::convertVmUSB $vme]
}

##
# addMonitorList
#    Intended for use by devices that must be monitored during data taking.
#    The slow controls system reserves stack 7 as a periodically triggered stack
#    that will read a defined set of values from devices that can be fetched
#    later on.  This method would be used to add registers/RAM etc. that
#    would be read in that list.
#
# @param vme - VMELlist.
#
itcl::body AXLM72Generic::addMonitorList vmelist {

}

##
# processMonitorList
#    Called after monitor list execution has completed.  This method allows the
#    object to fetch its data from the list and
#
# @param data - list of remaining data to be processed. This is a list of integers,
#               one per remaining byte of data.
# @return int - Number of bytes of data consumed from the list.  This is used
#               to determine where the first byte of data for the next
#               control module is.
#
itcl::body AXLM72Generic::processMonitorList data {
    return 0;               # We don't use any data.
}
##
# getMonitoredData
#    Called in response to a client's request to retrieve the monitored data.
#    since we have no data we're just going to return an empty list.
#
# @return list of data gotten during last call to processMonitorList
#
itcl::body AXLM72Generic::getMonitoredData {} {
    return [list]
}

##
#  Methods that actually do something.
#

#
# Set
#   Sets a value in a register or does a special function.
#
# @param vme  - The vme controller object connected to the device.
# @param what -  Name of the variable to be modified.  This is one of the following:
#               *  An integer number - that's the offset from the base address
#                  that will be modified.  value is the value to put in that
#                  offset.
#               * "Access"   - Accessses the bus.  value  is the code to write
#                  in the bus access register.
#               * "Release"  - Releases the bus.  Value is ignored.
#               * "Boot"     - Puts the FPGA in boot mode.
#               * "BootSource" - Sets the FPGA's boot source code.  Value is the
#                  boot source code.
#               * "Configure" - Loads FPGA firmware into SRAMA and boots from it.
#                  value is the filename of the firmware file.
#
# @param value - Value associated with what.  See above for the meaning of
#                value which can vary depending on thevalue of "what".
#

itcl::body AXLM72Generic::Set {vme what value} {
    SetController $vme
    
    #  Decode what and dispatch accordingly:
    
    if {[string is integer -strict $what]} {
        
        Write base $what $value
    } elseif {$what eq "Access"} {
        AccessBus $value
    } elseif {$what eq "Release"} {
        ReleaseBus
    } elseif {$what eq "Boot"} {
        BootFPGA
    } elseif {$what eq "BootSource" } {
        SetFPGABoot $value
    } elseif {$what eq "Configure"} {
        Configure $value
    } else {
        return "ERROR - AXLMGeneric::Set $what $value - invalid value for 'what'."
    }
    return "OK"
}

##
# Get
#   Returns a value from the module.
#
# @param what  - The offset to the base of the module from which the value will
#                be gotten.
# @return value - the value or a string beginning "ERROR -"
#
itcl::body AXLM72Generic::Get {vme what} {
    SetController $vme
    set result [Read base $what]
    return  $result
}
