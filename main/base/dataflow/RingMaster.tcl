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



#---------------------------------------------------------------------------
#
#  This script manages membership in all the ring buffers. It is responsible
#  for detecting the exit of clients.
#
#  The protocol is very simple ring clients connect and maintain a connection for
#  the life of their execution.  Clients only intereseted in info need not maintain
#  the connection.
# 
#  Once connected the following commands are understood:
#
#   CONNECT ringname connecttype pid comment
#     Indicates the client has connected to the ring 'ringname' with a connection type
#     connecttype that is either 'producer' or 'consumer'.  The client's pid is
#     provided as well as a descriptive comment.
#
#   DISCONNECT ringname connecttype pid
#     Indicates a 'clean' disconnect from a ring. This is done by a destruction of a 
#     CRingBuffer object that was created for the connection. 
#
#   LIST
#
#     Lists information about the ring usage.
#  REGISTER ring
#     Reports the creation of a new ring.
#  UNREGISTER ring
#     Reports the deletion of an existing ring.
#  REMOTE ring
#     Requests ring from the data to be hoisted via a socket.
#
#  On success, CONNECT and DISCONNECT reply with
#    "OK\n"
#   to the caller.
#  LIST replies with
#     "OK\n"
#  Followed by ring usage as described later.
#
#
#   CONNECT and DISCONNECT are only allowed on sockets that are connected from localhost.
#   LIST is allowed from any connection.
#
#  Errors in the protocol close the socket.
#
#  If a socket close is detected, the server releases all un-disconnected ring buffer pointers
#  still owned by the client.
#
#-------------------------------------------------------------------------------


# Additional packages required:

set libdir [file join [file dirname [info script]] .. TclLibs]
set libdir [file normalize $libdir]
set ServiceName "RingMaster"
lappend auto_path $libdir
package require removetcllibpath;   # Probably not a problem except in testing.
package require portAllocator
package require log
package require ring
package require cmdline

set options {
  {f.arg "" "log file path"}
  {v.arg 1 "log level verbosity"}
}

set usage ": RingMaster  \[options\]"

if {[catch {::cmdline::getoptions argv $options $usage} params]} {
  puts $params
  exit
}


#  Locate the directory in which the hoister lives and
#  build a full path for it:

set bindir [file join [file dirname [info script]] .. bin]
set bindir [file normalize $bindir]
set hoisterProgram [file join $bindir ringtostdout]


# Provide the shared memory directory.. This may need to be changed if the code s ported
# to a non linux system:

set shmDirectory [file join / dev shm]

# Global variables/constants:


set localhost   [list "127.0.0.1" "::1"];		# IP address of localhost connections.
set knownRings  [list];			# Registered rings.

array set ::RingUsage [list]

set rotateTime  [expr {60*60*24}];   #rotate time seconds.

#----------------------------------------------------------------------
#
#  Determine if a ring name is a remote proxy.
#
# parameters:
#    name - Name of the remote ring.
# Returns:
#    true if so
#    false if not.
#
# If the host translates we'll assume this is a proxy.
#
proc isRemoteProxy name {
    emitLogMsg debug "isRemoteProxy '$name'"
    set host [file rootname $name]
    set ring [file extension $name]

    # Must have a non-blank extension:

    if {$ring eq ""} {
	return "false"
    }

    if {![catch {exec host $host} ip]} {
	return "true"
    }
    return "false"
}

#-------------------------------------------------------------------------------
#
# Enumerate the existing rings.  The rings are assumed to be in device special
# files located in 'shmdir'.  For each regular file in that directory, we will
# ask for the usage from the ring buffer.  If that fails, then the shared memory
# region is assumed not to be a ring buffer.
# If the ring has a . in it, and the stuff prior to it is a valid
# hostname, We assume that this is a defunct proxy ring and
# remove it.
#
# The rings are enuemrated into the global variable ::knownRings above.
#
proc enumerateRings {} {
    emitLogMsg debug "enumerateRings"
    set files [glob -nocomplain -- [file join $::shmDirectory *]]
    set ::knownRings [list]

    emitLogMsg debug "Initial file list: $files"

    foreach file $files {
      emitLogMsg debug "Trying $file"
      if {[file type $file] eq "file"} {
        emitLogMsg debug "Is ordinary"
        set shmname [file tail $file]
        emitLogMsg debug "ring name: $shmname"
        if {[catch {ringbuffer usage $shmname} data] == 0} {
          emitLogMsg debug "Is a ring"
          # See if this is a defunct proxy ring:
          #
          if {[isRemoteProxy $shmname]} { 
            emitLogMsg info "Deleting remote proxy $file"
            catch {file delete -force -- $file}
          } else {
            emitLogMsg info "Adding preexisting $shmname to list of known ring buffers"
            lappend ::knownRings $shmname
          }
        } else {
          emitLogMsg info "Failed while attempting to read $shmname as a ring buffer"
          puts $data
        }
      }
    }

}


#-------------------------------------------------------------------------------
#
# Start hoisting data to a remote client.  The remote client has specified
# REMOTE ringname.  If the ring exists, we will start an instance of 
# ringtostdout that is pointed at the socket as stdout.  Once started,
# we rundown all the resources associated with the socket and close it.
#
# Parameters:
#   socket    - The socket requesting remote access to the ring data.
#   client    - The IP address of the client.
#   tail      - The command.. should look like REMOTE ringname.
#
proc RemoteHoist {socket client tail} {
    emitLogMsg debug "RemoteHoist $socket $client '$tail'"
  emitLogMsg info "REMOTE request from $client"
    # Ensure client provided a ring:

    if {[llength $tail] != 2} {
	emitLogMsg error "'$tail' is not a valid request"
  puts $socket "ERROR Invalid message format"
	releaseResources $socket $client
	return
    }
    # Ensure the ring exists.
    
    set ringname [lindex $tail 1]
    if {[lsearch -exact $::knownRings $ringname] == -1} {
      set msg "$ringname is not a known ringbuffer. "
      append msg "feeder pipeline cannot be started."
      emitLogMsg error $msg
	puts $socket "ERROR $ringname does not exist"
    } else {
	puts $socket "OK BINARY FOLLOWS"
	exec -- $::hoisterProgram $ringname $client >@ $socket  &
	releaseResources $socket $client
      emitLogMsg info "feeder pipeline started to send data from local ring(=$ringname) to host(=$client)"
    }


}
#------------------------------------------------------------------------------
# Kill clients of a specified ring.  This is done when a ring is being unregistered.
#
#  Parameters:
#    ringName - Name of the ring on which to operate.
#
# NOTE: This is unix specific as it assumes the existence of a 'kill' command that
#       can be execed.
#

proc killClients ringName {
    emitLogMsg debug "killClients $ringName"
    set status [catch {ringbuffer usage $ringName} ringUsage]
    
    # If status is 1 the ring probably doesn't actually exist any more so no way to
    # get client info ...just return.

    if {$status} {
	    return
    }

    # start with the producer:
    
    set producerPID [lindex $ringUsage 3]
    if {$producerPID != -1} {
      emitLogMsg debug "Killing producer: $producerPID"
      catch {exec -- kill -9 $producerPID}
    }
    # Now the consumers:

    set consumerInfo [lindex $ringUsage 6]
    foreach client $consumerInfo {
      set pid [lindex $client 0]
      if {$pid != -1} {;	# Should not need this but...
	    emitLogMsg debug "Killing consumer: $pid"
	    catch {exec -- kill -9 $pid}
	}
    }
}
#-------------------------------------------------------------------------------
#
#  Unregisteran existing ring.  If the ring is known, it is removed from the
#  ::knowRings list.  Otherwise an error messagse is returned to the client.
#  this sort of error is not sufficient to disconnect the client.
#
# Parameters:
#   socket - Socket connected to the client.
#   client - IP address of the client.
#   tail   - the message.
#
proc Unregister {socket client tail} {
    emitLogMsg debug "Unregister $socket $client '$tail'"
    emitLogMsg info "UNREGISTER request from $client"

    if {[llength $tail] != 2} {
	emitLogMsg error "'$tail' is not a valid message"
  puts $socket "ERROR Invalid message format"
	releaseResources $socket $client
	return
    }

    set ring  [lindex $tail 1]
    set which [lsearch -exact $::knownRings $ring]
    if {$which != -1} {
      emitLogMsg info "Killing clients of ring(=$ring)"
      killClients $ring
      emitLogMsg info "Removing ring(=$ring) from list of known rings"
      emitLogMsg debug "Removing $ring from $::knownRings"
      set ::knownRings [lreplace $::knownRings $which $which]
      puts $socket "OK"
    } else {
      emitLogMsg info "Ignoring attempt to unregister ring(=$ring). The ring is already unregistered."
      puts $socket "OK"
    }
}

#-------------------------------------------------------------------------------
#
#   Register a new ring. If the ring is unknown it is added to the
#   ::knownRing list.  If it is known that's an error, but not
#  sufficient to kill the connection to the client.
#
#
# Parameters:
#   socket - Socket connected to the client
#   client - Client ip.
#   tail   - The full message we received.
# 

proc Register {socket client tail} {
    emitLogMsg debug "Register $socket $client '$tail'"
    emitLogMsg info "REGISTER request from $client"

    if {[llength $tail] != 2} {
      emitLogMsg error "'$tail' is not a valid message"
      puts $socket "ERROR Invalid message format"
      releaseResources $socket $client
      return
    }
    set ring [lindex $tail 1]

    if {[lsearch -exact $::knownRings $ring] == -1} {
      if {[catch {ringbuffer usage $ring} msg]} {
        emitLogMsg error "Cannot register ring(=$ring) if associated shared memory segment is corrupt of doesn't exist."
        puts $socket "ERROR $ring shared memory is either corrupt or does not exist"
        return
      }
      emitLogMsg info "Adding ring(=$ring) to list of registered rings"
      emitLogMsg debug "Appending $ring -> $::knownRings"
      lappend ::knownRings $ring
      puts $socket "OK"
    } else {
      # we don't care about duplicate registration. 
      emitLogMsg info "Ignoring duplicate registration attempt for ring(=$ring)"
      puts $socket "OK"
    }

}
#-------------------------------------------------------------------------------
# pruneRings
#    Goes through the list of ringbuffers know to us.
#    for each ring buffer name, determines if the file associated with it
#    still exists.   If it does not:
#    1.  The processes we know of associated with the ring are destroyed.
#    2.  The ring is removed from ::knownRings.
#
proc pruneRings {} {
    set rings $::knownRings

    set deleted [list]
    set clients [list]
    foreach ring $::knownRings {
      set ringFile [file join $::shmDirectory $ring]
    
      if {![file exists $ringFile]} {
        emitLogMsg info "Must delete $ring fing buffer shared memory file missing."
        lappend deleted $ring
        # Can't use killClients because that relies on ring usage
        # which gets information from the shared memory that is no longer
        # there!!. So we iterate over ::RingUsge indices:
        
        foreach index [array names ::RingUsage] {
          set usages $::RingUsage($index)
          foreach usage $usages {
            if {[lindex $usage 0] eq $ring} {
              emitLogMsg info "Must remove client with $usage pid = [lindex $usage 2]"
              lappend $clients $index
              exec -- kill -9 [lindex $usage 2];    # Kill the process.
            }
          }
        }
        
      }
    }
    # The rings we need to remove from ::knownRings are in deleted.
    # the indices whose sockets we need to kill are in clients:
    
    # Release resources associated with the deleted rings:
    
    foreach client $clients {
      set usage $::RingUsage($client)
      releaseResources $client [lindex $usage 4]
    }
    # Remove the rings from the ::knownRings list:
    
    foreach ring $deleted {
      set index [lsearch -exact $::knownRings $ring]
      if {$index != -1} {
        set ::knownRings [lreplace $::knownRings $index $index]
      }
    }
}
    

#-------------------------------------------------------------------------------
#
#  Figure out the known rings.. return them alphabetized.
#
proc listRings {} {
    emitLogMsg debug listRings
    pruneRings
    return [lsort $::knownRings]
}

#--------------------------------------------------------------------------------
# 
# Remove a usage entry from the RingUsage array.  It is a non-error no-op to
# request the removal of a nonexistent item.
#
# Parameters:
#   socket  - The socket from which the entry is removed.
#   name    - Name of the ring buffer.
#   type    - Type of connection.
#   pid     - Process id of the client.
#
proc removeUsageEntry {socket name type pid}  {
  emitLogMsg debug "removeUsageEntry $socket $name $type $pid"

  if {[array names ::RingUsage $socket] ne ""} {
    set usage $::RingUsage($socket)
    emitLogMsg debug "initial usage: $usage"
    set index 0
    foreach item $usage {
      set uname [lindex $item 0]
      set utype [lindex $item 1]
      set upid  [lindex $item 2]
      if {($uname eq $name)      &&
          ($utype eq $type)      &&
          ($upid  eq $pid)} {
        set usage [lreplace $usage $index $index]
        emitLogMsg info "Removing usage entry : $usage"
        emitLogMsg debug "final usage: $usage"
        set ::RingUsage($socket) $usage
        return
      }

      incr index
    }
  }
}

#---------------------------------------------------------------------------------
#
#  The socket must be closed.  If the client on the socket owns any resources,
#  the must be disconnected.
#
#  Parameters:
#    socket - The socket to close
#    host   - The host the socket was connecteD to.
#
proc releaseResources {socket host} {
  emitLogMsg debug "releaseResources $socket $host"

  if {[array names ::RingUsage $socket] ne ""} {
	# There are resources to kill off:

    set usage $::RingUsage($socket)
    set name    [lindex $usage 0 0]
    set type    [lindex $usage 0 1]
    set pid     [lindex $usage 0 2]
    set comment [lindex $usage 0 3]
    emitLogMsg info "Removing usage entry: ring=$name, type=$type, pid=$pid, comment='$comment'"
    unset ::RingUsage($socket)
    emitLogMsg debug $usage
  
    foreach connection $usage {
        set ring   [lindex $connection 0]
        set type   [lindex $connection 1]
  
        if {$type eq "producer"} {
          emitLogMsg debug "Disconnecting producer from ring(=$ring)"
          catch {ringbuffer disconnect producer $ring}
        } else {
          set typeList [split $type .]
          set index [lindex $typeList 1]
          emitLogMsg debug "Disconnecting consumer $index from ring(=$ring)"
          catch {ringbuffer disconnect consumer $ring $index}
        }

    }
  }

  catch {close $socket}
}
#---------------------------------------------------------------------------------
#
#   Process connect messagse.  The form of this message, is:
#   CONNECT rigname connecttype pid comment
#
#   This information is recorded in the socket's entry in the RingUsage array.
# Parameter:
#   socket   - the socket that sent the CONNECT message.
#   client   - IP address of the client.
#   message  - The full message.
#
proc Connect {socket client message} {
    emitLogMsg debug "Connect $socket $client '$message'"
    emitLogMsg info "CONNECT request from $client"

     # The client must be local:

    if {$client ni $::localhost} {
      emitLogMsg error "Ring connection failed. Connections are only allowed from localhost"
      puts $socket "ERROR Ring connections from remote hosts are forbidden"
      releaseResources $socket $client
      return
    }
    #  The message must have a ringname, a connection type a pid and a comment:

    if {[llength $message] != 5} {
      emitLogMsg error "Ring connection failed. '$message' is not a valid connection message"
      puts $socket "ERROR Invalid message format"
      releaseResources $socket $client
      return
    }

    # Pull out the pieces of the message:

    set name     [lindex [lindex $message 1] 0];# Strips the {}'s sent in.
    set type     [lindex $message 2]
    set pid      [lindex $message 3]
    set comment  [lindex $message 4]

    # Just record this:

    lappend ::RingUsage($socket) [list $name $type $pid $comment $client]

    emitLogMsg info "Added usage entry : ring=$name, type=$type, pid=$pid, comment='$comment'"

    puts $socket "OK"
}

#---------------------------------------------------------------------------------
# Disconnect
#   Process a disconnect message.  The form of this message is:
# DISCONNECT ringname client message
#
#  All we do is remove the information about this entry from the
#  socket's entry in the RingUsage array.
# 
# Parameters:
#   socket       - The socket that received the message.
proc Disconnect {socket client message} {
    emitLogMsg debug "Disconnect $socket $client '$message'"
    emitLogMsg info "DISCONNECT request from $client"

    # The client must be local:

    if {$client ni $::localhost} {
	emitLogMsg error  "Ring disconnect failed. Disconnections only allowed from localhost."
	releaseResources $socket $client
  return
    }
    # The message must have a rigname an connection type and a pid.

    if {[llength $message] != 4} {
	emitLogMsg error "Ring disconnection failed. '$message' is not a valid disconnect message"
	releaseResources $socket $client
  return
    }

    # pull out the pieces we need:

    set name    [lindex $message 1]
    set type    [lindex $message 2]
    set pid     [lindex $message 3]

    emitLogMsg debug "Removing entry from $socket : $name $type $pid"

    removeUsageEntry $socket $name $type $pid

    puts $socket "OK"
}
#--------------------------------------------------------------------------------
#
#  Process the LIST command.  This command has the form:
#  LIST
#
#   It returns the string OK\r\n followed by the usage from the
#   ring buffer Tcl command's usage for each known ring buffer.
#
#
# Parameters:
#   socket      - connection to the client.
#   client      - host f the client.
#   message     - Full message text.
proc List {socket client message} {
    emitLogMsg debug "List $socket $client '$message'"
    # The message text can be only the LIST command:
    emitLogMsg info "LIST requested by $client"

    if {$message ne "LIST"} {
      releaseResources $socket $client
      return
    }
    set rings [listRings]
    set result [list]
    foreach ring $rings {
      if {![catch {ringbuffer usage $ring} usage]} {
          lappend result [list $ring $usage]
      } else {
        emitLogMsg warning "Failed while retrieving usage information for registered ring(=$ring)"
      }
    }
    puts $socket "OK"
    puts $socket $result
}


#---------------------------------------------------------------------------------
#
# onMessage
#   called when a socket becomes readable:
#   - If the socket is at eof, cleanup the clients mess.
#   - Otherwise process the client's commands.
#
# Parameters:
#   socket    - The socket open on the client.
#   client    - The client's IP address in dotted form.
#
proc onMessage {socket client} {
  emitLogMsg debug "onMessage $socket $client"
    if {[eof $socket]} {
	emitLogMsg info "Socket connection lost from $client"
	releaseResources $socket $client
	return
    }
    set message [gets $socket]
    emitLogMsg debug "received message '$message'"
    
    # Often we get an empty messages just before the eof ignore those.

    if {$message eq ""} {
	return
    }

    # Process log and acknowledge the client's message.

    set message [string trimright $message];# chop off carriage control chars.
    set command [lindex $message 0]

    if       {$command eq "CONNECT"} {
	Connect $socket $client $message

    } elseif {$command eq "DISCONNECT"} {
	Disconnect $socket $client $message
    } elseif {$command eq "LIST" } {
	List $socket $client $message
    } elseif {$command eq "REGISTER"} {
	Register $socket $client $message
    } elseif {$command eq "UNREGISTER"} {
	Unregister $socket $client $message
    } elseif {$command eq "REMOTE"} {
	RemoteHoist $socket $client $message
    } elseif {$command eq "DEBUG"} {
	# Enable/disable debug logging.
	set state [lindex $message 1]
	::log::lvSuppress debug  $state
    } else {
	# Bad command means close the socket:

	emitLogMsg info "Invalid command $command from $socket closing"
	releaseResources $socket $client
	
    }
    emitLogMsg debug "Processed '$message' from client at $client"
}


#---------------------------------------------------------------------------------
#
#  onConnection:
#    called when a new connection to the serve is received.
#    The client socket is configured with line buffering.
#    A fileevent is established so that when the socket becomes readable,
#    the onMessage is invoked with the client socket as a parameter.
# Parameter:
#   channel    - Channel open on the client socket.
#   clientaddr - The IP address of the host connecting to us.
#                note that the ::localhost variable contains the IP
#                address that a localhost connection will give.
#   clientport - The client's port (not really relevant).
#
proc onConnection {channel clientaddr clientport} {
    emitLogMsg debug "onConnection $channel $clientaddr $clientport"
    emitLogMsg info "New socket connection from $clientaddr"

    # Set the channel to line buffering and establish the handler:

    fconfigure $channel -buffering line
    fileevent  $channel readable [list onMessage $channel $clientaddr]

}

## \brief Enable or disable all logging levels
#
# \param state    state to set (0 = disabled, 1 = enabled)
#
proc setStateOfAllLogLevels {state} {
  if {$state} { 
    set suppress 0
  } else {
    set suppress 1
  }

  foreach level {emergency alert critical error warning notice info debug} {
     ::log::lvSuppress $level $suppress
  }
}

## \brief Set the logging verbosity
#
# \param level  logging level value (0, 1, or 2)
#
# Logging levels imply:
# 0    -  quiet (no logging)
# 1    -  normal (all levels besides debugging)
# 2    -  verbose (all levels)
#
proc setLoggingVerbosity {level} {
  if {$level == 0 } {
    # disable all log levels
    setStateOfAllLogLevels off
  } elseif {$level == 1} {
    setStateOfAllLogLevels on
    ::log::lvSuppress debug 1
  } else {
    setStateOfAllLogLevels on
  }
}


proc emitLogMsg {level msg} {

    set time [clock format [clock seconds]]

    set logMsg "$time\t$msg"

    ::log::log $level $logMsg
}
##
# openWithBackup
#  try to open a file with the given mode (usually a writable mode).
#  If that fails the file is opened in ~
#
# @param path - Path name to file.
# @apram mode - Open mode (defaults to 'w').
# @return file descriptor.
#
proc openWithBackup {path {mode w}} {
  if {[catch {open $path $mode} fd]} {
    ## Failed.
    
    set name [file tail $path]
    set name [file join ~ $name]
    set fd [open $name $mode]
    set path $name
  }
  set ::logFilename $path
  return $fd
}
##
# logCommand
#    This is the command used to by the log system to issue log messages.
#    We use this to support log rotation. We only have one level of rotation:
#    ::logFilename and ::logFilename.1
#   If the atime of the log file currently being used is older than rotateTime ago,
#   the log file is rotated.
#
# @param level - the log level
# @param msg   - the log message.
#
# Global variables:
#    rotateTime   - Number of seconds between rotations.
#    logFilename  - Name of the log file.
#    logFile      - channel on which logging is done.
#
proc logCommand {level msg} {
    #  Check for need to rotate.
    puts "Log: $level : $msg"
    file stat $::logFilename  fInfo
    set now [clock seconds]
    set age [expr {$now - $fInfo(atime)}]
    puts "Logfile is $age Seconds"
    if {$age > $::rotateTime} {
      # Time to rotate the log file:
      
      close $::logFile
      file delete -force ${::logFilename}.1
      file rename -force  $::logFilename ${::logFilename}.1
      file delete -force $::logFilename
      set ::logFile [open $::logFilename w]
    }
    puts $::logFile  "$level -[clock format [clock seconds]] - $msg"
    flush $::logFile
}
#---------------------------------------------------------------------------------
#
# Set up the logging environment
#
# remove old file if it exists
set logFilePath [dict get $params f]
if {$logFilePath eq {}} {
  set logFile stderr
} else {
  set logFile [openWithBackup $logFilePath w+]
}

::log::lvChannelForall $logFile
::log::lvCmdForall logCommand

# First enable all logging levels:

set verbosityLevel [dict get $params v]
setLoggingVerbosity $verbosityLevel

# Setup log writing so that we can rotate.



#
#  Entry point. We get our listen port from the NSCL Port manage.  This 
#  also registers us for lookup by clients.
#
#
set allocator [portAllocator new]
#
#  Entry point. We get our listen port from the NSCL Port manage.  This 
#  also registers us for lookup by clients.
#
#
set allocator [portAllocator new]

set services [$allocator listPorts]
foreach service $services {
  if {[lindex $service 1] eq $ServiceName} {
    puts stderr "The ringmaster is already running!"
    exit -1
  }
}

while {1} {
    if {[catch {set listenPort [$allocator allocatePort $ServiceName]}] == 0} {
	break
    }
    after 1000;			# Retry connection in a second.
}
emitLogMsg debug "PortManager allocated a listen port: $listenPort"

# Establish the log destination:


#  Disable the ones I don't want:

enumerateRings


socket -server onConnection $listenPort

emitLogMsg debug "Server listen established on port $listenPort entering event loop"

set forever 0
vwait forever;				# Start the event loop.

