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



# @file checkdriver.tcl
#   Provides common base code for tcl drivers for the VM-USB (probably just fine for the CCUSB as
#   well.  This includes
#   - parameter type/range checking.
#   - object conversion (turn swig object id into a usable object).
#

package provide VMUSBDriverSupport 1.0
if {[array names ::env MVLC_TRANSLATOR] eq ""} {
  package require DriverSupport 
  package require cvmusb 
  package require cvmusbreadoutlist
}

# Establish the namespace in which the procs will live.
#
namespace eval ::VMUSBDriverSupport {

}

# These are not needed for mvlcgen:

if {[array names ::env MVLC_TRANSLATOR] eq ""} {
  ##
  # convertVmUSB
  #
  # Convert a VMUSB driver id passed into the Tcl code from swig wrappers
  # to an object of the same name
  #
  # @param name - VMUSB object name passed in by swig.
  #
  # @return string - Name of object useable by Tcl code.
  #                  in general this will be the same as the input driver name.
  #
  proc ::VMUSBDriverSupport::convertVmUSB name {
      cvmusb::CVMUSB -this $name
      return $name
  }
} else {
  proc ::VMUSBDriverSupport::convertVmUSB name {
    return $name;                 # MVLCGen requires no conversion.
}

if {[array names ::env MVLC_TRANSLATOR] eq ""} {
  ##
  # convertVmUSBReadoutlist
  #
  #  Convert a VMSUSB readout list id passed into the Tcl code from swig wrappers
  #  to an object of the same name.
  # 
  # @param name - VMUSB object name passed in by swig.
  #
  # @return string -name of an object usable by Tcl code. 
  #                 in general this will be $name.
  #
  proc ::VMUSBDriverSupport::convertVmUSBReadoutList name {
      
      cvmusbreadoutlist::CVMUSBReadoutList -this $name
      return $name

  }
} else {
  proc ::VMUSBDriverSupport::convertVmUSBReadoutList name {
    return $name;     # MVLCgen Does not require conversion.
}
##
# checkRange
#  Checks the range of a numeric value against optinoal limits.
#  note that this is normally considered an internal proc,
#  however you can call it from outside.
#
# @param value   - The value to check, any numeric type is ok.
# @param low     - If this is not "" this is the smallest value allowed.
# @param high     = If this is not "" this is the largest value allowed.
#
# @throw - Errro if value fails a range check.
#
# @note missing trailing parameters are allowed and treated as "".
#
proc ::VMUSBDriverSupport::checkRange {value {low ""} {high  ""}} {
  ::DriverSupport::checkRange $value $low $high
}

##
# validInt 
#  Validates an integer value with possible range checking.
#
# @param value - The value to check.
# @param low   - Optional parameter if non-empty the smallest integer value can take.
# @param high  - Optional parameter if non-empty the largest integer value can take.
#
# @return -nothing useful-
#
# @throw - Error if out of range.  If you don't catch this, eventually
#          the VM-USB framework catches it and aborts the run-start.
#
proc ::VMUSBDriverSupport::validInt {value {low ""} {high ""}} {
  ::DriverSupport::validInt $value $low $high
}
##
# validReal
#   Validates a real value with possible range checking.'
#
# @param value - The value to check.
# @param low   - The smallest number value can have (inclusive).
# @param high  - The largest number value can have (inclusive).
#
# @return -nothing useful-
#
# @throw - if value is not textually a valid real, or if a limit is nonempty and
#          the value does not make the limit.
#
proc ::VMUSBDriverSupport::validReal {value {low ""} {high ""}} {
  ::DriverSupport::validReal $value $low $high
}
##
# validEnum
#    Validates an enumerated value to ensure it is in an allowed set:
#
# @param value - The value to check
# @param set   - List of allowed values in the set.
#
# @return -nothing useful-
# 
# @throw - I value is not in set throws an error.
#
# 
proc ::VMUSBDriverSupport::validEnum {value set} {
  ::DriverSupport::validEnum $value $set
}

##
# validBool
#    Validates a boolean value
#
# @param value
#
# @return -nothing useful-
#
# @throw If value does not pass string is boolean -strict
#
proc ::VMUSBDriverSupport::validBool {value} {
  ::DriverSupport::validBool $value
}

##
# validList
#    Validates a list with an arbitrary checker
#
# @param value   - The list of values.
# @param fewest  - Fewest elements the list can have.
# @param most    - Largest no. of elements the list can have.
# @param checker - name of proc that checks the value of each list element
# @param args    - Remainder of the args are passed in to the checker.
#                  e.g checker is callded $checker $value {*}$args
# 
# @return -nothing useful-
#
# @throw If the list size does not fit the requirements of fewest/most.
# @throw Obviously if a list element fails.  These are caught munged a bit
#        to make the message clearer and then rethrown.
#
proc ::VMUSBDriverSupport::validList {value {fewest ""} {most ""} {checker ""} {args ""} } {
  ::DriverSupport::validList $value $fewest $most $checker $args
}
##
# validIntList:
#    Validates a list of integers.
#   
# @param value   - the list of values
# @param fewest  - If not empty the fewest number of list elements allowed.
# @param most    - If not empty the maximum number of list elements allowed.
# @param low     - If not empty the minimum value for all the list elements.
# @param high    - If not empty, the maximum value for all list elements.
#
# @return -nothing useful-
#
# @throw error if any of the constraints are not met.
#
proc ::VMUSBDriverSupport::validIntList {value {fewest ""} {most ""} {low ""} {high ""}} {
    ::DriverSupport::validList $value $fewest $most ::DriverSupport::validInt $low $high
}
##
# validBoolList 
#   validates a list of booleans:
#
# @param value   - the list of values
# @param fewest  - If not empty the fewest number of list elements allowed.
# @param most    - If not empty the maximum number of list elements allowed.
# @return -nothing useful-
#
# @throw error if any of the constraints are not met.
#
proc ::VMUSBDriverSupport::validBoolList {value {fewest ""} {most ""}} {
    ::DriverSupport::validList $value $fewest $most ::DriverSupport::validBool
}


## These conversions are not needed for mvlc:
if {[array names ::env MVLC_TRANSLATOR] eq ""} { 
  proc ::VMUSBDriverSupport::convertToReadoutList {atcllist} { 

    set len [llength $atcllist]
    set vecptr [cvmusbreadoutlist::vecuint32_create $len ]

    for {set i 0} {$i < $len} {incr i} {
      cvmusbreadoutlist::vecuint32_set $vecptr $i [lindex $atcllist $i] 
    }
    
    set rdolist [cvmusbreadoutlist::CVMUSBReadoutList rdolist \
                  [cvmusbreadoutlist::vecuint32_ptr2ref $vecptr]]
    return $rdolist

  }



  ##
  # Convert a std::vector<uint8_t> to a TCL list of 32-bit integers
  #
  # This makes use of the cvmusb::uint8_vector_get and cvmusb::uint8_vector_size
  # procs to form the command. The proc will only succeed if the number of elements
  # in the std::vector<uint8_t> is divisible by 4 (i.e. it converts cleanly to 
  # a uint32_t). 
  #
  # 
  #
  proc ::VMUSBDriverSupport::convertBytesListToTclList {data_} { 

    upvar $data_ data

    set pkg cvmusb
    
    set nbytes [${pkg}::uint8_vector_size $data]
    if {($nbytes%4) != 0} {
      error "VMUSBDriverSupport::convertBytesListToTclList size of bytes list must be divisible by 4"
    }
    
    set intList [list] 
    for {set byte 0} {$byte < $nbytes} {incr byte 4} {
        set byte0 [${pkg}::uint8_vector_get $data $byte]
        set byte1 [${pkg}::uint8_vector_get $data [expr $byte+1]]
        set byte2 [${pkg}::uint8_vector_get $data [expr $byte+2]]
        set byte3 [${pkg}::uint8_vector_get $data [expr $byte+3]]
        set int [expr ($byte3<<24)|($byte2<<16)|($byte1<<8)|$byte0]
      
        lappend intList $int
    }

    return $intList
  }

}