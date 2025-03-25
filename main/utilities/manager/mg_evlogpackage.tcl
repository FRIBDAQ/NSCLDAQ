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
# @file  mg_evlogpackage.tcl
# @brief API to manage event loggers.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide eventloggers 1.0
package require sqlite3
package require programs;      # For program initiation procs.
package require sequence;      # For output relay from programs.
package require containers;    # For embedded containerized systems.
package require kvstore;       # Key value store access.
##
# We export the following procs:
#
# eventlog::add          - Add a new logger to the list.
# eventlog::listLoggers  - List defintions of all loggers.
# eventlog::rm           - Remove a logger from the list.
# eventlog::enable       - Enable an individual logger.
# eventlog::enableAll    - Enable all logger s.
# eventlog::disable      - Disable a logger.
# eventlog::disableAll   - Disable all logging
# eventlog::enableRecording - Turn on logging.
# eventlog::disableRecording - Turn off logging.
# eventlog::isRecording  - Return state of recording flag.
#
# eventlog::start        - conditionally start all enabled event loggers.
# eventlog::stop         - hard kill all running event loggers.
#


#  Create the namespace in which the procs live.

namespace eval eventlog {
    #
    #   List of running loggers - each element is a logger definition dict.
    #
    variable runningLoggers [list]
    
    
}
#------------------------------------------------------------------------------
# Private procs:

##
# _getOption
#    Gets an option from an option dictionary with a default value:
#
# @param dict  - dictionary value containing options.
# @param key   - Key containing the option.
# @param default - Value to return if the dict does not have the key.
#
proc ::eventlog::_getOption {dict key default} {
    
    if {[dict exists $dict $key]} {
    
        return [dict get $dict $key]
    } else {
    
        return $default
    }
}
##
# ::eventlog::_ensureNotDuplicate
#    Error if the definition of a logger duplicates an existing one with
#    respect to either the ring or the destination
#
# @param db   - Database command.
# @param dest - Recording destination directory (or tree).
# @param src  - Recording source ringbuffer.
#
proc ::eventlog::_ensureNotDuplicate {db dest src} {
    set c [$db eval {
        SELECT COUNT(*) FROM logger WHERE ring = $src OR destination = $dest
    }]
    if {$c > 0} {
        error \
          "There is already a logger defined that has either '$src' as a source \
or '$dest' as a destination"
        
    }
}
##
# ::eventlog::_getContainerId
#    If a container is not null, returns its id.   It is an error for
#    a non-empty container name to not exist.
#
# @param db   - database command.
# @param name - Container name. "" for no container.
# @return mixed
# @retval ""  - If $name eq ""
# @retval integer - Id of the container name if it exists.
#
proc ::eventlog::_getContainerId {db name } {
    if {$name eq ""} {
        return "";                # Null id.
    }
    set existing [::container::listDefinitions $db]
    
    foreach def $existing {
        if {[dict get $def name] eq $name} {
            return [dict get $def id]
        }
    }
    error "There is no container named '$name'"
}
#----------------------------------------------------------------------------
#  Public entry points:

##
# eventlog::add
#     add a new event logger.
# @param db     - Database command.
# @param root   - Root of DAQ (in container if the event logger runs in a container)
# @param source - Data source (ring buffer URI).
# @param destination - Where the event data are stored.
#                 * For a partial logger, this is the director in which the
#                   event files are made.
#                 * For a full logger, this is the top of the managed directory
#                   tree.
# @param options - Dict of options.
#                  The default value of the options parameter is an empty dict.
#                  The keys we pay attention to are:
#        -  host   - Host in which the logger runs.  localhost is the default.#
#        -  partial - Boolean that if true indicates this logger acts like a v11
#                    multi-loggers (that is it does ot attempt to manage the filesystem).
#                    Defaults to false (full management of directory tree)
#        -  critical - Boolean that, if true, indicates that if the logger fails,
#                     a shutdown will be initiated.  True by default.
#        -  enabled - Boolean that, if true, indicates the loggers will log
#                     the next run.  This is true by default.
#        -  container - Name of the container the logger is run in.  This is
#                     empty by default, indicating the logger is run in the
#                     host environment.
# @return id   - Id of the event logger in the table.
#
# @note it is illegal to:
#    -   Have a duplicate destination directory.
#    -   Have a duplicate ringbuffer source.
#    -   Specify a container that does not exist.
# @note that the destination directory is created if it does not yet exist.
#
proc ::eventlog::add {db root source destination {options {}}} {
    
    # Handle the options>
    
    set host [::eventlog::_getOption $options host localhost]
    set partial [::eventlog::_getOption $options partial 0]
    set critical [::eventlog::_getOption $options critical 1]
    set enabled  [::eventlog::_getOption $options enabled 1]
    set container [::eventlog::_getOption $options container ""]
    
    ::eventlog::_ensureNotDuplicate $db $destination $source
    set container [::eventlog::_getContainerId $db $container]
 
 
    $db eval {
        INSERT INTO logger
            (
                daqroot, ring, host, partial, destination,
                critical, enabled, container_id
            ) VALUES (
                $root, $source, $host, $partial, $destination, $critical,
                $enabled, $container    
            )
    }
    return [$db last_insert_rowid]
}
##
# ::eventlog::listLoggers
#    Return a list containing the definitions of all event loggers.
#
# @param db  - database command.
# @return list of dicts - Each list element is a dict that describes one
#              event logger.  THe dict keys are:
#        - id      - Id of the logger (Primary key).
#        - daqroot - DAQ Root directory of the logger.
#        - ring    - URI of the data source ring buffer.
#        - host    - Name/IP address of the host in which the logger runs.
#        - partial - Boolean indicating if  the logger is a partial logger.
#        - destination - Destination directory to which events are logged.
#        - critical - Boolean that's true if the logger is critical.
#        - enabled  - True if the logger is currenly enabled.
#        - container- Name of the container.
#        - container_id - Id of the container.
# @note - we use knowledge of the container schema.
#
proc ::eventlog::listLoggers {db} {
    set result [list]
    $db eval {
        SELECT logger.id AS loggerid, daqroot, ring, host, partial, destination,
               critical, enabled, container_id, container.container AS container_name
        FROM logger
        LEFT JOIN container ON container_id = container.id
    } values {
        
        lappend result [dict create                                         \
            id $values(loggerid)      daqroot $values(daqroot)             \
            ring $values(ring)         host $values(host)                    \
            partial $values(partial)   destination $values(destination)     \
            critical $values(critical)  enabled $values(enabled)             \
            container $values(container_name)                               \
            container_id $values(container_id)                              \
        ]
        
    }
    
    
    return $result
}
##
# ::eventlog::rm
#   Remove an event logger.
#
# @param db   - Database command.
# @param id   - Id of the logger to remove.
#  It is an error if the logger does not exist.
#
proc ::eventlog::rm {db id} {
    $db eval {DELETE FROM logger WHERE id = $id}
    if {[$db changes] == 0} {
        error "There is no event logger with the id '$id'"
    }
}
##
# eventlog::enable
#   Enable a logger given its id.
#
# @param db - database command.
# @param id - id of logger to enable.
#
proc eventlog::enable {db id} {
    $db eval {
        UPDATE logger SET enabled = 1 WHERE id = $id
    }
    if {[$db changes] == 0} {
        error "There is no event logger with the id '$id'"
    }
}
##
# eventlog::disable
#  Same as above but disables:

proc eventlog::disable {db id} {
    $db eval {
        UPDATE logger SET enabled = 0 WHERE id = $id
    }
    if {[$db changes] == 0} {
        error "There is no event logger with the id '$id'"
    }
}
##
# eventlog::enableAll
#   Turn on event logging in all loggers.
#
# @param db - database command.
#
proc eventlog::enableAll {db} {
    $db eval {
        UPDATE logger set enabled = 1
    }
}
##
# eventlog::disableAll
#    same as above but the loggers are all disabled.

proc eventlog::disableAll {db} {
    $db eval {
        UPDATE logger set enabled = 0
    }
}

##
# eventlog::enableRecording
#   Turn on recording next run.
#
# @param db -  database command.
#
proc eventlog::enableRecording {db} {
    $db eval {
        UPDATE recording SET state = 1
    }
}
##
# eventlog::disableRecording
#   Same as above but recording is turned off.

proc ::eventlog::disableRecording {db} {
    $db eval {
        UPDATE recording SET state = 0
    }
}
##
# eventlog::isRecording
#   Return the recording state flag.
# @param db - database command.
#
proc eventlog::isRecording {db} {
    db eval {
        SELECT state FROM recording LIMIT 1
    }
}
#-------------------------------------------------------------------------
#  This set of bits and pieces are used to manage the running
#  event loggers.
#
# Private procs:

##
# ::eventlog::_registerLogger
#    Add a logger to the list of active loggers:
#
# @param def - logger definitions
# @param fd  - file descriptor on which logger output can be read (ignored).
#
proc ::eventlog::_registerLogger {def fd} {
    lappend ::eventlog::runningLoggers $def
}
##
# ::eventlog::_exitForbidden
#    Determine if the exit of a critical event logger is a bad thing:
#
# It's bad for one to exit if:
#   - The state is BEGIN and there's not a current transition to
#     any other state (event log exited with run in progress)
#   - The current transition is BEGIN (event log exited as it started up)
#
# @param db   - database handle
# @return bool - true if the event loggers should not really be exiting.
#
proc ::eventlog::_exitForbidden {db} {
    set transition [::sequence::getCurrentTransition]
    set state       [::sequence::currentState $db]
    
    if {($state eq "BEGIN") && ($transition eq "")} {
        return 1
    }
    if {($transition ne "") && ([$transition cget -type] eq "BEGIN")} {
        return 1
    }
    return 0
}

##
# ::eventlog::_unregisterLogger
#    Called when a logger exits.
#    - Remove it from the list of loggers.
#    - If the logger was critical, force a transition to SHUTDOWN
#
# @param db  - database verb.
# @param def - logger definition.
#
proc ::eventlog::_unregisterLogger {db def} {
    ::sequence::relayOutput "Unregister logger $def"
    set index [lsearch -exact $::eventlog::runningLoggers $def]
    if {$index ne -1} {
        set ::eventlog::runningLoggers \
            [lreplace $::eventlog::runningLoggers $index $index]
        
    }
    # Since all event loggers are transitory,
    # we need to know if it's actually ok that one exited.
    
    if {[dict get $def critical] && [::eventlog::_exitForbidden $db]} {
	::sequence::relayOutput "----> Exit of $def was forbidden"
        set host [dict get $def host]
        set dest [dict get $def destination]
        ::sequence::relayOutput "Critical logger in $host -> $dest exited."
        ::sequence::relayOutput "Forcing SHUTDOWN transition."
        ::sequence::transition $db SHUTDOWN
    }
}

##
# ::eventlog::_handleOutput
#     Really we just relay this to the ouptut clients.
#    Handles output from an event logger:
#     @param def - logger definition.
#     @param db  - Database command.
#     @param fd  - File descriptor that can be read.
#
proc ::eventlog::_handleOutput {def db fd} {
    
    set host [dict get $def host]
    set dest [dict get $def destination]
    set isCritical [dict get $def critical]
    set status [catch {
    if {[eof $fd] } {
        ::eventlog::Log "Event logger in $host -> $dest exited."
        close $fd
	    ::sequence::relayOutput "Calling unregister logger to $dest"
        ::eventlog::_unregisterLogger $db $def
    } else {
        set line [gets $fd]    
        ::eventlog::Log "Event logger in $host -> $dest :  $line"
    }
    } msg]
    if {$status} {
        eventlog::Log "Error $msg $::errorInfo"
    }
}

##
# ::eventlog::_writeLoggerScript
#    Write a script that starts the logger - this will run either in or out
#    of a container depending on the mode of the logger.  The script is neutral
#    to the continerness.
# @param db     - database verb.
# @param def    - Logger definition.
# @return string -name of the filename written.
# @note the script is written in the $::container::tempdir directory and has a
#       name of the form: eventlog_dest_host_[clock seconds]  where dest is the
#       is the destination directory with / mapped to -
#
proc eventlog::_writeLoggerScript {db def} {
    set host [dict get $def host]
    set rawdest [dict get $def destination]
    set root   [dict get $def daqroot]
    set ring   [dict get $def ring]
    set partial [dict get $def partial]
    
    set fnamedest [string map [list / -] $rawdest]
    set fname [file join $::container::tempdir eventlog_${fnamedest}_${host}_[clock seconds]]
    set fd [open $fname w]
    
    puts $fd "#!/bin/bash"
    puts $fd ". $root/daqsetup.bash -f"
    puts $fd "export RECORD_DEST=$rawdest"
    puts $fd "export RECORD_SRC=$ring"
    puts $fd "export RECORD_PARTIAL=$partial"
    puts $fd "export RUN_NUMBER=[kvstore::get $db run]"
    puts $fd
    puts $fd [file join $root bin eventlog_wrapper]
    
    close $fd
    file attributes $fname -permissions 0744
    return $fname
}
##
# ::eventlog::_instalLogger
#     Install the fd handler and register a logger.
#
# @param def - the logger definition.
# @param db  - Database verb.
# @param fd  - fd open on the output pipe.
#
proc ::eventlog::_installLogger {def db fd} {
    fconfigure $fd -buffering line
    fileevent $fd readable [list ::eventlog::_handleOutput $def $db $fd]
    ::eventlog::_registerLogger $def $fd    
}
##
# ::eventlog::_runBare
#    Runs an event logger in the native system (with no container).
#    - write the script
#    - use an ssh pipe to run it in the specified system.
#
# @param db  - database command.
# @param def - definition.
#
proc ::eventlog::_runBare {db def} {
    set scriptPath [::eventlog::_writeLoggerScript $db $def]
    set host [dict get $def host]
    
    set fd [open "|ssh -T  $host $scriptPath |& cat" w+]
    ::eventlog::_installLogger $def $db  $fd
}

##
# ::eventlog::_runContainerized
#    Run a logger containerized.  Much like running a program containerized
#    - Figure out the host and container and, if necessary, start the persistent
#      container in the remote system.
#    - A script is written to run the appropriate event log container in the
#      container.
#    - The script is fired off in the container with stdout/stderr captured
#      here via filehandlers both to relay input and to detect logger exits.
#
# @param db    - Database command.
# @param def   - logger definition. See listLoggers for a desription of this
#                dict, however at the time this comment is being written the
#                definition contains the following keys:
#        - id      - Id of the logger (Primary key).
#        - daqroot - DAQ Root directory of the logger.
#        - ring    - URI of the data source ring buffer.
#        - host    - Name/IP address of the host in which the logger runs.
#        - partial - Boolean indicating if  the logger is a partial logger.
#        - destination - Destination directory to which events are logged.
#        - critical - Boolean that's true if the logger is critical.
#        - enabled  - True if the logger is currenly enabled.
#        - container- Name of the container.
#        - container_id - Id of the container.
#
proc ::eventlog::_runContainerized {db def} {
    set container [dict get $def container]
    set host      [dict get $def host]
    if {[list $container $host] ni [::program::activeContainers]} {
        :::program::_activateContainer $db $container $host
    }
    set scriptPath [::eventlog::_writeLoggerScript $db $def]
    set fd [::container::run $container $host $scriptPath]
    
    ::eventlog::_installLogger $def $db $fd
}


##
# ::eventlog::_startLogger
#   Starts a single logger.  How this is done depends on whether or not the
#   logger runs in a container or not.
#
# @param db   - database object command.
# @param def  - Logger definition dict.  See listLoggers for a description of the
#               dict.
#
proc ::eventlog::_startLogger   {db def} {
    set containerName  [dict get $def container]
    if {$containerName eq ""} {
        ::eventlog::_runBare $db $def
    } else {
        ::eventlog::_runContainerized $db $def
    }
}

#---------------------------------------------------------------------
# public:
#


##
# eventlog::start
#    Conditionally starts all of the event loggers.
#    By conditionally, we mean that an event logger is only started if:
#    - Global recording is enabled.
#    - The event logger itself is enabled.
#
# @param db - database object command.
#
proc eventlog::start {db} {
    if {[eventlog::isRecording $db]} {
        set loggers [::eventlog::listLoggers $db]
        foreach l $loggers {
            if {[dict get $l enabled]} {
                ::eventlog::_startLogger $db $l
            }
        }
    }   
}
##
# eventlog::Log
#    Send a message to all clients monitoring the manager's output relay.
#
# @param msg - the message to send.
#
proc ::eventlog::Log {msg} {
    ::sequence::relayOutput $msg
}
