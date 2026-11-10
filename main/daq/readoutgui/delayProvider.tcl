#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#      NSCL Data Acquisition Group 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

##
# @file delayProvider.tcl
# @brief A mechanism for inserting a delay between begins received by 
#        data providers. 
# @author Jeromy Tompkins 
#

package provide Delay_Provider 1.0

# Establish the namespace in which the methods and state live and
# define the state variables.
#

namespace eval ::Delay {
  variable destroyCmd 0 ;#< After id for destroying progress dialog
  
  variable instances;    # holds parameterizations of each instance
  array set instances [list] ; # indexed by source id.
  variable delay_index 0;  #index to append to delay variable.
  
}

#------------------------------------------------------------------------------
#  Public interface of the data source provider.

##
# parameters
#    Returns the set of parameters that are required to create the source.
#    For now these are:
#    - delay  - The time to delay in milliseconds 
#
proc ::Delay::parameters {} {
    return [dict create delay [list {milliseconds to delay on begin}] \
                        enddelay [list {milliseconds to delay on end}]]
}

##
# start
#   Do nothing.
#
# @param params - Dict containing the parameterization of the source.
#
#
proc ::Delay::start params {
  
  set id [dict get $params sourceid]

  #  Error for duplicated id:
  #
  if {[array names instances $id] ne "" } {
    error "Delay already has an instance $id"
  }
  
  set delayTime [dict get $params delay]
  set endDelayTime [dict get $params enddelay]
  set ::Delay::instances($id) [list $delayTime $endDelayTime]
}
##
# check
#   Check the status of the connection to the source.
#   Do nothing
#
# @param id - Source id of the provider instance.
# @return always true 
#
proc ::Delay::check id {
  return 1
}
##
# stop
#   Stop the data source.  Remove the id from the array
#
# @param id - Id of the source to close.
#
proc ::Delay::stop id {
  if {[array  names ::Delay::instances $id] eq ""} {
    error "Delay::stop - $id is not a known source"
  }
  array unset ::Delay::instances $id
}
##
# begin
#   Wait for the specified amount of time in the delay parameter
#
# @param id - Source id.
# @param run  - Run number desired.
# @param title - Title desired.
#
proc ::Delay::begin {id run title} {
  if {[array  names ::Delay::instances $id] eq ""} {
    error "Delay::begin- $id is not a known source"
  }
  
  set config $::Delay::instances($id)
  set delayTime [lindex $config 0]
  
  if {$delayTime != 0} {
    ::Delay::_delayWithFeedback $delayTime
  }
}

##
# end
#   Wait the end delay.
#
# @param id - Id of the source to end.
#
proc ::Delay::end id {
  if {[array  names ::Delay::instances $id] eq ""} {
    error "Delay::end - $id is not a known source"
  }
  set config $::Delay::instances($id)
  set endDelayTime [lindex $config 1]
  
  if {$endDelayTime != 0} {
    ::Delay::_delayWithFeedback $endDelayTime
  }
}
#   Support pause/resume...they do nothing but they do provide
#   The capability.

proc ::Delay::pause id {}
proc ::Delay::resume id {}

##
# init
#   Do nothing
#
proc ::Delay::init id {}

##
# capabilities
#   Returns a dict describing the capabilities of the source:
#   - canpause - false
#   - runsHaevTitles - true
#   - runsHaveNumbers - true
#
# @return dict - as described above.
#
proc ::Delay::capabilities {} {
    return [dict create \
        canPause        true    \
        runsHaveTitles  true    \
        runsHaveNumbers true    \
    ]
}

## 
#  Called to initiate the delay. Since the delay is
#  by definition synchronous we can assume only
#  one delay is in progress at a time and, therefore,
#  use a single, common variable to synchronize
#  nonetheless - being chicken, Each delay
#  _will_ use a separate variable named
#  ::Delay::done_n where n is an integer
#  from delay_index.
#  This is probably not needed -- after all there's only 
#  one Delay progress user interface element.
#
proc ::Delay::_delayWithFeedback {duration} {
  if {[winfo exists .delay]} {
    catch {after cancel $::Delay::destroyCmd}
    destroy .delay

  } 
  #  Make the user interface.
  toplevel .delay
  ttk::label .delay.title -text "Delay in progress"
  ttk::progressbar .delay.progress -orient horizontal -mode determinate \
    -maximum $duration -value 0
  grid .delay.title -sticky new -padx 8 -pady 8
  grid .delay.progress -sticky new -padx 8 -pady 8
  grid rowconfigure .delay 0 -weight 1
  grid columnconfigure .delay 0 -weight 1
  
  
  # Compute the sub-delay lengths in ms:

  set increment [expr int($duration/10)]; 
  if {$increment == 0} {set increment 1}
  
  #  increment the delay variable index:

  incr ::Delay::delay_index

  #  kick off the delay:

  ::Delay::_delay $increment $duration ::Delay::done_${::Delay::delay_index} 0 .delay.progress

  vwait ::Delay::done_${::Delay::delay_index}
  
  set ::Delay::destroyCmd [after 1000 destroy .delay]; 
  
}
##
# ::Delay::_delay:
#
#  This sets iself up to be called from the event loop after each
#   delay increment.  Doing so, solves issue #19 - excessive delays
#   due to the pile up of events during synchronous segmented delays
#   (I think).
#
# @param increment - ms between each delay.
# @param total     - Total duration.
# @param signalVar - Fully qualified name of ariable used to signal completion.
# @param progress  - Progress through the delay.
# @param widget    - ttk::progressbar being used to keep track of delay progress.
#
proc ::Delay::_delay {increment total signalVar progress widget} {
    #  Are we done yet?

    if {$progress >= $total} {
        incr $signalVar
    } else {
        $widget configure -value $progress
        set progress [expr {$progress + $increment }]

        # queue the next iteration:

        after $increment [list ::Delay::_delay $increment $total $signalVar $progress $widget]
    }
}

