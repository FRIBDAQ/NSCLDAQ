#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321


package provide mcfd16rc 1.0

package require snit
package require Utils
package require usbcontrolclient

## A class that formulates RC commands for the MCFD-16 and initializes a
#  transactions through a communication proxy. The proxy is intended to abstract
#  away the differences between the RCbus implemented between different devices.
#  FOr example, an MRC-1 needs some special treatment in comparison to using an
#  MxDC device.  This implements the same interface as the MCFD16USB class so
#  that it can be used in place of it.
snit::type MCFD16RC {

  variable _proxy

  ## Construct the object
  #
  # @param proxy  a proxy object to communicate with a RCbus controller
  #
  constructor {proxy} {
    set _proxy $proxy
  }

  ## Set a threshold for a channel
  #
  # For consistency with the USB protocol, writing to the common channel
  # is accomplished by writing to channel 16. 
  #
  # @param ch   target channel (must be in range [0,16])
  # @param val  threshold value (must be in range [0,255])
  #
  # @returns response from device
  #
  # @throws error if value is not in range [0,255]
  # @throws error if ch is not in range [0,16]
  # @throws error if communication failed
  method SetThreshold {ch val} {
    if {![Utils::isInRange 0 255 $val]} {
      set msg "MCFD16RC::SetThreshold Invalid value provided. Must be in range"
      append msg " \[0,255\]."
      return -code error $msg
    }

    set adr [$type ComputeAddress threshold $ch]
    return [$_proxy Write $adr $val]
  }

  ## Read the threshold for a given channel
  #
  # As in the SetThreshold method, the common value is selected with ch=16.
  #
  # @param ch   target channel (must be in range [0,16])
  # 
  # @returns response from device
  #
  # @throws error if ch is not in range [0,16]
  # @throws error if communication failed
  method GetThreshold {ch} {
    set adr [$type ComputeAddress threshold $ch]
    return [$_proxy Read $adr]
  }

  ## @brief Write the polarity for a channel pair
  #
  # To set the common value, use ch=8. 
  #
  # @param ch     the target channel pair (must be in range [0,8])
  # @param negpos value to write (must be either "neg" or "pos")
  #
  # @returns response from device
  #
  # @throws error if value is neither "neg" or "pos"
  # @throws error if ch is not in range [0,8]
  # @throws error if communication failed
  method SetPolarity {ch negpos} {
    set val 0
    switch $negpos {
      pos { set val 0}
      neg { set val 1}
      default { 
        set msg {Invalid value provided. Must be "pos" or "neg".}
        return -code error -errorinfo MCFD16RC::SetPolarity $msg
      }
    }

    set adr [$type ComputeAddress polarity $ch]
    return [$_proxy Write $adr $val]
  }

  ## @brief Read the polarity for a channel pair
  #
  # @param the targ channel pair (must be in range [0,8])
  #
  # @returns string
  # @retval neg   - polarity is negative
  # @retval pos  - polarity is positive 
  #
  # @throws error if returned value is not 0 or 1
  # @throws error if ch is not in range [0,8]
  # @throws error if communication failed
  method GetPolarity {ch} {
    set adr [$type ComputeAddress polarity $ch]
    set val [$_proxy Read $adr]
    if {[catch {dict get {0 pos 1 neg} $val} res]} {
      set msg "MCFD16RC::GetPolarity Value returned by module not understood. "
      append msg "Expected 0 or 1 but received $val."
      return -code error $msg
    }
    return $res
  }

  ## @brief Write the gain value for a channel pair
  #
  # To set the common value, use chpair=8
  #
  # @param chpair target channel pair (must be in range [0,8])
  # @param val    value to write (must be in range [0,2])
  #
  # @returns response from the device
  #
  # @throws error if chpair is not in range [0,8]
  # @throws error if val is not in range [0,2]
  # @throws error if communication failed
  method SetGain {chpair val} {
    if {![Utils::isInRange 0 2 $val]} {
      set msg "MCFD16RC::SetGain Invalid value provided. Must be in range"
      append msg " \[0,2\]."
      return -code error $msg
    }

    set adr [$type ComputeAddress gain $chpair]
    return [$_proxy Write $adr $val]
  }

  ## @brief Read the gain for a channel pair
  #
  # To read the common value, use chpair=8
  #
  # @param chpair   target channel pair (must be in range [0,8])
  #
  # @returns value from device
  #
  # @throws error if chpair is not in range [0,8]
  # @throws error if communication failed
  method GetGain {chpair} {
    set adr [$type ComputeAddress gain $chpair]
    return [$_proxy Read $adr]
  }


  ## @brief Write the width
  #
  # To write the common value, use chpair=8
  #
  # @param chpair   target channel pair (must be in range [0,8])
  # @param val      value to write (must be in range [5,255])
  #
  # @returns value from device
  #
  # @throws error if value is not in range [5,255]
  # @throws error if chpair is not in range [0,8]
  # @throws error if communication failed
  method SetWidth {chpair val} {
    if {![Utils::isInRange 5 255 $val]} {
      set msg "MCFD16RC::SetWidth Invalid value provided. Must be in range"
      append msg " \[5,255\]."
      return -code error $msg
    }

    set adr [$type ComputeAddress width $chpair]
    return [$_proxy Write $adr $val]
  }


  ## @brief Read the width for a channel pair
  #
  # To read the common value, use chpair=8
  #
  # @param chpair   target channel pair (must be in range [0,8])
  #
  # @returns value from device
  #
  # @throws error if chpair is not in range [0,8]
  # @throws error if communication failed
  method GetWidth {chpair} {
    set adr [$type ComputeAddress width $chpair]
    return [$_proxy Read $adr]
  }

  ## @brief Write the dead time
  #
  # To write the common value, use chpair=8
  #
  # @param chpair   target channel pair (must be in range [0,8])
  # @param val      value to write (must be in range [5,255])
  #
  # @returns value from device
  #
  # @throws error if value is not in range [5,255]
  # @throws error if chpair is not in range [0,8]
  # @throws error if communication failed
  method SetDeadtime {chpair val} {
    if {![Utils::isInRange 5 255 $val]} {
      set msg "MCFD16RC::SetDeadtime Invalid value provided. Must be in range"
      append msg " \[5,255\]."
      return -code error $msg
    }

    set adr [$type ComputeAddress deadtime $chpair]
    return [$_proxy Write $adr $val]
  }

  ## @brief Read the dead time for a channel pair
  #
  # To read the common value, use chpair=8
  #
  # @param chpair   target channel pair (must be in range [0,8])
  #
  # @returns value from device
  #
  # @throws error if chpair is not in range [0,8]
  # @throws error if communication failed
  method GetDeadtime {chpair} {
    set adr [$type ComputeAddress deadtime $chpair]
    return [$_proxy Read $adr]
  }

  ## @brief Write the delay
  #
  # To write the common value, use chpair=8
  #
  # @param chpair   target channel pair (must be in range [0,8])
  # @param val      value to write (must be in range [0,4])
  #
  # @returns value from device
  #
  # @throws error if value is not in range [0,4]
  # @throws error if chpair is not in range [0,8]
  # @throws error if communication failed
  method SetDelay {chpair val} {
    if {![Utils::isInRange 0 4 $val]} {
      set msg "MCFD16RC::SetDelay Invalid value provided. Must be in range"
      append msg " \[0,4\]."
      return -code error $msg
    }

    set adr [$type ComputeAddress delay $chpair]
    return [$_proxy Write $adr $val]
  }

  ## @brief Read the delay for a channel pair
  #
  # To read the common value, use chpair=8
  #
  # @param chpair   target channel pair (must be in range [0,8])
  #
  # @returns value from device
  #
  # @throws error if chpair is not in range [0,8]
  # @throws error if communication failed
  method GetDelay {chpair} {
    set adr [$type ComputeAddress delay $chpair]
    return [$_proxy Read $adr]
  }

  ## @brief Write the constant fraction
  #
  # To write the common value, use chpair=8
  #
  # @param chpair   target channel pair (must be in range [0,8])
  # @param val      value to write (must be either 20 or 40)
  #
  # @returns value from device
  #
  # @throws error if value is not 20 or 40
  # @throws error if chpair is not in range [0,8]
  # @throws error if communication failed
  method SetFraction {chpair val} {

    switch $val {
      20 { set val 0 }
      40 { set val 1 }
      default {
        set msg "MCFD16RC::SetFraction Invalid value provided. Must be either "
        append msg "20 or 40."
        return -code error $msg
      }
    }

    set adr [$type ComputeAddress fraction $chpair]
    return [$_proxy Write $adr $val]

  }

  ## @brief Read the constant fraction value
  #
  # To read the common value, use chpair=8
  #
  # @param chpair   target channel pair (must be in range [0,8])
  #
  # @returns value from device
  #
  # @throws error if response from device is not understood
  # @throws error if chpair is not in range [0,8]
  # @throws error if communication failed
  method GetFraction {chpair} {
    set adr [$type ComputeAddress fraction $chpair]
    set val [$_proxy Read $adr]

    # if possible, map 0 --> 20 and 1 --> 40
    if {[catch {dict get {0 20 1 40} $val} res]} {
      set msg "MCFD16RC::GetFraction Value returned by module not understood. "
      append msg "Expected 0 or 1 but received $val."
      return -code error $msg
    }
    return $res
  }

  ## @brief Transition the device between common and individual mode
  #
  # @param mode   desired config (must be either "common" or "individual")
  #
  # @returns response from the device
  #
  # @throws error if mode is neither common or individual
  # @throws error if there is a communication failure
  method SetMode {mode} {
    set val 0
    switch $mode {
      common { set val 0 }
      individual { set val 1 }
      default {
        set msg "MCFD16RC::SetMode Invalid value provided. Must be either "
        append msg {"common" or "individual".}
        return -code error $msg
      }
    }

    set adr [dict get $offsetsMap mode]
    return [$_proxy Write $adr $val]
  }

  ## @brief Retrieve the device mode
  #
  # @returns string
  # @retval common      - device is in common mode
  # @retval individual  - device in individual mode 
  #
  # @throws error if communication failed
  # @throws error if response from device is not understood
  method GetMode {} {
    set adr [dict get $offsetsMap mode]
    set val [$_proxy Read $adr]

    if {[catch {dict get {0 common 1 individual} $val} res]} {
      set msg "MCFD16RC::GetMode Value returned by module not understood. "
      append msg "Expected 0 or 1 but received $val."
      return -code error $msg
    }
    return $res
  }

  ## @brief Start a pulser
  #
  # There are two pulser built into the MCFD16. The argument provided specifies
  # which one and the mapping is as follows:
  #   1 - 2.5  MHz
  #   2 - 1.22 kHz 
  #
  # @param chpair   target channel pair (must be in range [0,8])
  # @param pulser   index of pulser (must be 1 or 2)
  #
  # @returns value from device
  #
  # @throws error if value is neither 1 nor 2
  # @throws error if communication failed
  method EnablePulser {pulser} {
    if {$pulser ni [list 1 2]} {
      set msg {MCFD16RC::EnablePulser Invalid value provided. Must be either}
      append msg { 1 or 2.}
      return -code error $msg
    }

    set adr [dict get $offsetsMap pulser]
    return [$_proxy Write $adr $pulser]
  }

  ## @brief Stop either of the pulsers
  #
  # @throws error if communication failure
  method DisablePulser {} {
    set adr [dict get $offsetsMap pulser]
    return [$_proxy Write $adr 0]
  }

  ## @brief Check to see if the pulser is enabled
  #
  # @returns response from device (content of addr 118) 
  #
  # @throws error if communication failure
  method PulserEnabled {} {
    set adr [dict get $offsetsMap pulser]
    return [$_proxy Read $adr]
  }

  ## @brief Write the channel mask for the device
  #
  # @param mask   bit mask to disable channel pairs (must be in range [0,255])
  #
  # @returns response from device
  #
  # @throws error if mask is not in range [0,255]
  # @throws error communication failure
  method SetChannelMask {mask} {
    if {![Utils::isInRange 0 255 $mask]} {
      set msg {MCFD16RC::SetChannelMask Invalid mask value. Must be in range }
      append msg {[0,255].}
      return -code error $msg
    }
    set adr [dict get $offsetsMap mask]
    return [$_proxy Write $adr $mask]
  }

  ## @brief Read the channel mask
  #
  # @returns the integer value of the channel mask
  #
  # @throws error if communication failure
  method GetChannelMask {} {
    set adr [dict get $offsetsMap mask]
    return [$_proxy Read $adr]
  }

  ## @brief Set the remote control mode
  #
  # @param on   desired rc state (must be boolean)
  #
  # @returns response from the device
  #
  # @throws error if on is not boolean
  # @throws error if communication failure
  method EnableRC {on} {
    if {![string is boolean $on]} {
      set msg {MCFD16RC::EnableRC Invalid value provided. Must be a boolean.}
      return -code error $msg
    }

    set on [string is true $on]
    set adr [dict get $offsetsMap rc]
    return [$_proxy Write $adr $on]
  }

  ## @brief Read the state of remote control mode
  #
  # @returns integer
  # @retval 0 - off
  # @retval 1 - on
  #
  # @throws error if communication failure
  method RCEnabled {} {
    set adr [dict get $offsetsMap rc]
    return [$_proxy Read $adr]
  }

  #############################################################################
  #
  # TYPE METHODS

  
  typevariable offsetsMap ;# maps parameter name to corresponding address
  typevariable paramRangeMap ;#  dict of ranges for each param

  ## Type constructor called when the snit::type becomes callable.
  typeconstructor {
    # the offsets map stores the offset for the 
    set offsetsMap [dict create threshold {indiv  0 common 64} \
                                gain      {indiv 16 common 65} \
                                width     {indiv 24 common 66} \
                                deadtime  {indiv 32 common 67} \
                                delay     {indiv 40 common 68} \
                                fraction  {indiv 48 common 69} \
                                polarity  {indiv 56 common 70} \
                                mode      72 \
                                rc        73 \
                                mask      83 \
                                pulser    118 ]

    set paramRangeMap [dict create threshold {low 0 high 16} \
                                   gain      {low 0 high  8}\
                                   width     {low 0 high  8}\
                                   deadtime  {low 0 high  8}\
                                   delay     {low 0 high  8}\
                                   fraction  {low 0 high  8}\
                                   polarity  {low 0 high  8}]

  }

  ## @brief Compute address of a channel for a parameter
  #
  # @param param  name of parameter 
  # @param ch     channel associated with that param
  #
  # @returns address of channel
  #
  # @throws error if param not found
  # @throws error if ch out of range for param
  typemethod ComputeAddress {param ch} {
    # initialize a value to return
    set addr 0

    # get parameter info
    set range [dict get $paramRangeMap $param]
    set low [dict get $range low]
    set high [dict get $range high]

    set offsets [dict get $offsetsMap $param]
    if {$ch == $high} {
      # at max channel value --> common setting
      set addr [dict get $offsets common]
    } elseif {($ch>=$low) && ($ch<$high)} {
      # individual chan
      set offset [dict get $offsets indiv]
      set addr [expr $offset+$ch]
    } else {
      # bad channel
      set msg "Invalid channel provided. Must be in range \[$low,$high\]."
      return -code error $msg
    }

    return $addr
  }
}


###############################################################################
###############################################################################
###############################################################################
###############################################################################
#

##  @brief A snit::type for talking to the MxRDCBus slow-controls plugin
#
# A class to control formatting of Read and Write operations into standard Set
# and Get operations known to the controlClient. It formats the device number
# and parameter address for a into an encoded string and then calls Set or Get
# methods in a controlClient. It maintains the device number as an option and is
# thus expected to be tied to a single MxDC device serving as the RCbus proxy.
# For this class to be effective, the VMUSBReadout slow controls server must be
# up and running.
snit::type MXDCRCProxy {

  option -module  ;# name of the slow control module to talk to
  option -devno   ;# device address on RCbus

  component _comObject ;# controlClient to communicate with the slow controls
  delegate option * to _comObject

  ## Constructor 
  #
  # Create the controlClient and configure. done.
  #
  constructor {args} {
    install _comObject using controlClient %AUTO% 
    $self configurelist $args
  }

  ## Destructor 
  #
  # _comObject should be killed off implicitly because it is an installed 
  # component
  #
  destructor {
  }

  ## A convenience method that makes Write and Read operations look the same
  #  Not really used...
  #
  method Transaction {paramAddr {val {}}} {

    set result ""
    if {$val eq {}} {
      set result [$self Read $paramAddr] 
    } else {
      set result [$self Write $paramAddr $val]
    } 

    return $result
  }

  ## @brief Main method to set up a Set operation.
  #
  # An encoded string of the device number and the parameter address is
  # generated and then passed to the communication object's Set method.
  #
  # @param paramAddr    parameter address for desired param
  # @param val          value to write 
  #
  # @returns response from the device
  #
  # @throws error if Set command response starts with ERROR
  #
  method Write {paramAddr val} {
    # encode the parameter address and device number
    set param [$self _formatParameter [$self cget -devno] $paramAddr]
    set result [$_comObject Set [$self cget -module] $param $val] 

    # check whether the response contains ERROR
    if {[$self _transactionFailed $result]} {

      set errmsg [$self _transformToFailureMessage [list Set $param \
                                                             $val \
                                                             $result]]
      return -code error $errmsg
    }

    return $result
  }

  ## @brief Main method to set up a Get operation.
  #
  # An encoded string of the device number and the parameter address is
  # generated and then passed to the communication object's Get method.
  #
  # @param paramAddr    parameter address for desired param
  #
  # @returns response from the device
  #
  # @throws error if Set command response starts with ERROR
  #
  method Read {paramAddr} {
    # encode the parameter and device address
    set param [$self _formatParameter [$self cget -devno] $paramAddr]
    set result [$_comObject Get [$self cget -module] $param]

    # check for failure responses
    if {[$self _transactionFailed $result]} {
      set errmsg [$self _transformToFailureMessage [list Get $param \
                                                             $result]]
      return -code error $errmsg
    }

    return $result
  }

  ## Retrieve the communication object
  #
  # In general this would return a controlClient instance, but doesn't have to.
  # 
  # @returns current communication object instance
  method getComObject {} { return $_comObject }

  ## Encode the device number and parameter into a string
  #
  # @param devNo      device number
  # @param paramAddr  parameter address
  #
  # @returns encoded string
  method _formatParameter {devNo paramAddr} {
    return "d${devNo}a${paramAddr}"
  }

  ## Check for response beginning with "ERROR"
  #
  # @param response the string to parse
  #
  # @returns boolean
  # @retval 0 - string did not begin with ERROR
  # @retval 1 - string begins with ERROR
  method _transactionFailed {response} {
    set result 0
    if {$response ne {}} {
      set result [string equal [lindex $response 0] "ERROR"]
    }
    return $result
  }

  ## Construct an informative message from a failure response
  #
  # This takes a normal string of words separated by spaces, and parses it
  # to form a new message string. This assumes that the beginning of the string
  # "ERROR - " and will produce some peculiar results if it does not.
  #
  # @param arglist    the response string
  #
  # @returns informative message of failure
  #
  method _transformToFailureMessage {arglist} {
    set errMessage [lindex $arglist end]
    set tokens [split $errMessage " "]  ;# split into tokens
    set tokens [lreplace $tokens 0 1] ;# remove ERROR and - 
    
    # start forming the message to return
    set message [lrange $arglist 0 [expr [llength $arglist]-2]] ; # use cmd descriptors
    append message " failed with message : \"" ; # insert some text
    append message [join $tokens " "] ;# add back in the orig message without
                                       # "ERROR - "
    append message "\"" ; # add a nice closing parenthese (sp?)

    return $message
  }
} ;# end of MXDCRCProxy snit::type

