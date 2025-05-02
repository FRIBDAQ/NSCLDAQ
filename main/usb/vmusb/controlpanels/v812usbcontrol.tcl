#!/bin/sh
# Start wish \
    exec tclsh ${0} ${@}

package require Tk

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321



# Implements a control panel for a single CAEN V812 CFD or 895
# Invoke as follows:
#
#   v812usbcontrol  filename ?host ?port??
#   v895usbcontrol  filename ?host ?port??
#
#  Where filename is the name of the configuration file the CFD has been
#  configured with. This file must also include
#   set base   baseaddress   
#
#   Where baseaddress is the address of the CFD in A24 space.
#
#  

#  There's a bit of dirt that we do here.
#  We use the V812 GUI widget but:
#  - Provide our  own CFD812 and CFDSTATE packages  that are essentially
#    no-ops
# - Hook into the -command switch to keep track of changes the gui makes.
# - Add a "Commit" button that makes the changes.
#
#  The only difference between the 895 and the 812 is that the 895, a leading
#  edge discriminator has no dead-time setting.
#  Issue #280 - Added parameters to allow host/port selection.


## locate the distro's TclLibs dir and prepend it to auto_path:

set here [file dirname [info script]]
set libdir [file join $here .. TclLibs]
set auto_path [concat $libdir $auto_path]
#puts $auto_path

package provide CFD812   2.0;             # Hokey.
package require usbcontrolclient
package require caenv812Gui;             # our widget.


set serverPort 27000;    # Where Readout is listening!
set serverHost localhost; # Host on which readout is listening.




#------------------------------ Dummy CFD812 package -------------------

namespace eval CFD812 {}

proc CFD812::Map {base name {crate 0}} {
    return dummy
}

proc CFD812::Unmap name {}

proc CFD812::SetThreshold {name channel value} {}

proc CFD812::SetWidth {name bank value} {}

proc CFD812::SetDeadtime {name bank value} {}

proc CFD812::SetMask {name value} {}
 
proc CFD812::SetMultiplicityThreshold {name value} {}

proc CFD812::GetSerialNumber {name} {return -Unknown-}

proc CFD812::GetBase {name} {return -Unknown-}

#------------------------------------------------------------------

#
#  If the user makes changes, mark the .commit button red and 
#  create/change a global variable that describes the change.
#  The globals help the commit button decide what to do.
#  as .commit will re-write that file as well.
#

proc changed {action id value} {
    global disableDeadtime

    if {($action ne "deadtime") || (!$disableDeadtime)} {
	set ::changed($action$id) $value
	.commit config -activebackground red -background red
    }


    
}



# Load a CFDState based on the information in the load file.
# Any info not provided is ignored.
#

proc loadState name {
    global disableDeadtime

    # Thresholds

    for {set c 0} {$c < 16} {incr c} {
	if {[array name ::thresholds $c] ne ""} {
	    .panel setProperties [list [list threshold$c $::thresholds($c)]]

	}
    }
    # Widths
    
    for {set w 0} {$w < 2} {incr w} {
	if {[array name ::widths $w] ne ""} {
	    .panel setProperties [list [list width$w $::widths($w)]]
	}
    }

    # Deadtimes

    if {!$disableDeadtime} {
	for {set d 0} {$d < 2} {incr d} {
	    if {[array name ::deadtimes $d] ne ""} {
		.panel setProperties [list [list deadtime$d $::deadtimes($d)]]
	    }
	}
    }

    # Majority level

    if {[info globals majority] ne ""} {
	.panel setProperties [list [list majority $::majority]]
    }

    # Mask

    if {[info globals enables] ne ""} {
	.panel setProperties [list [list enables $::enables]]
    }

 
}
#
#  Update the device from the changed array.
#  the indices of the changed array are almost right.. we just need to
#  map 'mask -> inhibits (which by the way are really enables >sigh<
# The majority level gets calculated to the register value by the server.
# We poke commands of the form:
#   Set $::name something somevalue
# Expect OK back and complain if we don't get it.
#
proc updateDevice sock {
    fconfigure $sock -buffering line

    foreach item [array names ::changed] {
	set value $::changed($item)
	
	# Need to negate thresholds.

	if {$value < 0} {
	    set value [expr -$value]
	}

	if {$item eq "mask"} {
	    set item inhibits
	}
	if {$item ne "deadtime"} {
	    puts $sock "Set $::name $item $value"
	    flush $sock
	    set result [gets $sock]
	    
	    if {[string range $result 0 4] eq "ERROR"} {
		tk_messageBox -icon error -title "Comm error" -type ok \
		    -message "Got an error from the device:\n $result \n Click OK to continue"
	    }
	}
    }
    
}


#  This commits the changes to the device and to the configuration file.
#  note that if we can't contact the tclserver on port we will update the file
#  in any event, from the complete CFDSTATE.  This allows us to use the
#  Gui to initialize the file for when the Readout actually starts.
#

proc commit {} {
    global disableDeadtime
    # Write the configuration file:

    set fd [open $::configFile w]

    puts $fd "# Config file saved by v812usbcontrol.tcl [clock format [clock seconds]]"

    puts $fd "set base $::base"
    puts $fd "set name $::name"

    for {set chan 0} {$chan < 16} {incr chan} {
	puts $fd "set thresholds($chan) [CFDState::GetThreshold CFD $chan]"
    }
    for {set s 0} {$s < 2} {incr s} {
	puts $fd "set widths($s) [CFDState::GetWidth CFD $s]"
	if {!$disableDeadtime} {
	    puts $fd "set deadtimes($s) [CFDState::GetDeadtime CFD $s]"
	}
    }

    puts $fd "set majority [CFDState::GetMultiplicity CFD]"
    puts $fd "set enables  [CFDState::getMask CFD]"


    close $fd

    # If possible update the device.

    if {[catch {socket $::serverHost $::serverPort} sock]} {
	tk_messageBox -icon info -title {No Readout} -type ok \
	    -message {Readout is not running, only changing the configuration file}
    } else {
	updateDevice $sock
	close $sock
    }


    # Unset the changed array.
   
    catch {unset ::changed}

    #  Re-set the color of the widgets.


    set color [. cget -background]
    .commit config -background $color -activebackground $color


}
##
# setConnectionInfo
#
#  Updates the connection information from the connection prompter.
#
# @param widget - Control prompter widget.
#
# @note - the widget is also destroyed.
#
proc setConnectionInfo widget {
    set ::serverHost [$widget cget -host]
    set ::serverPort [$widget cget -port]
    set parent [winfo parent $widget]
    bind $parent <Destroy> ""
    destroy $parent
    incr ::prompted
}
##
# cancelPrompt
#
# handles various forms of cancellation of the prompting widget.
#
# @param widget - promting widget.
#
proc cancelPrompt widget {
    set parent [winfo parent  $widget]
    bind $parent <Destroy> ""
    exit
}

##
#  usage
#    Output program usage to stderr.
#
proc usage {} {
    puts stderr "Usage:"
    puts stderr "   v812usbcontrol|v895usbcontrol config-file ?host ?port??"
    puts stderr "      v812usbcontrol - indicates you are controlling a CAEN V812 device"
    puts stderr "      v895usbcontrol - indicates you are controlling a CAEN V895 device"
    puts stderr "      config-file    -  is a path to the configuration filename."
    puts stderr "                       This file contains the initial conditions of the device."
    puts stderr "                       It is updated whenever you click 'commit'"
    puts stderr "     host            - If provided, specifies the host  VMUSBReadout runs in"
    puts stderr "     port            - If provided, specfies the --port value for VMUSBReadout"
    puts stderr " If at least host is provided, you will not be interactively prompted for"
    puts stderr " connection parameters.  If host is provided but not port, the port"
    puts stderr " defaults to 27000 which is the default port used by VMUSBReadout"
    puts stderr " If you are not promped for connection parameters a message will be output"
    puts stderr " indicating the host and port the program will connect to"
    
}
#-------------------------- Entry point: --------------------------


# Depending on the type of module (depending on the program run)
# disable or not, the deadtime.


set myProgram [file tail $argv0]
if {$myProgram eq "v895usbcontrol"} {
    set disableDeadtime 1
} else {
    set disableDeadtime 0
}

if {[llength $argv] < 1} {
    usage
    exit -1
}

set configFile [lindex $argv 0]

# See if the host was given and override the default if so:

set hostgiven 0
if {[llength $argv] > 1} {
    set serverHost [lindex $argv 1]
    set hostgiven 1
}
# Override the port if that was given:
# Ignore excess parameters.

if {[llength $argv] >=3} {
    set serverPort [lindex $argv 2]
}


source $configFile




wm title . $name

if {[info var base] eq ""} {
    error "Config file does not define the base address"
}
if {[info var name] eq ""} {
   error "Config File does not define the name"
}


# Allow the user to pick the server/port if at least one was not specified
# on the command line.
#  This is not displayed if at least the host was given
#  (Issue #280).

if {!$hostgiven} {
    wm withdraw .

    toplevel .prompter
    slowControlsPrompter .prompter.p -host $serverHost -port $serverPort \
        -type VMUSBReadout -okcmd [list setConnectionInfo %W] \
        -cancelcmd [list cancelPrompt %W]
    set prompted 0
    #pack .prompter.p
    grid .prompter.p -sticky nsew
    grid rowconfigure    .prompter.p 0 -weight 1
    grid columnconfigure .prompter.p 0 -weight 1

    bind .prompter <Destroy> [list cancelPrompt .prompter.p]
    vwait prompted

    wm deiconify .
} 

puts "Will connect to the control server $serverHost:$serverPort"

cfdv812Gui .panel   -base $base -name CFD




button      .commit  -text "Commit" -command commit
loadState CFD;                    # Load the state data from the file.
.panel configure  -command changed 

if  {$disableDeadtime} {
    .panel configure -disable deadtimes
}


pack .panel .commit
