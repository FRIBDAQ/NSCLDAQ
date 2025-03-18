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
# @file Pixie16Server.tcl
# @brief Provides a server wrapping for the pixie16 package.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
# A bit about this package.
# This package assumes some other bit of code is listening for and validating
# connections.  Each time a connection is seen, an object of type
# Pixie16Server should be created. The object will interact with the
# client on the other end of the socket to perform operations and
# submit replies.  When the client closes its socket the server will
# use an after to schedule object destruction.  Assuming there's an event loop
# is no big assumption because it's pretty much necessary for all Tcl TCP programming.
#

package provide pixieserver 1.0
package require snit
package require pixie16

##
# @class Pixie16Server
#   Provides the server class. The server is completely textual.  It takes
#   commands, which will be described below and returns textual results.
#   The results consist of a status (for now 0 success, -1 for failure).
#   Some operations require a run not be in progress.  These will return
#   -1000 status if the module being accessed is in progress.
#   Finally -1001 is an invalid request of some sort (includes malformed).
#   Succesful returns may also have additional data.
#
# Requirements:
#   There must be a valid pixie16Cfg file present, as well as a pxisys.ini
#   in the server's cwd.
# Server requests: See each method for details.
#   Inventory - Returns a inventory of the modules in the crate.
#   Readchanpar - Reads a channel level parameter
#   Readmodpar  - Reads a module lelel parameter.
#   Writechanpar - Writes a channel level parameter.
#   Writemodpar  - Writes a module level parameter.
#   Saveparams   - Writes a .set file.
#   Loadparams   - Loads a .set file.
#   Boot         - Loads firmware into a module. The initial DSP Paramters come from the
#                  configuration file.
#   AdjustOffsets - Run an offset adjustment run in a selected module.
#
#  Options:
#    -init (bool)   - This is read only.  If true (at construction time),
#                     pixie16::init will be called
#                     from the constructor. Any module boots will have to be done
#                     by a client.
#   -socket (int)   - Socket connected to the server. This is mandatory.
#
snit::type Pixie16Server {
    option -init -readonly 1 -default 0
    option -socket -readonly 1 -default ""
    
    ##
    # constructor
    #   args - the options list.
    #
    constructor args {
        $self configurelist $args
        
        if {$options(-socket) eq ""} {
            error "Pixie16Server must be constructed with an -socket "
        }
        
        ##
        # Set up the socket:
        #   - buffering - line
        #   - translation auto.
        #   - input handler [mymethod _onInput]
        #
        
        chan configure $options(-socket) -blocking 1 -buffering line -translation auto
        chan event     $options(-socket) readable [mymethod _OnInput]
        
        #  If requested, access the system:
        
        if {$options(-init)} {
            pixie16::init
        }
    }
    ##
    # AdjustOffsets
    #   Perform an offset adjustment run in a module
    # @args the parameters in the request - should be one, the
    #        module index.
    #
    method AdjustOffsets {args} {
        if {![$self _ValidateParamCount $args 1 AdjustOffsets]} return
        set module [lindex $args 0]
        
        set status [catch {pixie16::adjustOffsets $module} msg] 
        if {$status} {
            puts $options(-socket) "-1 $msg"
        } else {
            puts $options(-socket) "0"
        }
        flush $options(-socket)
    }
        
    
    ##
    # Inventory
    #   Returns an inventory of the modules.  The result is multi-line. The first
    #   line is the status and, on success, the number of subsequent lines.
    #   Each subsequent line contains the following space separated fields:
    #   - Slot - (integer) number of the slot containing the module described.
    #   - Revision - (integer) the module revision number.
    #   - Serial - (integer) the module serial number.
    #   - bits - (integer) then number of bits of resolution.
    #   - mhz  - (integer) module sampling frequency in MHz.
    #
    #  Inventory is legal if the module is active.
    #  Inventory is not allowed to have any parameters.
    #
    # @param args - parameters to the command.
    # @note called from the _OnInput dispatcher.
    #
    method Inventory {args} {
        # Validate and report argument errors:

        if {![$self _ValidateParamCount $args 0 Inventory]} return
        
        set status [catch {pixie16::inventory} info]
        if {$status} {
            puts $options(-socket) "-1 $info"
        } else {
            
            puts $options(-socket) "0 [llength $info]"
            foreach module $info {
                set slot   [lindex $module 0]
                set rev    [lindex $module 1]
                set serial [lindex $module 2]
                set bits   [lindex $module 3]
                set mhz    [lindex $module 4]
                puts $options(-socket) "$slot $rev $serial $bits $mhz"
            }
        }
        
        flush $options(-socket)
    }
    ##
    # Readchanpar
    #   Reads a module level parameter.  This can be done when the
    #   module is active.  We need three parameters:
    #  -  Module index (not slot).
    #  -  Module channel.
    #  -  Parameter name (see Pixie16ReadSglChanPar for a list of valid names).
    #
    # @param args - the request tail.
    # @note we let pixie16::readchanpar do much of the parameter error checking.
    #       we only require three parameters.
    #
    method Readchanpar args {
        if {![$self _ValidateParamCount $args 3 Readchanpar]} return
        
        set module [lindex $args 0]
        set chan   [lindex $args 1]
        set name   [lindex $args 2]
        set status [catch {pixie16::readchanpar $module $chan $name} result]
        if {$status} {
            puts $options(-socket) "-1 result"
        } else {
            puts $options(-socket) "0 $result"
        }
        flush $options(-socket)
    }
    ##
    # Readmodpar
    #   Reads a module level parameter.  This requires two parameters:
    #    - Integer module index (not slot).
    #    - textual parameter name (see Pixie16ReadSglModParm for a list).
    #
    # @param args - list of parameters passed by the dispatcher.
    # @note we let pixie16::readmodpar do most of the validation.  We
    #     just validate the number of parameters.
    #
    method Readmodpar args {
        if {![$self _ValidateParamCount $args 2 Readmodpar]} return
        
        set module [lindex $args 0]
        set what   [lindex $args 1]
        set status [catch {pixie16::readmodpar $module $what} result]
        if {$status} {
            puts $options(-socket) "-1 $result"
            flush $options(-socket)
            return
        }
        puts $options(-socket) "0 $result"
        flush $options(-socket)
    }
    ##
    # Writechanpar
    #   Write a channel level paramete. THis requires the following parameters:
    #   - module index (integer)
    #   - channel number (integer)
    #   - What (text) parameter selector see Pixie16WriteSglChanPar for a list.
    #   - value (double) New parameter value.
    #
    #  This method requires that data taking be halted in the selected module.
    #  @param args -list of parameters passed in by the dispatcher.
    #  @note we leave much of the parameter validity checking to pixie16::writechanpar.
    #
    method Writechanpar args {
        if {![$self _ValidateParamCount $args 4 Writechanpar]} return
        
        set module [lindex $args 0]
        set chan   [lindex $args 1]
        set what   [lindex $args 2]
        set value  [lindex $args 3]
        
        if {[catch {pixie16::isActive $module} msg]} {;  # isActive could fail.
            puts $options(-socket) "-1 $msg"
            flush $options(-socket)
            return
        } elseif {$msg} {
            puts $options(-socket) "-1000 Module number $module is actively taking data"
            flush $options(-socket)
            return
        }
        
        # try the operation:
        
        set status [catch {pixie16::writechanpar $module $chan $what $value} msg]
        if {$status} {
            puts $options(-socket) "-1 $msg"
        } else { 
            puts $options(-socket) 0
            
        }
        flush $options(-socket)
        return
    }
    ##
    # Writemodpar
    #   Write a module level parameter.  This requires three parameters:
    #  - Module number (index not slot).
    #  - Parameter name (text) -see Pixie16WriteSglModPar for legal values.
    #  - value (int) new parameter value.
    #
    #   Note that the module must be inactive at this time.
    #
    # @param args - the parameter tail.
    # @note we let pixie16::writemodpar do the parameter checking for the most
    #      part.
    #
    method Writemodpar args {
        if {![$self _ValidateParamCount $args 3 Writemodpar]} return
        
        set mod   [lindex $args 0]
        set what  [lindex $args 1]
        set value [lindex $args 2]
        
        if {[catch {pixie16::isActive $mod} msg]} {
            puts $options(-socket) "-1 $msg"
            flush $options(-socket)
            return
        } elseif {$msg} {
            puts $options(-socket) "-1000 module number $mod is actively taking data"
            flush $options(-socket)
            return
        }
        set status [catch {pixie16::writemodpar $mod $what $value} msg]
        if {$status} {
            puts $options(-socket) "-1 $msg"
            
        } else {
            puts $options(-socket) 0
        }
        flush $options(-socket)
    }
    ##
    # Saveparams
    #   Writes a .set file of the current crate parameters.
    #   Data taking must be inactive as we've experienced corrupted .set files
    #   when writing them during active data taking.
    #   One additional request parameter is required, the name of the file to
    #   which data will be written.
    # @param args -the request arguments.
    # @note - the filename cannot have list separators "{} " in it as those can
    #         cause the argument parser to think there are more than one parameters.
    # @note - Filename validity processing is done by pixie16::save
    #
    method Saveparams args {
        if {![$self _ValidateParamCount $args 1 Saveparams]} return
        
        set status [catch pixie16::isActive msg]
        if {$status} {
            puts $options(-socket) "-1 $msg"
            flush $options(-socket)
            return
        }
        set orStatus [lindex $msg 0]
        if {$orStatus} {
            puts $options(-socket) "-1000 There is at least one active module: $msg"
            flush $options(-socket)
            return
        }
        if {[catch {pixie16::save $args} msg]} {
            puts $options(-socket) "-1 $msg"
        } else {
            puts $options(-socket) 0
        }
        flush $options(-socket)
    }
    ##
    # Loadparams
    #   Loads parameters from a .set file into the system.
    #   Data taking must be inactive to use this request.
    #   One additional parameter, the .set file, must be supplied.
    #
    # @param args - the filename.
    # @note most of the validity checking on the parameter is done by
    #     pixie16::restore and it's API call.
    #
    method Loadparams args {
        if {![$self _ValidateParamCount $args 1 LoadParams]} return
        
        if {[catch {pixie16::isActive} msg]} {
            puts $options(-socket) "-1000 At least one module is active $msg"
        } else {
	    set orstatus [lindex $msg 0]
	    if {$orstatus} {
		puts $options(-socket) "-1000 There is at least one active module: $msg"
	    } elseif  {[catch {pixie16::restore $args} msg] } {
                puts $options(-socket) "-1 $msg"
            } else {
                puts $options(-socket) 0
            }
        }
        flush $options(-socket)
    }
    ##
    # Boot
    #   Boots a module.  The initial DSP parameters are loaded from the
    #   .set file in the cfgPixie16.txt file.  Note that the active/inactive
    #   status is ignored as the boot will turn the module inactive. This is done
    #   to allow a severely hung system to boot modules to recover.
    #
    # @args - index of the module to boot.
    #
    method Boot args {
        if {![$self _ValidateParamCount $args 1 Boot]} return
        
        if {[catch {pixie16::boot $args} msg]} {
            puts $options(-socket) "-1 $msg"
        } else {
            puts $options(-socket) 0
        }
        flush $options(-socket)
    }
    
    
    #-------------------------------------------------------------------------
    # 'private' methods.
    #
    ##
    # _ValidateParamCount
    #    Given args and a required parameter count sends an error to the
    #    client if the parameter count does not match:
    #
    # @param param  - the args value.
    # @param count  - required parameter count.
    # @param request - Name of the request.
    # @return bool - true if the parameter count is ok else false.
    #
    method _ValidateParamCount {param count request} {
        if {[llength $param] != $count} {
            puts $options(-socket) "-1001 - $request requires $count parameters"
            flush $options(-socket)
            return false
        }
        return true
    }
    ##
    # _OnInput
    #   Called when the socket is readable.
    #  - On EOF close the socket and schedule destruction.
    #  - Read the request line, validate it against our methods and dispatch.
    #
    #
    method _OnInput {} {
        if {[eof $options(-socket)]} {
            close $options(-socket)
            after 1 $self destroy
        } else {
            set line [gets $options(-socket)]
            set words [split $line " "];   # Force to a valid list.
            set operation [lindex $words 0]
            set tail      [lrange $words 1 end]
            
            
            if {$operation ni "Inventory Readchanpar Readmodpar Writechanpar Writemodpar Saveparams Loadparams Boot AdjustOffsets"} {
                puts $options(-socket) "-1001 '$operation' is not a recognized request."
                flush $options(-socket)
            } else {
                $self $operation {*}$tail
            }
        }
        
    }
}

    

