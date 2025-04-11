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
# @file multilogger.tcl
# @brief Provide mechanism to create loggers on multiple rings.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
#  This optional plugin provides a mechanism to log data  from multiple rings
#  in addition to the default event logger.  Data are logged only when event
#  recording is enabled.  The package provide:
#   -  A multi-logger menu bar entry that includes commands that allow you to
#      configure the rings that can be logged as well as select which loggers
#      will be active at any time (see menu commands below).
#   - A state transition callback bundle that manages the enabled loggers
#     at appropriate state transitions.
#
#  What is a logger:
#    A logger is defined as a source ring (URI) and a target directory.
#    At this time the only logger that can be run is $DAQBIN/eventlog with pretty
#    vanilla switches.  In addition, loggers can be enabled or disabled.
#    At the start of a recording run, only the enabled loggers are started.
#    Unlike then normal event recording bundle we are going to run the loggers
#    with stdout/stderr going to a pipe and:
#    # Monitor that pipe for output which is logged.
#    # Monitor that pipe for unexpected exits.
#    # Use the closure of the pipe from the eventlogger's side to determine
#      when an event logger (--oneshot is used) exits.
#
#  Menu Commands:
#    The package adds a menubar entry that contains the following menu commands:
#    *  Add Logger...     - Allows you to define a new logger.
#    *  Remove loggers... - Allows you to destroy an existing logger.
#    *  Enable Loggers... - Allows you to specify which loggers will be active.
#    *  List Loggers      - Lists the loggers and their states.
#    *  [ ] Record Always - If checked multiloggers are launched even if
#                           recording is disabled.
#                     
#
#   Normally the loggers only function when the readoutGUI is recording, however
#   if the ::multilogger::recordAlways flag is set, recording is always done.
#   NOTE: in that case it's important to always ensure there are unique
#         run numbers as the eventloggers will refuse to write over existing
#         event files.
#
#   NOTE:  in record always mode, if the main recording checkbutton is not set
#          the run number is incremented at the end of run to dodge efforts
#          to record over existing files.
#
#
#  This package is installed by default but you must load it
#  (e.g. with  ReadoutCallouts.tcl) to incorporate it into the readout gui.
#
#
package provide multilogger 1.0
package require Tk
package require ReadoutGui
package require RunstateMachine
package require DataSourceUI
package require ExpFileSystemConfig
package require ReadoutGUIPanel
package require snit
package require csv
package require stageareaValidation
package require DAQParameters
package require dialogwrapper
package require textprompter

# Namespace to hold our variables:

namespace eval multilogger {
    variable Initialized 0;                       # Protect against multi load:
    variable Loggers   [list];                    # List of EventLogger objects.
    variable configFile [file join ~ .multiloggers] ; # Initial configuration file
    variable recordAlways 0;                      # If true, always record data.
    namespace export enter leave attach
}

##
# @class EventLogger
#    This class encapsulates an event logger:
#
# OPTIONS:
#   *  -ring     - URI from which data are taken.
#   *  -out      - Directory in which the data from that ring are logged.
#   *  -enable   - True/false reflecting the enabled state.
#   *  -timeout  - Number of seconds to wait for the logger to exit.
#
# METHODS:
#   *  start - Start the logger (if enabled).
#   *  stop  - Stop the logger waiting for it to finish.
#
# Note that start/stop insulate the caller from the -enable state, that is:
#   *  start is a noop if enable is false.
#   *  stop is a noop if there is no active logger.
#
snit::type EventLogger {
    option -ring
    option -out
    option -enable   0
    option -timeout 20
    option -sources  1
    
    variable loggerPids [list]
    variable loggerFd     -1
    variable loggerProgram [file join $::env(DAQBIN) eventlog]
    variable datafilePrefix ""
    
    variable expectingExit 0;             # Determines if pipe exists are bad.
    variable waitDone      0;             # for vwait when waiting on exits.
    variable run           -1;            # GUI run number.
    variable startTime     -1;            # When started.
    variable currentDataSize 0;           # Current data size in MB for calculating rate
    variable dataRate        0
    constructor args {
        $self configurelist $args
    }
    
    ##
    # start
    #   Start our event logger:
    #   - The logger command is constructed.
    #   - The logger is started on a pipeline that captures stdin/stderr
    #   - File events are used to make input event driven.
    #   - The event logger's PIDS are captured.
    #
    method start {} {
        if {$options(-enable)} {
            #
            #  Ensure the output directory is writable:
            #
            
            file attributes $options(-out) -permissions u+rw
            
            # Save the run number and start time for end run renaming.
            
            set run [ReadoutGUIPanel::getRun]
            set startTime [clock seconds]
            
            # Construct the logger command and start it.
            
            set timestamp [clock format [clock seconds] -format {%d%b%Y-%H%M%S}]
            set datafilePrefix $timestamp-run
            
            set command [list $loggerProgram                        \
                --source=$options(-ring) --path=$options(-out)      \
                --oneshot                                           \
                --number-of-sources=$options(-sources)              \
                --prefix=$datafilePrefix                            \
                --segmentsize=[DAQParameters::getEventLoggerFileSegmentSize]g \
            ]
            if {[::DAQParameters::getUseChecksumFlag]} {
                append command " --checksum"
            }

            set loggerFd [open "| $command |& cat" r]

            # Enable event driven handling of output from the logger.
            
            fileevent $loggerFd readable [mymethod _handleInput $loggerFd]
            set expectingExit 0
            set loggerPids [pid $loggerFd]

            set ring $options(-ring)
            set out  $options(-out)
            set msg "$ring -> $out started eventlog as pid [lindex $loggerPids 0]"
            ::ReadoutGUIPanel::Log MultiLogger: log $msg

            set currentDataSize 0.00
            eval [mymethod _calculateDataRate]
        }
        
    }
    ##
    # aboutToStop
    #    Flag that it's ok for the loggers to exit.
    #
    method aboutToStop {} {
        if {$options(-enable)} {
            set expectingExit 1
        }
    }
    ##
    # stop
    #  Stop the event logger.  We are going to wait at most the timeout
    #  seconds for the logger to actually exit...well actually, the logger
    #  should exit on its own and all we really need to do is wait for it
    #  and kill it off should it not exit within the timeout.
    #
    method stop {} {
        if {$options(-enable) } {
            
            # First of all only do anything if there is a logger
            # 
            if {[llength $loggerPids] > 0} {
                puts "Stopping multilogger: $options(-ring) -> $options(-out)"
                set expectingExit 1;            # So _handleInput does not freak.
                set afterId                                 \
                    [after [expr $options(-timeout)*1000]   \
                     [list incr [myvar waitDone]]        \
                ]
                vwait [myvar waitDone]
                if {[llength $loggerPids] == 0} {
    
                    # Logger exit was observed:
                    
                    after cancel $afterId;             # Cancel timeout.

                    set currentDataSize [lindex [eval [mymethod _getDataInfo]] 1]
                    
                } else {
                    
                    # Logger did not exit:
                    # Shut it down the hard way:
                    
                    catch {close $loggerFd};        # Since this reports errors.
                    set loggerFd -1
                    foreach pid $loggerPids {
                        catch [exec kill -9 $pid];   # Explicitly kill the pipe elements.
                    }
                    set loggerPids [list]
                    
                    # report the problem:
                    
                    set ring $options(-ring)
                    set out  $options(-out)
                    tk_messageBox -icon error -title {Timed out waiting for logger to exit}  \
                        -type ok                                                            \
                        -message "Multilogger $ring -> $out failed to exit within timeout"
                }
            }
        }
    }
    ##
    # getPid
    #   Get the logger PID list.
    #
    method getPid {} {
        return $loggerPids
    }
    ##
    # _handleInput
    #   Called when the logger fd is readable:
    #   - Actual input is relayed as a log message to the console.
    #   - Closed pipe means the program exited.  If this was expected, just
    #     kill off  the fd and the Pids.  Otherwise, raise a stink as well
    #     as killing off the fd and pids.
    #
    method _handleInput {channel} {

        set pid [lindex [pid $channel] 0]

        if {![eof $channel]} {
            fconfigure $channel -blocking 0
            set data [read $channel];     # Gulp in all waiting data.
            fconfigure $channel -blocking 1;  # Else fileevent triggers.

            if {[string length $data]>0} {
              set ring $options(-ring)
              set out  $options(-out)
              ::ReadoutGUIPanel::Log MultiLogger: output "$ring -> $out (pid=$pid): \"$data\""
        
        } elseif {$loggerFd eq $channel} {
            set ring $options(-ring)
            set out  $options(-out)
            
            catch {close $channel}
            incr waitDone;             # Trigger vwait to finish if waiting.
            set loggerFd -1;           # Set the variables back to show the logger
            set loggerPids [list];     # no loger exists.
            catch {close $loggerFd}
            incr [myvar waitDone];             # Trigger vwait to finish if waiting.
            set loggerFd -1;           # Set the variables back to show the logger
            set loggerPids [list];     # no loger exists.
            
            # If the exit was unexpected, yell:
            
            if {! $expectingExit || ($::Pending::pendingState ne "Halted")} {
		# We may already be halted in which case the source exit is
		# expected, check this before issuing error message (issue #270):
		set sm [::RunstateMachineSingleton %AUTO%]
		set currentState [$sm getState]
		$sm destroy
		if {$::Pending::pendingState ne "None" || $currentState ne "Halted"} {
		    set msg "$ring -> $out (pid=$pid) exited unexpectedly! pending $::Pending::pendingState current $currentState"
		    set dlgmsg "MultiLogger: $msg Check log for errors."
		    tk_messageBox -icon error -type ok -title {Logger exited} \
			-message $dlgmsg 		    
		    ::ReadoutGUIPanel::Log MultiLogger: error $msg
		}
            } else {
                set msg "$ring -> $out (pid=$pid) exited normally."
                ::ReadoutGUIPanel::Log MultiLogger: log $msg
            }
		
        } else { 
    
            # Log this occurrence as a warning. It is not really an error necessarily, but
            # should be noted as something potentially bizarre.
            set ring $options(-ring)
            set out  $options(-out)
            set msg "$ring -> $out: closing eventlog process (pid=$pid) different than most recently launched! "
            append msg "This is not a bad thing if there was a failure during startup of the a previous run."
            ::ReadoutGUIPanel::Log MultiLogger: warning $msg
            
            # clean up the pids associated with the pipe and close the pipe 
            set pids [pid $channel]
            catch {close $channel}
            foreach pid $pids {
                catch {exec kill -9 $pid}
		catch {close $channel}
		incr waitDone;             # Trigger vwait to finish if waiting.
		set loggerFd -1;           # Set the variables back to show the logger
		set loggerPids [list];     # no loger exists.
		catch {close $loggerFd}
		incr [myvar waitDone];             # Trigger vwait to finish if waiting.
		set loggerFd -1;           # Set the variables back to show the logger
		set loggerPids [list];     # no loger exists.
		
		# If the exit was unexpected, yell:
		
		if {! $expectingExit || ($::Pending::pendingState ne "Halted")} {
		    set msg "$ring -> $out (pid=$pid) exited unexpectedly!"
		    
		    set dlgmsg "MultiLogger: $msg Check log for errors."
		    tk_messageBox -icon error -type ok -title {Logger exited} \
			-message $dlgmsg 
		    
		    ::ReadoutGUIPanel::Log MultiLogger: error $msg 
		} else {
		    set msg "$ring -> $out (pid=$pid) exited normally."
		    ::ReadoutGUIPanel::Log MultiLogger: log $msg
		}
		

            }
        }
      }
    }

    ##
    # _calculateDataRate
    #    Updating data rate and total sizes periodically when running
    #
    method _calculateDataRate {} {
        set currentInfo [eval [mymethod _getDataInfo]]
        set dataRate [format %.2f [expr {([lindex $currentInfo 1]-$currentDataSize)/3.}]]
        set currentDataSize [lindex $currentInfo 1]

        if {! $expectingExit} {
            after 3000 [mymethod _calculateDataRate]
        } else {
          set dataRate 0.00
        }
    }

    ##
    # _getDataInfo
    #    Collecting data size information for all segments of a run
    #
    method _getDataInfo {} {
        set eventDir $options(-out)
        set fileBaseName [format $datafilePrefix-%04d $run]

        # Which files exist:
        set segments [glob -nocomplain \
              [file join $eventDir $fileBaseName*.evt]]

        set nsegments [llength $segments]
        set size 0;             # For when there are no segments yet.
        foreach segment $segments {
            if {![catch {file size $segment} segsize]} {
              set size [expr {$size + $segsize/1024.0}]
            }
        }
        set size [format %.2f [expr {$size/1024.0}]]

        return [list $nsegments $size]
    }

    method getRate {} {
        return $dataRate
    }

    method getTotal {} {
        return $currentDataSize
    }
}


##
# @class AddLogger
#   Form for adding a new logger.  This can be wrapped in a dialog wrapper
#   to create a dialog.
#
# OPTIONS:
#    -ring   - Ring to be logged from.
#    -out    - Output directory.
#    -enable - Initial enable.
#    -timeout - Timeout waiting for exits.
#
snit::widgetadaptor AddLogger {
    option -ring
    option -out
    option -enable   0
    option -timeout 20
    option -sources  1
    

    
    ##
    # Constructor:
    #   1. Create the hull as a ttk::frame
    #   2. Process the options.
    #   3. Create and layout the widgets so they are bound to the
    #      options.
    #
    constructor args {
        installhull using ttk::frame
        
        $self configurelist $args
        
        #ttk::label $win.rlabel -text {Ring URI}
        #ttk::entry $win.ring   -textvariable [myvar options(-ring)]
        textprompt $win.ring -text {Ring URI} -textvariable [myvar options(-ring)]
        
        #ttk::label  $win.lout   -text {Output directory}
        #ttk::entry  $win.outdir -textvariable [myvar options(-out)]
        textprompt $win.outdir -text {Output directory} \
            -textvariable [myvar options(-out)]
        ttk::button $win.outbrowse -text Browse... -command [mymethod _browseDir]
        
        ttk::label $win.ltimeout -text {End Run timeout}
        ttk::spinbox $win.timeout -from 2 -to 1000 -textvariable [myvar options(-timeout)]
        
        
        ttk::checkbutton $win.enable -text {Enable} -onvalue 1 -offvalue 0 \
            -variable [myvar options(-enable)]
        
        ttk::label $win.lsources -text {Number of sources}
        ttk::spinbox $win.sources -from 1 -to 1000 -textvariable [myvar options(-sources)]
        
        
        grid $win.ring                   -sticky w
        grid $win.outdir $win.outbrowse    -sticky w
        grid $win.ltimeout $win.timeout              -sticky w
        grid $win.lsources $win.sources              -sticky w
        grid $win.enable                   -column 1 -sticky w
    }
    
    ##
    # _browseDir
    #    Browse for a directory.  When/if accepted, the directory is loaded
    #    into options(-out)
    #
    method _browseDir {} {
        set outdir [tk_chooseDirectory -mustexist 1 -initialdir $options(-out) \
            -parent $win -title {Choose output directory}                       \
        ]
        if {$outdir ne ""} {
            set options(-out) $outdir
        }
    }
}

##
# @class SelectLoggers
#   Presents a list of loggers that can be selected.
#
# OPTIONS
#   -loggers   - Set of selected loggers (expressed as objects conforming to the
#                EventLogger interface.  This must be set at construction time.
#
# METHODS
#   getSelected - Returns the list of selected loggers.
#   setButton   - Sets the state of a button by logger index.
#
#
# The user interface is just a listing of the loggers with checkboxes next to
# them.  Checked boxes indicate selection.
#
#
snit::widgetadaptor SelectLoggers {
    option -loggers -default [list] -readonly 1 ;# list of event loggers.
    
    variable checkbuttons    [list]            ; # list of checkbutton widgets.
    
    ##
    #  Construction:
    #    1.  Install the hull as a ttk::frame.
    #    2.  Process options.
    #    3.  Build up the widgets and lay them out.
    #
    constructor {args} {
        installhull using ttk::frame
        $self configurelist $args
        
        set buttonIdx 0
        
        foreach logger $options(-loggers) {
            set ring [$logger cget -ring]
            set odir [$logger cget -out]
            set ena  [$logger cget -enable]
            if {$ena}  {
                set enaTxt Enabled
            } else {
                set enaTxt Disabled
            }
            set labelText "$ring -> $odir Logger is $enaTxt"
            
            set b $win.b$buttonIdx
            lappend checkbuttons [ttk::checkbutton $b -text $labelText]
            pack $b -fill x -expand 1
            incr buttonIdx
        }
    }
    ##
    # setButton
    #   Sets the state of a specified button.
    #
    # @param idx - index of the button widget.
    # @param state - boolean new state.
    #
    method setButton {idx state} {
        if {$state} {
            set flag selected
        } else {
            set flag !selected
        }
        set checkbutton [lindex $checkbuttons $idx]
        $checkbutton state $flag
    }
    
    ##
    # getSelected
    #   Return a list of the selected loggers:
    #
    method getSelected {} {
        set result [list]
        foreach button $checkbuttons logger $options(-loggers) {
            $button instate {selected} {
                lappend result $logger
            }
        }
        return $result
    }
}

##
# @class LoggerList
#    List loggers and their characteristics.
#
# OPTIONS:
#   -loggers  - List of objects that supply the EventLogger OPTIONs.
#
#
snit::widgetadaptor LoggerList {
    option -loggers -configuremethod _LoggersChanged -default [list]
    
		variable rateSchedule
		variable totalSchedule
    
    ##
    # Constructor:
    #   1. Set up the frame as the hull
    #   2. Process the configuration options.
    #
    #   @note  - the configuremethod for -loggers will take care of
    #            creating and laying out the widgets.
    #
    constructor args {
        installhull using ttk::frame
        $self configurelist $args
    }
    
    ##
    # _LoggersChanged
    #    Get rid of children of the hull
    #    Repopulate given the new list.
    #
    # @param optname - the option being configured.
    # @param value    - its new value.
    #
    method _LoggersChanged {optname value} {
        set options($optname) $value
        
        # Destroy all children of the hull:
        
        foreach w [winfo children $win] {
            destroy $w
        }
        #  Rebuild the display
        
        ttk::label $win.ringtitle -text "From Ring"
        ttk::label $win.dirtitle  -text "To directory"
        ttk::label $win.enabled   -text "Is enabled"
        ttk::label $win.timeout   -text "End run timeout (Sec)"
        ttk::label $win.sources   -text "Event sources"
        grid $win.ringtitle $win.dirtitle $win.enabled $win.timeout \
             $win.sources                                           \
            -sticky w  -padx 5 -pady 5
        
        set idx 0
        foreach logger $value {
            set ring   [$logger cget -ring]
            set dest   [$logger cget -out]
            set enflag [$logger cget -enable]
            set tmo    [$logger cget -timeout]
            set srcs   [$logger cget -sources]
            if {$enflag} {
                set enableText Yes
            } else {
                set enableText No
            }
            ttk::label $win.ring$idx -text $ring
            ttk::label $win.dest$idx -text $dest
            ttk::label $win.ena$idx  -text $enableText
            ttk::label $win.tmo$idx  -text $tmo
            ttk::label $win.srcs$idx -text $srcs
            
            grid $win.ring$idx $win.dest$idx $win.ena$idx $win.tmo$idx \
                $win.srcs$idx                                           \
                -sticky w -padx 5
            
            incr idx
        }
        
    }
}
##
# @class LoggingRateList
#    List logging rates
#
# OPTIONS:
#   -loggers  - List of objects that supply the EventLogger OPTIONs.
#
#
snit::widgetadaptor LoggerRateList {
    option -loggers -configuremethod _LoggersChanged -default [list]


    ##
    # Constructor:
    #   1. Set up the frame as the hull
    #   2. Process the configuration options.
    #
    #   @note  - the configuremethod for -loggers will take care of
    #            creating and laying out the widgets.
    #
    constructor args {
        installhull using toplevel
        $self configurelist $args
    }

    ##
    # Destructor:
    #   Clearing after jobs
    #
    destructor {
        variable rateSchedule
        variable totalSchedule

        after cancel $rateSchedule
        after cancel $totalSchedule
    }

    ##
    # _LoggersChanged
    #    Get rid of children of the hull
    #    Repopulate given the new list.
    #
    # @param optname - the option being configured.
    # @param value    - its new value.
    #
    method _LoggersChanged {optname value} {
        set options($optname) $value

        # Destroy all children of the hull:

        foreach w [winfo children $win] {
            destroy $w
        }
        #  Rebuild the display

        ttk::label $win.ringtitle -text "From Ring"
        ttk::label $win.dirtitle  -text "To directory"
        ttk::label $win.enabled   -text "Is enabled"
        ttk::label $win.rate      -text "Rate (MB/s)"
        ttk::label $win.total     -text "Total (MB)"
        grid $win.ringtitle $win.dirtitle $win.enabled $win.rate \
             $win.total                                          \
            -sticky w  -padx 5 -pady 5

        set idx 0
        foreach logger $value {
            set ring   [$logger cget -ring]
            set dest   [$logger cget -out]
            set enflag [$logger cget -enable]
            if {$enflag} {
                set enableText Yes
            } else {
                set enableText No
            }
            ttk::label $win.ring$idx  -text $ring
            ttk::label $win.dest$idx  -text $dest
            ttk::label $win.ena$idx   -text $enableText
            ttk::label $win.rate$idx  -text 0.00 -justify right
            eval [mymethod _updateLabel $win.rate$idx $logger rate]
            ttk::label $win.total$idx -text 0.00 -justify right
            eval [mymethod _updateLabel $win.total$idx $logger total]

            grid $win.ring$idx $win.dest$idx $win.ena$idx $win.rate$idx \
                $win.total$idx                                          \
                -sticky w -padx 5

            incr idx
        }
    }

    ##
    # _updateLabel
    #     Updating data rate and total size labels periodically
    #
    #  @param label - widget object
    #  @param logger - EventLogger object to retrieve information
    #  @param target - either "rate" or "total" to update
    #
    method _updateLabel {label logger target} {
        variable rateSchedule
        variable totalSchedule

        if {$target == "rate"} {
            $label configure -text [eval [mymethod _formatText [$logger getRate]]]
            set rateSchedule [after 3000 [mymethod _updateLabel $label $logger $target]]
        } elseif {$target == "total"} {
            $label configure -text [eval [mymethod _formatText [$logger getTotal]]]
            set totalSchedule [after 3000 [mymethod _updateLabel $label $logger $target]]
        }
    }

    ##
    # _formatText
    #   Formatting number in float with two decimal points
    #
    # @param value - value to format
    #
    method _formatText {value} {
        return [regsub -all {\d(?=(\d{3})+($|\.))} $value {\0,}]
    }
}

#------------------------------------------------------------------------------
#  Internal utility methods.
#

##
# ::multilogger::_validateLoggerConfig
#    Validate the configuration of a logger:
# @param ring   - must not be empty (TODO - See if the ring actually exists?).
# @param outdir - Must be non empty and an existing, writeable directory.
# @param enable - Must be a valid boolean.
# @param timeout - Must be a an integer strictly greater than zero.
# @param sourcdes - number of event sources in ring.
# @return boolean - true if all te validations above pass else false.
#
proc ::multilogger::_validateLoggerConfig {ring outdir enable timeout sources} {
    if {$ring eq ""}                          { return 0 }
    
    if {$outdir eq ""}                        {return 0}
    if {![file isdirectory $outdir]}          {return 0}
#    if {![file writable $outdir]}             {return 0}
    
    if {![string is boolean -strict $enable]} {return 0}
    
    if {![string is integer -strict $timeout]} {return 0}
    if {$timeout <= 0}                         {return 0}
    
    if {![string is integer -strict $sources]} {return 0}
    if {$sources <= 0}                        {return 0}
    
    return 1
}

#-------------------------------------------------------------------------------
#  User interface procs that handle the menu commands:

##
# multilogger::addLogger
#   Called to create a new logger.
#   The logger definition form (AddLogger) is wrapped in a DialogWrapper in
#   a top level and we wait on user input.  If the user exited with Ok and
#   specified everything we need, we create an appropriate EventLogger object
#   and add it to the list of loggers.
#
proc ::multilogger::addLogger {} {
    #
    #  If somehow we get here when the logger prompt is already displayed
    #  don't do it:
    #
    if {![winfo exists .addlogger]} {
        toplevel .addlogger
        set dialog [DialogWrapper .addlogger.dialog]
        set container [$dialog controlarea]
        set form [AddLogger $container.form \
            -enable 1 -out [::ExpFileSystemConfig::getStageArea] \
        ]
        $dialog configure -form $form
        pack $dialog
        set action [$dialog modal]

        if {$action eq "Ok"} {
            set ring    [$form cget -ring]
            set outdir  [$form cget -out]
            set enable  [$form cget -enable]
            set timeout [$form cget -timeout]
            set sources [$form cget -sources]
            
            if {[::multilogger::_validateLoggerConfig $ring $outdir $enable $timeout $sources]} {
                lappend ::multilogger::Loggers [EventLogger %AUTO%             \
                    -ring $ring -out $outdir -enable $enable -timeout $timeout \
                    -sources $sources
                ]
            } else {
                tk_messageBox -icon error -parent $dialog -title {Bad logger def} \
                    -message {The logger was incompletely or improperly defined} 
            }
            ::multilogger::saveLoggers
            if {[winfo exists .loggingRates]} {
              ::multilogger::loggingRates
            }
        }
        destroy .addlogger
    }
}

##
# ::multilogger::listLoggers
#    Lists the defined loggers and their characteristics.  Note that the main
#    event logger is not listed, only the loggers we manage.
#
proc ::multilogger::listLoggers { } {
    
    #  Create the dialog if it's not up.
    
    if {![winfo exists .listloggers]} {
        
        toplevel .listloggers
        set dlg  [DialogWrapper .listloggers.dialog -showcancel 0]
        set container [$dlg controlarea]
        
        set lister [LoggerList $container.form -loggers $::multilogger::Loggers]
        $dlg configure -form $lister
        
        pack $dlg
        $dlg modal
        destroy .listloggers
    } else {
        # Update the -loggers value if it is (don't think this can happen but..)
        
        set container [.listloggers.dialog controlarea]
        $container.form configure -loggers $::multilogger::Loggers
        $dlg modal
        destroy .listloggers
    }  
}

##
# ::multilogger::enableLoggers
#    Allow users to enable/disable loggers.
#    The actual dialog is a form that contains  instructional text at the
#    top and the SelectLoggers widget at the bottom.  The SelectLoggers
#    widget is initialized with checkboxes set when the logger is enabled.
#    and cleared when not.
#
proc ::multilogger::enableLoggers {} {
    if {![winfo exists .enableloggers]} {
        toplevel .enableloggers
        message  .enableloggers.help -text \
        {In the list of loggers below, check the boxes next to those you want
enabled an uncheck those you don't want enabled}
        pack .enableloggers.help -fill x -expand 1
        
        set dlg [DialogWrapper .enableloggers.dialog]
        set container [$dlg controlarea]
        
        set sel [SelectLoggers $dlg.select -loggers $::multilogger::Loggers]
        set idx 0
        foreach logger $::multilogger::Loggers {
            $sel setButton $idx [$logger cget -enable]
            incr idx
        }
        
        $dlg configure -form $sel
        pack $dlg -fill x -expand 1
        
        set result [$dlg modal]
        
        if {$result eq "Ok"} {
            set selection [$sel getSelected]
            foreach logger $::multilogger::Loggers {
                if {$logger in $selection} {
                    set state 1
                } else {
                    set state 0
                }
                $logger configure -enable $state
            }
            ::multilogger::saveLoggers
        }
        destroy .enableloggers
    }
 
}

##
# ::multilogger::deleteLoggers
#      Pop up a dialog to allow users to delete event loggers.
#      List the loggers using Dialog wrapped SelectLoggers.
#      Once the loggers have been accepted, present a dialog that shows which
#      ones will be deleted for confirmation.
#
proc ::multilogger::deleteLoggers {} {
    if {![winfo exists .delloggers] && ![winfo exists .confirmdelloggers]} {
        
        # Prompt for deletions:
        
        toplevel .delloggers
        message .delloggers.help \
            -text {Select the event loggers to delete below}
        pack .delloggers.help -fill x -expand 1
        
        set dlg [DialogWrapper .delloggers.dialog]
        set container [$dlg controlarea]
        set form [SelectLoggers $dlg.select -loggers $::multilogger::Loggers]
        $dlg configure -form $form
        
        pack $dlg -fill x -expand 1
        
        set result [$dlg modal]
        
        # IF ok prompt for confirmation:
        
        if {$result eq "Ok"} {
            set pendingDelete [$form getSelected]
            destroy .delloggers
            
            # Treat an empty selection like a cancel:
            
            if {[llength $pendingDelete] > 0} {
                toplevel .confirmdelloggers
                message  .confirmdelloggers.help \
                    -text {Really delete these event loggers?}
                pack .confirmdelloggers.help -fill x -expand 1
                set dlg [DialogWrapper .confirmdelloggers.dialog]
                set container [$dlg controlarea]
                set form [LoggerList $container.form -loggers $pendingDelete]
                $dlg configure -form $form
                pack  $dlg -fill x -expand 1
                tkwait visibility $dlg
                
                set result [$dlg modal]
                destroy .confirmdelloggers
                
                # Confirmed so delete:
                
                if {$result eq "Ok"} {
                    foreach logger $pendingDelete {
                        set idx [lsearch -exact $::multilogger::Loggers $logger ]
                        if {$idx == -1} {
                            error "BUG - Deleting logger not in registered list!"
                        }
                        set  ::multilogger::Loggers [lreplace $::multilogger::Loggers $idx $idx]
                        $logger destroy
                    }
                    ::multilogger::saveLoggers
                    if {[winfo exists .loggingRates]} {
                      ::multilogger::loggingRates
                    }
                }
            }
        } else {
            destroy .delloggers
        }
    }
}

##
# ::multilogger::saveLoggers
#    Save the loggers to the program's dot file
#    See mulitlogger::loadLoggers below for the format of this file.
#
proc ::multilogger::saveLoggers {} {
    set fd [open $::multilogger::configFile w]
    
    foreach logger $::multilogger::Loggers {
        set ring [$logger cget -ring]
        set out  [$logger cget -out]
        set ena  [$logger cget -enable]
        set tmo  [$logger cget -timeout]
        set srcs [$logger cget -sources]
        
        puts $fd [::csv::join [list $ring $out $ena $tmo $srcs]]
    }
    
    close $fd
}
##
# ::multilogger::loadLoggers
#    If the file ~/.multiloggers exists, load the logger setup from there.
#    This is a csv files with fields that are
#    * ring uri         - refers to the source ring.
#    * logger directory - The directory in which logging is done.
#    * enable-flag      - True if the logger is enabled.
#    * timeout          - Seconds to wait for the logger to exit on end run.
#
proc ::multilogger::loadLoggers {} {
    if {[file readable $::multilogger::configFile]} {
        set fd [open $::multilogger::configFile r]
        while {![eof $fd]} {
            set line [gets $fd]
            set infoList [::csv::split $line]
            if {[llength $infoList] == 5}  {
                set ring [lindex $infoList 0]
                set out  [lindex $infoList 1]
                set ena  [lindex $infoList 2]
                set tmo  [lindex $infoList 3]
                set srcs [lindex $infoList 4]
                lappend ::multilogger::Loggers [EventLogger %AUTO%      \
                    -ring $ring -out $out -enable $ena -timeout $tmo    \
                    -sources $srcs                                      \
                ]
            }
        }
        close $fd
    } else {
        # Make an empty one:
        
        set fd [open $::multilogger::configFile w]
        close $fd
    }
}
#-----------------------------------------------------------------------------
#  Snit megawidget that shows which loggers are still active
#  as they're being shut down.  The hull is a top level.
#
#
#  @class LoggerList - the displays the list of active loggers.
#
# OPTIONS
#  -loggers - The list of logger objects.
#
snit::widgetadaptor multilogger::LoggerDisplay {
    option -loggers -default [list] -configuremethod _CfgLoggerlist
    component loggers
    
    variable items [list];       #Item ids in the treeview.
    
    ##
    #  Build the component widgets,
    #  and lay them out as appropriate.
    #
    constructor args {
        installhull using toplevel
        wm title $win {Stopping multiloggers}
        set f [ttk::frame $win.f]
        install loggers using ttk::treeview $f.loggers \
            -columns [list Ring Directory] -selectmode none -show headings \
            -yscrollcommand  [list $f.sb set]
        $loggers heading 0 -text Ring
        $loggers heading 1 -text Directory
        ttk::scrollbar $f.sb -orient vertical -command [list $loggers yview]
        
        grid $f -sticky nsew
        grid $loggers $f.sb -sticky nsew
        
            
    }
        #---------------------------------------------------------------------
        # Private methods
        
        ##
        # _CfgLoggerlist
        #    Called to configure the set of loggers.
        #   - Clear the logger list.
        #   - for each logger extract the logger ring and logger directory
        #      and add it tothe tree.
        #
        #  @param optn  -name of the option being provided.
        #  @param optv  - valueof the option
        method _CfgLoggerlist {optn optv} {
            set options($optn) $optv
            
            # Clear the tree:
            
            foreach item $items {
                $loggers delete $item
            }
            set items [list]
            
            foreach logger $optv {
                set dir [$logger cget -out]
                set ring [$logger cget -ring]
                
                set item [$loggers insert {} end]
                $loggers set $item 0 $ring
                $loggers set $item 1 $dir
                
                lappend items $item
            }
        }
            
        
}
##
# ::multilogger::loggingRates
#    A window for showing logging rate with file size change
#
proc ::multilogger::loggingRates {} {
    #  Create the dialog if it's not up.

    if {![winfo exists .loggingRates]} {
        LoggerRateList .loggingRates -loggers $::multilogger::Loggers
    } else {
        # Update the -loggers values
        .loggingRates configure -loggers $::multilogger::Loggers
    }
}

#------------------------------------------------------------------------------
#  Our state transition methods:
#

##
# attach
#   Called when our bundle is attached to the state manager.
#   NO-OP
# @param state - current state of the system.
#
proc ::multilogger::attach state {
    
}

##
# leave
#   Called as a state is left:
#    Leaving Halted for Active, with recording enabled, start the loggers.
#
# @param from - initial state (Being left).
# @param to   - Next state.
#
proc ::multilogger::leave {from to} {
    if {($from eq "Halted") && ($to eq "Active")} {
        if {[::ReadoutGUIPanel::recordData] || $::multilogger::recordAlways} {
          ::StageareaValidation::correctAndValidate
            foreach logger $::multilogger::Loggers {
                $logger start
            }
            after 1000;             # Allow loggers to initialize.
            ::multilogger::loggingRates
        }
    }
    if {($from in {Active Paused}) && ($to eq "Halted")} {
        #  Let the loggers know it's ok for to exit

        foreach logger $::multilogger::Loggers {
            $logger aboutToStop
        }
    }
}

##
# enter
#   Called as a state is entered:
#   If entering Haltd from either Active or Paused, stop all loggers.
#   We don't need to check for recording active. stop is a no-op for loggers
#   that did not start.
#
# @param from - prior state.
# @param to   - State being entered.
#
proc ::multilogger::enter {from to} {
    if {($from in {Active Paused}) && ($to eq "Halted")} {
       
        # ensure they all vanished within their timeout or kill:
        
        # Put up a little top level GUI thta shows which
        # Multiloggers need stopping 
        
        multilogger::LoggerDisplay .loggerstostop -loggers $::multilogger::Loggers]
        set remaining $::multilogger::Loggers
        foreach logger $::multilogger::Loggers {
            $logger stop
            
            # remove the stopped logger from the GUI.

            set remaining [lrange $remaining 1 end]
            .loggerstostop configure -loggers $remaining
        }
        # Destroy the top level.
         
         destroy .loggerstostop
        
        # If multilogger is recording but the 'central' eventlogger is not,l
        # increment the run number to avoid stomping on event files next time
        # around.

        if {$::multilogger::recordAlways && ![::ReadoutGUIPanel::recordData]} {
                ::ReadoutGUIPanel::setRun [expr {[::ReadoutGUIPanel::getRun] + 1}]
            }
    }
}


#-----------------------------------------------------------------------------
# One time package initialization.


proc ::multilogger::initPackage {} {
    if {!$::multilogger::Initialized } {
        
        # Setup the menu:
        
        set myMenu [::ReadoutGUIPanel::addUserMenu multilog MultiLogger]
        $myMenu add command -label {Add Logger...} -command ::multilogger::addLogger
        $myMenu add command -label {List Loggers...}  -command ::multilogger::listLoggers
        $myMenu add separator
        $myMenu add command -label {Enable Loggers...} -command ::multilogger::enableLoggers
        $myMenu add command -label {Delete Loggers...} -command ::multilogger::deleteLoggers
        $myMenu add separator
        $myMenu add command -label {Show Loggers Data Rate...} -command ::multilogger::loggingRates
        $myMenu add separator
        $myMenu add checkbutton -offvalue 0 -onvalue 1 -variable ::multilogger::recordAlways -label {Record Always}
        
        # Load the initial loggers from last time if they exist else build the
        # ~/.multiloggers file.
        
        ::multilogger::loadLoggers
        
        
        set ::multilogger::Initialized 1
    }
    # Register the callback bundle just after the event logger, so that 
    # if the event logger bundle fails, this bundle does not transition
    
    set sm [::RunstateMachineSingleton %AUTO%]
    
    $sm addCalloutBundle multilogger EventLog
    if {0} {
    set bundles [$sm listCalloutBundles]
    set evtLogIndex [lsearch -exact $bundles EventLog]
    if {$evtLogIndex ne -1} { 

      # add it after EventLog only if there is a bundle after EventLog
      if {$evtLogIndex < [expr [llength $bundles]-1]} {
        set bundleAfterEvtLog [lindex $bundles [expr $evtLogIndex+1]]
        $sm addCalloutBundle multilogger $bundleAfterEvtLog
      } else {
        # add it to the end of the bundles 
        $sm addCalloutBundle multilogger
      }
    } else {
      #add it to the end of the bundles
      $sm addCalloutBundle multilogger
    }
    }
    $sm destroy
}

after 500 ::multilogger::initPackage

