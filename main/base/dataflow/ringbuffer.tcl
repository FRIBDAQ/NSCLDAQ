#!/bin/sh
# start tclsh: \
exec tclsh ${0} ${@}

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
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

#--------------------------------------------------------------------------
#
#   This file contains a script that implements the ringbuffer
#   command.  The ringbuffer command is a utility that provides
#   shell access to ring buffer management.
#   The following syntaxes are supported:
#    ringbuffer create ?--datasize=n? ?--maxconsumers=n?   name
#    ringbuffer format ?--maxconsumers=n?                  name
#    ringbuffer delete                                     name
#    ringbuffer status ?--host=hostname?                  ?pattern?
#    ringbuffer list   ?--host=hostname?
#
#  --datasize  - sets the size of the data region of a ring.
#                This can be expressed as bytes (e.g. 100000),
#                multiples of 1024 (e.g. 1000k) and multiples
#                of 1024*1024 (e.g. 100m).
#  --maxconsumers - sets the maximum number of cnosumers that can attach
#                to the ring at any given time.
#  --host      - Sets the name of the host that is the target of the
#                query.
#  name        - The name of a ring buffer.
#  pattern     - A pattern that is matched against the known rings. If glob
#                characters are used in the pattern, the pattern must be
#                quoted to prevent shell expansion.
#
#
#--------------------------------------------------------------------------


#  Required packages, and search list modifications.

set libdir [file join [file dirname [info script]] .. TclLibs]
set libdir [file normalize $libdir]
lappend auto_path $libdir

package require removetcllibpath
package require ring
package require ringutils

package require portAllocator
package require struct::matrix
package require report

#  Global variables:

set defaultDataSize     8m
if {[array names env NSCLDAQ_DEFAULT_RINGMBYTES] eq "NSCLDAQ_DEFAULT_RINGMBYTES"} {
    set s  $env(NSCLDAQ_DEFAULT_RINGMBYTES)
    if {[string is integer $s]} {
        set defaultDataSize  ${s}m
    }
}
set defaultMaxConsumers 100
set defaultHostname     localhost



#  Report styles:

::report::defstyle simpletable {} {
    data set [split "[string repeat "| "   [columns]]|"]
    top set  [split "[string repeat "+ - " [columns]]+"]
    bottom set [top get]
    top    enable 
    bottom enable
}

::report::defstyle captionedtable {{n 1}} {
    simpletable
    topdata    set [data get]
    topcapsep  set [top get]
    topcapsep  enable
    tcaption   $n
}



#--------------------------------------------------------------------------
#
# put the program usage string to stderr.
#
proc usage {} {
    puts stderr "Usage"
    puts stderr " ringbuffer create ?--datasize=n? ?--maxconsumers=n?   name"
    puts stderr " ringbuffer format ?--maxconsumers=n?                  name"
    puts stderr " ringbuffer delete                                     name"
    puts stderr " ringbuffer status ?--host=hostname? ?--all? ?--user=user1,..?  ?name?"
    puts stderr " ringbuffer list   ?--host=hostname?"

}
#---------------------------------------------------------------------------
# Given a PID, use ps to get the program name:
#
# Parameters:
#   pid - the process id to get info from
# Returns:
#   name of the program as  PS gives it.  
# Note:
#   If the pro cess has exited e.g. 'None' is returned.
#
proc processName {pid} {

    set status [catch [list exec ps -p $pid -o cmd=] info]
    if {$status} {
        return None
    }
    return [lindex $info 0]   ;  # Rest is program args.
}

#--------------------------------------------------------------------------
#
#  Parse a command tail list.  The command is assumed to be divided into
#  parameters and options.  This proc will return a list that can be used
#  with array set to construct an array.  The discussion here will assume
#  that's been done.
#
#  The array will have elements for each option that is specified.
#  The value of each option array element will be true or false for
#  options that don't have a value true if the option was specified,
#  false if not.  For options that have a value, the array element
#  will be the value (default or supplied) of the option.
#  The special element Parameters will be the ordered list of all command
#  words that were not recognized as options.
#
# Parameters:
#   words    -  A list of command line words to process.
#   options  -  A list of option definitions.  These come in two forms:
#               optionname  - Initializes the element to false and will
#                             set the element to true if the option is
#                             spotted on the command line.
#               optionname=default
#                           - Initializes the element to default and
#                             sets it to the option value if the value
#                             is spotted on the command line.
#               options are matched as given. Therfore in order to specify
#               the --host switch for the status and list commands,
#               --host=localhost  must be specified.
# Returns:
#   a list suitable for use with array set e.g.:
#
#  array set parsedOptions [decodeArgs $tail [list --datasize=8m --maxconsumers=100]]
#
#  is typical usage.
#
#  Rationale for writing:
#    Yes there's a cmdline package in tcllib that does a lot of what I'd need, but
#    does not support the --option=value syntax I want to support.
#
proc decodeArgs {words options} {
    
    # produce the intial value for the local result array:

    set result(Parameters) [list]
    foreach option $options {
	set optAndDefault [split $option =]
	set optionname    [lindex $optAndDefault 0]
	if {[llength $optAndDefault] == 2} {
	    set result($optionname) [lindex $optAndDefault 1]
	} else {
	    set result($optionname) false
	}
    }
    # Ready to parse the command words:

    foreach word $words {
	set wordAndValue [split $word =]
	set wordName     [lindex $wordAndValue 0]
	if {[array names result $wordName] eq ""} {
	    lappend result(Parameters) $word
	} else {
	    set result($wordName) true
	    if {[llength $wordAndValue] == 2} {
		set result($wordName) [lindex $wordAndValue 1]
	    }
	}
    }
    return [array get result]
}
		       
#--------------------------------------------------------------------------
#
#  Converts a size into an integer.
#
# Parameters:
#   parameter is a size to convert this can be an integer or an integer
#   followed by a letter k (which multiplies the integer by 1024),
#   or the letter m (which multiplieds the integer by 1024*1024).
# Returns the result or an error.
proc size parameter {
    if {[string is integer $parameter]} {
	return $parameter
    }
    set multiplier [string index $parameter end]
    set integer    [string range $parameter 0 end-1]

    if {![string is integer $integer]} {
	error "$parameter is not a valid size"
    }
    if {$multiplier eq "k"} {
	return [expr $integer*1024]
    }
    if {$multiplier eq "m"} {
	return [expr $integer*1024*1024]
    }

    error "$parameter does not end with a vaild scale letter"
}

#--------------------------------------------------------------------------
#  Formats the usage information into a tabular report and 
#  shoots that out stdout.
#
#  The form of the table is e.g.:
# 
# Name   data-size(k)  free(k)   max-consumers   producer maxget(k) minget(k) client clientdata(k)
# aring  10240         512       1000            1234     512        255       
#                                                                              1240  512
#                                                                              2376  255
#                                                                              4000   0
# nextring....
#
# We use the struct::matrix and report packages from tcllib to creat the report.
#
# Parameters:
#   info
#     A list of ring information raw from the RingMaster, selected and 
#     sorted by the caller.
# 
proc displayUsageData info {
    ::struct::matrix reportData
    reportData add columns 9
    reportData insert row 0 [list Name data-size(k) free(k) max_consumers producer maxget(k) minget(k) client clientdata(k)]

    foreach item $info {
	set name      [lindex $item 0]
	set ringdata  [lindex $item 1]
	set size      [lindex $ringdata 0]
	set size      [expr $size/1024]
	set free      [lindex $ringdata 1]
	set free      [expr $free/1024]
	set consumer  [lindex $ringdata 2]
	set producer  [lindex $ringdata 3]
	set maxget    [lindex $ringdata 4]
	set maxget    [expr $maxget/1024]
	set minget    [lindex $ringdata 5]
	set minget    [expr $minget/1024]
	set clients   [lindex $ringdata 6]

	# Now The header for a ring:

	reportData insert row end [list $name $size $free $consumer $producer $maxget $minget - -]

	# List the client information:

	foreach client $clients {
	    set pid [lindex $client 0]
        set program [processName $pid]
	    set get [lindex $client 1]
	    set get [expr $get/1024]
        set consumer "$pid\n ([file tail $program])"
    
	    reportData insert row end [list - - - - - - - $consumer $get]
	}
    }
    # Format the report:


    ::report::report r 9 style captionedtable 1
    puts [r printmatrix reportData]
    
    reportData destroy
}
##
# filterRingStats
#
#  Given a ring usage list, filters the result by the 
#  values given by options (see ringbuffer status).
#  There are restrictions:
#
#   *  Only ringbuffers in the localhost can be filtered by user
#      at this time, so any of the --filter options are not
#      legal with --host
#   *  --all and --user are mutually exclusive.
#
# @param ringList - ring status list from the ring master.
# @param opts     - list in array set form that allows us to
#                   reconstruct the parse of the command line
#                   options.
#
# @return list - ringList filtered.
#
proc filterRingStats {ringList opts}  {
    array set options $opts
    set filter 1;                  # Assume we can filter.
    
    if {$options(--host) ne $::defaultHostname} {
	puts stderr "*Warning* filtering not done on remote rings"
	set filter 0
    }

    if {$options(--all) && ($options(--user) ne $::tcl_platform(user))} {
	error "The --all and --user options are mutually exclusive"
    }

    # --all present basically does not filter:
    if {$options(--all)} {
	set filter 0
    } else {
	set userList [split $options(--user) ,]
    }

    set result [list]
    foreach item $ringList {
	if {$filter} {
	    set ringName [lindex $item 0]
	    set ringFile [file join /dev/shm $ringName] ; #linux specific
	    set owner [file attributes $ringFile -owner]
	    if {$owner in $userList} {
		lappend result $item
	    }
	    
	} else {
	    lappend result $item
	}
    }
    return $result
}
#--------------------------------------------------------------------------
#
# create a ring buffer.
#
# Parameters:
#   tail  - the list of parameters that follow the command line:
#
proc createRing tail {
    set options [list                                           \
		     --datasize=$::defaultDataSize              \
		     --maxconsumers=$::defaultMaxConsumers]

    set tail [lrange $tail 1 end]
    array set parse [decodeArgs $tail $options]

    if {[llength $parse(Parameters)] != 1} {
	usage
	exit -1
    }
    ringbuffer create $parse(Parameters) [size $parse(--datasize)] $parse(--maxconsumers)
}

#--------------------------------------------------------------------------
#
#  Format an existing ring buffer.
#  This should not be done with an active ring.
#
# Parameters:
#   tail - the command tail.
#
proc formatRing tail {
    set options [list                               \
		     --maxconsumers=$::defaultMaxConsumers]
    set tail [lrange $tail 1 end]
    array set parse [decodeArgs $tail $options]
   
    if {[llength $parse(Parameters)] != 1} {
	usage
	exit -1
    }
    ringbuffer format $parse(Parameters) $parse(--maxconsumers)
}

#--------------------------------------------------------------------------
#
# Delete an existing ring.
# 
# Parameters;
#   tail - the command tail.
#
proc deleteRing tail {
  if {[llength $tail] != 2} {
    usage
    exit -1
  }

  if {[catch {ringbuffer remove [lindex $tail 1]} msg ]} {
    puts "FAILURE: $msg"
    exit -1
  }
}

#--------------------------------------------------------------------------
#
#  Show the usage of the known rings.
#
proc displayStatus tail {
    set tail [lrange $tail 1 end]
    set pattern "*"
    set options  [list  --host=$::defaultHostname --all --user=$::tcl_platform(user)]
    array set parse [decodeArgs $tail $options]

    if {[llength $parse(Parameters)] == 1} {
	set pattern $parse(Parameters)
    }
    if {[llength $parse(Parameters)] > 1} {
	usage
	exit -1
    }

    # Get the usage and filter it by the pattern:

    set info [getRingUsage $parse(--host)]
    set resultList [list]
    foreach item $info {
	set ringname [lindex $item 0]
	if {[string match $pattern $ringname]} {
	    lappend resultList $item
	}
    }
    # Sort the list of rings by the ring name.

    set resultList [lsort -index 0 $resultList]

    set resultList [filterRingStats $resultList [array get parse]]

    # Now format the info so humans can read it.


    displayUsageData $resultList
}

#--------------------------------------------------------------------------
# List the rings known to a host.
#
proc listRings tail {
    set tail [lrange $tail 1 end]
    set options [list                           \
		     --host=$::defaultHostname]
    array set parse [decodeArgs $tail $options]

    if {[llength $parse(Parameters)] != 0} {
	usage
	exit -1
    }

    set info [getRingUsage $parse(--host)]
    foreach ring [lsort -ascii $info] {
	puts [lindex $ring 0]
    }
}
#--------------------------------------------------------------------------
# Entry point.
# 
#  We need an args variable, it must have at least one parameter,
#  the subcommand.


if  {[info globals argv] eq ""} {
    error "ringbuffer.tcl is intended to be a shell command !!"
}

if {[llength $argv] < 1} {
    usage
    exit -1
}

# Get the subcommand and dispatch it. Yes we could use switch
# but the if chain has always been clearer to me:

set subcommand [lindex $argv 0]

if       {$subcommand eq "create"} {
    createRing $argv
} elseif {$subcommand eq "format"} {
    formatRing $argv
} elseif {$subcommand eq "delete"} {
    deleteRing $argv
} elseif {$subcommand eq "status"} {
    displayStatus $argv
} elseif {$subcommand eq "list"} {
    listRings $argv
} else {
    usage 
    exit -1
}
exit 0
