#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

package require statemanager

##
# @file tcltest1.tcl
# @brief Test the state monitor tcl callbacks.
# @author Ron Fox <fox@nscl.msu.edu>


set reqURI [lindex $argv 0]
set subURI [lindex $argv 1]

statemanager::statemonitor start $reqURI $subURI


proc test {from to} {
    puts "Transition $from -> $to"
}


statemanager::statemonitor register NotReady test
statemanager::statemonitor register Ready    test
statemanager::statemonitor register Booting  test

vwait forever
