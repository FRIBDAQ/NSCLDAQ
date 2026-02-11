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

##
# @file evbRdoCallouts.tcl
# @brief Readout Callouts for event builder/ReadoutGUI.
# @author Ron Fox (fox@nscl.msu.edu)

#++ REMOVE
package provide evbcallouts 2.0
package require snit
package require portAllocator
package require ReadoutGUIPanel
package require Experiment 2.0
package require ring
package require StateManager
package require Thread
package require ringsourcemgr
package require EndrunMon
package require evbui;               # So we can use the megawidgets.
package require DAQParameters

namespace eval ::EVBC {
    variable registered 0;            # nonzero if the event bundle is registered.
    variable initialized 0
    variable pipefd    "";            # Holds the fd to the pipe to the evbpipeline
    variable evbpids   [list];        # Holds list of PIDS that are the event builder.
    
    #variable InitialDt 1;             # Initial/default glom dt value.
    
    # Figure out where we are and hence the root of the daq system:
    # We assume we are in one directory below TclLibs in computing this:
    # this results in
    #  ::EVBC::scriptdir - The director this script is in.
    #  ::EVBC::daqtop    - canonicalized top level directory of DAQ installation.
    #
    variable scriptdir [file dirname [info script]]

    variable daqtop    [file join [file normalize $scriptdir] .. ..]
    
    #
    #  Application options passed to initialize:
    #
    #  The runningEVBOptions variable stores a snit::type holding the currently 
    #  running version of the parameters. If a user wants to start the parameters
    #
    variable applicationOptions ""
    variable runningEVBOptions ""
    

    # Communicating with the output ring monitoring thread:
    
    variable monitorTid            "";            # Thread id
    variable monitorMutex          "";            # Mutex handle for condition variable
    variable monitorCondVar        "";            # Condition variable signalled at end of run.
    
    #  Suffix for the application:
    
        variable appNameSuffix       ""

    # Configuration parameters for the event builder:
    
    variable window             ""
    
    
    variable XoffThreshold      ""
    variable XonThreshold       ""

    variable perQXoffThreshold  ""
    variable perQXonThreshold   ""
}


#-------------------------------------------------------------------------------
##
# @class EVBC::StartOptions
#
#  This snit type is really just used to provide options and defaults for
#  EBC::start.
#
# OPTIONS:
#    * -teering   - if present provides the name of an intermediate ring that
#                   gets the ordered fragments.
#    * -glombuild - If true glom will build events.
#    * -glom     - ticks in the coincidence interval used to build events.
#                   required if -glombuild is true.
#    * -glomtspolicy - Timestamp policy for glom.
#    * -destring  - Final destination ring of glom's output.
#                   defaults to the users's name.
#    * -glomid    - Source id to assign to built physics events
#    * -maxfragments - maximum number of fragments glombuilds into an event.
#                   prevents stuck timestamps from crashing.
#
snit::type EVBC::StartOptions {
    option -teering   0
    option -teeringname "" 
    option -glombuild -configuremethod checkGlomBuild -default 0
    option -glomdt 1 
    option -glomid -default 0
    option -glomtspolicy -configuremethod checkTsPolicy -default latest
    option -destring -default $::tcl_platform(user) -configuremethod updateLoggerRing
    option -maxfragments -default 1000;    #Same default as program.
    
    variable policyValues [list earliest latest average]
    
    ##
    # constructor
    #   For reasons I'm not clear on snit won't let me -default
    #   an option value to a global or namespaced variable
    #  Therefore we use a constructor to set the initial value of
    #  -glomdt to its default value which is external to allow for some DRYness
    #
    constructor args {
        $self configurelist $args
    }
    ##
    # updateLoggerRing
    #    Called when destring is modified.  This is needed to update the
    #    readoutGUI configuration for the ring buffer used by eventLogBundle.
    #
    #  @param opt  - name of the option being modified (-destrin)
    #  @param value - new value for the option.
    #  @return <nothin>
    #
    method updateLoggerRing {opt value} {
	set options($opt) $value

# 	Configuration::Set EventLoggerRing tcp://localhost/$value
	
    }
    ##
    # checkTsPolicy
    #    must be one of earliest, latest, or average
    #
    # @param optname - name of option in which to save this.
    # @param value   - Proposed new value.
    #
    method checkTsPolicy {optname value} {
        if {$value ni $policyValues} {
            error "--glomtspolicy was $value must be one of {[join $policyValues ", "]}"
        }
        set options($optname) $value
    }
    ##
    # getGlomTsPolicies
    #    Get a  list of the glom timestamp policies.
    #
    method getGlomTsPolicies {} {
        return $policyValues
    }


    ##
    # checkGlomBuild
    #
    # This just makes sure that the value stored in -glombuild is either 0 or 1
    #
    method checkGlomBuild {opt val} {
      set options(-glombuild) [string is true $val]
    }
    
}


#------------------------------------------------------------------------------
##
#  @class EVBC::AppOptions
#
#   Options used to define the state of the high level API:
# OPTIONS
#   *  EVBC::startOptions options are installed as a component.
#   * -gui  - enable or disable the GUI.
#   * -restart - Do or don't restart the event buildeer pipeline each onBegin
#   * -setdestringasevtlogsource - Force the value in dest. ring text entry to
#                override the global eventlog settings when written to
#
snit::type EVBC::AppOptions {
    component startOptions
    
    option -gui     true
    option -restart true
    option -setdestringasevtlogsource 0

    delegate option * to startOptions
    delegate method * to startOptions
    
    ##
    # constructor
    #   just installs startOptions and processes any command line parameters:
    #
    constructor args {
        set startOptions [EVBC::StartOptions %AUTO%]
        
        $self configurelist $args
    }


    ## Return a new EVBC::AppOptions that has the same state 
    #
    method clone {} {

      # get all of the options and their values and make a dict of them
      set state [dict create]
      foreach opt [$self info options] {
        set value [$self cget $opt]
        dict set state $opt $value 
      }

      # return a new snit object with the same params
      return [[$self info type] %AUTO% {*}$state]

    }
}

##############################################################################
#
#  Low level API:
#
##############################################################################

#-------------------------------------------------------------------------------
##
# @fn EVBC::start option....
#
#   Start the event builder  An EVBC::StartOptions object is used to hold
#   default options.  These are overidden by any options that are passed on
#   the command line.
#
# @param args Tcl list of options and their values.  Must be an even number
#             of elements.
#
# @note The variable EBVC::pipefd is modified to contain the file descriptor
#       open on stdin of the pipeline on success.
# @exception If incomplete or incorrect options are provided.
#
proc EVBC::start args {
    
    # If the pipeline is already active that's an error:
    
    if {$EVBC::pipefd ne ""} {
        error "EVBC::start event builder pipeline is already active"
    }
    
    # Create the start options modified by the args - syntactically validates too.
    
    set options [EVBC::StartOptions %AUTO%]
    if {[llength $args] > 0} {
         $options configure {*}$args
    }
    EVBC::_ValidateOptions $options

    # construct the pipeline:
    # The head of the pipeline is tclsh so that we can
    # push the startOrderer script into the pipe and maintain
    # reads on the pipe.
    #
    set bindir [file join $EVBC::daqtop bin]
    set ::EVBC::appNameSuffix [clock seconds]
    set orderer [file join $bindir startOrderer]
    set program [file join $bindir Orderer]
   # set pipecommand "valgrind --tool=callgrind $program 2> orderer.err ";        # TODO - this should be @TCLSH_CMD@
    set pipecommand "$program 2> orderer.err ";        # TODO - this should be @TCLSH_CMD@
    #  If -teering is not null hook teering into the pipeline:
    
    set intermediateRing [$options cget -teering]
    if {$intermediateRing} {
        set teering "[file join $bindir teering] --ring=[$options cget -teeringname]"
        append pipecommand " | " $teering
    }
    #
    #  Figure out the glom command and hook it in.
    #
    
    # set glom "valgrind --tool=callgrind [file join $bindir glom] --dt=[$options cget -glomdt] "
    set glom "[file join $bindir glom] --dt=[$options cget -glomdt]"
    if {![$options cget -glombuild]} {
        append glom " --nobuild "
    }
    append glom " --sourceid=[$options cget -glomid]"
    append glom " --timestamp-policy=[$options cget -glomtspolicy] "
    append glom "  --maxfragments [$options cget -maxfragments]"
    append pipecommand " | $glom  "
    #
    #  Ground the pipeline in the -destring 
    #
    set stdintoring "[file join $bindir stdintoring] [$options cget -destring]"
    append pipecommand " | $stdintoring |& cat  "; # The cat captures stderr.
    
    #
    #  Create the pipeline:
    #
    
    
    set EVBC::pipefd [open "| $pipecommand" w+]
    set EVBC::evbpids [pid $::EVBC::pipefd];      # Capture the PIDS.

    fconfigure $EVBC::pipefd -buffering line -blocking 0
    fileevent $EVBC::pipefd readable EVBC::_PipeInputReady
    
    
    # Push the startup script into the pipe:
    
    puts $EVBC::pipefd "source $orderer"
    ::flush $EVBC::pipefd
    puts $EVBC::pipefd "set ::OutputRing [$options cget -destring]"
    ::flush $EVBC::pipefd
    puts $EVBC::pipefd "start $::EVBC::appNameSuffix"
    ::flush $EVBC::pipefd
    
    # Construct and set the GUI title:

    set localhost [exec hostname]
    set title "Event builder -> [$options cget -destring]@$localhost"
    puts $EVBC::pipefd "wm title . {$title}"
    
    # If any parameters have been set push those out now:
    
    foreach param [list window XoffThreshold XonThreshold perQXoffThreshold perQXonThreshold] {
        set value [set ::EVBC::$param]
        if {$value ne ""} {
            EVBC::configParams $param $value
        }
    }
    
        

    #
    # Next wait for the event orderer service to become available:

    set where [winfo geometry .]
    toplevel .waiting
    wm geometry .waiting $where
    label    .waiting.for -text "Waiting for event builder to start up"
    pack     .waiting.for
    
    set timeoutPasses 1000
    set port ""
    for {set i 0} {$i < $timeoutPasses} {incr i} {
        set port [::EVBC::_getEvbPort]
        
        if {$port eq ""} {
            update idletasks
            after 500
        } else {
            set i $timeoutPasses
        }
    }
    destroy .waiting
    if {$port eq ""} {
        error "Event builder failed to start within timeout"
    }

}

proc EVBC::getAppName {} {
    set me      $::tcl_platform(user)
    return "ORDERER:$me:$::EVBC::appNameSuffix"
}

#------------------------------------------------------------------------------
##
# @fn EVBC::stop
#
#   Stop the event builder pipeline.
#
#  @exception The event builder pipeline is not running.
#
proc EVBC::stop {} {

    EVBC::_CheckPipeline EVBC::stop
    
    # Tell the interpreter to exit.
    # note that the pipefd close will trigger _PipeInputReady which will
    # in turn close the pipefd after reaping any errors.

    set EVBC::evbpids [list];              # Expecting the exit so empty the pidlist.
    
    # Push an exit and mark us not connected.
    
    puts $EVBC::pipefd exit
    ::flush $EVBC::pipefd
                   
    
}
#------------------------------------------------------------------------------
##
# @fn EVBC::reset
#
#  Reset the timestamps etc. in the event builder.
#  This initializes the concept of the eventbuilders history so it loses all
#  memory of ever seeing a fragment.
#
# @exception The event buidler pipeline is not running.
#
proc EVBC::reset {}  {
    EVBC::_CheckPipeline EVBC::reset
    puts $EVBC::pipefd EVB::reset
}
#------------------------------------------------------------------------------
##
# @fn EVBC::flush
#
#  Forces all event builder queues to flush through to the output.
#
#  @exception The event builder pipeline is not running.
#
proc EVBC::flush {} {
    #EVBC::_CheckPipeline EVBC::flush
    
    #puts $EVBC::pipefd EVB::flushqueues
}

##
# getOrdererPort
#   Returns the event orderer port number
#
proc EVBC::getOrdererPort {} {
   #
    #  Figure out what port the event builder is running on... or if it's running
    #
    
    set port [::EVBC::_getEvbPort]
    
    if {$port eq ""} {
        error "EVBC::startRingSource Unable to locate the event builder service"
    }
    return $port    
}


#------------------------------------------------------------------------------
## @fn EVBC::registerRingSource
#
# Adds a ring source to the RingSourceMgr callout bundle. This is actually just
# a convenience method so that people can remain dealing with only the EVBC::
# namespace. In the end, the EVBC callout bundle does almost nothing for setting up 
# the ring sources.
#
# @param sourceRingUrl          - URL of the source ring.
# @param timestampExtractorLib  - Path to the timestamp extractor shared object.
# @param id                     - Source id used to associate event fragments
#                                 with an input queue.
# @param info                   - Long description used to identify the source
#                                 in the event orderer GUI.
# @param expectHdrs             - All data must have body headers. Timestamp lib
#                                 and id can be left as empty strings
# @param oneshot                - If provided number of ends that result in exit.
# @param timeout                - If provided, timeout in seconds after first end to 
#                                 wait for all ends in --oneshot mode.
# @param offset                 - Optional time offset.
# @param defaultsrcid           - Source id used for body headerless items (e.g
#                                 ring format).  This defaults to the first of the
#                                 dis in the id list.
# @param divisor                - optional timestamp divisor.
# @note Event sources are subprocesses of us but not subprocesses of the
#       the event building pipeline.
#
#
proc ::EVBC::registerRingSource {                                           \
    source lib id info {expectHdrs 0} {oneshot {}} {timeout {}} {timeoffset 0} \
    {defaultsrcid {}} {divisor 1}} {
    
    if {$defaultsrcid eq ""} {
        set defaultsrcid [lindex $id 0]
    }
   ::RingSourceMgr::addSource           \
        $source $lib $id $info $expectHdrs $oneshot $timeout $timeoffset    \
        $defaultsrcid $divisor
}

#------------------------------------------------------------------------------
## @fn EVBC::registerS800Source
#
# Convenience method for registering an S800 source. It is just a call to 
# EVBC:registerRingSource but the s800 timestamp extractor is provided for
# the user.
#
# @param ringUrl          - URL of the source ring.
# @param id               - Source id used to associate event fragments
#                           with an input queue.
# @param desc             - Long description used to identify the source
#                           in the event orderer GUI.
#
proc ::EVBC::registerS800Source {ringUrl id {desc {S800 USB data}}} { 
    #
    # Figure out the timestamp extractor path:
    #
    set extractor  [file join $EVBC::daqtop lib libS800TimeExtractor.so]
    
    ::EVBC::registerRingSource $ringUrl $extractor $id $desc
}
#------------------------------------------------------------------------------
##
# @fn EVBC::startRingSource [obsolete - use RingSourceMgr::startSource for new apps]
#
#   Starts a ring buffer event source
#
# @param args -set of parameters expected by ::RingSourceMgr::startSource
# @note Event sourcese are subprocesses of us but not subprocesses of the
#       the event building pipeline.
#
proc EVBC::startRingSource {args} {
  ::RingSourceMgr::startSource {*}$args
}


##
# @fn EVBC::startS800Source
#
#  Starts the s800 fragment source.
#
# @param ringUrl - URL of ring gettting S800 data.
# @param id      - Id of data source.
# @param desc    - short description defaults to "S800 USB data".
#
proc EVBC::startS800Source {ringUrl id {desc {S800 USB data}}} {
    #
    # Figure out the timestamp extractor path:
    #
    set extractor  [file join $EVBC::daqtop lib libS800TimeExtractor.so]
    
    EVBC::startRingSource $ringUrl $extractor $id $desc
    
}

###############################################################################
#
# High level API
#
##############################################################################

#-----------------------------------------------------------------------------
##
# EVBC::initialize 
#
#  For the most part this just creates, populates and validates the
#  initialization options.  The options are maintained in an EVBC::AppOptions
#  object stored in EVBC::applicationOptions
#
# @param args - Optional configuration options.
#
proc EVBC::initialize args {
    #
    # Create and optionally configure the application objects.
    #
    if {!$EVBC::initialized} {
        set EVBC::initialized true
        set EVBC::applicationOptions [EVBC::AppOptions %AUTO%]
        
        if {[llength $args] > 0} {
            $EVBC::applicationOptions configure {*}$args
        }
        EVBC::_ValidateOptions $EVBC::applicationOptions
        
        # if -gui is true, start it and paste it:
        
        if {[$EVBC::applicationOptions cget -gui] && [$EVBC::applicationOptions cget -restart]} {
            EVBC::_StartGui
        }

        # if -gui is true, start it
        if {[$EVBC::applicationOptions cget -gui] && ![::EVBC::_guiExists]} {
            EVBC::_StartGui
        }
    }
    #
    #  If the app is being destroyed kill the event builder too:
    
    bind . <Destroy> +[list EVBC::_Exiting %W]

}
#------------------------------------------------------------------------------
##
# @fn EVBC::onBegin
#
#   Call this from the user's OnBegin method.
#   -  If -restart is true kills the event builder and vwaits for the pipefd
#      to change.
#   -  If the fd is empty indicating the event builder is not running, starts it.
#   - If the user defined a startEVBSources proc and we started ther event builder
#     invoke it.
#   - If the UI exists, then disable it completely.
#
proc EVBC::onBegin {} {

    if {!$EVBC::initialized} {
        tk_messageBox -icon error -title {Not initialized} -type ok \
         -message {evbrdocallouts was incorporated but the event builder was not initialized}
        exit 1
        error "OnStart has not initialized the event builder package."
    }
    if {([$EVBC::applicationOptions cget -restart] || [_paramsChanged]) && ($EVBC::pipefd ne "")} {
        EVBC::stop
        # reset the sources so that we don't skip them.
        ::RingSourceMgr::resetSources
        vwait EVBC::pipefd;      # Will become empty on exit.
    }
    
    
    # IF needed, create the destination and the intermediate ring:

    set teering [$EVBC::applicationOptions cget -teering]
    set teeringname [$EVBC::applicationOptions cget -teeringname]
    if {$teering} {
	catch {ringbuffer create $teeringname}
    }
    set destring [$EVBC::applicationOptions cget -destring]
    if {$destring ne ""} {
        catch {ringbuffer create $destring}
    }
    
    #  If needed restart the EVB and disable the GUI...if it exists
    
    if {$EVBC::pipefd eq ""} {
        EVBC::start \
            -teering   $teering   \
            -teeringname  $teeringname   \
            -glombuild [$EVBC::applicationOptions cget -glombuild] \
            -glomdt    [$EVBC::applicationOptions cget -glomdt]    \
            -glomid    [$EVBC::applicationOptions cget -glomid]    \
            -glomtspolicy [$EVBC::applicationOptions cget -glomtspolicy] \
            -destring  $destring -maxfragments [$EVBC::applicationOptions cget -maxfragments]
        
        
        if {[::EVBC::_guiExists]} {
            EVBC::_DisableGUI 
        }
            
        if {[info commands startEVBSources] ne ""} {
          ::EVBC::_waitForEventBuilder
          startEVBSources
          
            # Before allowing the reaout(s) to start we want to wait a bit to be
            # sure the sources are connected to the ring master and all that rot.
            # this is needed in case the begin run action is quick.  We'll wait
            # two seconds for now.  In testing with systems that essentially don't
            # have initialization, I observed that sometimes the begin run events
            # would get missed and this is why I think it happens.
        
            after 2000
          
        }
    } else {
	EVBC::reset
    }
    _updatePriorParams
    
}
#------------------------------------------------------------------------------
##
# @fn EVBC::onEnd
#
#  Call this from the user's OnEnd method.
#   - Flushes the event builder.
#   - reenables the gui if it exists.
proc EVBC::onEnd {} {
    catch {EVBC::flush} ;  # Catch in case the evb exited badly.
    
    if {[::EVBC::_guiExists]} {
            EVBC::_EnableGUI
    }
}

#-------------------------------------------------------------------------------
# @fn EVBC::configParams
#    Send dynamic configuration options (EVB::config command).
#    If the event builder is actually running, the config param is sent now
#    If not, the config param is sent on all subsequent event builder starts.
#
# @param parameter - the parameter to configure must be one of
#                    * window set number of seconds in the build window.
#                    * XoffThreshold - set the number of queued bytes before xoffing.
#                    * XonThreshold  - set then umber of queued bytes at which XON
#                    * perQXoffThreshold - set the number of Fragments in a queue for xoff
#                    * perQXonThreshold  - set the number of Fragments in a queue for xon
# @param value    - A new positive integer value (all config parameters above take
#                   positive integers).
#
# @note - it is an error (not caught) for XonThreshold to be larger than Xoffthreshold.
# @note - I actually anticipate the user is going to mostly adjust the build window
#         so that the event builder latency gets better matched to its sources.
#
proc EVBC::configParams {parameter value} {
    
    # Validate the parameter name:
    
    set configParams [list window XoffThreshold XonThreshold perQXoffThreshold perQXonThreshold]
    if {$parameter ni $configParams} {
        error "EVBC::configure $parameter must be one of [join $configParams {, }]"
    }
    # Validate the parameter value:
    
    if {![string is integer -strict $value]} {
        error "EVBC::configure $parameter value $value must be an integer and is not"
    }
    if {$value <= 0} {
        error "EVBC::configure $parameter value $value must be strictly > 0"
    }
    
    set EVBC::$parameter $value;         # Send next start.
    
    ## Send now if we can.
    
    if {$::EVBC::pipefd ne ""}  {
        # send the command and flush the pipe:
        
        puts $EVBC::pipefd "EVB::config set $parameter $value"
        ::flush $EVBC::pipefd
    }
}
    

#------------------------------------------------------------------------------
## 
# @fn EVBC::isRunning
#
# Checks whether there is a channel associated with the event builder. Notice that
# if the eof occurs, the EVBC::pipefd gets reset to "".
#
# @returns boolean
# @retval 0 - EVB is not running
# @retval 1 - EVB is running
proc EVBC::isRunning {} {
  return [expr {$::EVBC::pipefd ne ""}]
}
###############################################################################
#
# Private procs.
#
###############################################################################

##
# EVBC::_getEvbPort
#   @return mixed
#   @retval integer - port number on which the event builder is waiting for
#                    connections.
#   @retval string "" - The event builder isn't yet advertising
#
proc EVBC::_getEvbPort {} {
    set appName [::EVBC::getAppName]
    set user    $::tcl_platform(user)
    set manager [::portAllocator %AUTO%]
    set result  [$manager findServer $appName $user]
    $manager destroy
    
    return $result
}

##
# EVBC::_paramsChanged
#
#  @return bool - true if the event builder parameters have changed
#                 since the last time around (priors different from current).
#
#  This can be used to see if an event builder restart is required.
#
proc EVBC::_paramsChanged {} {

    # if the runningEVBOptions does not exist yet because this is the first time around,
    # skip the check...
    if {$::EVBC::runningEVBOptions eq {}} {
      return false
    }

    set options [$EVBC::applicationOptions info options]
    # check to see if any of the app options are different than the currently running event builder
    foreach opt $options {
      if {[$::EVBC::applicationOptions cget $opt] != [$::EVBC::runningEVBOptions cget $opt]} {
        return true
      }
    }
    return false;
    
}
##
# EVBC::_updatePriorParams
#
#    Updates all the prior values from the current values.  This is book keeping
#    that is used to determine when a persistent event builder must be restarted
#    in order to incorporate changes in the desired event builder parameters.
#    Restarts are required, in that case because many of the parameters are
#    static parameters of the pipeline elements gotten via command line switches
#    and there's no mechanism to change them once they've started.
#
proc EVBC::_updatePriorParams {} {

    # the EVBC::runningEVBOptions object will be created the first time this proc gets called
    if {$::EVBC::runningEVBOptions eq {}} {
      set ::EVBC::runningEVBOptions [::EVBC::AppOptions %AUTO%]
    }

    set options [$::EVBC::applicationOptions info options]
    foreach opt $options {
      $::EVBC::runningEVBOptions configure $opt [$::EVBC::applicationOptions cget $opt]
    }
}

#------------------------------------------------------------------------------
##
# @fn [private] EVBC::_CheckPipeline
#
#   Errors if the event builder pipeline is not running
#
# @param msgPrefix - prefixes the error message.
#
proc EVBC::_CheckPipeline {msgPrefix} {
    if {$EVBC::pipefd eq ""} {
        error "$msgPrefix the event buider pipleline is not running"
    }
}
#------------------------------------------------------------------------------
##
# @fn [private] EVBC::_PipeInputReady
#
# Called when the event builder pipeline has input ready.
# Read the input line and _Output it.
#
# If the EOF is reached close the pipe and collect the error message as well.
#
proc EVBC::_PipeInputReady {} {
    if {[eof $EVBC::pipefd]} {
        catch {close $EVBC::pipefd} msg
        EVBC::_Output "Event builder pipeline exited $msg"
        set EVBC::pipefd ""
        #
        # Ensure the entire pipeline is dead too, and bitch if this is unexpected:
        #
        if {[llength $::EVBC::evbpids] != 0} {
            tk_messageBox -icon error -title {EVB pipe exit}  -type ok \
            -message {An element of the event builder pipeline exited.  Killing the entire pipe}
            foreach pid $::EVBC::evbpids {
                catch {exec kill -9 $pid}
            }
        }
    } else {
        EVBC::_Output [gets $EVBC::pipefd]
    }
}

#-------------------------------------------------------------------------------
##
# @fn [private] EVBC::_Output
#
#  Outputs a message.
#  TODO: While debugging this goes to stdout... later it needs to go to the
#  output window of the readoutGUI.
#
# @param msg - the message to output.
#
proc EVBC::_Output msg {
    ReadoutGUIPanel::outputText "[clock format [clock seconds]] $msg"
}

#-----------------------------------------------------------------------------
##
# @fn [private] EVBC::_ValidateOptions
#
#  Does semantic validation of program options and EVBC::start options.
#
# @param options - An option object.  This can be eitherEV
#                  an instance of either EVBC::StartOptions or EVBC::AppOptions
#
proc EVBC::_ValidateOptions options  {
    
    # If not -glombuild put in a default -glomdt of 1 since it's ignored:
    
    if {![$options cget -glombuild]} {
        $options configure -glomdt 1
    }
    #  -glomdt is required
    
    if {[$options cget -glomdt] eq ""} {
        error "EVBC::start -glomdt must be given a value"
    }
    
}
#-------------------------------------------------------------------------
#   Code for the event builder control panel.

##
# ::EVBC::_checkWarnRestart
#    Determines if the user needs to be warned about an event builder restart.
#    *   A warning dialog is popped up if there's not been a change yet
#        (indicating the proposed change is the first), the event builder
#        -restart option is not set, and the event builder has already been started
#        at least once
#
#  @return boolean  true if the proposed change should be backed out.
#
proc ::EVBC::_checkWarnRestart {} {
    
    # EVBC::runningEVBOptions is created when the event builder is started
    if {(![::EVBC::_paramsChanged]) && (![$::EVBC::applicationOptions cget -restart]) \
        && ($::EVBC::runningEVBOptions ne {})} {
        set result [tk_messageBox                   \
            -title {EVB Restart needed}             \
            -message {A change to event builder parameters will require the
 event builder be restarted at the next begin run.  Are you sure you want to do
 this?}                                              \
            -type yesno                              \
            -icon warning
        ]
        
        return [expr {$result eq "no" ? 1 : 0}]
    }
    return false
}
##
# ::EVBC::_onTsPolicyChanged
#    Called when the user attempts to change the timestamp policy.
#    This change is only done if:
#    *   The event builder is in -restart mode.
#    *   If the event builder is not in -restart mode, and the user says its ok
#        to restart it.
#  @param w      - The control panel widget.
#  @param policy - the new policy.
#
proc ::EVBC::_onTsPolicyChanged {w policy} {
    if {![::EVBC::_checkWarnRestart]} {
        $::EVBC::applicationOptions configure -glomtspolicy $policy
    } else {
        $w configure -tspolicy [$::EVBC::applicationOptions cget -glomtspolicy]
    }
}
##
# ::EVBC::_onGlomParamsChanged
#    Called whenever the user changes a glom parameter.  The glom parameters
#    that can be modified are the build/no build and the coincidence time window.
#
#    See ::EVBC::_onTsPolicyChanged for when we accept the user's change.
# @param w - widget that is the control panel.
# @param build - Boolean indicating if event building should be done.
# @param dt    - Coincidence window for the build.
# @param maxf  - max frags
#
proc ::EVBC::_onGlomParamsChanged {w build dt maxf} {
    
    if {![::EVBC::_checkWarnRestart]} {
    
        $::EVBC::applicationOptions configure -glomdt $dt
        $::EVBC::applicationOptions configure -glombuild $build
        $::EVBC::applicationOptions configure -maxfragments $maxf
    } else {
    
        $w configure -build [$::EVBC::applicationOptions cget -glombuild]
        $w configure -dt    [$::EVBC::applicationOptions cget -glomdt]
        $w configure -maxfragments $maxf
    }
}
##
#  ::EVBC::_onTeeChange
#
#    Called when there's been a change to the parameters controlling the use
#    of an intermediate ring.  An intermediate ring allows users to snoop on the
#    event builder ordered fragments before they hit glom.
#
# @param w  - The ::EVBC::intermedRing widget that is managing those parameters.
#
# @note EVBC::_checkWarnRestart is used to potentially veto the change.
#
proc ::EVBC::_onTeeChange w {
    if {![::EVBC::_checkWarnRestart]} {
        $::EVBC::applicationOptions configure -teering [$w cget -tee]
        $::EVBC::applicationOptions configure -teeringname [$w cget -ring]
    } else {
        $w configure -tee [$::EVBC::applicationOptions cget -teering]
        $w configure -ring [$::EVBC::applicationOptions cget -teeringname]
    }
}
##
# ::EVBC::_onDestRingChanged
#    Called when the output ring controls have changed.  After potentially
#    asking the user if it's ok to make the changes, the changes get made.
#
# @param w - An ::EVBC::destring widget that contains the controls.
#
proc ::EVBC::_onDestRingChanged w {
    if {![::EVBC::_checkWarnRestart]} {
        $::EVBC::applicationOptions configure -destring [$w cget -ring]
        $::EVBC::applicationOptions configure \
            -setdestringasevtlogsource [$w cget -record] 
        if {[$EVBC::applicationOptions cget -setdestringasevtlogsource] } {
            ::Configuration::Set EventLoggerRing "tcp://localhost/[$::EVBC::applicationOptions cget -destring]"
        }
    } else {
        $w configure -ring [$::EVBC::applicationOptions cget -destring]
        $w configure -record [$::EVBC::applicationOptions cget -setdestringasevtlogsource]
    }
}

##
# @fn EVBC::_StartGui
#
#  Create the event builder GUI and glue it into the ReadoutGUI frame.
#  TODO: For now this all goes into a toplevel .evgui
#
#   The GUI contains:
#   *  A checkbox that enables/disables glom building.
#   *  A glom --dt spinner.
#   *  An entry for the destination ring name.
#   *  A checkbox to enable an intermediate fragmen ring.
#   *  An entry for the name of that ring.
#
proc EVBC::_StartGui {} {
    
    ::EVBC::eventbuildercp .evbcp
    grid .evbcp -sticky nsew -padx 9 -pady 9 
    grid anchor . center

    #  Connect .evbcp to the glom parameters, and set the initial values
    #  of the UI:
    
    ::EVBC::updateGuiFromOptions $::EVBC::applicationOptions
}
##
# updateGuiFromOptions
#
#   Sets the value of the event builder GUI to be consistent with the EVBC::AppOptions
#   object that is passed in as a parameter.
#
#   It is up to the caller to ensure that the GUI and its fields exist.
#
#   \param appOpts an EVBC::AppOptions object
#
proc ::EVBC::updateGuiFromOptions {appOpts} {
    
    #  Connect .evbcp to the glom parameters, and set the initial values
    #  of the UI:
    
    .evbcp configure -tspolicy [$appOpts cget -glomtspolicy]
    .evbcp configure -build    [$appOpts cget -glombuild]
    .evbcp configure -dt       [$appOpts cget -glomdt]
    .evbcp configure -maxfragments [$appOpts cget -maxfragments]
    
    .evbcp configure -tscommand [list ::EVBC::_onTsPolicyChanged .evbcp %P] 
    .evbcp configure -glomcmd   [list ::EVBC::_onGlomParamsChanged .evbcp %B %T %M]
    
    
    #  Connect the tee ring controls and set initial values of the UI:
    
    .evbcp configure -tee     [$appOpts cget -teering]
    .evbcp configure -teering [$appOpts cget -teeringname]
    .evbcp configure -teecommand [list ::EVBC::_onTeeChange %W]
    
    # Connect the output ring controls and set the UI's initial values.
    
    .evbcp configure -oring   [$appOpts cget -destring]
    .evbcp configure -record  [$appOpts cget -setdestringasevtlogsource]
    .evbcp configure -oringcommand [list ::EVBC::_onDestRingChanged %W]
    
}
# @fn EVBC::_EnableGUI
#
#  Enbable the gui.
#  To start with all non terminal states are set normal
#  Once that's done we need to selectively disable elements depending on the state
#  of the UI itself.
#
#
proc EVBC::_EnableGUI {} {
    .evbcp configure -state normal
}
#-----------------------------------------------------------------------------
##
# @fn EVBC::_DisableGui
#
#  Disables the GUI.
#  This is pretty easy.. we just need to set the state of all terminal
#  widgets to disabled:
#
proc EVBC::_DisableGUI {} {
    .evbcp configure -state disabled
}

#------------------------------------------------------------------------------
##
# @fn EVBC::_Exiting
#
#   Called when we're exiting.. if the event builder is running
#   kill it off:
# @param w - the widget that's being destroyed.
#             We wish we could care about . but that's caught by the main stuff.
#
proc EVBC::_Exiting w {
    if {($w eq ".runnumber") && ($EVBC::pipefd ne "")} {
        EVBC::stop

    }
}
#------------------------------------------------------------------------------
##
# @fn EVBC::_waitForEventBuilder
#
#  Waits for the event builder to be ready to accept TCP/IP connections
#  By now it's assumed the event builder is visible to the port manager.
#
#
proc EVBC::_waitForEventBuilder {} {

    # Figure out which port the event builder is listening on:

    
    set port [EVBC::_getEvbPort]
    
    if {$port eq ""} {
        error "EVBC::_waitForEventBuilder - event builder isn't advertising port"
    }

    #  Try to connect every 100ms until success:

    while {[catch {socket localhost $port} fd]} {
        after 100
    }
    close $fd
}


#-------------------------------------------------------------------------------
#
#  The code below provides a state machine bundle that is now the preferred
#  way to register this code.  Note that the user is still going to have
#  to provide callbaks to start the event sources.
#  TODO:  Maybe each data source provider can include a call that will
#         know how to start an associated evb data source for each instance
#         it manages?
#


##
#  EVBC::attach
#    Called when the bundle is attached to the state manager
#    Initialize the event builder. 
#
# @param state - the current state.
#
# @note you can use EVBC::configure to set the configuration of the
#       manager.  This includes enabling the GUI and so on.
#
proc ::EVBC::attach state {
}
##
# EVBC::enter
#   Called when a new state is entered.
#   * Active -> Halted  invoke onEnd
# @param from - state that we left.
# @param to   - State that we are entring.
#
proc ::EVBC::enter {from to} {
    if {($from eq "Active") && ($to eq "Halted")} {
        ::EVBC::onEnd
    }
    if {$to eq "NotReady"} {
        catch {::EVBC::stop}
    }
}
##
# EVBC::leave
#   Called when a state is being left.
#   * Halted -> Active   invoke onBegin - we do this inleave to get the jump
#     on when the data sources start spewing data.
#
# @param from - State we are leaving.
# @param to   - State we are about to enter.
#
proc ::EVBC::leave {from to} {
    if {($from eq "Halted") && ($to eq "Active")} {
        ::EVBC::onBegin
    }
}


##
#  useEventBuilder
#     Register the event builder with the state manager
#
#     This also adds the RingSourceMgr immediately afterwards to work with
#     it.
#
#
proc EVBC::useEventBuilder {} {

    if {$EVBC::registered == 0} {
        set stateMachine [RunstateMachineSingleton %AUTO%]
        set callouts [$stateMachine listCalloutBundles]
        $stateMachine addCalloutBundle EVBC [lindex $callouts 0]
        set ::EVBC::registered 1
        $stateMachine addCalloutBundle ::RingSourceMgr [lindex $callouts 0]
        ::EndrunMon::register [lindex $callouts 0]
    }
    
}

##
# @brief Checks to see if the standard window for evb controls exists
#
proc ::EVBC::_guiExists {} {
  return [winfo exists .evbcp]
}

##
#  configure
#     Configure the event builder options.
#
# @param args - configuration parameters.  This is a list
#               of alternating option and option values.
#
# @note The event builder must have already been registered via EVBC::useEventBuilder
# @note If necessary the GUI is added/removed.
#
proc ::EVBC::configure args {
    
    # First configure
    
    $::EVBC::applicationOptions  configure {*}$args
    
    # Now see what we need to do with the GUI (start/stop leave alone).
    
    set gui [$::EVBC::applicationOptions cget -gui]
    if {$gui && ![::EVBC::_guiExists]} {
        ::EVBC::_StartGui
    }
    
    if {(!$gui) && [::EVBC::_guiExists]} {
        destroy .evbcp 
    }
    # If the GUI is active we must update its elements
    
    if {[::EVBC::_guiExists]} {
        ::EVBC::updateGuiFromOptions $::EVBC::applicationOptions
    }
}

namespace eval ::EVBC {
    namespace export attach enter leave
}


