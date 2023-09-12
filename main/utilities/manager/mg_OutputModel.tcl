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
# @file mg_OutputModel.tcl
# @brief Provides a model - like access to output from the
#        manager.

package provide ManagerOutput 1.0
package require snit
package require portAllocator
package require clientutils


##
# @class ManagerOutputModel
#   Provides a model like object that makes output from the
#   manager availbable.
#   Key features:
#     - Provides a callback to handle input that separates the program name
#       from the actual text.
#     - Attempts to re-try connections to the manager if they fail.
#     - Provides a call back when re-connect retries are exceeded.
#     - Provides callbacks when connection state changes.
#     - Is port agile in the sense that on each connection attempt it will
#       use the port manager to resolve the service name of the manager.
# @note  This class relies heavily on the event loop and therefore must either be
#      used in a Tk application or in an application that expclicitly invokes
#      the event loop in a timely manner (e.g. with vwait).
# OPTIONS
#  -   -host   - host the manager is running in.
#  -   -service - Service of the manager's output relay port.  Defaults to:
#               DAQManager-outputMonitor
#  -   -user   - user running the service.
#  -   -output  - Script to handle output from the manager.
#  -   -connected - Script called when a connection to the manager was established.
#  -   -disconnected - Script called when a connection to the manager was lost.
#  -   -connectionretries - Number of times the connection attempt is retried.
#  -   -connectioninterval - Seconds between connection retries.
#  -   -connectionabandoned - Called when connection retries have been exhausted.
#
# CALLBACK PARAMETERS
#    -output - gets the instance command and the output.
#    all others get the instance command only.
# @note that -host and -service are required when the object is made and
#       cannot be modified.
# METHODS:
#    connect   - Can be called when connection attempts have been abandoned
#                to start another round of connection attempts.
#
snit::type ManagerOutputModel {
    option -host -readonly 1 
    option -service -default DAQManager-outputMonitor -readonly 1 
    option -user    -default $tcl_platform(user) -readonly 1 
    option -output -default [list]
    option -connected -default [list]
    option -disconnected -default [list]
    option -connectionretries -default 100
    option -connectioninterval -default 1
    option -connectionabandoned -default [list]
    option -logfile -default ""
    
    variable socket [list]
    variable afterid -1
    variable reconnectCount 0
    
    constructor {args} {
        $self configurelist $args
        if {$options(-host) eq ""} {
            error "Construction must supply a -host option"
        }
        if {$options(-service) eq ""} {
            error "Construction must not supply an empty -service string"
        }
        if {$options(-user) eq "" } {
            error "Construction must nut supply an empty -user string"
        }
        
        $self connect;       # Start trying to connect
    }
    destructor {
        if {$socket ne ""} {
            catch {close $socket}
        }
        if {$afterid ne -1} {
            after cancel $afterid
        }
    }
    
    #------------------------------------------------------------------------
    # Private utility methods.
    
    ##
    # _dispatch
    #   Dispatches to a callback.
    # @parameter optname - name of the option to callback to.
    # @parameter args   - additional args to pass to the callback.
    #                     These are specific to the callback.
    #
    method _dispatch {optname args} {
        set script $options($optname)
        if {$script ne ""} {
            foreach arg $args {
                lappend script $arg
            }
            uplevel #0 $script
        }
    }
    ##
    # _onInput
    #   Called when there's input available on the socket.  Note that
    #   this can also happen if the peer closed the socket.  In that case
    #   we should have an EOF indication and will attempt to connect.
    #
    method _onInput {} {
        if {[eof $socket]} {
            close $socket
            set socket ""
            $self _dispatch -disconnected $self
            $self connect
        } else {
            set input [gets $socket]
            $self _dispatch -output $self $input

            # If we have a non empty log file then
            # Append the input to that file:
            #
            if {$options(-logfile) ne ""} {
                set fd [open $options(-logfile) a]
                set ts [clock format [clock seconds] -format {%D %T}]
                puts $fd "$ts: $input"
                close $fd
            }
        }
    }
    
    ##
    # _retryOrFail
    #   Called when a connection attempt fails.
    #   - increment the reconnect count.
    #   - If it's exceeded -connectionretries invoke -connectionabandoned
    #   - If not then reschedule _attemptConnection
    #
    method _retryOrFail {} {
        incr reconnectCount
        if {$reconnectCount < $options(-connectionretries)} {
            after [expr 1000*$options(-connectioninterval)] [mymethod _attemptConnection]
        } else {
            $self _dispatch -connectionabandoned $self
            set afterId -1
        }
    }
    ##
    # _attemptConnection
    #   - Use the port allocator to locate the server.
    #   - If we can locate it, attempt to connect to it.
    #   - If any of this fails, invoke _retryOrFail which will figure out
    #     what to do.
    #
    #  On a successful connection, we invoke the -connected callback passing ourself
    #  as a parameter.
    #
    method _attemptConnection {} {
        set afterid -1;              # There's no after scheduled.
        
        set status [catch {
            ::clientutils::getServerPort $options(-host) $options(-user) \
                $options(-service)
        } port]
        if {$status} {
            # Couldn't even find the service.
            
            $self _retryOrFail
            return
        }
        # Now attempt to connect to the service:
        
        set status [catch {
            socket $options(-host) $port
        } s]
        if {$status} {
            # Could not connect
            
            $self _retryOrFail
            return
        }
        set socket $s
        fconfigure $s -buffering line
        fileevent $s readable [mymethod _onInput]
        $self _dispatch -connected $self;     # Let the client know we connected.
    }
    
    #-----------------------------------------------------------------------
    # Public methods.
    
    ##
    # connect
    #   Starts to attempt to connect to the server.
    #   - If a connection attempt is in progress (retries nonzero and afterid
    #     not -1), The connection attempts are re-started.
    #   - If a connection exist it is closed an a reconnect done.
    #
    method connect {} {
        # Deal with existing connection and existing attempts to connect.
        if {$socket ne "" } {
            catch {close $socket}
        }
        if {$afterid != -1} {
            after cancel $afterid
        }
        set  reconnectCount 0
        
        $self _attemptConnection
    }
    
}
    


    

    
