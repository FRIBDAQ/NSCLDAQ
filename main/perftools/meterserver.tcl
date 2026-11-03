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
# @file meterserver.tcl
# @brief Service that clients can write meter info to.
# @author Ron Fox <fox@nscl.msu.edu>
#

package require Tk
source meter.tcl

array set meters [list];     # Meter widgets indexed by name.
set meterIndex 0;            # uniquifying meter widget index


##
# create a new meter.
#
proc newMeter {tail} {
    set name [lindex $tail 0]
    set from [lindex $tail 1]
    set to   [lindex $tail 2]
    set scaleType [lindex $tail 3]
    set i $::meterIndex
    incr ::meterIndex
    
    
    if {$scaleType eq "log"} {
        set logval 1
    } elseif {$scaleType eq "linear"} {
        set logval 0
        set range [expr {$to - $from}]
        set majorInterval [expr {$range/10}]
    } else {
        error "Invalid scaletype: $scaleType"
    }
    if {[array names ::meters $name] ne ""} {
        
        # Already exists.
        set w $::meters($name)
        $w configure -from $from -to $to -log $logval
        return
    }

    set w [controlwidget::meter .m$i  \
        -from $from -to $to -log $logval -height 500]
    if {!$logval} {
        $w configure -majorticks $majorInterval -minorticks 5
    }
    set l [label .l$i -text $name]
    
    grid $w -row 0 -column $i
    grid $l -row 1 -column $i
    set ::meters($name) $w
    
}
proc setMeterValue {tail} {
    set name [lindex $tail 0]
    set value [lindex $tail 1]
    
    set w $::meters($name)
    $w set $value
}

##
# lines from the client can be one of the following:
#    meter name from to log|linear - asks for a new meter.
#    value name floating-point-value - Sets the value of meter 'name'
#
#
proc processLine {line} {
    set command [lindex $line 0]
    if {$command eq ""} return
    
    if {$command eq "meter" } {
        newMeter [lrange $line 1 end]
    } elseif {$command eq "value"} {
        setMeterValue [lrange $line 1 end]
    } else {
        puts stderr "Illegal request '$line'"
    }
}

##
#  Accept a line from the client.
#
proc onInput ch {
    if {[eof $ch] } {
        close $ch
        return
    }
    
    set line [gets $ch]
    processLine $line
}
##
#  accept a new connection.
#
proc onConnect {ch clientIP clientPort} {
    
    puts "Connected to client $clientIP $clientPort"
    fileevent $ch readable [list onInput $ch]
    fconfigure $ch -buffering line
    
}



###
# Entry point
#   Usage:
#     meterserver.tcl listen-port
#

if {[llength $argv] != 1} {
    puts stderr "Usage:"
    puts stderr "  mmeterserver listen-port"
    puts stderr "     listen-port -the port number on which meterserver accepts connections."
    exit -1
    
}

set port [lindex $argv 0]
socket -server onConnect $port
