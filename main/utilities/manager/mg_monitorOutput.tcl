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
# @file mg_monitorOutput.tcl
# @brief  Program that monitors program output via the manager.
# @author Ron Fox <fox@nscl.msu.edu>
#
if {[array names env DAQTCLLIBS] ne ""} {
    lappend auto_path $env(DAQTCLLIBS)
}

package require Tk
package require ManagerOutput
package require ManagerOutputController
package require dialogwrapper
package require OutputWindow
package require snit



##
# This program provides monitoring of program output that's sent by various
# programs to the manager.   It provides:
# *    Menubar with settings that can control the OutputWindow's:
#     rows, columns, history and columns.  The debug messages setting is visible
#     but has no effect since we don't send debug log entries has no effect.
# *   Connection settings.
# *   Output window (view used by the controller object).
# *   Status bar that shows the connection status.
#
# On connection abandonment, the program prompts the user to try again
# or exit.
#

#-----------------------------------------------------------------------------
#  Special purpose megawidgets

##
# @class ConnectionStatus
#   Provide a status bar that shows the connection state.
#
# OPTIONS
#   -   -status -  status text to provide.
snit::widgetadaptor ConnectionStatus {
    option -status [list]
    
    constructor args {
        installhull using ttk::frame 
        ttk::label $win.label -text "Connection Status:"
        ttk::label $win.status -textvariable [myvar options(-status)]
        grid $win.label $win.status -sticky w
        
        $self configurelist $args
    }
}
##
# @class ConnectionParameters
#    Provides a form that can prompt for connection parameters:
#
# OPTIONS
#    -retries - number of retries.
#    -interval - Seconds between retries.
#
snit::widgetadaptor ConnectionParameters {
    option -retries 1
    option -interval 1
    
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.rlabel -text "Retries before giving up"
        ttk::spinbox $win.retries -from 1 -to 1000 \
            -textvariable [myvar options(-retries)]
        ttk::label $win.ilabel -text "Time in secs between retries"
        ttk::spinbox $win.interval -from 1 -to 1000 \
            -textvariable [myvar options(-interval)]
        
        grid $win.rlabel $win.retries -sticky nsw -padx 5
        grid $win.ilabel $win.interval -sticky nsw -padx 5
        
        $self configurelist $args
    }
    
}
    




#------------------------------------------------------------------------------
#  Private utility procs.


##
# _startLogging
#    Start logging data received from the manager to file.
# @param model - name of the model that accepts data from the manager.
# @param menu  - menu widget.
# @param startIndex - our index in the menu.
# @param stopIndex  - Index of the stop menu entry.
#
proc _startLogging {model menu startIndex stopIndex} {
    # get the log file - if it's not empty then proceed:

    set filename [tk_getSaveFile  \
        -confirmoverwrite 0 -defaultextension .log \
        -title {File to append log to:} -filetypes [list \
            { {Log files}  .log }                        \
            { {Text files} .txt }                        \
            { {All files}   *   }                        \
        ]]
    if {$filename ne ""} {
        $model configure -logfile $filename
        $menu entryconfigure $startIndex -state disabled
        $menu entryconfigure $stopIndex -state normal
    }
}

##
# _stopLoggging
#   Disable logging by setting them model's log file to "" and 
#   ghosting ourselves while enabling the start entry:
#
# @param model - name of the model that accepts data from the manager.
# @param menu  - menu widget.
# @param startIndex - our index in the menu.
# @param stopIndex  - Index of the stop menu entry.
#
proc _stopLogging {model menu startIndex stopIndex} {
    $model configure -logfile ""
    $menu entryconfigure $startIndex -state normal
    $menu entryconfigure $stopIndex -state disabled
}

##
# _usage
#    Outputs and error message, the program usage and exits.
#    See the output message below for the actual program usage.
#
# @param msg - error message text
#
proc _usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "   \$DAQBIN\mg_monitorOutput host user ?service?"
    puts stderr "\nMonitors output from programs run by the DAQ manager\n"
    puts stderr "Where:"
    puts stderr "   host - the host the manager is running in."
    puts stderr "   user - The user that ran the manager"
    puts stderr "   service - the service the DAQ manager advertises as its REST service"
    puts stderr "             the manager's output monitor service is derived"
    puts stderr "             from that by this program."
    
    exit -1
}
#-----------------------------------------------------------------------------
# Menu processing

##
# _cfgOutputWindow
#    Pop up/process a dialog to configure the output window.
#
#  @param win - the output window.
#
proc _cfgOutputWindow {win} {
    toplevel .cfgout
    DialogWrapper .cfgout.dialog
    set parent    [.cfgout.dialog controlarea]
    set form [OutputWindowSettings $parent.form]
    
    # Configure the form to match the settings in the output window:
    
    $form configure   -history [$win cget -history] 
    set debug [expr {"debug" in [$win cget -showlog]}]
    $form configure -debug $debug
    
    .cfgout.dialog configure -form $form
    
    pack .cfgout.dialog
    
    # Let the dialog prompt and, if 'Ok' process the results.
    
    set result [.cfgout.dialog modal]
    if {$result eq "Ok"} {
        $win configure  -history [$form cget -history]
        set debug [$form cget -debug]
        set classes [$win cget -showlog]
        if {$debug} {
            if {"debug" ni $classes} {
                lappend classes debug [list -foreground blue]
            }
        } else {
            if {"debug" in $classes} {
                set index [lsearch -exact $classes debug]
                set classes [lreplace $classes $index $index+1]
            }
        }
        $win configure -showlog $classes
    }
    destroy .cfgout
    
}
##
# _cfgConnections
#   Configure the connection characteristics of the model.
#   These are:
#    - The connection interval.
#    - The number of retries before giving up.
#
# @param model - the model object command
#
proc _cfgConnections {model} {
    toplevel .cfgconn
    DialogWrapper .cfgconn.dialog
    set container [.cfgconn.dialog controlarea]
    
    set form [ConnectionParameters $container.form]
    $form configure \
        -retries [$model cget -connectionretries] \
        -interval [$model cget -connectioninterval]
    .cfgconn.dialog configure -form $form
    pack .cfgconn.dialog
    
    if {[.cfgconn.dialog modal] eq "Ok"} {
        $model configure -connectionretries [$form cget -retries] \
            -connectioninterval [$form cget -interval]
    }
    
    destroy .cfgconn
}

#----------------------------------------------------------------------------
# Event handling

##
# _onConnected
#   Update the connection status line to indicate we're connected.
#
proc _onConnected {model} {
    .status configure -status Connected
}
##
# _onDisconnected
#   Update the connection status line to indicate we're not connected.
#
proc _onDisconnected {model} {
    .status configure -status Disconnected
}
##
# _onRetryConnection
#     Called when the model gave up on reconnecting.  Let the user know
#     something is seriously wrong with the manager and give them a chance
#     to retry the connection dance or exit.
#
proc _onRetryConnection {model} {
    set result [tk_messageBox  \
        -title Retry? -type retrycancel -icon question \
        -message "Looks like the DAQ manager server is down, do you want to retry connecting to it?"
    ]   
    if {$result eq "retry"} {
        $model connect;                 # Initiate connection 
    } else {
        exit
    }

}
#-----------------------------------------------------------------------------
# Entry.

set nargs [llength $argv]

if {($nargs < 2) || ($nargs > 3)} {
    _usage "Incorrect number of command parameters."
}

# Extract the command line parameters.

set host [lindex $argv 0]
set user [lindex $argv 1]
set service ""
if {$nargs ==3} {
    set service [lindex $argv 2]
}

#  Create the user interface/view

OutputWindow .output
ConnectionStatus .status

grid .output  -sticky nsew
grid .status  -sticky nsew
grid rowconfigure . 0 -weight 1
grid rowconfigure . 1 -weight 0
grid columnconfigure . 0 -weight 1


menu .bar
. configure -menu .bar
menu .bar.settings -tearoff 0
.bar.settings add command -label "Output Settings..." \
    -command [list _cfgOutputWindow .output]
.bar.settings add command -label "Connection Settings..." \
    -command [list _cfgConnections model]
.bar add cascade -label "Settings" -menu .bar.settings

# The logging menu:

menu .bar.logging -tearoff 0
.bar.logging add command -label {Start Logging} \
    -command [list _startLogging model .bar.logging 0 1]
.bar.logging add command -label {Stop Logging} \
     -command [list _stopLogging model .bar.logging 0 1] -state disabled
.bar add cascade -label "Logging" -menu .bar.logging



# Create the model.

if {$service eq ""} {
    ManagerOutputModel model -host $host -user $user \
        -connected _onConnected -disconnected _onDisconnected \
        -connectionabandoned _onRetryConnection
} else {
    ManagerOutputModel model -host $host -user $user -service $service \
        -connected _onConnected -disconnected _onDisconnected \
        -connectionabandoned _onRetryConnection
}


# Bind the view and model together into a controller.

ManagerOutputController controller -model model -view .output
