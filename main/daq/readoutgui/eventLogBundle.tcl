#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file eventLogBundle.tcl
# @brief Callaback bundle that encapsulates event logging.
# @author Ron Fox <fox@nscl.msu.edu>

package provide eventLogBundle 1.0
package provide Experiment     2.0;   # For compatibility with event builder.


package require stageareaValidation

package require Tk
package require DAQParameters
package require RunstateMachine
package require DataSourceManager

package require ExpFileSystem
package require ReadoutGUIPanel
package require Diagnostics
package require ui
package require snit

package require ring

package require portAllocator
package require DataSourceUI
package require versionUtils
package require StateManager
package require dialogwrapper
package require textprompter



##
# @class eventLogBundle
#
#  Provides a callback bundle for the run state machine singleton that manages
#  the event logger.  The main stuff that gets done (all of these only if
#  event logging is turned on):
#
#  * Halted -> Active (leave): Starts the event logger and waits for the logger
#                              start file.
#  * {Paused,Active} -> Halted (leave): Waits for the event logger done files.
#                                       and finalizes the event and ancillary data.
#  * {Paused,Active} -> Halted (enter): Cleans up the logger start and logger
#                                       done files.
#  * {Paused, Active} -> NotReady(enter):
#                                       If the eventlogPID is set force the
#                                       eventlog to exit and finalize the run.
#
#            
#
#  The EventLogger DAQ configuration parameter is used to determine which
#  event logger is started (DaqParameters).

#------------------------------------------------------------------------------
#  Establish the namespace and the namespace variables:

namespace eval ::EventLog {
    
    ##
    # @var loggerPid       - When the event logger is active, this is its pid.
    # @var startupTimeout  - When starting the event logger, this is how long
    #                        to wait in seconds for the startup file to appear.
    # @var shutdownTimeout - When the run ends this is how long to wait in
    #                        seconds for the end file to appear.
    # @var filePollInterval- How long in ms between polls for a file to appear.
    # @varl protectFiles   - If true the finalized event files are protected
    #                        against accidental removal. Used in testing.
    #
    variable loggerPid         -1
    variable startupTimeout    10
    variable shutdownTimeout   30
    variable filePollInterval 100
    variable protectFiles       1
    
    #  For our status line:
    
    variable statusBarManager ""
    variable statusbar         ""
    variable statusUpdateId    -1;     # After Id used to poll for status updates.

    # Installation root:
    #  Assumes we're in a subdirectory of TclLibs relative to the installation
    #  root.
    #
    variable DAQRoot  [file normalize \
        [file join [file dirname [info script]] .. ..]]
    
    ##
    # @var loggerfd       - File descriptor open on the event logger.
    # @var expectingExit  - True if expecting the event logger to exit.
    #                       If not true and we detect an event logger exit we're
    #                       going to kick up a fuss.
    #
    
    variable loggerfd     [list]
    variable expectingExit 0
    variable needFinalization 0
    
    ## 
    # @var failed         - Indicates whether or not the system has failed or succeeded
    variable failed 0
    
    ##
    # incremented when the eventLogger exits...this can be a vwait target:
    
    variable eventLogEnded 0

    #--------------------------------------------------------------------------
    #  Recorded-run lifecycle (issue: run number not advancing).  The design:
    #  * Finalization is keyed on a per-run record (runPhase/pendingRun), never
    #    on whether a logger pid still exists, so an EOF consumed early by
    #    another callout bundle or a provider cannot skip it.
    #  * The event logger child is owned until its termination is CONFIRMED
    #    (non-consuming observation of every pipeline pid) and then reaped by
    #    the single owner: closing the Tcl pipeline channel.  Nothing is
    #    finalized under a writer whose termination is not confirmed.
    #  * Cleanup (filesystem) and advancement (run number) are separate,
    #    retryable phases; the number is advanced exactly once.
    #
    # @var loggerState  - none | running | exited (EOF seen, reaped) |
    #                     killed (killed by us, reaped) | unterminated
    #                     (kill did not confirm; still owned, never finalized).
    # @var loggerExitStatus - "ok" or the error text from [close]
    #                     (CHILDSTATUS/CHILDKILLED) once reaped; else "".
    # @var loggerPids   - Every pid of the owned logger pipeline, captured at
    #                     open.  Observation/kill only ever touch these.
    # @var eofSeen      - EOF was seen on the logger's pipe (the readable
    #                     handler is unregistered at that point).
    # @var runPhase     - none | recording | ending | cleaned | advanced.
    # @var pendingRun   - Run number captured at start for the owned run (-1).
    # @var runOutcome   - "" | complete | incomplete | failed | unknown, with
    #                     runOutcomeReason.  First classification is kept;
    #                     only worsening updates are accepted.
    # @var nextRun      - Run number to advance to (frozen at cleanup).
    # @var unresolvedRun- A run whose finalization FAILED; Begin is refused
    #                     until resolveFailedRun is invoked for that number.
    # @var ending       - runEnding in progress (owner of the lifecycle).
    # @var abortRequested/abortReason - a NotReady transition arrived while
    #                     ending; recorded and completed by the owner.
    # @var waitResult   - vwait target: eof | timeout | poll | abort.
    # @var reapGrace    - seconds to wait for termination confirmation.
    # @var observeProc  - command {pid} -> exited|alive|unknown (injectable).
    # @var killProc     - command {pids} that kills them (injectable).
    # @var interactive  - If true dialogs are shown (when Tk has a main
    #                     window); tests set 0.
    # @var needFinalization - compatibility: derived from runPhase.
    #
    variable loggerState      none
    variable loggerExitStatus ""
    variable loggerStderr     ""
    variable loggerPids       [list]
    variable eofSeen          0
    variable runPhase         none
    variable pendingRun       -1
    variable runOutcome       ""
    variable runOutcomeReason ""
    variable nextRun          -1
    variable unresolvedRun    -1
    variable unresolvedReason ""
    variable ending           0
    variable abortRequested   0
    variable abortReason      ""
    variable waitResult       ""
    variable reapGrace        2
    variable observeProc      ::EventLog::_observeProcfs
    variable killProc         ::EventLog::_killPids
    variable interactive      1
    variable sleepTick        0
    
    # Export the bundle interface methods
    
    namespace export attach enter leave

}

# For compatibility with 10.x event builders...provide shutdownTimeout but
# trace changes to update ::EventLog::shutdownTimeout

namespace eval ::Experiment {
    variable fileWaitTimeout $::EventLog::shutdownTimeout
}


proc ::EventLog::_updateShutdownTimeout {name1 name2 op} {
    set ::EventLog::shutdownTimeout $::Experiment::fileWaitTimeout
}
trace add variable ::Experiment::fileWaitTimeout write ::EventLog::_updateShutdownTimeout 


#------------------------------------------------------------------------------
#
# Data methods:
#

##
# ::EventLog::getPid
#
# @return integer - The PID of the event logger if is active or -1 if it is not.
#
proc ::EventLog::getPid {} {
    return $::EventLog::loggerPid
}
##
# ::EventLog::setStartupTimeout
#
#  Set a new value in seconds for the startup time out.  This is the number
#  of seconds the package will wait for the eventLogger to create its startup
#  file (.started) before declaring a timeout.
#
# @param newTimeout - the new timeout value in seconds
#
proc ::EventLog::setStartupTimeout {newTimeout} {
    set EventLog::startupTimeout $newTimeout
}
##
# ::EventLog::getStartupTimeout
#
# @return int - The current event log start timeout.  See ::EventLog::setStaruptTimeout
#               for a description of that value.
#
proc ::EventLog::getStartupTimeout {} {
    return $::EventLog::startupTimeout
}
##
# ::EventLog::setShutdownTimeout
#
#   Sets the current shutdown timeout to a new value.  See ::EventLog::getShutdownTimeout
#   for more information about just what this is.
#
# @param newTimeout - new value of the timeout.
# #
proc ::EventLog::setShutdownTimeout {newTimeout} {
    set ::EventLog::shutdownTimeout $newTimeout
}

##
# ::EventLog::getShutdownTimeout
#
#  @return int Number of seconds to wait for the event loggers .exited
#              file to appear.
#
proc ::EventLog::getShutdownTimeout {} {
    return $::EventLog::shutdownTimeout
}
##
# ::EventLog::setFilePollInterval
#
#  Set a new value for the file poll interval.  This is how often a check is done
#  for the appearance of an expected file ( e.g. .exiting).
#
# @param newInterval - the new poll interval in milliseonds.
#
proc ::EventLog::setFilePollInterval {newInterval} {
    set ::EventLog::filePollInterval $newInterval
}
##
# ::EventLog::getFilePollInterval
#
# @return int - The current file poll interval.  See ::EventLog::setFilePollInterval
#               for more about what this means.
#
proc ::EventLog::getFilePollInterval {} {
    return $::EventLog::filePollInterval
}
proc ::EventLog::getLoggerPath {} {}

#------------------------------------------------------------------------------
# Utility methods

##
# _getStartFile
#   @return the path to the .started file.
#
proc ::EventLog::_getStartFile {} {
  set run [::EventLog::_currentRun]
  set destDir [::ExpFileSystem::getRunDir $run]
      
  set startFile [file join $destDir .started]
  return $startFile
}
##
# _getExitFile
#    Get path to the current .exited file.
#
proc ::EventLog::_getExitFile {} {
  

  set run [::EventLog::_currentRun]
  set destDir [::ExpFileSystem::getRunDir $run]
  
  set result [file join $destDir .exited]
  return $result

}
#------------------------------------------------------------------------------
#  Recorded-run lifecycle helpers.

##
# _currentRun
#   @return the run number the owned run refers to (captured at start) or,
#           if none is owned, whatever the GUI shows.
proc ::EventLog::_currentRun {} {
    if {$::EventLog::pendingRun != -1} {
        return $::EventLog::pendingRun
    }
    return [::ReadoutGUIPanel::getRun]
}
##
# _log - log to the EventLogManager tab.
proc ::EventLog::_log {severity msg} {
    ::ReadoutGUIPanel::Log EventLogManager $severity $msg
}
##
# _setPhase - set the run phase, keeping the compatibility flag in step.
proc ::EventLog::_setPhase {phase} {
    set ::EventLog::runPhase $phase
    set ::EventLog::needFinalization \
        [expr {$phase in [list recording ending cleaned]}]
}
##
# isRunPending
#   @return true if a recorded run is owned and not yet fully finalized
#           (phase recording, ending or cleaned).  Headless consumers
#           (offline orderer) use this instead of looking at the pid.
proc ::EventLog::isRunPending {} {
    return [expr {$::EventLog::runPhase in [list recording ending cleaned]}]
}
##
# _setOutcome
#   Record the run outcome.  The first classification is kept; only a
#   worsening (complete -> incomplete -> failed/unknown) replaces it.
proc ::EventLog::_setOutcome {outcome reason} {
    set rank [dict create "" 0 complete 1 incomplete 2 failed 3 unknown 3]
    if {[dict get $rank $outcome] >= [dict get $rank $::EventLog::runOutcome]} {
        set ::EventLog::runOutcome       $outcome
        set ::EventLog::runOutcomeReason $reason
    }
    if {$outcome ne "complete"} {
        ::EventLog::_log error "Run $::EventLog::pendingRun $outcome: $reason"
    }
}
##
# _reportError
#   Log an error and, when interactive and a Tk main window exists, show
#   a modal dialog parented to it.  Headless consumers only get the log.
proc ::EventLog::_reportError {title msg} {
    ::EventLog::_log error "$title: $msg"
    if {$::EventLog::interactive && ([info commands tk_messageBox] ne "") \
            && ![catch {winfo exists .} exists] && $exists} {
        tk_messageBox -icon error -type ok -title $title -message $msg -parent .
    }
}
##
# _sleep - sleep ms while keeping the event loop (timers, fileevents) alive.
proc ::EventLog::_sleep {ms} {
    after $ms [list incr ::EventLog::sleepTick]
    vwait ::EventLog::sleepTick
}
##
# _notifyWait
#   Wake _waitForLoggerExit with an informational reason (eof | poll).  Never
#   overwrites an authoritative reason (abort, timeout) already recorded.
proc ::EventLog::_notifyWait {what} {
    if {$::EventLog::waitResult eq ""} {
        set ::EventLog::waitResult $what
    }
}
##
# _observeProcfs
#   Non-consuming termination observation of ONE owned pid via Linux procfs.
#   @return exited  - the process is a zombie/dead (Z or X): the owner's
#                     blocking close will reap it without waiting.
#           alive   - any other state.
#           unknown - not Linux, unreadable, or malformed.  Never treated as
#                     termination.
#   @note Only ever called with pids from loggerPids (our unreaped children),
#         so the pid cannot have been reused.  Readability of /proc/<pid>/stat
#         does not imply ownership; membership is enforced by the caller.
proc ::EventLog::_observeProcfs {pid} {
    if {![string is integer -strict $pid] || ($pid <= 0)} {
        return unknown
    }
    if {[catch {
        set fd [open /proc/$pid/stat r]
        set line [read $fd]
        close $fd
    }]} {
        return unknown
    }
    set idx [string last ")" $line]
    if {$idx < 0} {
        return unknown
    }
    set state [string index [string trim [string range $line [expr {$idx + 1}] end]] 0]
    if {$state eq ""} {
        return unknown
    }
    if {$state in [list Z X]} {
        return exited
    }
    return alive
}
##
# _observeLogger
#   Observe every owned pipeline pid.
#   @return exited only if ALL are exited; unknown if ANY is unknown (or
#           nothing is owned); otherwise alive.
proc ::EventLog::_observeLogger {} {
    if {[llength $::EventLog::loggerPids] == 0} {
        return unknown
    }
    set result exited
    foreach pid $::EventLog::loggerPids {
        set o [$::EventLog::observeProc $pid]
        if {$o eq "unknown"} {
            return unknown
        }
        if {$o eq "alive"} {
            set result alive
        }
    }
    return $result
}
##
# _killPids - default kill implementation (SIGKILL each owned pid).
proc ::EventLog::_killPids {pids} {
    foreach pid $pids {
        catch {exec kill -9 $pid}
    }
}
##
# _reapLogger
#   The single reaping owner.  Precondition: termination of every owned pid
#   has been confirmed (observed exited), so the blocking [close] of the
#   pipeline returns at once and yields the exit status.  Idempotent.
#   @param how - exited | killed
#   @return exit status text ("ok" or the [close] error).
proc ::EventLog::_reapLogger {how} {
    set fd [lindex $::EventLog::loggerFd end]
    if {$fd eq ""} {
        return $::EventLog::loggerExitStatus
    }
    catch {fileevent $fd readable [list]}
    #  [close] on a read pipeline fails whenever the child wrote to stderr,
    #  even on a clean exit (errorCode NONE); only CHILDSTATUS/CHILDKILLED/
    #  CHILDSUSP are real failures.  Keep the stderr text separately.
    set ::EventLog::loggerStderr ""
    if {[catch {close $fd} msg]} {
        set code $::errorCode
        switch -- [lindex $code 0] {
            NONE {
                #  The only benign failure: the child wrote to stderr and
                #  exited with status 0.
                set ::EventLog::loggerStderr [string trim $msg]
                set ::EventLog::loggerExitStatus ok
            }
            CHILDSTATUS {
                set ::EventLog::loggerStderr [string trim $msg]
                set ::EventLog::loggerExitStatus "exit status [lindex $code 2]"
            }
            CHILDKILLED {
                set ::EventLog::loggerStderr [string trim $msg]
                set ::EventLog::loggerExitStatus "killed by [lindex $code 2]"
            }
            CHILDSUSP {
                set ::EventLog::loggerStderr [string trim $msg]
                set ::EventLog::loggerExitStatus "suspended by [lindex $code 2]"
            }
            default {
                #  Anything else (e.g. POSIX ...) is an unexpected close
                #  failure: never declare success for it.
                set ::EventLog::loggerExitStatus "close error ([lindex $code 0]): $msg"
            }
        }
        if {$::EventLog::loggerStderr ne ""} {
            ::EventLog::_log output "Event logger stderr: $::EventLog::loggerStderr"
        }
    } else {
        set ::EventLog::loggerExitStatus ok
    }
    set ::EventLog::loggerFd    [list]
    set ::EventLog::loggerPid   -1
    set ::EventLog::loggerState $how
    return $::EventLog::loggerExitStatus
}
##
# _confirmTerminated
#   Bounded (reapGrace seconds) wait for every owned pid to be observed
#   exited.  @return 1 if confirmed, 0 otherwise.
proc ::EventLog::_confirmTerminated {} {
    set deadline [expr {[clock milliseconds] + $::EventLog::reapGrace*1000}]
    while {1} {
        if {[::EventLog::_observeLogger] eq "exited"} {
            return 1
        }
        if {[clock milliseconds] >= $deadline} {
            return 0
        }
        ::EventLog::_sleep 50
    }
}
##
# _terminateLogger
#   Kill the owned logger, confirm termination and reap it.  If termination
#   cannot be confirmed the logger stays owned in state 'unterminated' and
#   nothing downstream may finalize.
#   @return 1 if reaped, 0 if unterminated.
proc ::EventLog::_terminateLogger {} {
    set fd [lindex $::EventLog::loggerFd end]
    if {$fd ne ""} {
        catch {fileevent $fd readable [list]}
    }
    $::EventLog::killProc $::EventLog::loggerPids
    if {[::EventLog::_confirmTerminated]} {
        ::EventLog::_reapLogger killed
        return 1
    }
    set ::EventLog::loggerState      unterminated
    set ::EventLog::loggerExitStatus "termination not confirmed"
    ::EventLog::_log error "Event logger (pids $::EventLog::loggerPids) could not be \
confirmed terminated within $::EventLog::reapGrace s; it remains owned and the run is not finalized"
    return 0
}
##
# _waitForLoggerExit
#   Wait, bounded by shutdownTimeout, for the running logger to exit.
#   Wake-ups: eof (readable handler reaped it), poll (EOF was seen while the
#   child was still alive: observe again), timeout, abort (a NotReady
#   transition arrived while we own the ending).  On timeout/abort the
#   logger is killed and its termination confirmed before returning.
#   On return loggerState is exited, killed or unterminated.
proc ::EventLog::_waitForLoggerExit {} {
    set ::EventLog::waitResult ""
    set timer [after [expr {$::EventLog::shutdownTimeout*1000}] \
                   [list set ::EventLog::waitResult timeout]]
    set poll ""
    while {($::EventLog::loggerState eq "running") && ($::EventLog::waitResult eq "") \
            && !$::EventLog::abortRequested} {
        if {$::EventLog::eofSeen && ($poll eq "")} {
            set poll [after 200 [list ::EventLog::_notifyWait poll]]
        }
        vwait ::EventLog::waitResult
        if {$::EventLog::waitResult eq "poll"} {
            set ::EventLog::waitResult ""
            set poll ""
            if {[::EventLog::_observeLogger] eq "exited"} {
                ::EventLog::_reapLogger exited
                incr ::EventLog::eventLogEnded
            }
        }
    }
    after cancel $timer
    if {$poll ne ""} {
        after cancel $poll
    }
    #  abortRequested is authoritative: an abort recorded by a nested
    #  transition is honoured even if an eof/poll wake-up was serviced in
    #  between (those never overwrite it, see _notifyWait).
    if {$::EventLog::abortRequested} {
        ::EventLog::_setOutcome incomplete \
            "aborted while waiting for the event logger to exit ($::EventLog::abortReason)"
    }
    if {$::EventLog::loggerState eq "running"} {
        if {$::EventLog::waitResult eq "timeout"} {
            ::EventLog::_setOutcome incomplete "timed out after $::EventLog::shutdownTimeout s \
waiting for the event logger to exit; it was killed"
            ::EventLog::_reportError "Run $::EventLog::pendingRun incomplete" \
                "Timed out after $::EventLog::shutdownTimeout seconds waiting for the event logger \
to exit; it is being killed and the run will be finalized as incomplete."
        }
        ::EventLog::_terminateLogger
    }
}
##
# _beginBarrier
#   Pure (no Tk) check of whether a new run may start.
#   @return "" if it may, else the reason.
proc ::EventLog::_beginBarrier {} {
    if {$::EventLog::ending} {
        return "Run $::EventLog::pendingRun is still being finalized; wait for that to finish before starting a run"
    }
    if {[::EventLog::isRunPending]} {
        return "Run $::EventLog::pendingRun has not been finalized (phase $::EventLog::runPhase,\
 outcome '$::EventLog::runOutcome'); it must be finalized before a new run can start"
    }
    if {$::EventLog::unresolvedRun != -1} {
        return "Run $::EventLog::unresolvedRun could not be finalized: $::EventLog::unresolvedReason.\
  Resolve it (::EventLog::resolveFailedRun $::EventLog::unresolvedRun) before starting a new run"
    }
    return ""
}
##
# resolveFailedRun
#   Guarded operator resolution of a run whose finalization failed.  The run
#   number must name the unresolved run.
proc ::EventLog::resolveFailedRun {run} {
    if {$::EventLog::unresolvedRun == -1} {
        error "There is no unresolved run"
    }
    if {$run != $::EventLog::unresolvedRun} {
        error "Run $run is not the unresolved run ($::EventLog::unresolvedRun)"
    }
    ::EventLog::_log output "Run $run (failed: $::EventLog::unresolvedReason) marked resolved by the operator at [clock format [clock seconds]]"
    set ::EventLog::unresolvedRun    -1
    set ::EventLog::unresolvedReason ""
}
##
# _promptResolveFailedRun
#   Optional guarded GUI resolution: only when interactive with a Tk main
#   window; asks a yes/no question naming the run.  Headless: never prompts.
#   @return 1 if the run was resolved.
proc ::EventLog::_promptResolveFailedRun {} {
    if {$::EventLog::unresolvedRun == -1} {
        return 0
    }
    if {!$::EventLog::interactive || ([info commands tk_messageBox] eq "") \
            || [catch {winfo exists .} exists] || !$exists} {
        return 0
    }
    set run $::EventLog::unresolvedRun
    set answer [tk_messageBox -type yesno -icon warning -parent . \
        -title "Unresolved run $run" \
        -message "Run $run could not be finalized: $::EventLog::unresolvedReason\n\nMark run $run as resolved and allow a new run to start?"]
    if {$answer eq "yes"} {
        ::EventLog::resolveFailedRun $run
        return 1
    }
    return 0
}
#
# ::EventLog::_extractEventLogVersion
#
# Given a path containing a path name, this will extract a version
# number
#
proc ::EventLog::_getLoggerVersion {evtlogpath} {

  # Open a pipe to read from
  set pipe [open "|$evtlogpath --version" r]
  chan configure $pipe -buffering line

  # enable blocking because I want to make sure that I get
  # the value immediately and don't proceed otherwise.
  chan configure $pipe -blocking on

  set line [read $pipe]
  if { [string equal $line ""] } {
    error "Cannot determine eventlog version"
  } else {
    if {[catch {close $pipe} msg]} {
      puts stderr "Exceptional exit of eventlog : $msg"
    }
  }

  # Trim off the newline and whitespace at the end
  set line [string trim $line "\n "]


  set splitLine [split $line { }]
  if {[llength $splitLine] < 2 && ([lindex $splitLine 0] ne "EventLog")} {
    error "eventlog --version returned something different from \"EventLog VSN#\" : \"$splitLine\""
  }
  return [lindex $splitLine 1]
}


##
# ::EventLog::_computeLoggerSwitches
#
# @param loggerVersion the version of the eventlog program
#
# @return the command line options the logger should use:
#
proc ::EventLog::_computeLoggerSwitches {{loggerVersion 1.0}} {
    
    # Base switches:
    
    set ring   [DAQParameters::getEventLoggerRing] 

    # Compatibility with 10.x:

    if {[info proc ::Experiment::spectrodaqURL] ne ""} {
      set ring [::Experiment::spectrodaqURL localhost]
    }
  
    # These are the initial switches to use
    set switches "--source=$ring --oneshot"

    if {[::DAQParameters::getUseChecksumFlag]} {
      # Check that the logger in use returns a version that is greater
      # than or equal to 11.0. This is equivalent to 11.0 <= loggerVersion
      # as is actually computed
      set minVersion 11.0-rc6 
      set parsedVersion [::versionUtils::parseVersion $loggerVersion]
      set parsedMinVersion [::versionUtils::parseVersion $minVersion]
      if {[::versionUtils::lessThan $parsedMinVersion $parsedVersion]} {
        append switches " --checksum"
      } else {
        return -code 1 \
               "The selected version of eventlog\
                does not support the --checksum option! Go to\
                the Settings > Event Recording and deselect\
                the \"Compute checksum\" option"
      }
    }
    
    # If requested, use the --number-of-sources switch:
    
    if {[DAQParameters::getUseNsrcsFlag]} {
        set sm [DataSourcemanagerSingleton %AUTO%]
        set mySources [llength [$sm sources]]
        set adtlSources [DAQParameters::getAdditionalSourceCount]
        set totsrc [expr {$mySources + $adtlSources}]
        $sm destroy
        append switches " --number-of-sources=$totsrc"
    }
    
    #  If the event directory exists, that's an error.
    #  Create the event directory and set it to be where
    #  eventlog writes it's segments.
    

    set run [ReadoutGUIPanel::getRun]
    set destDir [::ExpFileSystem::getRunDir $run]
    if {[file exists $destDir]} {
      return -code 1 \
       "The directory for this run: $destDir already exists."
        
    }
    # In order to create the directory, we'll need to
    # set the permissions of the directory above it
    # to allow write access by us... we'll save the current
    # perms and restore them when done:
    
    set expDir [file dirname $destDir]
    set originalPerms [file attributes $expDir -permissions]
    file attributes $expDir -permissions u=rwx,g=rx
    file mkdir $destDir
    file attributes $expDir -permissions $originalPerms
    
    append switches " --path=$destDir"

    # If requested, get the run number from the
    # GUI rather than the event segments.
    
    if {[DAQParameters::getRunNumberOverrideFlag]} {
        set run [::ReadoutGUIPanel::getRun]
        append switches " --run=$run"
    }
    
    #  Set the segment size:
    
    append switches " --segmentsize=[DAQParameters::getEventLoggerFileSegmentSize]g"
    
    # Set the --prefix flag  
    #    append switches " --prefix=[::DAQParameters::getRunFilePrefix]"

    return $switches
}

##
# ::EventLog::_startLogger
#  Start the event logger and set it's pid in the loggerPid variable.
#  @note The event logger is started as a pipeline open on an fd for read.
#        We will establish a file readable handler for the event logger so that
#        can relay input to the output log windows and throw up an error dialog
#        if the fd closes unexpectedly.
#
proc ::EventLog::_startLogger {} {
    ReadoutGUIPanel::isRecording
    set logger [DAQParameters::getEventLogger] 
    set loggerVsn [::EventLog::_getLoggerVersion $logger]
    set destDir [::ExpFileSystem::getRunDir [::EventLog::_currentRun]]
    set existedBefore [file exists $destDir]
    set switches [::EventLog::_computeLoggerSwitches $loggerVsn];  # creates destDir
    
    if {[catch {open "| $logger $switches" r} fd]} {
        #  Pre-spawn failure.  Roll back only a run directory this attempt
        #  created and that is still empty; anything else is left for the
        #  stagearea precheck to report.  (Open failing is not proof that no
        #  process launched, which is why only an empty directory is removed.)
        if {!$existedBefore && [file isdirectory $destDir]} {
            set contents [glob -nocomplain -directory $destDir -tails * .*]
            set contents [lsearch -all -inline -not -regexp $contents {^\.\.?$}]
            if {[llength $contents] == 0} {
                catch {
                    set expDir [file dirname $destDir]
                    set originalPerms [file attributes $expDir -permissions]
                    file attributes $expDir -permissions u=rwx,g=rx
                    file delete $destDir
                    file attributes $expDir -permissions $originalPerms
                }
            }
        }
        error "Could not start the event logger '$logger': $fd"
    }
    #  Ownership is captured here, before anything else can fail.
    set ::EventLog::loggerFd         $fd
    set ::EventLog::loggerPid        [pid $fd]
    set ::EventLog::loggerPids       [pid $fd]
    set ::EventLog::loggerState      running
    set ::EventLog::loggerExitStatus ""
    set ::EventLog::eofSeen          0
    set ::EventLog::waitResult       ""
    ::EventLog::_setPhase recording
    
    set fd [lindex $::EventLog::loggerFd end]
    fconfigure $fd -buffering line
    fileevent $fd readable ::EventLog::_handleInput
}
##
# ::EventLog::_handleInput
#    - If input comes in, read it and log it to the console window.
#    - If there's an EOF on input and it's unexpected, throw up an error
#      that the event logger looks like it unexpectedly exited.
#    - Either way on EOF, mark the logger exited and close the File descriptor.
#      setting the variable to [list]
#
proc ::EventLog::_handleInput {} {
    set fd [lindex $::EventLog::loggerFd end]
    if {$fd eq ""} {
        return;                       # Stale callback; the logger was reaped.
    }
    if {[eof $fd]} {
        #  Unregister first so a persistent EOF can never spin the event loop.
        fileevent $fd readable [list]
        set ::EventLog::eofSeen 1
        
        #  EOF is not termination: confirm every owned pid is gone before the
        #  owner reaps.  A child that closed its output but is still running
        #  stays owned; runEnding's wait polls for it (and kills on timeout).
        if {[::EventLog::_observeLogger] ne "exited"} {
            ::EventLog::_log warning "The event logger closed its output but has not exited \
(pids $::EventLog::loggerPids); waiting for it to exit"
            ::EventLog::_notifyWait poll;      # Arm observation if the owner is waiting.
            return
        }
        # Need to close off the fd before the pop up shows as that will
        # re-enter the event loop.  _reapLogger is the single reaping owner.
        
        set msg [::EventLog::_reapLogger exited]

        # Log to the output window and pop up and error.  It's ok to exit
        # at this time if we're expecting it or if the pending state is halted.
        # This covers the idea that the end run passed through the eventlogger
        # before we got a chance to expect it to here.

        if {!$::EventLog::expectingExit && ($::Pending::pendingState ne "Halted")} {
	    # We may already be halted in which case the source exit is
	    # expected, check this before issuing error message (issue #274):
	    set sm [::RunstateMachineSingleton %AUTO%]
	    set currentState [$sm getState]
	    $sm destroy
	    if {$::Pending::pendingState ne "None" || $currentState ne "Halted"} {
		::ReadoutGUIPanel::Log EventLogManager error "Unexpected event log error! $msg pending $::Pending::pendingState current $currentState"
		::Diagnostics::Error {The event logger exited unexpectedly check EventLogManager tab for errors.!!}
	    }
        } else {
            #  Finalization is deliberately NOT done here: this handler can run
            #  inside an event loop serviced by an earlier callout bundle or a
            #  data source provider before our enter method runs.  runEnding
            #  finalizes the owned run whether or not the EOF got here first.
        }
        incr ::EventLog::eventLogEnded
        ::EventLog::_notifyWait eof
    } else {
        set line [gets $fd]
        ::ReadoutGUIPanel::Log EventLogManager output $line
    }
}
##
# Wait for the appearance of a file.  The event logger uses . files to synchronize
# with us about the occurence of various events.
#
# @param name - Name of the file to wait for.
# @param waitTimeout - Number of seconds to wait for the file to appear.
# @param pollInterval - Number of ms between checks for the file.
#
# @return bool - true if the file appeared prior to the timeout. false if not.
#
proc ::EventLog::_waitForFile {name waitTimeout pollInterval} {
    set waitTimeoutMs [expr {$waitTimeout * 1000}];   # Wait timeout in milliseconds
    while {$waitTimeoutMs > 0} {
        if {[file exists $name]} {
            return 1
        }
        incr waitTimeoutMs -$pollInterval
        after $pollInterval
#	update idletasks;			# keep the event loop semi-live. TODO: Deactivate buttons.
    }
    return 0
}
##
#  _finalizeRun
#
#  Finalizes a run   This means:
#  * Creating a new run directory.
#  * mv-ing the event files in to this new run directory.
#  * Making symlinks for each event file segment in complete directory (event view).
#  * Copying the metadata into the new run directory.
#
# @note: daqdev/NSCLDAQ#1033 - Event files written with this bundle
#        Go directly to the exeriment/runmmm directory.
#        All we need to do is:
#        1.   Destroy the links to event segments.
#        2.   Do the copy.
#        3.   Make links in the complete dir that point to the run dir.
#
proc ::EventLog::_finalizeRun {} {
    if {$::EventLog::runPhase ni [list recording ending]} {
        return;                              # Nothing to clean up (idempotent).
    }
    if {$::EventLog::loggerState ni [list exited killed]} {
        error "BUG: _finalizeRun called while the event logger is '$::EventLog::loggerState'"
    }
    set srcdir [::ExpFileSystem::getCurrentRunDir]
    set completeDir [::ExpFileSystem::getCompleteEventfileDir]
    set run $::EventLog::pendingRun
    set destDir [::ExpFileSystem::getRunDir $run]
    
    #  IF the run dir does not exist there's a real problem here.  The number
    #  is still consumed (it was handed to a logger) but the outcome is FAILED
    #  and stays unresolved until the operator resolves it (Begin barrier).
    
    if {![file isdirectory $destDir]} {
        set msg "The run directory $destDir which should hold the event files \
for $run either does not exist or is not a directory"
        ::EventLog::_setOutcome failed $msg
        set ::EventLog::unresolvedRun    $run
        set ::EventLog::unresolvedReason $msg
        set ::EventLog::nextRun [expr {$run + 1}]
        ::EventLog::_setPhase cleaned
        ::EventLog::_reportError "Error no event directory" $msg
        return
    }
    #  The filesystem work below is idempotent; if it throws the phase stays
    #  'ending' with the outcome preserved so that a retry redoes only this.
    
    if {[catch {
        #  Remove any links to event files in the srcdir
        
        set  fileBaseName [::ExpFileSystem::genEventfileBasename $run]
        set  eventFiles [glob -nocomplain [file join $srcdir ${fileBaseName}*.evt]]
        foreach file $eventFiles {
          if {[catch {file delete -force $file} msg]} {
            ::ReadoutGUIPanel::Log EventLogManager warning "Unable to remove link $file : $msg"
          }
        }
        # Now we make links for all event files (.evt) files in the event directory
        # in the complete directory:
        
        set perms [file attributes $completeDir -permissions];    # Must set complete
        file attributes $completeDir -permissions u+w;            # writeable.
        
        set eventFiles [glob -nocomplain [file join $destDir ${fileBaseName}*.evt]]
        foreach file $eventFiles {
          set linkName [file join $completeDir [file tail $file]]
          if {[catch {exec ln -sr $file $linkName} msg]} { ;   # Want to force relative.
            puts stderr "Could not link $linkName -> $file : $msg"
          }
        }
        file attributes $completeDir -permissions $perms; # Restor prior perms.
        
        #  Now what's left gets recursively/link-followed copied to the destDir
        #  using tar.
        
        set tarcmd "(cd $srcdir; tar chf - .) | (cd $destDir; tar --warning=no-timestamp -xpf -)"
        set tarStatus [catch {exec sh << $tarcmd} msg]
        if {$tarStatus} {
            #  Existing policy: warn, the number is still consumed; the markers
            #  and protection are left alone so the operator can inspect.
            ::EventLog::_setOutcome incomplete "copy of files from $srcdir to $destDir failed: $msg"
            ::EventLog::_reportError {Tar Failed} \
                "Copy of files from $srcdir to $destDir failed: $msg, Fix problem and move files manually."
        } else {
            #  Kill off the start file and exitfiles:
            
            ::EventLog::deleteStartFile
            ::EventLog::deleteExitFile
            
            # If required, protect the files:
            #   - The destDir is set to 0555
            #   - The parent dir is set to 0555.
            #   - A chmod -R is done to set the contents to 0x555 as well.
            
            if {$::EventLog::protectFiles} {
                set files [glob -nocomplain -directory $destDir -types {f d} *]
                if {[llength $files]>0} {
                  exec sh << "chmod -R 0555 $files"
                  file attributes $destDir -permissions 0555
                  file attributes [file join $destDir ..] -permissions 0555
                }
            }
        }
    } msg]} {
        set trace $::errorInfo
        ::EventLog::_log error "Cleanup of run $run failed and will be retried: $msg"
        return -code error -errorinfo $trace "Cleanup of run $run failed: $msg"
    }
    if {$::EventLog::runOutcome eq ""} {
        ::EventLog::_setOutcome complete ""
    }
    set ::EventLog::nextRun [expr {$run + 1}]
    ::EventLog::_setPhase cleaned
}
##
# ::EventLog::_advanceRun
#   Advancement phase: cleaned -> advanced by setting the run number to the
#   frozen nextRun.  Exactly once; a failure keeps the phase 'cleaned' with
#   the identity and outcome intact so that a retry only repeats this step,
#   and is reported as an error (never a silent success).
#
proc ::EventLog::_advanceRun {} {
    if {$::EventLog::runPhase ne "cleaned"} {
        return
    }
    set run  $::EventLog::pendingRun
    set next $::EventLog::nextRun
    if {[catch {::ReadoutGUIPanel::setRun $next} msg]} {
        set trace $::errorInfo
        ::EventLog::_log error "Could not advance the run number to $next after run $run: $msg; \
advancement is still pending"
        return -code error -errorinfo $trace "Could not advance the run number to $next: $msg"
    }
    ::EventLog::_setPhase advanced
    set reason ""
    if {$::EventLog::runOutcomeReason ne ""} {
        set reason ": $::EventLog::runOutcomeReason"
    }
    ::EventLog::_log output "Run $run finalized ($::EventLog::runOutcome$reason); run number advanced to $next"
    set ::EventLog::pendingRun -1
    set ::EventLog::nextRun    -1
}
##
# ::EventLog::_getSegmentInfo
#
#   Looks at the current event file areas to see how many segments there are
#   and how much total space that consumes.
#
# @return list first element is the number of event segments found, the second
#              the total Mbytes of storage used.
# @note If there are segments that don't have links in the current dir,
#       they are created at this time.
#
proc ::EventLog::_getSegmentInfo {} {
    set currentDir [ExpFileSystem::getCurrentRunDir]
    set run [::ReadoutGUIPanel::getRun]
    set eventDir [::ExpFileSystem::getRunDir $run]
    set  fileBaseName [::ExpFileSystem::genEventfileBasename $run]
    
    # Which files exist:
    
    set segments [glob -nocomplain \
          [file join $eventDir $fileBaseName*.evt]]
        
            
   
    set    nsegments [llength $segments]
    set size 0;             # For when there are no segments yet.
    foreach segment $segments {
        if {![catch {file size $segment} segsize]} {
          set size [expr {$size + $segsize/1024.0}]
        }
        # Do I need to make a new link in the current dir:
        
        set baseName [file tail $segment]
        set linkName [file join $currentDir $baseName]
        if {![file exists $linkName]} {

          catch {exec ln -sr $segment $linkName}
        }
    }
    set size [format %.2f [expr {$size/1024.0}]]
    
    
    return [list $nsegments $size]
}

##
# ::EventLog::_setStatusLine
#
#   Manage the data in the event logger status line:
#
# @param repeatInterval - ms after which to schedule an update.
#
proc ::EventLog::_setStatusLine repeatInterval {
    set run [::ReadoutGUIPanel::getRun]
    
    set fileinfo [::EventLog::_getSegmentInfo]
    
    $::EventLog::statusBarManager setMessage $::EventLog::statusbar \
        "Recording data for Run: $run : \
[lindex $fileinfo 0] segments found totalling [lindex $fileinfo 1] Mbytes"
    
    set ::EventLog::statusUpdateId \
        [after $repeatInterval [list ::EventLog::_setStatusLine $repeatInterval]]
}

##
#   Check whether or not the .started file lives in the 
#   experiment/current directory
#
proc ::EventLog::_dotStartedExists {} {
  set currentPath [::ExpFileSystem::getCurrentRunDir]
  return [file exists [::EventLog::_getStartFile]]
}

##
#   Check whether or not the .exited file lives in the 
#   experiment/current directory
#
proc ::EventLog::_dotExitedExists {} {
  
  return [file exists [::EventLog::_getExitFile]]
}

##
#   Check whether or not .evt files exist in the 
#   experiment/current directory
#
#   @returns boolean indicating whether there are any files ending in .evt
#
proc ::EventLog::_runFilesExistInCurrent {} {
  set currentPath [::ExpFileSystem::getCurrentRunDir]
  set evtFiles [glob -directory $currentPath -nocomplain *.evt]
  return [expr {[llength $evtFiles] > 0} ]
}

##
# ::EventLog::listIdentifiableProblems
#
# Checks for a few things:
# 1. experiment/run# directory already exists
# 2. experiment/current/*.evt files exist
# 
#
# @returns a list of error messages
proc ::EventLog::listIdentifiableProblems {} {

  set errors [list]

  # check if run directory exist!
  set msg [EventLog::_duplicateRun]
  if {$msg ne ""} {
    lappend errors $msg
  } 
  
  # check if experiment/current/*.evt files exist
  if {[::EventLog::_runFilesExistInCurrent]} {
    set msg    "EventLog error: the experiment/current directory contains run "
    append msg "segments and needs to be cleaned."
    lappend errors $msg
  }

  return $errors
}
##
# correctFixableProblems
#
#  Some startup issues can be corrected:
# 1. experiment/current/.started exists
# 2. experiment/current/.exited exists
#
#  In this case these files are just deleted.
#
proc ::EventLog::correctFixableProblems {} {
  # check if experiment/current/.started exists
  if {[::EventLog::_dotStartedExists]} {
    ::EventLog::deleteStartFile

  } 
  
  # check if experiment/current/.exited exists
  if {[::EventLog::_dotExitedExists]} {
    ::EventLog::deleteExitFile
  } 
    
}

##
# deleteStartFile
#   Kill off the .started file.
#
proc ::EventLog::deleteStartFile {} {
    set startFile [::EventLog::_getStartFile]
    set dirname [file dirname $startFile]
    set oldPerms [file attributes $dirname -permissions]
    file attributes $dirname -permissions u=rwx
    file delete -force $startFile
    file attributes $dirname -permissions $oldPerms
    
}
##
# deleteExitFile
#   Delete the .exited file
#
proc ::EventLog::deleteExitFile {} {
    
    set exitFile [::EventLog::_getExitFile]
    set dirname [file dirname $exitFile]
    set oldPerms [file attributes $dirname -permissions]
    file attributes $dirname -permissions u=rwx
    file delete -force $exitFile 
    
    file attributes $dirname -permissions $oldPerms
}
#------------------------------------------------------------------------------
# Actions:

##
# ::EventLog::runStarting
#
#   Called when the run is about to start:
#   * Ensure we are in the correct cwd for the event logger (from the daq filesystem).
#   * Start the event logger.
#   * Wait for the .started file to appear.
#
proc ::EventLog::runStarting {} {

  set barrier [::EventLog::_beginBarrier]
  if {$barrier ne ""} {
    error $barrier
  }
  #  Defensive only: the barrier above refuses while a run is owned.
  if {$::EventLog::loggerState eq "running"} {
    ::EventLog::_log warning "An event logger from a previous run is still running; killing it"
    ::EventLog::_terminateLogger
  }

  # Now if desired start the new run.
  ::StageareaValidation::correctAndValidate

  if {[::ReadoutGUIPanel::recordData]} {
    #  Capture the identity of the run now; _startLogger takes ownership of
    #  the logger (phase 'recording') as soon as it is spawned.
    
    set ::EventLog::pendingRun       [::ReadoutGUIPanel::getRun]
    set ::EventLog::runOutcome       ""
    set ::EventLog::runOutcomeReason ""
    set ::EventLog::nextRun          -1
    set ::EventLog::abortRequested   0
    set ::EventLog::abortReason      ""
        
    set startFile [::EventLog::_getStartFile]

    ::EventLog::_startLogger
    ::EventLog::_waitForFile $startFile $::EventLog::startupTimeout \
                                        $::EventLog::filePollInterval
    ::StageareaValidation::deleteStartFile
    set ::EventLog::expectingExit 0
    ::EventLog::_setStatusLine 2000
  }
}
##
##
# ::EventLog::runEnding
#
#  Finalize the owned recorded run, if any.  Owner of the ending lifecycle:
#  a re-entrant call (nested transition) is ignored; a NotReady arriving
#  while we run is recorded by enter and completed here.
#  Phases: recording/ending -> (wait/confirm/classify, cleanup) -> cleaned
#          -> (advance the run number) -> advanced.
#  Errors (termination not confirmed, cleanup or advancement failure) are
#  thrown after the state has been recorded so that the transition fails
#  visibly; the run stays owned and the next call retries only what is left.
#
proc ::EventLog::runEnding {} {
    if {![::EventLog::isRunPending]} {
        ReadoutGUIPanel::normalColors
        return
    }
    if {$::EventLog::ending} {
        ::EventLog::_log warning "runEnding re-entered for run $::EventLog::pendingRun; ignored (the outer call owns it)"
        return
    }
    set ::EventLog::ending 1
    set status [catch {::EventLog::_runEnding} msg]
    set trace $::errorInfo
    set ::EventLog::ending 0
    ReadoutGUIPanel::normalColors
    if {$status} {
        return -code error -errorinfo $trace $msg
    }
}
##
# _runEnding - body of runEnding (see there).
#
proc ::EventLog::_runEnding {} {
    set run $::EventLog::pendingRun
    set ::EventLog::expectingExit 1
    if {$::EventLog::runPhase eq "recording"} {
        ::EventLog::_setPhase ending
    }
    if {$::EventLog::runPhase eq "ending"} {
        #  1. The writer must be gone.  An abort already recorded (sources
        #     stopped without an end, or a nested NotReady) means no END_RUN
        #     will come: kill now under our ownership instead of waiting.
        #     Otherwise wait (bounded).  Retry the kill if a previous attempt
        #     could not confirm termination.
        
        if {$::EventLog::abortRequested && ($::EventLog::loggerState eq "running")} {
            ::EventLog::_setOutcome incomplete \
                "run aborted ($::EventLog::abortReason) before the event logger could see the end of run"
            ::EventLog::_terminateLogger
        }
        if {$::EventLog::loggerState eq "running"} {
            ::EventLog::_waitForLoggerExit
        }
        if {$::EventLog::loggerState eq "unterminated"} {
            ::EventLog::_terminateLogger
        }
        if {$::EventLog::loggerState ni [list exited killed]} {
            ::EventLog::_setOutcome unknown \
                "the event logger's termination is not confirmed (state $::EventLog::loggerState)"
            ::EventLog::_reportError "Run $run: recovery required" \
                "The event logger for run $run could not be confirmed terminated; the run is not \
finalized and no new run can start until this is resolved."
            error "Run $run cannot be finalized: event logger termination not confirmed"
        }
        
        #  A previous attempt that could not confirm termination left the
        #  outcome 'unknown'; termination is confirmed now, so classify afresh.
        
        if {$::EventLog::runOutcome eq "unknown"} {
            set ::EventLog::runOutcome       ""
            set ::EventLog::runOutcomeReason ""
        }
        
        #  2. Classify once.  Only a clean exit is expected to leave the
        #     .exited marker; wait for it only in that case.
        
        if {$::EventLog::runOutcome eq ""} {
            if {$::EventLog::loggerState eq "killed"} {
                ::EventLog::_setOutcome incomplete "the event logger was killed"
            } elseif {$::EventLog::loggerExitStatus ne "ok"} {
                ::EventLog::_setOutcome incomplete \
                    "the event logger exited abnormally ($::EventLog::loggerExitStatus): $::EventLog::loggerStderr"
            } elseif {[string match "*Timed out with*ends still not seen*" $::EventLog::loggerStderr]} {
                #  eventlog's own diagnostic for ending on its data timeout
                #  with END_RUN items missing (eventlogMain.cpp).
                ::EventLog::_setOutcome incomplete \
                    "the event logger reported: $::EventLog::loggerStderr"
            } elseif {![::EventLog::_waitForFile [::EventLog::_getExitFile] \
                        $::EventLog::shutdownTimeout $::EventLog::filePollInterval]} {
                ::EventLog::_setOutcome incomplete \
                    "the event logger exited but did not create [::EventLog::_getExitFile]"
            } else {
                #  Frozen here, BEFORE cleanup deletes the marker, so a cleanup
                #  retry never re-examines evidence that cleanup removed.
                ::EventLog::_setOutcome complete ""
            }
            if {$::EventLog::runOutcome ne "complete"} {
                ::EventLog::_reportError "Run $run incomplete" $::EventLog::runOutcomeReason
            }
        }
        #  An abort recorded while a yielding step above ran (dialog, sleep)
        #  is applied before anything is committed.
        if {$::EventLog::abortRequested} {
            ::EventLog::_setOutcome incomplete "aborted ($::EventLog::abortReason)"
        }
        
        #  3. Cleanup (idempotent; throws and stays 'ending' on failure).
        
        ::EventLog::_finalizeRun
        
        #  Cancel the after that updates the event segments and set a new
        #  status line entry indicting the run ended.
        
        if {$::EventLog::statusUpdateId != -1} {
            after cancel $::EventLog::statusUpdateId
            set EventLog::statusUpdateId -1
            if {$::EventLog::runOutcome eq "complete"} {
                set msg {Run ended}
            } else {
                set msg "Run $run ended ($::EventLog::runOutcome): $::EventLog::runOutcomeReason"
            }
            $::EventLog::statusBarManager setMessage $::EventLog::statusbar $msg
        }
    }
    
    #  4. Advancement (exactly once; throws and stays 'cleaned' on failure).
    
    ::EventLog::_advanceRun
}


#-------------------------------------------------------------------------------
#
#  Bundle methods:

##
# ::EventLog::attach
#
#    Called when the bundle is attached to the state machine
#  
# @param state - Current state.
#
proc ::EventLog::attach {state} {

    StageareaValidation::correctFixableProblems
}

## 
# ::EventLog::precheckTransitionForErrors
#
# IF we are transitioning to Active from halted, make sure that we don't have
# any detectable problems (@see ::StageareaValidation::correctAndValidate)
#
# @param from   state before transition
# @param to     state after transition
#
proc ::EventLog::precheckTransitionForErrors {from to} {
  set msg {}
  if {$from eq "Halted" && $to eq "Active"} {
    #  Begin barrier (pure; applies with Record off too).  The optional GUI
    #  resolution prompt is separate and only offered when interactive with
    #  a Tk main window; headless callers get the plain error.
    set barrier [::EventLog::_beginBarrier]
    if {($barrier ne "") && ($::EventLog::unresolvedRun != -1) \
            && ![::EventLog::isRunPending] && !$::EventLog::ending} {
      if {[::EventLog::_promptResolveFailedRun]} {
        set barrier [::EventLog::_beginBarrier]
      }
    }
    if {$barrier ne ""} {
      return $barrier
    }
    if {[::ReadoutGUIPanel::recordData]} {
      ::StageareaValidation::correctFixableProblems;         # Some things can be fixed :-)
      set msg [::StageareaValidation::listIdentifiableProblems]
    }
  }
  return $msg
}
##
# ::EventLog::enter
#
#   Called when the run enters a new state.  We care about transitions:
#   {Paused, Active} -> Halted.
#
proc ::EventLog::enter {from to} {
  if {($from in [list Active Paused]) && ($to eq "Halted")} {
    #  Finalize the owned run.  Keyed on the run record, not on whether the
    #  logger process is still alive: its EOF may already have been consumed
    #  by an earlier callout bundle or a provider that serviced the event loop.
    if {[::EventLog::isRunPending]} {
      ::EventLog::runEnding
    } elseif {[::ReadoutGUIPanel::recordData]} {
      ReadoutGUIPanel::normalColors
    }
  }
  if {($to eq "NotReady") && [::EventLog::isRunPending]} {
    if {$::EventLog::ending} {
      #  Nested: an emergency transition arrived while the outer runEnding
      #  owns the lifecycle.  Record it and wake the outer wait; the outer
      #  call kills/reaps/finalizes and records the superseding outcome.
      set ::EventLog::abortRequested 1
      set ::EventLog::abortReason    "$from -> NotReady"
      ::EventLog::_log warning "Abort ($from -> NotReady) requested while run $::EventLog::pendingRun \
is being finalized; the finalization in progress completes it as incomplete"
      set ::EventLog::waitResult abort
      return
    }
    if {($from in [list Active Paused]) || !$::EventLog::expectingExit} {
      #  The data sources are being stopped without ending the run (or the
      #  run never got going): the logger will never see END_RUN items.
      #  Record the abort; the teardown itself runs inside runEnding so that
      #  the single owner (ending=1) covers the yielding confirmation wait.
      set ::EventLog::abortRequested 1
      set ::EventLog::abortReason    "$from -> NotReady"
    }
    #  Otherwise (Halted -> NotReady because an earlier enter callback failed
    #  after the sources were ended) the logger gets its normal bounded wait.
    ::EventLog::runEnding
  }
}
##
# ::EventLog::leave
#
#   Called when the run leaves a state.
#   If the state is Halted->Active, we start the event logger
#
# @param from - State that we are leaving
# @param to   - State we will enter.
#
proc ::EventLog::leave {from to} {
  
  if {($from eq "Halted") && ($to eq "Active")} {
    #  Begin barrier for every Begin path (button, timed, remote), Record
    #  off included.
    set barrier [::EventLog::_beginBarrier]
    if {$barrier ne ""} {
      ::EventLog::_log error $barrier
      error $barrier
    }
  }
  # None of this needs to be done if we're not recording.
  
  if {[::ReadoutGUIPanel::recordData]} {
    if {($from eq "Halted") && ($to eq "Active")} {
        if {[catch {::EventLog::runStarting} msg]} {
          set trace $::errorInfo
          set ::EventLog::failed 1
          if {$::EventLog::runPhase eq "none"} {
            set ::EventLog::pendingRun -1;   # Nothing was spawned: nothing owned.
          }
          ::ReadoutGUIPanel::Log EventLogManager error "$msg : $trace"
          error "$msg : $trace"
        }
        # reset the failure state
        set ::EventLog::failed 0
    }
    if {($from in [list "Active" "Paused"]) && ($to eq "Halted") } {
      if {[::EventLog::isRunPending]} {
        set  ::EventLog::expectingExit 1;   # The logger exits once the sources end.
      }
      set ::EventLog::failed 0
    }
  }
}

#-------------------------------------------------------------------------------
#
# Bundle registration
#


##
# ::EventLog::register
#
#   Register the event logger package as a callback bundle with the run
#   state machine singleton:
#
proc ::EventLog::register {} {
    set sm [::RunstateMachineSingleton %AUTO%]
    $sm addCalloutBundle EventLog
    $sm destroy
    
    # Create our status bar... just a long label that we'll update
    # every second when runs are active.
    
    set ::EventLog::statusBarManager      [::StatusBar::getInstance]
    set ::EventLog::statusbar \
        [$::EventLog::statusBarManager addMessage {No event segments recorded yet}]
    
    #
    #  Arrange for the event logging parameters to be saved/restored.
    #
    set sm [StateManagerSingleton %AUTO%]
}
##
#  ::EventLog::unregister
#
#   Unregisters the event logger package from the Run state machine singleton.
#   this is really only supplied for testing purposes (maybe). But
#   could potentially be used for special applications.
#
proc ::EventLog::unregister {} {
    set sm [::RunstateMachineSingleton %AUTO%]
    $sm removeCalloutBundle EventLog
    $sm destroy
    
}
#----------------------------------------------------------------------------
#
#   The code in this section provides user interface code to
#   prompt for the event logger's parameters and stor them so that
#   the next call of ::EventLog::_startLogger will use those parameters.
#   The supported parameters are:
#   *  EventLogger - the event logger program to use.  This program must
#                    support or at least ignore the -oneshot switch.
#   *  EventLoggerRing - The name of the ring buffer from which the
#                    event logger gets data.   This should be a URI
#                    not a local ring name.  However, if it is not a URI,
#                    tcp://localhost/ is prepended to the ring name.
#
#

##
# @class EventLog::RingBrowser
#
#  A ring browser window.  Allows users to select a ring from a set of ring
#  in the form ring@hostname
#
# LAYOUT:
#  +----------------------------+
#  |  +-------------------+-+   |
#  |  | ring listbox      |s|   |
#              ...
#  |  +-------------------+-+   |
#  +----------------------------+
# OPTIONS:
#      -rings  - Information about the rings as passed in from the ring master.
#
# METHODS:
#   getSelected - Returns the selected ring in the form ring@hostname
#
snit::widgetadaptor EventLog::RingBrowser {
    option -rings -default [list] -configuremethod _stockListbox
    
    ##
    # constructor
    #
    #    Constructs the user interface and runs the configurlist method
    #    which may (or may not) stock the listbox.
    #
    # @param args - the option/value pairs.
    #
    constructor args {
        installhull using ttk::frame
        
        listbox $win.list -selectmode single -yscrollcommand [list $win.vsb set]
        ttk::scrollbar $win.vsb -orient vertical -command [list $win.list yview]
        
        grid $win.list $win.vsb -sticky nsew
        
        $self configurelist $args
    }
    #-------------------------------------------------------------------------
    # Public methods:
    
    ##
    # getSelected
    #
    #  @return if there's an active selected ring, returns it other wise
    #          returns an empty string.
    #
    method getSelected {} {
        return [$win.list get [$win.list index active]]
    }
    #-------------------------------------------------------------------------
    # Configuration methods
    #
    
    ##
    # _stockListbox
    #
    #   Update the set of rings that are shown in the list box.
    #
    # @param optname - option name (-ring)
    # @param optvalue - List of ring usage statistics from the ringmaster.
    #                   The keypoints are:
    #                  * The first element of each list item is the ring name.
    #                  * Proxy rings are of the form host.ring
    #
    method _stockListbox {optname value} {
        set options($optname) $value
        
        $win.list delete 0 end
        
        foreach ringUsage $value {
            set name [lindex $ringUsage 0]
            set nameList [split $name .]
            if {[llength $nameList] == 1} {
                set ringName $nameList@localhost
            } else {
                #
                #  This code allows for rings remote rings with .'s in them
                #  (though not local rings).
                #
                set host [lindex $nameList 0]
                set ring [join [lrange $nameList 1 end] .]
                set ringName $ring@$host
            }
            $win.list insert end $ringName
        }
    }
    
    
}

##
# @class EventLog::ParameterPrompter
#
#     Provides a dialog form which allows users to override the current set of
#     event logger parameters.
#     By dialog form we mean a form that can be attached to a DialogWrapper.
# LAYOUT:
#     +-------------------------------------------------+
#     | Event Log program <current value> [Browse]      |
#     | Data Source       <current value> [Known Rings] |
#     +-------------------------------------------------+
#
# Key:  Stuff that's not quoted in something are labels, Stuff quoted in
#       <> are entries, and stuff quoted in [] are buttons.
#
#  @note the [Local Rings] button provides a list of rings to choose from
#        The list includes the local rings and the proxy rings that have already
#        been defined.
#        
#
#  OPTIONS:
#      -logger  - Value of the event logger.
#      -ring    - URI that points to the ring buffer.
#      -usensrcs - Boolean... start eventlogger with --number-of-sources switch
#      -additionalsrcs - integer - number of sources in the event builder not
#                    managed by the data source manager.
#      -forcerun  - Boolean... force the run number from GUI rather than using the
#                   one in the begin event (used if no sources provide begin events.)
#      -usechecksum - Boolean... start eventlogger with --checksum switch
#      -segmentsize - size of segments in in gbytes -defaults to 2.
#

snit::widgetadaptor EventLog::ParameterPrompter {
    component additionalSources
    
    option -logger 
    option -ring
    option -usensrcs          -configuremethod _enableDisableAdditionalSources
    option -additionalsources -configuremethod _setAdditionalSources \
                              -cgetmethod      _getAdditionalSources
    option -forcerun
    option -usechecksum       
    option -stagearea
    option -prefix
    option -segmentsize  -default 2;          # units are gigabytes.
    
    ##
    # constructor
    #   Build and stock the useer interface.  We're going to bind the
    #   entry values to the option variables so  there's no need to
    #   build -configuremethod methods to track those.
    #
    # @param args - Configuration option/values.
    #
    # @note - The defaults for the parameters are gotten from the
    #         current configuration so typically the dialog is self
    #         configured.
    #
    constructor args {
        installhull using ttk::frame
        
        # Can't seem to do this when tcl is compiling to byte code.
        
        set options(-logger)            [::DAQParameters::getEventLogger]
        set options(-ring)              [::DAQParameters::getEventLoggerRing]
        set options(-usensrcs)          [::DAQParameters::getUseNsrcsFlag]
        set options(-additionalsources) [DAQParameters::getAdditionalSourceCount]
        set options(-forcerun)          [DAQParameters::getRunNumberOverrideFlag]
        set options(-usechecksum)       [DAQParameters::getUseChecksumFlag]
        set options(-stagearea)         [ExpFileSystem::getStageArea]
        set options(-prefix)            [::DAQParameters::getRunFilePrefix]
        set options(-segmentsize)       [::DAQParameters::getEventLoggerFileSegmentSize]
        
        

        
        #ttk::label  $win.loglabel -text {Event log program}
        #ttk::entry  $win.logger       \
        #    -textvariable [myvar options(-logger)] -width 40
        textprompt $win.logger -text {Event log program} \
          -textvariable [myvar options(-logger)] -width 40
        ttk::button $win.browselogger \
            -text {Browse...} -command [mymethod _browseLogger]
        
        #ttk::label $win.datasourcelabel -text {Data Source Ring URI}
        #ttk::entry $win.datasource    \
        #    -textvariable [myvar options(-ring)] -width 40
        textprompt $win.datasource -text {Data Source Ring URI} \
            -textvariable [myvar options(-ring)] -width 40 
        ttk::button $win.knownrings    \
            -text {Known Rings...} -command [mymethod _browseRings]
        
        ttk::spinbox $win.segsize -width 8 -from 2 -to 10000000 -textvariable [myvar options(-segmentsize)]
        ttk::label $win.seglabel -text {seg. size GB}
        
        message $win.help -text "
The next three settings are a bit advanced as they have to do with multiple \
source and event building where some of the sources are not NSCLDAQ sources. \n
'Use  --number-of-sources' should normally be checked if you are using the \
event logger from nscldaq-11.0 or later but not checked if you need to use an \
earlier event logger.  Use 'Additional sources' to adjust the number of end run \
events to expect.  If 0, --number-of-sources is set to the number of event \
sources you specified to this program.  This parameter can be negative if \
some of the sources we're controlling don't produce end of run events. \n
Check the 'Use GUI Run number' if none of your data sources produce a begin \
run event from which the event file name can be derived or if the run numbers they \
do produce are not those the GUI requests.  Note again, this requires the \
NSCLDAQ-11.0 eventlog program or later. "
        
        set f [ttk::frame $win.sourceparams]
        ttk::checkbutton $f.usensrcs -variable [myvar options(-usensrcs)] \
            -onvalue 1 -offvalue 0 -text {Use --number-of-sources} \
            -command [mymethod _updateAdditionalSources]
        ttk::label       $f.adsrclabel -text {Additional Sources}
        install additionalSources using \
            ttk::spinbox $f.additionalsources -from -10 -to 10 -increment 1 \
                -width 4
        $f.additionalsources set $options(-additionalsources)
        $self _updateAdditionalSources
        
    
        ttk::checkbutton $win.forcerun -text {Use GUI Run number} \
            -variable [myvar options(-forcerun)] -onvalue 1 -offvalue 0
        
        ttk::checkbutton $win.usechecksum -text {Compute checksum} \
            -variable [myvar options(-usechecksum)] -onvalue 1 -offvalue 0

        #ttk::label $win.stageareaLbl -text {Stagearea path} 
        #ttk::entry $win.stageareaEntry -textvariable [myvar options(-stagearea)] \
        #              
        textprompt $win.stageareaEntry -text {Stagearea path} \
              -textvariable [myvar options(-stagearea)] \
              -width 40
        ttk::button $win.stageareaBrowse -text "Browse..." -command [mymethod _browseStagearea]

        #ttk::label $win.prefixLbl -text {Run file prefix} 
        #ttk::entry $win.prefixEntry -textvariable [myvar options(-prefix)] \
        #              -width 40
        textprompt $win.prefixEntry -text {Run file prefix} \
              -textvariable [myvar options(-prefix)] -width 40
        grid $win.logger $win.browselogger -sticky w
        grid $win.datasource $win.knownrings -sticky w
        grid $win.stageareaEntry $win.stageareaBrowse -sticky w
        grid $win.prefixEntry -sticky e
        grid $win.seglabel    -sticky e
        grid $win.segsize -row 4 -column 1 -sticky w
        grid $win.help -columnspan 3 -sticky ew
        
        grid $f.usensrcs          -row 0 -column 0 -sticky w
        grid $f.adsrclabel        -row 0 -column 1 -sticky w -padx 30
        grid $f.additionalsources -row 0 -column 2 -sticky e 
        grid $f -columnspan 3     -sticky nsew
        
        grid $win.usechecksum     -sticky w
        grid $win.forcerun
        
        $self configurelist $args
        
    }
    #------------------------------------------------------------------------
    # Configuration handlers:
    #
    
    ##
    # _enableDisableAdditionalSources
    #
    #   Enables or disables the aditionalSources compoment depending on
    #   the state of the new value of -usensrcs
    #
    # @param optname - Name of option being configured.
    # @param value   - new value for the option
    #
    method _enableDisableAdditionalSources {optname value} {
        set options($optname) $value
        $self _updateAdditionalSources
    }
    
    ##
    # _setAdditionalSources
    #
    #  Called to configure a new number of sources.  Sets the spinbox value
    #  from the new option.  There's no real point in maintaining the
    #  options array value as the spinbox will just change out from underneath us
    #  so we use a cget handler (See _getAdditionalSources below)
    #
    # @param optname - name of the option being configured.
    # @param value   - New requested value.
    #
    method _setAdditionalSources {optname value} {
        $additionalSources set $value
    }
    ##
    # _getAdditionalSources
    #
    #   Get the value of the additiona sources spinbox.
    #
    # @param optname - option name -- ignored.
    #
    method _getAdditionalSources optname {
        return [$additionalSources get]
    }
    #------------------------------------------------------------------------
    # Private methods
    #
    
    ##
    # _updateAdditionalSources
    #
    #  Set the state of the additional sources spinbox depending on the
    #  whether or not that option is enabled.
    #
    method _updateAdditionalSources {} {
        if {!$options(-usensrcs)} {
            $additionalSources configure -state disabled
        } else {
            $additionalSources configure -state normal
        }
    }
    
    ##
    # _browseLogger
    #
    # Browse the NSCLDAQ installation space for event logger programs.
    # Allow the user to select one.  This is just a file browser window where:
    #  * The initial directory is the bin directory of the installation tree
    #    in which we've been installed.
    #  * The default filetype is ""
    #  * File types allowed are "", .sh .bash or all files.
    #
    method _browseLogger {} {
        set file [tk_getOpenFile  \
            -initialdir [file join $::EventLog::DAQRoot bin]  \
            -parent $win -title "Choose event logger" \
            -filetypes [list \
                { {All Files}     *}                          \
               { {Shell scripts} {.sh}       }              \
                { {Bash scripts}  {.bash}     }              \
            ]]
        if {$file ne ""} {
            set options(-logger) $file
        }
    }
    
    ##
    # _browseStagearea
    #
    # Browse for a directory to use as the directory to use as the 
    # stagearea. Note that this demands that the directory already
    # exists.
    #
    method _browseStagearea {} {
        set path [tk_chooseDirectory  \
            -initialdir [file join $::env(HOME) bin]  \
            -parent $win -title "Choose stagearea" \
            ]
        if {$path ne ""} {
            set options(-stagearea) $path
        }
    }

    ##
    # _browseRing
    #  Pops up a dialog that provides a list of the ring buffers
    #  and the hosts they belong to and allows the user to select from
    #  a ring from them...or not.
    #  If a ring was selected, it populates the options(-ring) entry.
    
    method _browseRings  {} {
        toplevel $win.ringbrowser
        set dlg [DialogWrapper $win.ringbrowser.dialog]
        $dlg configure \
            -form [EventLog::RingBrowser [$dlg controlarea].f \
                -rings [ring usage]]
        pack $dlg
        set action [$win.ringbrowser.dialog modal]

        
        if {$action eq "Ok"} {
            set ring  [[$dlg controlarea].f getSelected]
            #
            #  User may click Ok without selecting a ring!
            #
            if {$ring ne ""} {
                set options(-ring) [ringToUri $ring]
            }
        }
        destroy $win.ringbrowser
        
    }
    
    #--------------------------------------------------------------------------
    #   Procs
    #
    
    ## ringToUri
    #
    #  Convert a ring name of the form name@host to a valid ring URI.
    #
    # @param ringName - The ring name in the form ring@host
    #
    # @return string  - The URI for the ring.
    #
    proc ringToUri ringName {
        set ringInfo [split $ringName @];   # list of {ring hostname}
        return tcp://[lindex $ringInfo 1]/[lindex $ringInfo 0]
    }
    
    
}
##
# EventLog::promptParameters
#
#   Proc that instantiatesthe parameter prompter and, if OK was fetched,
#   sets the parameters in the configuration.
#
proc EventLog::promptParameters {} {
    toplevel .eventlogsettings
    set dlg [DialogWrapper  .eventlogsettings.dialog]
    set ctl [$dlg controlarea]
    $dlg configure \
        -form [EventLog::ParameterPrompter $ctl.f]
    pack $dlg
    set action [$dlg modal]
    
    if {$action eq "Ok"} {
        Configuration::Set EventLogger               [$ctl.f cget -logger]
        Configuration::Set EventLoggerRing           [$ctl.f cget -ring]
        Configuration::Set EventLogUseNsrcsFlag      [$ctl.f cget -usensrcs]
        Configuration::Set EventLogAdditionalSources [$ctl.f cget -additionalsources]
        Configuration::Set EventLogUseGUIRunNumber   [$ctl.f cget -forcerun]
        Configuration::Set EventLogUseChecksumFlag   [$ctl.f cget -usechecksum]
        Configuration::Set EventLogSegmentSize       [$ctl.f cget -segmentsize]
        set priorStageArea [Configuration::get StageArea]
        Configuration::Set StageArea                 [$ctl.f cget -stagearea]
        
        # If we're usin gthe nsrcs flag and it would currently be negative warn:
        
        if {[DAQParameters::getUseNsrcsFlag]} {
            set sm [DataSourcemanagerSingleton %AUTO%]
            set mySources [llength [$sm sources]]
            set adtlSources [DAQParameters::getAdditionalSourceCount]
            set totsrc [expr {$mySources + $adtlSources}]
        
            if {$totsrc < 0} {
                tk_messageBox -parent .eventlogsettings -title {Negative source count} \
                    -icon warning -type ok \
                    -message "Your total source count is negative: $mySources managed by us $adtlSources additional sources -> $totsrc total sources"
            }
            $sm destroy
        }
        #  If the stagearea changed ensure the directory structure is there and good:
        
        if {$priorStageArea ne [Configuration::get StageArea]} {
            ExpFileSystem::CreateHierarchy
        }
    }
    destroy .eventlogsettings
}
