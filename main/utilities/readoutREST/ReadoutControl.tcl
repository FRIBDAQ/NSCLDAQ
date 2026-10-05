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
# @file   ReadoutControl.tcl
# @brief  Provides a REST interface to control the READOUT program.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require restutils

##
#  This segment of the REST interface supports run control operations.
#  specifically INIT, BEGIN, SHUTDOWN and END (REST does not support PAUSE/RESUME)
#  in keeping with the manager restricted state machine.
#  The query parameter operation is required and its value is the command
#  being attempted
#  These must be POST operations.

Url_PrefixInstall /control ControlOperation

##
# _validTransition
#   Given the operation and current state, determines if the state transition
#   is legal.
#   - INIT - requires idle.
#   - BEGIN - requires idle.
#   - END   - Requires active.
#   - SHUTDOWN -is always legal.
#
# @param op   - requested operation.
# @param state - Curent state.
# @return bool - True if op is valid in this state.
proc _validTransition {op state} {
    if {$op eq "INIT"} {
        return [expr {$state eq "idle"}]
    } elseif {$op eq "PAUSE"} {
        return [expr {$state eq "active"}]
    } elseif {$op eq "RESUME"} {
        return [expr {$state eq "paused"}]
    } elseif {$op eq "BEGIN"} {
        return [expr {$state eq "idle"}]
    } elseif {$op eq "END"} {
        return [expr {$state eq "active" || $state eq "paused"}]
    } elseif {$op eq "SHUTDOWN"} {
        return 1;         # Always legal.
    } else {
        return 0;          # Illegal op.
    }
}

##
# _transition
#   Request the underlying Readout to perofrm the requested
#   state transition.
#
# @param op - the requested operation.
# @note we assume that errors in op value have already been caught (e.g. by
#       _validTransition above)
#
proc _transition {op} {
    if {$op eq "INIT"} {
        init
    } elseif {$op eq "BEGIN"} {
        begin
    } elseif {$op eq "END"} {
        end
    } elseif {$op eq "PAUSE"} {
        pause
    } elseif {$op eq "RESUME"} {
        resume
    } elseif {$op eq "SHUTDOWN"} {
        after 500 exit;        # Ensures we get our response out to the client. 
    }
}

##
# ControlOperation
#   Provides the entry point.
#
# @param socket - the socket object on which we reply.
# @param tail   - The remainder of the URL (post the base part.)
# The result is the usual JSON object with:
#
#    -  status  - OK for success.
#    -  message - Error message if status is not Success.
# Along with:
#    -  newstate - The new state of the system after the completion of the operation.
# @note - some systems may go through a transitory state (e.g. 'starting') before
#       reaching a stable state (e.g. 'active')  in this case it's possible
#       newstate will be this transitory state.
# @note - we assume that the Readouts we're plugged into support the following commands:
#    - begin
#    - init
#    - end
#    - runstate
proc ControlOperation {socket tail} {
    if {$tail eq ""} {
        if {[GetRequestType $socket] eq "POST"} {
            set info [GetPostedData $socket]
            if {![dict exists $info operation]} {
                ErrorReturn $socket "Control requires an 'operations' query parameter"
            } else {
                set op [dict get $info operation]
                set currentState [runstate]
                if {[_validTransition $op $currentState]} {
                    _transition $op
                    set newState [runstate]
                    Httpd_ReturnData $socket application/json [json::write object \
                        status [json::write string OK]                          \
                        message [json::write string ""]                         \
                        newstate [json::write string $newState]                 \
                    ]
                } else {
                    ErrorReturn $socket "It is not valid to '$op' when the state is '$currentState'"
                }
            }
        } else {
            ErrorReturn $socket "Control operations must use a POST method"
        }
    } else {
        ErrorReturn $socket "'$tail' is not an implemented subcommand"
    }
}
