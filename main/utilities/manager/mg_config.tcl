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
# @file mg_config.tcl
# @brief  Master experiment configuration editor.
# @author Ron Fox <fox@nscl.msu.edu>
#
package require Tk
set daqbin [file normalize [file dirname [info script]]];  # Where programs live.
##
#  Really  this is just a script that fires off the other editors.
#

##
# Below is a dict whose keys are the menu entries on the dispatcher menu
# and whose values are the program to launch.
# Note we use a dict rather than an array because dict keys preserve definition
# order and we want control over the order of 'menu' entries.
#
set ApplicationMenu [dict create                                 \
    "Container wizrd"   [file join $daqbin mg_container_wizard] \
    "Containers"        [file join $daqbin mg_cfgcontainers]    \
    "Readout Wizard"    [file join $daqbin mg_readout_wizard]   \
    "Program Wizard"    [file  join $daqbin mg_program_wizard]  \
    "Programs"          [file join $daqbin mg_cfgprogram]       \
    "Event log wizard"   [file join $daqbin mg_logwizard]        \
    "Event logging"     [file join $daqbin mg_cfgEvlog]        \
    "Sequence Definition" [file join $daqbin mg_seqedit]        \
    "State Machine"     [file join $daqbin mg_stateedit]        \
    "Users and Roles"   [file join $daqbin mg_authedit]         \
    "Key Value store"   [file join $daqbin mg_kvedit]           \
]

set ConfigurationCreator [file join $daqbin mg_mkconfig]
#------------------------------------------------------------------------------
# Utility procs
##
#  We need to establish the configuration database.  If there are no
#  parameters, the user has the option of creating one (via mg_mkconfig).
#  Otherwise it must be on the command line.
#

proc _Usage {msg} {

    puts stderr $msg
    puts stderr "\nUsage:"
    puts stderr "    mg_config ?configuration-file?"
    puts stderr "\nWhere:"
    puts stderr "    configuration-file is the configuration database file.  If"
    puts stderr "    not supplied, the user is prompted to create one (if they"
    puts stderr "    wish to)."
    exit -1
}
##
# _createDb
#   Give the user the option to create a new database file.
#
# @return database filename if created, empty if not.
#
proc _createDb { } {
    set answer [tk_messageBox -parent . -type yesno -title "New DB" \
        -icon question                                              \
        -message {Create a new configuration database?  If not supply an existing database file on the commandline}    
    ]
    if {$answer eq "no"} {
        return ""
    }
    set dbFilename [tk_getSaveFile -defaultextension .db -parent .      \
        -title {Create database file}                               \
        -filetypes [list                                            \
            [list {Database file} .db]                              \
            [list {All files}     *  ]                              \
        ]                                                           \
    ]
    if {$dbFilename eq ""} {
        return ""
    }
    exec $::ConfigurationCreator $dbFilename
    return $dbFilename
}
##
# _Launch
#   Launch one of the specific editor programs.
#
# @param w  - widget containing the program lists.
# @param f  - filename to edit.
#
proc _Launch {w f} {
    # Figure out the name of the program that's selected:
    
    set idx [$w curselection]
    if {$idx ne ""} {
        set key [$w get $idx]
        set program [dict get $::ApplicationMenu $key]
        catch {exec $program $f}
    }
    
}
#------------------------------------------------------------------------------
# Entry point


    
set dbFile "";                             # Initially no database file.

if {[llength $argv] == 0} {
    set dbFile [_createDb]
    if {$dbFile eq ""} {
        exit 0;                            # User chose not to.
    }
} elseif {[llength $argv] != 1} {
    _Usage {Incorrect number of command line parameters}
    
} else {
    set dbFile [lindex $argv 0]
}

listbox .l -selectmode single
dict for {name value} $ApplicationMenu {
    .l insert end $name
}
ttk::button .exit -text Exit -command [list exit 0]
pack .l .exit -fill both -expand 1

bind .l <Double-1> [list _Launch .l $dbFile]


    
    



