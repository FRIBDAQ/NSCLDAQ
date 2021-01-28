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
# @file   mg_containerpackage.tcl
# @brief Package for handling containers in the config file.

package provide containers 1.0
package require sqlite3

# @author Ron Fox <fox@nscl.msu.edu>
#

##
# This package provides an api to the container part of the
# NSCLDAQ manager.   The following are API elements:
#   container::add      - Adds a container entry to the database.
#   container::remove   - Remove a container.
#   container::activate - Activates a container in a specific remote host.
#   container::run      - Runs a command in a container
#   container:deactivate - Deactivates a container in a specified host.
#
#  A few notes:
#     - Activation, Deactivation and running a command in the containers
#       makes several assumptions:
#       *  Common file system across hosts (home directory at least).
#       *  ssh keys have been set up so that ssh host won't prompt for a password.
#      - Activation means that a container instance is started in a specific
#        host and is then available for commands to be run in it.
#      - Some temporary files will be dropped into e.g. ~/.daqmanager
#        These files are used to generate init scripts.
#      - Each container is activated with an init script mappedn into
#        /singularity.  The init_script specfied for the container by the user
#        is mapped to /singularity.user
#        Our init script allows the singularity run command to run an
#        arbitrary program.
#


# Establish our namespace.

namespace eval ::container {
    #
    #  Ensures the ~/.daqmanager directory exists:
    #
    variable tempdir [file normalize [file join ~ .daqmanager]]
    
    # Making this a separate proc also allows for testing.
    
    proc _setup {} {
        if {![file isdirectory $::container::tempdir]} {
            if {[file exists $::container::tempdir]} {
                file delete $::container::tempdir;      # in case there's an ordinary file.
            }
            file mkdir $::container::tempdir
        }
    }
    ::container::_setup
}

##
# container::add
#    Add a new container definition to the database:
#
# @param db    - Sqlite3 database command.
# @param name  - Container name.  Note that container wil be activated
#                with that name.
# @param image - The container image file path.
# @param init  - Filename containing the init script.  Note; the script
#                contents are sucked into the database.
# @param mountpoints  list of desired mount points.  Each mount point is
#                a one or two element sublist.  If one element, the element
#                is a filesystem point in the native filesystem that's mounted
#                to the same position in the container (e.g. equivalent to
#                --bind point).  If a two element list, the first element is the
#                native file system point to bind and the second is where to bind
#                it in the container (e.g. --bind point:where).
#
proc ::container::add {db name image init mountpoints} {
    # Duplicate container names are not allowed:
    
    
    if {[$db eval {SELECT COUNT(*) FROM container WHERE container=$name}] != 0} {
        error "There is already a container with the name $name"
    }
    set initContents [list]
    if {[file readable $init] } {
        set fd [open $init r]
        set initContents [read $fd]
        close $fd
    }
    $db transaction {
        $db eval {
            INSERT INTO container (container, image_path, init_script)
                VALUES ($name, $image, $initContents )
        }
        set pkey [$db last_insert_rowid];             # Primary key of inserted record.
        
        foreach binding $mountpoints {
            if {[llength $binding] == 1} {
                $db eval {
                    INSERT INTO bindpoint (container_id, path)
                        VALUES ($pkey, $binding)
                }
            } elseif {[llength $binding] == 2} {
                set path [lindex $binding 0]
                set mountpoint [lindex $binding 1]
                
                $db eval {
                    INSERT INTO bindpoint (container_id, path, mountpoint)
                        VALUES ($pkey, $path, $mountpoint)
                }
                
            } else {
                error "mountpoint: '$binding' must be either 1 or 2 elements"
            }
        }
    };                    # Commit happens here or rollback on error.
}
##
# container::remove
#   Removes a container from the database.  This is currently the only
#   way to edit a container definition, kill it off and replace it.
#   If the container is not redefined, it's the caller's responsibility to
#   ensure that all instances of the container are deactivated.
#
# @param db   - database instance command.
# @param name - name of the container to remove from the database.
# @note - it's not an error for the container name to not exist.
#
proc ::container::remove {db name} {
    # First the bind points with the container_id as name.
    # If name is not a container the subselect will be empty.
    
    $db eval {
        DELETE FROM bindpoint WHERE container_id IN (
            SELECT id FROM container WHERE container = $name
        )
    }
    
    # Now the root record:
    #
    $db eval {
        DELETE FROM container WHERE container = $name
    }
}
##
# container::activate
#   Starts a container off persistent in a remote system with the
#   an initialization script.  We're going to write one or maybe two
#   scripts to $::container::tempdir:  The first will look like:
#
# \verbatim
#     #!/bin/bash
#     if [ -x /singularity.user ]
#     then
#        source /singularity.user
#     fi
#     $@
# \endverbatim
#    This script gets bound to /singularity in the image.
#  The second script is the contents of init_script if it is not an empty
#  string. It is mapped to /singularity.user.
#
#  This allows us to singularity run <container>  command
#  and the command will get the benefit of anything done in the user's
#  container init_scsript.
#
#  These script are written to:
#     $::container::tempdir/container_init_id  where id is the container id.
#  and
#     $::container::tempdir/container_user_id.
#
#
# @param db   - database instance command.
# @param name - Container name (name field of a container)
# @param host - Host in which we want to activate the container.
# @return file-descriptor - open on stdout/stderr of the ssh session running
#               the instance.  The fd will hit an EOF when the container exits
#               or the host shuts down.
#
#  The container is activated with the bindpoints described in the bindpoint
#  table for the specifiec container.
#
proc container::activate {db name host} {
    #  pull the container info from the database.  container::add ensured
    #  there's at most one:
    
    set rootInfo [$db eval {
        SELECT id, container, image_path, init_script FROM container
    }]
    if {[llength $rootInfo] == 0} {
        error "There is no container named $name in the database."
    }
    set id    [lindex $rootInfo 0]
    set name  [lindex $rootInfo 1]
    set image [lindex $rootInfo 2]
    set init  [lindex $rootInfo 3]
    
    # get the bind points:
    
    set from [list]
    set to   [list]
    $db eval {
        SELECT path, mountpoint FROM bindpoint WHERE container_id=$id
    } {
        lappend from $path
        lappend to   $mountpoint
    }
    #  Now create our scripts
    
    set systemScript [file join $::container::tempdir container_init_$id]
    set fd [open $systemScript w]
    puts $fd "#!/bin/bash"
    puts $fd {if [ -r /singularity.user ] }
    puts $fd {then}
    puts $fd "  source /singularity.user"
    puts $fd {fi}
    puts $fd {$@}
    close $fd
    file attributes  $systemScript -permissions 0750
    set scriptbindings "--bind $systemScript:/singularity"
    
    if {$init ne ""} {
        set userScript [file join $::container::tempdir container_user_$id]
        set fd [open $userScript w]
        set init $init;             # Gets rid of the {} around the data.
        puts $fd $init
        close $fd
        file attributes $userScript -permissions 0750
        append scriptbindings , $userScript:/singularity.user
    }
    #  Now pick out all of the bindings and construct an additional
    # --bind option.
    
    set userbindings [list]
    $db eval {
        SELECT path, mountpoint FROM bindpoint WHERE container_id=$id
    } bindpoint {
        if {$bindpoint(mountpoint) eq ""} {
            lappend userbindings $bindpoint(path)
        } else {
            lappend userbindings $bindpoint(path):$bindpoint(mountpoint)
        }
    }
    set fsbindings ""
    if {[llength $userbindings] > 0} {
        set fsbindings "--bind [join $userbindings ,]"
    }
    
    #
    #   Start the container in the remote host using ssh.
    #puts "set fd open |ssh $host singularity instance start $fsbindings $scriptbindings $image $name |& cat r"
    set fd [open "|ssh $host singularity instance start $fsbindings $scriptbindings $image $name |& cat" r]
    return $fd
}
##
#  ::container::run
#     Run a command in a container that's  been activated on some host.
#     If we activated the container ourself, this is gauranteed to work.
#
# @param name     - Container name.
# @param host     - Host the instance is running on.
# @param command  - The command.
# @return fd      - File descriptor to receive command's stdout/stderr.
#
proc ::container::run {name host command} {
    return [open "|ssh $host singularity run instance://$name $command |& cat" w+]
}
##
#  ::container::deactivate
#
#   Stops a container instance that's running in a host.
#
# @param host - the host in which the
# @param name - container name.
#
proc ::container::deactivate {host name} {
    exec ssh $host singularity instance stop $name
}
