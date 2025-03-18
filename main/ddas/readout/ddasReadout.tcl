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
#            Ron Fox
#            Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

##
# @file ddasReadout.tcl
# @brief Driver for DDASReadout/ddasSort.
#

package require cmdline

#  Figure out how to add the NSCLDAQ TclLibs to the path:

if {[array names env DAQROOT] ne ""} {
    lappend auto_path [file join $env(DAQROOT) TclLibs]
    set bindir $::env(DAQBIN)
} else {
    
    #  Our next best guess is that its at ../TclLibs:
    
    set scriptdir [file dirname [info script]]
    set libdir [file normalize [file join $scriptdir .. TclLibs]]
    lappend auto_path $libdir
    set bindir [file normalize [file join $scriptdir .. bin]]
}
package require removetcllibpath
package require ssh
package require ringutils

##
# @brief Send a controlC to a file descriptor.
#
# @param fd File descriptor to send the character to.
#
proc controlC fd {
    puts -nonewline $fd "\003"
    flush $fd
}

#-------------------------------------------------------------------------------
#  Input handling procs.

##
# @brief Reacts to input on a subprogram's output.
# -  If there's an end file condition, that means the program exited so
#    we set ::forever which will exit our program.
# -  If not we read the line from the pipe and relay it to stdout.
#
# @param fd File descriptor open on readout's output pipe.
#
proc onInput {fd who} {
    if {[eof $fd]} {
        set ::forever 1
    } else {
        set line [gets $fd]
        puts $line
    }
}

##
# @brief Handles input on the stdin fd.
# - EOF means we're supposed to shutdown.
# - Data are relayed to the pipe passedin.
#
# @param infd    The fd on which input is awaiting.
# @param relayfd The fd to which input are relayed.
# @note The string 'exit' is treated specially. In that it sets the forever
# variable as we've been asked to exit.
#
proc relayToReadout {infd relayfd} {
    if {[eof $infd]} {
        set ::forever 1
    } else {
        set line [gets $infd]
        puts $relayfd $line
        flush $relayfd
        if {$line eq "exit" } {
            puts "Asked to exit!!!"
            set ::forever 1; # We've been asked to exit!
            
        }
    }
}
#------------------------------------------------------------------------------
#  Program entry point.

if {[array names env DAQBIN] ne ""} {
    set bindir $env(DAQBIN)
} else {
    set bindir [file dirname [info script]]
}

# Program usage string:

set usage "Driver for the split DDASReadout/ddasSort programs\nOptions:"
set options {
    {readouthost.arg "" "Host in which to run the DDASReadout program"}
    {readoutring.arg "" "Ring into which DDASReadout puts data"}
    {sorthost.arg    "" "Host in which the sorter runs"}    
    {sortring.arg    "" "Ringbuffer into which the sorter puts its data" }
    {cratedir.arg    "" "Directory in which to run DDASReadout"}
    {sourceid.arg    0  "Source ID with which to tag the data"}
    {fifothreshold.arg 20480 "FIFO Threshold value for DDASReadout"}
    {buffersize.arg    16384 "Buffer size value for DDASReadout"}
    {infinity.arg      "off" "Enable/disable infinity clock [on/off]"}
    {clockmultiplier.arg 1   "Timestamp multiplier for external clock"}
    {scalerseconds.arg   16  "Time between scaler reads"}
    {window.arg          10 "Sorting window in seconds."}
    {fastboot.arg "off" "Perform a settings-only boot [on/off]"}
    {port.arg        "" "Enable DDASReadout TclServer functionality on specified port"}
    {init-script.arg "" "DDASReadout initialization script run in the main interpreter"}
    {log.arg   "" "DDASReadout log file"}
    {debug.arg  0 "DDASReadout debug level [0-2]: higher numbers set more debugging output"}
}

set mandatory [list readouthost sortring sorthost cratedir]

proc usage {} {
    puts stderr [::cmdline::usage $::options $::usage]
}

#  Parse the command line arguments:

set parsed [cmdline::getoptions argv $options $usage]
set parsed [dict create {*}$parsed]

foreach option $mandatory {
    if {[dict get $parsed $option] eq ""} {
        puts stderr "-$option is required"
        usage
        exit -1
    }
}

#------------------------------------------------------------------------
# Construct the command line to run DDASReadout:

set ddasOptionMap {
    {port --port} {readoutring --ring} {sourceid --sourceid}
    {init-script --init-script} {log --log} {debug --debug}
}

set readoutDir    [dict get $parsed cratedir]
set readoutCmd    [file join $bindir DDASReadout]
set readoutHost   [dict get $parsed readouthost]
set fifoThreshold [dict get $parsed fifothreshold]
set bufferSize    [dict get $parsed buffersize]
set infinity      [dict get $parsed infinity]
set clkmult       [dict get $parsed clockmultiplier]
set scalerSecs    [dict get $parsed scalerseconds]
set rawRing       [dict get $parsed readoutring]
set fastboot      [dict get $parsed fastboot]


if {$infinity} {
    set infstring "INFINITY_CLOCK=1"
} else {
    set infstring ""
}

if {$fastboot} {
    set fbstring "DDAS_BOOT_WHEN_REQUESTED=1"
} else {
    set fbstring ""
}

set readoutCmd "$infstring $fbstring SCALER_SECONDS=$scalerSecs FIFO_THRESHOLD=$fifoThreshold EVENT_BUFFER_SIZE=$bufferSize $readoutCmd"

foreach optMapEntry $ddasOptionMap {
    set opt [lindex $optMapEntry 0]
    set mapping [lindex $optMapEntry 1]
    set optval [dict get $parsed $opt] 
    if {$optval ne ""} {
        append readoutCmd " $mapping=$optval"
    }
}

puts $readoutCmd
puts "to run in: $readoutHost"

#-------------------------------------------------------------------------
# Construct the command to run ddasSort:

set sortCmd [file join $bindir ddasSort]

# Figure out where we get data from:

set rdoRing $tcl_platform(user)
set suppliedRing [dict get $parsed readoutring]
if {$suppliedRing ne "" } {
    set rdoRing $suppliedRing
}
set rdoURI tcp://$readoutHost/$rdoRing



append sortCmd " --source=$rdoURI"
append sortCmd " --sink=[dict get $parsed sortring]"
append sortCmd " --window=[dict get $parsed window]"
set    sortHost [dict get $parsed sorthost]

puts $sortCmd
puts "To run in $sortHost"

# daqdev/NSCLDAQ#1019 Issue: ddasReadout script must create the raw ring.
# If not the sorter can come up and attempt to connect to the readout's ring
# before it gets made.
#  Issue #154 - fix typeo ofor readoutHost....

catch {ssh::ssh $readoutHost "$bindir/ringbuffer create $rdoRing"}


# Start the two programs on SSH Pipes:
# - We need the command input for readout.
# - We need the output/error for readout.
# - We need the output/error for the sorter.

set readoutfd [ssh::sshcomplex $readoutHost "(cd $readoutDir; $readoutCmd)" a+]
set readoutPid [pid $readoutfd]

# Wait for the readout ring to materialize:

if {![waitForRing $rdoRing 2 100 $readoutHost]} {
    error "Waiting for raw ring tcp://$readouHost/$rdoRing to exist timed out"
}

set sorterfd [ssh::sshcomplex $sortHost $sortCmd a+]
set sorterPid [pid $sorterfd]

fconfigure $sorterfd  -buffering line -blocking 0
fconfigure $readoutfd -buffering line -blocking 0
fconfigure stdin      -buffering line ; # Just in case.

# Set up file events for the various fds we care about:
# - stdin -- we'll relay command from that to readout's command input.
# - stdout/stderr for both programs gets relayed to our stderr.

fileevent $readoutfd readable  [list onInput $readoutfd DDASReadout]
fileevent $sorterfd  readable  [list onInput $sorterfd ddasSorter]
fileevent stdin      readable  [list relayToReadout   stdin $readoutfd]

puts "std fileevent: [fileevent stdin readable]"

# Then we vwait forever to run allow the software to process events.

vwait forever; # Will get set when readoutfd is closed.

puts "Waited."

# Don't really have to do this since the event loop won't run again but...

fileevent $readoutfd readable ""
fileevent $sorterfd  readable ""
fileevent stdin      readable ""

# Kill off the sorter -- we know the readout is dead because we got here
# when it's pipe closed:

puts "Killing $sorterPid"

catch {controlC $sorterfd}
catch {exec kill -9 $sorterPid}

catch {puts $readoutfd exit; flush $readoutfd}
after 1000;                                # Give time to exit then...
catch {controlC $readoutfd}                # ...control-C it.
catch {exec kill -9 $readoutPid};          # Just in case it was the sorter.

# These can have errors...

catch {close $eadoutfd}
catch {close $sorterfd}

exit 0


