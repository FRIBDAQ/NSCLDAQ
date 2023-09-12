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
# @file  mg_programpackage.tcl
# @brief Provide and API for programs
#
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide programs  1.0
package require containers
package require sqlite3

##
#  This package provides the api to the programs part fot the NSCLDAQ manager
#  subsystem.  Note that input/output monitoring requires that the host program
#  is running an event loop.
#  This package makes use of the containers API in that a program can optionally
#  be specified to run in a container.
#  It contains the following procs:
#
#  program::exists   - The named program has been defined.
#  program::add      - Adds a new program definition to the database.
#  program::replace  - Replace existing program with a new definition.
#  program::remove   - Removes a program definition from the database.
#  program::getdef    - Get program definition by name.
#  program::listDefinitions    - List of dicts of program definitions.
#  program::isActive  - Check if a program is active.
#  program::activeContainers - List the containers we activated.
#  program::activePrograms - lists of programs that are running.
#  program::run       - Runs a program in the database
#  program::kill      - send a SIGINT to a program to try to kill it.


namespace eval ::program {
    variable activeContainers [list]; # list of [list container host] active containers.
    variable activePrograms [list];    # List of still active programs.
    variable fds;                     # Array of file descriptors indexed by
    array set fds [list];             # Program name.

    #  Array indexed by program of output handlers.
    
    variable outputHandlers;
    array set outputHandlers [list]
    
    variable containeredPrograms;        # array indexed by container@host name
    array set containeredPrograms [list]; # Whose elements are lists of programs
                                       ; # Running in that container on that host.
                                       
    ::container::_setup;             # Setup the tempdir.
}
#-------------------------------------------------------------------------------
#  For testing:

proc ::program::_reinit { } {
    set ::program::activeContainers [list]
    set ::program::activePrograms   [list]
    array unset ::program::fds *
    array unset ::program::outputHandlers *
    array unset ::program::containeredPrograms *
    
}
#-------------------------------------------------------------------------------
#  Utilities:
#

##
# program::_filterGrep
#   Removes items from a list that contain the string 'grep'  This is used
#   in ps axuww|grep results in case the transient grep commands also appears
#   in the list as it will contain the string searched for.
#
# @param input - a list of strings.
# @return list - a possibily empty list of strings that don't contain "grep"
#
proc program::_filterGrep {input} {
    set result [list]
    foreach line $input {
        if {[string first grep $line] == -1} {
            lappend result $line
        }
    }
    return $result
}

##
# program::_typeId
#   @param db   - database command.
#   @param type - Program type string.
#   @return int - The id of the program type.
#   @retval -1  - No match.
#
proc ::program::_typeId {db type} {
    #
    #  The assumption is there will be 0 or 1 results:
    #
    set result [$db eval {                                   
        SELECT id FROM program_type WHERE type=$type
    }]
    if {$result eq ""} {
        return -1
    } else {
        return $result
    }
}
##
# program::_dictgetOrDefault
#  Get a value from a dict or a default if there is no key
#
# @param dict. - dictionary value.
# @param key   - Key to fetch.
# @param default - default value - defaults to empty string
#
proc ::program::_dictGetOrDefault {dict key {default {}}} {
    set result $default
    if {[dict exists $dict $key]} {
        set result [dict get $dict $key]
    }
    return $result
}
##
# _makeProgramCommand
#   Given the definition of a program, generate the command string to actually
#   run it. that's of the form path options parameters
#
# @param def - program definition dict.
# @return string - the program invocation string.
#
proc ::program::_makeProgramCommand {def} {
    set command [dict get $def path]
    if {[dict exists $def options]} {
        foreach option [dict get $def options] {
            set name [lindex $option 0]
            set value [lindex $option 1]
            if {$value ne ""} {
                append command  " " $name " " $value;   # Acceptable for in long options with gengetopt.
                    
                
            } else {
                append command " " $name  ;   # Just a flag, no value.
            }
        }
    }

    if {[dict exists $def parameters]} {
        foreach param [dict get $def parameters] {
            append command " " $param
        }
    }
    return $command
    
}

##
# _writeProgramScript
#   Writes a script to run a program this script will either be used by
#   _runBare directly or _runInContainer in a singularity run command
#   depending on how the program is run.
#
# @param fname - Full path of the file in which to write the script.
# @param def   - Definition.
#
# The file is written in the following order:
#  - init script if any.
#  - environment variable definitions (if any).
#  - command with options then parameters in that order.
#
proc ::program::_writeProgramScript {fname def} {

    set fd [open $fname w]
    puts $fd "#!/bin/bash"
    puts $fd "#   script to run [dict get $def path]@[dict get $def host]\n"
    
    # Write any initscript:
    
    if {[dict exists $def initscript] } {
        puts $fd "#User supplied initializationscript: \n"
        puts $fd [dict get $def initscript]
        puts $fd ""
    }
    
    # Write any environment settings:
    
    if {[dict exists $def environment]} {
        puts $fd "\n#User supplied environment variables: \n"
        foreach setting [dict get $def environment] {
            set name [lindex $setting 0]
            set value [lindex $setting 1]
            #  Quote the value so it can have embedded white space.
            puts $fd "$name=\"$value\";export $name"
        }
    }
    # If there's a working directory set it:
    
    if {[dict exists $def directory]} {
        puts $fd "cd [dict get $def directory]"
    }
    
    puts $fd "\n# Invoke user's program\n"
    puts $fd [_makeProgramCommand $def]
    
    close $fd
    file attributes $fname -permissions u+x;   # make it executable
}


##
# ::program::_handleContainerInput
#
#     Called when input from a container is available.   Note that this is
#     input from the ssh pipe that started the container iself, not the
#     container.  Given how containers get started, an EOF on the input
#     means the container was shut down (e.g. container::deactivate)
#     That means thate:
#     * We remove the container from the ::program::activeContainerList
#     * We remove the array element for the container from
#       ::program::containeredPrograms.
#     * We assume the programs themselves will die in their own time
#       which will result in their book keeping getting run down.
#
#
# @param container - name of container
# @param host      - host in wich the container is running.
# @param fd        - file descriptor ready to read.
#
proc ::program::_handleContainerInput {container host fd} {
    fconfigure $fd -blocking 0
    read $fd
    if {[eof $fd]} {
        
        array unset ::program::containeredPrograms ${container}@${host}
        set index [lsearch \
            -exact $::program::activeContainers [list $container $host] \
        ]
        set ::program::activeContainers   \
            [lreplace $::program::activeContainers $index $index]
        close $fd
    }
}
##
# Handy debugging hook enable output by setting debugOutput to 1.
# output will be logged to the file open on ::tty if there is one.
# so to debug, something in the system must do a set ::tty [open ... w]
# and ::debugOutput must be true.
#
set debugOutput 1
proc ::program::_log text {
    if {([info globals ::tty] ne "") && $::debugOutput} {
        puts $::tty $text
        flush $::tty
    }
}

##
# ::program::_outputWraper
#
#   Called when the fd on which a program is running becomes readable:
#   - If there's a specified handler for it we call that first. The handler
#     is expected to leave the fd open -- even if an end file condition is
#     detected because: see below:
#   - If there's no handler for the program, we just set the file descriptor
#     to non blocking and do a read to discard what might be buffered in the fd.
#   - If after all of this there's an eof condition on the file descriptor we
#      *  Close the file desriptor
#      *  Remove the program from activePrograms.
#      *  Remove the fd from the fds array.
#      *  Clear out any input handler from the outputHandlers
#
#  With respect to our handling of the blocking mode we restore that if we needed
#  to do the read.  This support a future use case where at some point we may
#  allow an output handler to be attached to a running program.
#
# @param name - program name.
# @param fd   - File descriptor ready to read.
#
proc ::program::_outputWrapper {name fd} {
    
    if {[array names  ::program::outputHandlers $name] eq $name} {
        
        uplevel #0 $::program::outputHandlers($name) $name $fd
        
    } else {
        
        set blocking [chan configure $fd -blocking]
        chan configure $fd -blocking 0
        chan configure $fd -blocking $blocking
    }
    #  Handle the case of an eof:
    #
    if {[eof $fd]} {
        ::program::_log "$name exited"
        
        catch {
        catch {close $fd}
        
        array unset ::program::fds $name        
        array unset ::program::outputHandlers $name
        set index [lsearch -exact $::program::activePrograms $name]
        set ::program::activePrograms \
            [lreplace $::program::activePrograms $index $index]
        
        } msg

    }
}
##
# _runBare
#   Runs a program in the bare host.  By bare host we mean the program is run
#   without the 'benefit' of a containerized environment.  When this happens,
#   the program is run on the end of an ssh -t pipeline.  What's actually run
#   is a shell script that defines any environment variables.  The shell
#   script name is of the form native_programname_host[clock seconds].
#   The script is dynamically written so the options/and parameters can be
#   put on the line. 
#
#   The program is registered in activePrograms and the fd open on the pipe
#   is saved in the fds array indexed by the program name.
#   If there is an ouptut handler, this is stored in the outputHandlers array.
#
#   Note that the output handler is jacketed by our own output handler that
#   tests for EOF and, if present, shuts down the program after the the
#   output handler's called if there is an EOF condition on the pipe.
#
# @param db   - Database command (I actually don't think we need this)
# @param def  - Program definition from ::program::getdef
# @return fd - the file descriptor open on the command's I/Out-errr
#  
proc ::program::_runBare {db def} {
    #
    # Let's build  a script to run this beast:
    
    set name [dict get $def name]
    set host [dict get $def host]
    set fname [file join $::container::tempdir \
            native_${name}_${host}_[clock seconds] \
    ]
    ::program::_writeProgramScript $fname $def
    
    set fd [open "|ssh -T  $host $fname |& cat" w+]
    
    
    return $fd
}
##
# _activateContainer
#
#   Start a container that's known not to be running.
#   while internal, this is a stable interface that can be invoked from other
#   manager modules.
#
# @param db        - database verb
# @param container - name of container.
# @param host      - host in which the contaiuner must be run.
#
proc ::program::_activateContainer {db container host} {
    set activeContainerValue [list $container $host]
    set containerfd [container::activate $db $container $host]
    fileevent $containerfd readable \
        [list ::program::_handleContainerInput $container $host $containerfd]
    lappend ::program::activeContainers $activeContainerValue

    #  Delay to let the container become active:
        
    after 400;                   #From the contaier tests.
}
##
# _runInContainer
#    Runs a program in a container.
#    - If the container is not running in the specified host, start it.
#    - Create the command line to ask the container to run.
#    - Ask the container to run it.
#
# @param db   - database command.
# @param def  - Command definition.
# @return fd  - file descriptor open on the program running in the container
proc ::program::_runInContainer {db def} {
    set container [dict get $def container_name]
    set host      [dict get $def host]
    set name      [dict get $def name]
    
    set activeContainerValue [list $container $host]
    if {[lsearch -exact $::program::activeContainers $activeContainerValue] == -1} {
        ::program::_activateContainer $db $container $host
    }
    #  The container is running so we can create our command script and ask the
    #  container to run it:
    
    set fname [file join $::container::tempdir \
        container_${container}_${name}_{$host}_[clock seconds] \
    ]
    ::program::_writeProgramScript $fname $def
    
    lappend ::program:containeredPrograms(${container}@${host})  $name
    
    return [::container::run $container $host $fname]
}
##
# _getRootRecordInfo
#
#  Given a program definition, returns the information needed to create/modify
#  the root record of the program.
#
#   @param db   - database name.
#   @param type - program type name.
#   @param host - program host.
#   @param options - addition options.
#
#  @return dict with the following keys:
#    *   - typeid  - id corresponding to type.
#    *   - containerid - id corresponding to container.
#    *   - initscript  - initialization script contents.
#    *   - dir         - working directory.
#    *   - service     - The service it will advertise.
#
proc ::program::_getRootRecordInfo {db type host {options {}}} {
    # Get the type id corresponsing to the program.
    #
    set typeId [program::_typeId $db $type]
    if {$typeId == -1} {
        error "$type is not a valid program type."
    }
    #  If there's a container in the options get its id:
    
    set containerId ""
    
    set cname [::program::_dictGetOrDefault $options container]
    if {$cname ne ""} {
        set def [container::listDefinitions $db $cname]
        if {$def eq ""} {
            error "There is no container named $cname"
        }

        set containerId [dict get $def id]
    }
    set initscript  [::program::_dictGetOrDefault $options initscript]
    if {$initscript ne ""} {
        set fd [open $initscript r]
        set initscript [read $fd]
        close $fd
    }
    
    set workingDir [::program::_dictGetOrDefault $options directory]
    set service    [::program::_dictGetOrDefault $options service]
    
    return [dict create                                                    \
            typeid $typeId containerid $containerId initscript $initscript \
            dir $workingDir service $service                               \
    ]
}
#-------------------------------------------------------------------------------
##
# program::exists
#   @param db     - Database command name.
#   @param name   - Name of the program.
#   @return bool - true if name exists, false if not.
#
#
proc ::program::exists {db name} {
    return [expr \
        {[$db eval {SELECT COUNT(*) FROM program WHERE name=$name}] != 0} \
    ]
}
##
# program::add
#    Adds a new program definition to the database.
#    It  is an error if a program with this name already exists.
#
#  @param  db    - database command used to access the sqlite3 config db.
#  @param  name  - Name of the new program.
#  @param  path  - Path to the program. If the program is run in a
#                  container this path must be its path within the container.
#  @param  type - Type of program (Transitory, Critical or Persistent)
#  @param  host  - Host in which the container is run.
#  @param  options - A dict that contains additional program options:
#                   *   container - If supplied this is the name of a container
#                                   and the program will be run inside that container.
#                   *   initscript- If supplied, this is a script of commands
#                                   that will be run prior to the program.
#                                   The contents of that script will be sucked
#                                   into the database.
#                   *   service   - If supplied, this is the daq port manager
#                                   name of a REST service that will be offered
#                                   by that program to participate in state transitions.
#                                   IF not provided but the program is part of the
#                                   state transition sequence old-stlye commands
#                                   will be sent to stdin instead.
#                   *   options   - list of program options of the form
#                                   [list [list optname optvalue]...].
#                   *   parameters - List of command line parameters.
#                   *   environment- list of environment variables to set in the
#                                    form [list [list name value]...].
#                   *   directory - working directory for the program if
#                                   it has specific wd requirements.
#                                   othwerwise the directory will be determined by
#                                   any container init script or be the  home.
#
#      
proc ::program::add {db name path type host {options {}}} {
    if {[program::exists $db $name]} {
        error "The program $name already has a definition"
    }
    
    set rootInfo [_getRootRecordInfo $db $type $host $options]
    
    
    #  Now that we have everything we need to do the root record insertion,
    # we start the transaction and get to work shoving crap into the database.
    
    set typeid [dict get $rootInfo typeid]
    set dir [dict get $rootInfo dir]
    set containerid [dict get $rootInfo containerid]
    set initscript [dict get $rootInfo initscript]
    set service [dict get $rootInfo service]
    $db transaction {
        $db eval {
            INSERT INTO program
                (name, path, type_id, host, directory, container_id,
                 initscript, service)
                VALUES ($name, $path, $typeid, $host, 
                        $dir,
                        $containerid ,   
                        $initscript,   
                        $service      
                )
        }
        set pgmid [$db last_insert_rowid]
        
        set opts [::program::_dictGetOrDefault $options options]
        foreach option $opts {
            set name [lindex $option 0]
            set value [lindex $option 1]
            $db eval {
                INSERT INTO program_option (program_id, option, value)
                VALUES ($pgmid, $name, $value)
            }
        }
    
        set parameters [::program::_dictGetOrDefault $options parameters]
        foreach parameter $parameters {
            $db eval {
                INSERT INTO program_parameter (program_id, parameter)
                VALUES ($pgmid, $parameter)
            }
        }
        
        set env [::program::_dictGetOrDefault $options environment]
        foreach var $env {
            set name [lindex $var 0]
            set value [lindex $var 1]
            $db eval {
                INSERT INTO program_environment (program_id, name, value)
                VALUES ($pgmid, $name, $value)
            }
        }
    }
}
##
# program::replace
#    Replace the definition of an existing program with something different.
#    Note that this supports renaming the program.
#    The main point is that the primary key of original definition remains
#    and therefore references to the old definition refer to the new one.
#
# @param db  - database command name.
# @param name - program name (existing).
# @param newname - new program name
# @param path  - Path to program.
# @param type - program type string.
# @param host  - host the program runs in.
# @param options - see add for the contents of this dict.
#
proc program::replace {db name newname path type host options} {
    set current [program::getdef $db $name];  # Error if no match.
    set id [dict get $current id]

    set rootInfo [_getRootRecordInfo $db  $type $host $options]
    set typeid [dict get $rootInfo typeid]
    set dir [dict get $rootInfo dir]
    set containerid [dict get $rootInfo containerid]
    set initscript [dict get $rootInfo initscript]
    set service [dict get $rootInfo service]
    
    $db transaction {
        #  Update the root record:
        
        $db eval {
            UPDATE program SET name=$newname,
                               path=$path,
                               type_id=$typeid,
                               host=$host,
                               directory=$dir,
                               container_id=$containerid,
                               initscript=$initscript,
                               service=$service
                    WHERE id = $id
                
        }
        #  Kill off the options, parameters and env and remake them:
        
        $db eval {
            DELETE FROM program_option WHERE program_id = $id
        }
        $db eval {
            DELETE FROM program_parameter WHERE program_id = $id
        }
        $db eval {
            DELETE FROM program_environment WHERE program_id = $id
        }
        set opts [::program::_dictGetOrDefault $options options]
        foreach option $opts {
            set name [lindex $option 0]
            set value [lindex $option 1]
            $db eval {
                INSERT INTO program_option (program_id, option, value)
                VALUES ($id, $name, $value)
            }
        }
    
        set parameters [::program::_dictGetOrDefault $options parameters]
        foreach parameter $parameters {
            $db eval {
                INSERT INTO program_parameter (program_id, parameter)
                VALUES ($id, $parameter)
            }
        }
        
        set env [::program::_dictGetOrDefault $options environment]
        foreach var $env {
            set name [lindex $var 0]
            set value [lindex $var 1]
            $db eval {
                INSERT INTO program_environment (program_id, name, value)
                VALUES ($id, $name, $value)
            }
        }
        
    }
}
    

##
# program::remove
#   Remove an existing program from the database.
#   this requires a transaction as we're going to clean up the program_option,
#   program_parameter and program_environment tables too (Even though strictly
#   speaking, once the root is gone it's all gone as row ids are not recycled).
#
# @param db   - database command.
# @param name - Name of the program to remove.
# @note It is an error to remove a program that does not exist.
#
proc ::program::remove {db name} {
    if {![::program::exists $db $name]} {
        error "There is no program named $name"
    }
    # We need to get the id of the program in order to remove all traces of it:
    
    set id [$db eval {
        SELECT id from program WHERE name = $name
    }]
    if {[llength $id] != 1} {
        error "program $name exists but searching for id gave [llength $id] hits"
    }
    # Removal needs to be atomic from multiple tables so:
    
    $db transaction {
        $db eval {
            DELETE FROM program WHERE id = $id
        }
        $db eval {
            DELETE FROM program_option WHERE program_id = $id
        }
        $db eval {
            DELETE FROM program_parameter WHERE program_id = $id
        }
        $db eval {
            DELETE FROM program_environment WHERE program_id = $id
        }
    }
}
##
# program::getdef
#    Return the definition dict of a program.  See program::listDefinitions
#
#  @param db  - database command.
#  @param name - Name of the command.
#  @return dict - see program::listDefinitions for the keys in that dict.
#  @note directory, container_id, container_name, initscript, service
#        are all optional keys. They are only present if there is data to supply.
#
proc ::program::getdef {db name} {
    
    #  First let's get the root directory and its type and container names
    #
    
    set result [dict create]
    
    db eval {
        SELECT program.id AS pgmid, program.name AS pgmname,
                path, type_id, host, directory,
                container_id, initscript, service,
                type, container
        FROM program
        LEFT JOIN program_type ON type_id = program_type.id
        LEFT JOIN container ON program.container_id = container.id
        WHERE program.name = $name
    } values {
        #  The mandatory keys:
        
        dict set result id $values(pgmid)
        dict set result name $values(pgmname) 
        dict set result path $values(path)
        dict set result type $values(type)
        dict set result  type_id $values(type_id) 
        dict set result host $values(host)
        # The optional keys are only set if there are no-null values:
        if {$values(directory) ne ""} {
            dict set result directory $values(directory)
        }
        if {$values(container_id) ne "" } {
            dict set result container_id $values(container_id) 
            dict set result container_name  $values(container)
        }
        if {$values(initscript) ne ""} {
            dict set result initscript $values(initscript)
        }
        if {$values(service) ne ""} {
            dict set result service $values(service)
        }
    }
    # If nothing got set in the result, there's no match and we have an
    # error:
    
    if {[llength [dict keys $result]] == 0} {
        error "There is no such program $name"
    }
    set programId [dict get $result id];  # the lookup key for the rest of the stuff.
    
    #  The options, parameters and environment keys are mandatory so:
    
    dict set result options [list]
    dict set result environment [list]
    $db eval  {
        SELECT option, value FROM program_option WHERE program_id = $programId
    } values {
        dict lappend result options [list $values(option) $values(value)]
    }
    
    set parameters [$db eval {
        SELECT parameter FROM program_parameter WHERE program_id = $programId
    } ]
    dict set result parameters $parameters
    
    $db eval {
        SELECT name, value FROM program_environment
        WHERE program_id = $programId
    } values {
        dict lappend result environment [list $values(name) $values(value)]
    }
    
    return $result
}

##
# program::listDefinitions
#    @param db
#    @return list of dicts.    Each dict descsribes a specific
#              program and has the following keys:
#              *   id - Primary key of the program.
#              *   name  - Program name.
#              *   path  - Path to program file.
#              *   type  - Textual type
#              *   type_id - Id of the program type.
#              *   host  - Host on which the program should run.
#              *   directory - Directory that will be cwd when the program runs.
#              *   container_id - Id of the container in which the program will run.
#              *   container_name - name of the container in which the program will run.
#              *   initscript - Script that runs before the program runs.
#              *   service   - Name of the service the program will register for
#                              REST communication.
#              *   options - List of option name and value pairs detailing
#                            the program options.
#              *   parameters - List of program parameters.
#              *   environment - List of environment variable, value pairs.
#  @note  - in order to factor out the generation of this dict, we
#           get a list of program names and successifly invoke getdef
#
proc ::program::listDefinitions {db} {
    set names [$db eval {SELECT name FROM program }]
    
    set result [list]
    foreach name $names {
        lappend result [program::getdef $db $name]
    }
    return $result
}
    

##
# program::isActive
#     Determine if a program is in the list of active programs.
#
#  @param name   - Name of the program.
#
proc ::program::isActive {name} {
    return [expr {[lsearch -exact $::program::activePrograms $name] != -1}]
}
##
# program::activeContainers
#    List the active containers.  Note that we can only know the containers we
#    started.  The list is returned as a list of pairs whose first element is a
#    container name and second, the host in which _we_ started that container.
#
# @return list - as described above.
#
proc ::program::activeContainers {} {
    return $::program::activeContainers
}
##
# program::activePrograms
#   Produce a list of active programs.  This list is a simple list of the names
#   of the programs that are currently known to be running.
# @return  list - list of active program names.
#
proc ::program::activePrograms { } {
    return $::program::activePrograms
}
##
# program::run
#   Run a program in its remote host.
#   - Program must exist.
#   - Program must not be active.
#   How the program is run depends on whether or not a container is needed.
# @param db   - Database command
# @param name - Program name.
# @param ?outputHandler? - If present and non-empty a script to handle output from
#                The program.
# @return fd  - File descriptor that was open for program I/O.
#               This helps us in testing.
proc ::program::run {db name {outputHandler {}}} {
    if {[::program::isActive $name]} {
        error "Name is already active."
    }
    set programDef [::program::getdef $db $name]
    
    # How we start a program in or outside of a container is drastically
    # different so figure out which is which and dispatch accordingly:
    
    if {[dict exists $programDef container_name]} {
        set fd [::program::_runInContainer $db $programDef]
    } else {
        set fd [::program::_runBare $db $programDef]
    }
    
    # Book keeping for all of this
        
    lappend ::program::activePrograms $name
    set fds($name) $fd
    
    if {$outputHandler ne ""} {
        set ::program::outputHandlers($name) $outputHandler
    }
    
    #  The fd handler is our output wrapper:
    
    fileevent $fd readable [list ::program::_outputWrapper $name $fd]
    return $fd
}
##
# ::program::kill
#    Force a program to exit.  This is actually a bit tricky.  We need to get
#    the pid of the program from the _remote_ system.  We're going to assume that
#    the command string for each program is unique within the host it runs.
#
#
#    * Form that command string for the program... call it cmdstring
#    * do an ssh $host psaxuww | grep $cmdstring | grep $tcl_platform(user) | grep -v grep
#    * and read the results.  If there are multiple matching lines we just look
#      at the first.
#    * The second list element of the first line is the pid, call it pid
#    * We then ssh localhost kill -9 $pid
#
#   All of this will cause whatever file descriptors are open on the program
#   to be closed and stuff will then rundown quite naturally.
#
# @param db    - database command.
# @param name  - Program name.
#
#  @note it is an error to do this to a program that no longer exists.
#
proc ::program::kill {db name} {
    set def [::program::getdef $db $name]
    set host [dict get $def host]
    
    if {[lsearch -exact $::program::activePrograms $name] == -1} {
        error "$name is not a running program."
    }
    set command [_makeProgramCommand $def]
    
    # DANGER!! you might think the elimination of grep itself from the
    #         output set could be done by piping grep -v grep in the
    #         pipeline below, however if there are no matches, (e.g. the
    #        program exited prior to the check), the grep -v won't have any
    #        matches and grep -v will exit with status 1.  That will cause
    #        exec to throw an error (child process exited abnormally).
    #       The scheme below gives a better error message if that's the case
    #       as well as a potential log useful in troubleshooting.
    #
    set programList [exec ssh -T $host ps axuww | \
        grep "$command" | grep $::tcl_platform(user) \
         ];
    set programList [split $programList "\n"];   #list of lines.
    set programList [::program::_filterGrep $programList]
    if {[llength $programList] == 0} {
        $::program::_log "NO process match for $name"
        error "Attempting ps axuww grepping for $command yielded no matches"
    }
    set program [lindex $programList 0]
    set pid     [lindex $program 1];     # the pid in the remote host.
    catch {exec ssh -T $host kill -9 $pid }
    
}
    

