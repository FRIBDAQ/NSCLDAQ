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
# @file makeNameFile.tcl
# @brief Create a name file from a scaler definition file.
# @author Ron Fox <fox@nscl.msu.edu>
#
# Stub packages
package provide Tk 8.0;             # Stub to make X11 unecessary.
package provide pagedisplay 1.0


#  Add TclLibs to the library search path - assuming we're in DAQBIN

set here [file dirname [info script]]
set libdir [file normalize $::env(DAQTCLLIBS)]; #[file normalize [file join $here .. TclLibs]]

lappend auto_path $libdir

#  This require pulls in the scaler program's configuration processing pkg.

package require removetcllibpath
package require scalerconfig

# Stubs for unused config bits.

proc page args {}
proc display_single args {}
proc display_ratio args {}
proc blank args {}
proc stripparam args {}
proc stripratio args {}
proc stripconfig args {}


##
# We need two parameters an input and output file:


if {[llength $argv] != 2} {
    puts stderr "Usage:"
    puts stderr "   makeNameFile scaler-def-file name-file"
    exit 1
}

set defFile  [lindex $argv 0]
set nameFile [lindex $argv 1]

source $defFile;           # Process the definition.

set fd [open $nameFile w]

foreach name [getScalerNames] {
    set ch [::scalerconfig::channelMap get $name]
    
    # The command name is channel_chnum.srcid - if there's a source id or
    #                     channel_chnum       - if not in wich case the
    #                                            srcid is assumed 0.
    
    set chspec [lindex [split $ch _] 1];     # Stuff after the _
    set specList [split $chspec .]
    if {[llength $specList] == 1} {
        lappend specList 0;                  # Default srcid to 0.
    }
    set wid [$ch cget -width]
    
    puts $fd "[lindex $specList 1] [lindex $specList 0] $wid $name"
}

close $fd
exit
