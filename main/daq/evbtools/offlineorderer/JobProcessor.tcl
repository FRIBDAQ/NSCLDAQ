#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#      NSCLDAQ Development Group
#
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

# This software is Copyright by the Board of Trustees of Michigan
# State University (c) Copyright 2297884. 

# @file  JobProcessor.tcl 
# @author Jeromy Tompkins 

package provide OfflineEVBJobProcessor 11.0

package require snit
package require eventLogBundle

package require DataSourceManager
package require DataSourceMonitor
package require EVBStateCallouts
package require ringsourcemgr

package require ExpFileSystem
package require OfflineEVBInputPipeline
package require OfflineEVBHoistPipeline
package require evbcallouts
package require evbconfigure

package require Thread
package require OfflineEVBOutputPipeline
package require RunStateObserver

## @namespace HoistConfig
#
# @brief A location to put data for building the startEVBSources
#        proc that is easy to find and reference.
#
namespace eval HoistConfig {
  variable tstamplib
  variable info
  variable ring
  variable id
  variable expectbh 
}


## @brief Coordinates the configuration and launching of the different pipelines
#
# This works with the RunstateMachine and its various callout bundles to 
# process a job. A job is defined as a set of the following objects:
# 
# OfflineEVBInputPipeParams
# OfflineEVBHoistPipeParams
# EVBC::AppOptions
# OfflineEVBOutputPipeParams
#
# These each are just structures containing the configuration parameters needed
# to properly set up the variable pipelines. The parameters that they control 
# are not exhaustive but do provide a large set of optional configuration parameters.
#
# The JobProcessor is passed these parameters sets through its options via a
# configure command. It expects that the RunstateMachine and DataSourceManager
# are both clean when it begins. It then fills them with the necessary callout
# bundles and an Offline data providers and registers them to the state machine.
# Beginning a run just amounts to transitioning the state machine to an Active mode.
# Because the user may choose to set up the system to work with multiple jobs, it 
# always cleans up after it is done.
#         
#
snit::type JobProcessor {

  option -inputparams  -default ""
  option -hoistparams  -default ""
  option -evbparams    -default ""
  option -outputparams -default ""

  option -runprocessor -default ""

  variable sourceManager ""
  variable stateMachine  ""
  variable completionStatus FAIL ; # assume failure unless told otherwise
  variable runStateObserver ""
  variable processingComplete 0

  ## @brief Pass the options
  #
  constructor {args} {
    set sourceManager [DataSourcemanagerSingleton %AUTO%]
    set stateMachine  [RunstateMachineSingleton %AUTO%]
    set processingComplete 0

    set runStateObserver [RunStateObserver %AUTO% -onend [mymethod onEndRun]]

    $self configurelist $args

  }

  ## Cleanup the data source manager and state machine if they exist
  #
  destructor {
    if {$sourceManager ne ""} {
      $sourceManager destroy
      set sourceManager ""
    }
    if {$stateMachine ne ""} {
      $stateMachine destroy
      set stateMachine ""
    }

    $runStateObserver destroy
  }

  ## @brief Register all of the bundles that we need
  # and no more
  #
  method setupStateMachine {} {
    # rdoCalloutsBundle has already been registered
    ::EventLog::register
    ::EVBStateCallouts::register
    ::RingSourceMgr::register  EventLog
    ::EVBConfigure::register
    ::DataSourceMgr::register
  }

  ## @brief return the data source manager known to this
  #
  method getDataSourceManager {} {
    variable sourceManager
    return $sourceManager

  }

  ## @brief return the state machine known to this
  #
  method getRunStateMachine {} {
    variable stateMachine 
    return $stateMachine
  }

  ## @brief Entry point for the actual processing
  # 
  # Sets up all the pipelines and then processes all of the files provided
  #
  method run {} {
    set completionStatus FAIL
    # hook to handle one-time startup procedures 
    $self setup

    set processingComplete 0
    if {![catch {$self startProcessing} msg]} {
     set newRing tcp://localhost/[$options(-evbparams) cget -destring]
     set currentRing [$runStateObserver cget -ringurl]
     if {$currentRing ne $newRing} {
       if {$currentRing ne {}} {
         $runStateObserver detachFromRing
       }
       $runStateObserver configure -ringurl $newRing
       $runStateObserver attachToRing
     }

      vwait [myvar processingComplete]

      # wait for some time to finish getting all of the end runs
      after 100

      if {$processingComplete == 1} {
        # job ended normally
        puts "we are going to try to end this thing"

        if {[::EventLog::isRunPending]} { 
          ::EventLog::runEnding
        }
        # the transition to active was successful so we should expect that 
        # processing succeeds.
        #
        # Tell the eventlog that it is okay for it to exit, because that is 
        # what we expect.
        #
        # Currently we have no mechanism for aborting a run in progress.
        #
        set completionStatus OK
        $self stopProcessing   ;# stop the processing pipelines

      } else {
        # run aborted
         puts "aborted"
        set completionStatus ABORT
        $self forceStopProcessing
      }

    }

    return $completionStatus
  }

  method onEndRun {item} {
    set processingComplete 1
  }

  ## @brief Load and configure the callout bundles 
  #
  #
  method setup {} {
    $self setupStateMachine 

    $self generateStartEVBSources 

    $self configureEVB

    $self configureEventLog

    $self setupDataSourceManager

  }

  ## @brief Launch pipelines that live for the duration of a file
  #
  # This is actually very simple because it merely transitions the
  # state machine into an Active state and tells the EventLog that 
  #
  # Failed transitions are detected and then repropagated back up to 
  # the caller to handle.
  #
  # @thows error on a failed transition.
  #
  method startProcessing {} {
    variable stateMachine

    # Transition the state machine to Starting (note this schedules
    # a transition to Active on its own)
    if {[catch {$stateMachine transition Starting} msg]} {
      # things may not like being forced back to NotReady
      catch {$stateMachine transition NotReady}
      $self tearDown
      set msg "JobProcessor::startProcessing failed to transition to Starting : $msg"
      tk_messageBox -icon error -message $msg
      return -code error $msg 
    }
    # We need to wait for the scheduled transition to succeed
    # before transitioning to Active because otherwise it will fail
    $self waitForHalted

    # exceptional behavior is caught by the transition
    if {[catch {$stateMachine transition Active} msg]} {
      # things may not like being forced back to NotReady
      catch {$stateMachine transition NotReady}
      $self tearDown
      set msg "JobProcessor::startProcessing failed to transition to Active : $msg"
      tk_messageBox -icon error -message $msg
      return -code error $msg
    }

  }

  ## @brief Iteratively check to see if the state machine has become Halted.
  #
  # This polls every 100 ms.
  #
  # During each iteration, the window should remain live because of
  # the update calls.
  #
  method waitForHalted {} {

    set state [$stateMachine getState]
    while {$state ne "Halted"} {
      after 100 
      update
      set state [$stateMachine getState]
    }
  }
 
  ## @brief Transition the state machine into a NotReady state
  #
  method stopProcessing {} {

    if {[$stateMachine getState] eq "Active"} {
      $stateMachine transition Halted
    }

    $stateMachine transition NotReady

    $self tearDown 
  }

  method forceStopProcessing {} {
    $stateMachine transition NotReady
    $self tearDown
  }

  ## @brief Transition the system into a clean state
  #
  method tearDown {} {

    $self clearDataSources
    
    $self tearDownStateMachine
  }


  ## @brief Remove all of the data providers 
  #
  # There really should only be one data provider in the
  # DataSourceManager: Offline. However, we will treat the
  # scenario that there are lots in case someone down the 
  # road decides we need more.
  #
  method clearDataSources {} {
    variable sourceManager

    # unload the Offline provider... catch it because it can fail 
    catch {$sourceManager unload Offline}

    # Remove all of the data sources
    set sources [$sourceManager sources]
    foreach source $sources {
      set id [dict get $source sourceid]
      $sourceManager removeSource $id
    }


  }

  ## @brief Remove all callout bundles from the state machine
  #
  method tearDownStateMachine {} {
    variable stateMachine

    set bundles [$stateMachine listCalloutBundles]
    foreach bundle $bundles {
      $stateMachine removeCalloutBundle $bundle
    }

  }


  ## Use the hoist parameters to construct the startEVBSources method
  #
  # THIS IS DIRTY AND COULD POTENTIALLY BE REPLACED WITH A hoist pipeline callout bundle
  method generateStartEVBSources {} {
    if {$options(-hoistparams) eq ""} {
      set msg "JobProcessor::generateStartEVBSources cannot proceed because "
      append msg  "-hoistparams are nonexistent."
      return -code error $msg
    }
    set ::HoistConfig::tstamplib  [$options(-hoistparams) cget -tstamplib]
    set ::HoistConfig::info       [$options(-hoistparams) cget -info]
    set ::HoistConfig::ring       [$options(-hoistparams) cget -sourcering]
    set ::HoistConfig::id         [$options(-hoistparams) cget -id]
    set ::HoistConfig::expectbh   [$options(-hoistparams) cget -expectbheaders]

    ::EVBC::registerRingSource tcp://localhost/$::HoistConfig::ring \
      $::HoistConfig::tstamplib \
      $::HoistConfig::id \
      $::HoistConfig::info \
      $::HoistConfig::expectbh
  }

  ## Pass the configuration information to the ::EVBC:: parameters
  #
  method configureEVB {} {

    # set up a convenience name for options(-evbparams) 
    set params $options(-evbparams)

    # -evbparams is unique b/c the options have already been encapsulated as
    # EVBC::AppOptions. We therefore use those by default.
    set res [catch {
      expr {[$params info type] eq "EVBC::AppOptions"}
    } msg]


    if {$res} {
      return -code error $msg
    } else {
      $params configure -gui false
      $params configure -restart false

      set opts [$params info options]
      foreach opt $opts {
        dict set optDict $opt [$params cget $opt]
      }
      EVBC::initialize {*}$optDict
    }

  }

  ## Pass the configuration info to the event log
  #
  method configureEventLog {} {
    set stagearea [$options(-outputparams) cget -stagearea]
    Configuration::Set StageArea  $stagearea
    if {! [file exists [ExpFileSystem::getStageArea]]} {
      ExpFileSystem::CreateHierarchy
    }

    # set up the parameters
    Configuration::Set EventLogRunFilePrefix     [$options(-outputparams) cget -prefix]
    Configuration::Set EventLoggerRing           [$options(-outputparams) cget -ringname]
    Configuration::Set EventLogUseNsrcsFlag      1 

    # when starting the eventlog the builder will compute how many data sources are 
    # registered and uses that number to set the value of the --number-of-sources
    # parameter. Since we are only expecting 1 data source, then we can set up the
    Configuration::Set EventLogAdditionalSources [expr [$options(-outputparams) cget -nsources]-1] 
    Configuration::Set EventLogUseChecksumFlag   1 
  }

  ##  Make sure that each time we start up a new job,
  #   the data sources are all started from a stopped
  #   state. 
  #
  method setupDataSourceManager {} {
    variable sourceManager
    $sourceManager load Offline
    set sources [$sourceManager sources]
    foreach source $sources {
      set id [dict get $source sourceid]
      $sourceManager removeSource $id
    }

    $sourceManager addSource Offline [dict create unglomid [$options(-inputparams) cget -unglomid] \
                                                ring [$options(-inputparams) cget -inputring] \
                                                file [$options(-inputparams) cget -file]]
  }


  method abortRun {} {
    puts "setting processingComplete -> 2"
    set processingComplete 2
  }

}


