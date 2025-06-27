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
  set run [ReadoutGUIPanel::getRun]
  set destDir [::ExpFileSystem::getRunDir $run]
      
  set startFile [file join $destDir .started]
  return $startFile
}
##
# _getExitFile
#    Get path to the current .exited file.
#
proc ::EventLog::_getExitFile {} {
  

  set run [ReadoutGUIPanel::getRun]
  set destDir [::ExpFileSystem::getRunDir $run]
  
  set result [file join $destDir .exited]
  return $result

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
    set switches [::EventLog::_computeLoggerSwitches $loggerVsn]
    
    set ::EventLog::loggerFd \
        [open "| $logger $switches" r]
    set ::EventLog::loggerPid [pid $::EventLog::loggerFd]
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
    if {[eof $fd]} {
        # Need to close off the fd before the pop up shows as that will
        # re-enter the event loop
        
        fileevent $fd readable [list]
        catch {close $fd} msg

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
            # ::EventLog::_finalizeRun;            # May need that if exit before wait.
        }
        set ::EventLog::loggerFd [list]
        set ::EventLog::loggerPid -1
        incr ::EventLog::eventLogEnded
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
    if {$::EventLog::needFinalization} {
        
        set srcdir [::ExpFileSystem::getCurrentRunDir]
        set completeDir [::ExpFileSystem::getCompleteEventfileDir]
        set run [ReadoutGUIPanel::getRun]
        set destDir [::ExpFileSystem::getRunDir $run]
        
        #  IF the run dir does not exist there's a real problem here:
        
        if {![file isdirectory $destDir]} {
          set msg "The run directory $destDir which should hold the event files \
for $run either does not exist or is not a directory"
          tk_messageBox -icon error -type ok -title "Error no event directory" \
            -message $msg
          return
        }
        
        #  Remove any links to event files in the srcdir
        
        
        set  fileBaseName [::ExpFileSystem::genEventfileBasename $run]
        set  eventFiles [glob -nocomplain [file join $srcdir ${fileBaseName}*.evt]]
        foreach file $eventFiles {
          catch {file delete -force $file}
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
            tk_messageBox -title {Tar Failed} -icon error -type ok \
                -message "Copy of files from $srcdir to $destDir failed: $msg, Fix problem and move files manually."
            set ::EventLog::needFinalization 0
            ReadoutGUIPanel::incrRun
            return
        }
        #
        #  Kill off the start file and exitfiles:
        
        ::EventLog::deleteStartFile
        ::EventLog::deleteExitFile
        
        # If required, protect the files:
        #   - The destDir is set to 0550
        #   - The parent dir is set to 0550.
        #   - A chmod -R is done to set the contents to 0x550 as well.
        
        if {$::EventLog::protectFiles} {
            set files [glob -nocomplain -directory $destDir -types {f d} *]
            if {[llength $files]>0} {
              exec sh << "chmod -R 0550 $files"
              file attributes $destDir -permissions 0550 
              file attributes [file join $destDir ..] -permissions 0550 
            }
        }
        set ::EventLog::needFinalization 0
        ReadoutGUIPanel::incrRun
    }
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

    #  If there's already an event logger just force it to exit.

  if {$::EventLog::loggerPid ne -1} {
    foreach pid $::EventLog::loggerPid {
      catch {exec kill -9 $pid};            # Catch because the pipeline could run down.
      set ::EventLog::loggerPid -1
    }
  }

  # Now if desired start the new run.
  ::StageareaValidation::correctAndValidate

  if {[::ReadoutGUIPanel::recordData]} {
    
        
    set startFile [::EventLog::_getStartFile]

    ::EventLog::_startLogger
    ::EventLog::_waitForFile $startFile $::EventLog::startupTimeout \
                                        $::EventLog::filePollInterval
    ::StageareaValidation::deleteStartFile
    set ::EventLog::expectingExit 0
    ::EventLog::_setStatusLine 2000
    set ::EventLog::needFinalization 1
  }
}
##
# Called when the run is ending.  We're only going to do something if the
# event logger's pid is not -1.  In that case:
# * Set that we expect an exit.
# * Wait for the .exited file
# * Set the PID -> -1
# * Finalize the run.
#
#  @note - We let the file readable handler handle closing the fd.
#
proc ::EventLog::runEnding {} {

    set startFile [::EventLog::_getStartFile]
    set exitFile [::EventLog::_getExitFile]

    # ne is used below because the logger could be a pipeline in which case
    # ::EventLog::loggerPid will be a list of pids which freaks out ==.
    
    if {$::EventLog::loggerPid ne -1} {
        
        set ::EventLog::expectingExit 1
        
        #  First do a vwait for eventLogEnded after disabling the
        #  begin/end etc. buttons.
        #  A timeout is used in case there's a problem and the event log
        #  never exists.
        
        
        set timeoutId [after \
            [expr {$::EventLog::shutdownTimeout*1000}]    \
            [list incr ::EventLog::eventLogEnded]         \
        ]
        set oldValue $::EventLog::eventLogEnded;      # so we know if there was a timeout.
        vwait ::EventLog::eventLogEnded
        after cancel $timeoutId
        
        if {$oldValue == $::EventLog::eventLogEnded} {
            
            # Wait timed out.
            tk_messageBox -title "EventLogger exit timeout" -icon warning -type ok \
                -message {Timed out waiting for eventlog to exit, killing it}
            foreach pid $::EventLog::loggerPid {
              catch {exec kill -9 $pid}; # in case waitforfile timed out.
            }
        } else {
            # Normal exit, this should fall through very quickly, now that
            #  we know the logger exited.
            
            ::EventLog::_waitForFile $exitFile $::EventLog::shutdownTimeout \
                $::EventLog::filePollInterval
            
            
        }
        
        set ::EventLog::loggerPid -1  
        ::EventLog::_finalizeRun
      

        #  Cancel the after that updates the event segments and set a new
        #  status line entry indicting the run ended.
        
        if {$::EventLog::statusUpdateId != -1} {
            after cancel $::EventLog::statusUpdateId
            set EventLog::statusUpdateId -1
            $::EventLog::statusBarManager setMessage $::EventLog::statusbar \
                {Run ended}
        }
    }
    ReadoutGUIPanel::normalColors
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
  #  None of this needs to be done if the event log is off.
  if {[::ReadoutGUIPanel::recordData]} {
      if {($from in [list Active Paused]) && ($to eq "Halted")} {
        # if the start was aborted then we should not try to cleanup
        if {! $::EventLog::failed} {
          ::EventLog::runEnding
        }
      }
      if {($from in [list Active Paused]) && ($to eq "NotReady")} {
        # if the start was aborted then we should not try to cleanup
        if {! $::EventLog::failed} {
          # Kill of the event log program since it's not going to see ends:
          foreach pid $::EventLog::loggerPid {
              if {$pid != -1} {
                  catch {exec kill -9 $pid}
              }
          }
          # Create the exit file:
          set fd [open [::EventLog::_getExitFile] w]
          puts $fd "dummy"
          close $fd
          ::EventLog::runEnding
        } 
  
      }
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
  
  # None of this needs to be done if we're not recording.
  
  if {[::ReadoutGUIPanel::recordData]} {
    if {($from eq "Halted") && ($to eq "Active")} {
        if {[catch {::EventLog::runStarting} msg]} {
          set trace $::errorInfo
          set ::EventLog::failed 1
          ::ReadoutGUIPanel::Log EventLogManager error "$msg : $trace"
          error "$msg : $trace"
        }
        # reset the failure state
        set ::EventLog::failed 0
    }
    if {($from in [list "Active" "Paused"]) && ($to eq "Halted") } {
      if {! $::EventLog::failed} { 
        set  ::EventLog::expectingExit 1
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
