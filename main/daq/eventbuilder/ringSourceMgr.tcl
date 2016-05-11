#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2015.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#      NSCLDAQ Development Group
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file ringSourceMgr.tcl


package provide ringsourcemgr 1.0

package require InstallRoot
package require portAllocator
package require evbcallouts
package require EndrunMon

#-------------------------------------------------------------------------------
#

## @brief callout bundle for launching ringFragmentSources
#
# This attempts to gather together the information for all clients to the EVB 
# into one place. The user provides the information to this by registering sources
# rather than by implementing their own startEVBSources proc. The startEVBSources proc
# is still usable and called by the EVBC bundle, but this is the preferred mechanism for 
# starting up data sources. The drawback of the startEVBSources proc is that it is 
# essentially a black box to the rest of the ReadoutGUI. There is no way to know what 
# actually happens in that proc and it could be used for things other than starting up
# clients to the event builder. If instead the user makes use of this, the information
# for all clients is stored and is available to the rest of ReadoutGUI at any time.
#
#
# @important The bundle should not be registered on its own. If the EVBC bundle does not precede
# this bundle, then the system will wait indefinitely for the event builder to start 
# up and never move on. For that reason, it should be registered using the 
# EVBC::useEventBuilder proc that registers the EVBC bundle and then this 
# immediately afterwards. 
#
namespace eval ::RingSourceMgr {
  variable sourceDict [dict create];            # Sources we defined
  variable diedSources [dict create];           # Sources that exited.
}

#------------------------------------------------------------------------------
## @brief Inserts an entry into the registry
#
# Because we are storing our source information in a dict whose key is the source uri, 
# there can only be one source per ring. I cannot foresee any reason we would want to 
# feed an eventbuilder the same data from a ring twice and this ensures that does not 
# happen.
# 
# @param source     uri of data source
# @param tstamplib  path to timestamp extractor library
# @param id         source id 
# @param info       textual description for source
# @param expectHdrs whether the source should have headers or not
# @param oneshot    If provided number of ends that result in exit.
# @param timeout    If provided, timeout in seconds after first end to wait for all ends
#                   in --oneshot mode.
<<<<<<< HEAD
#
proc ::RingSourceMgr::addSource [list source tstamplib {id ""} \
  {info ""} {expectHeaders 0} {oneshot ""} {timeout ""} ] {
=======
# @param offset     Time offset added to the timestamp from this source.
#
proc ::RingSourceMgr::addSource [list source tstamplib {id ""} \
  {info ""} {expectHeaders 0} {oneshot ""} {timeout ""} {offset 0} ] {
>>>>>>> master
    variable sourceDict

    # there will only ever be 1 ringFragmentSource launched per ring at a given a time
    # by using the ring url as the key.
    dict set sourceDict $source [dict create tstamplib $tstamplib \
      id         $id            \
      info       $info          \
      expecthdrs $expectHeaders \
      oneshot    $oneshot       \
      timeout    $timeout       \
<<<<<<< HEAD
=======
      offset     $offset        \
>>>>>>> master
      fd ""]
    
    
}

#------------------------------------------------------------------------------
##
# @brief Acquire the port for the event orderer service
#
#  Figure out whether port the event builder is running on.
#
# @returns the event orderer port number
proc ::RingSourceMgr::getOrdererPort {} {

  set portManager [::portAllocator create %AUTO%]
  set allocations [$portManager listPorts]
  set user $::tcl_platform(user)
  set appName [::EVBC::getAppName]
  set port ""
  foreach allocation $allocations {
    set name  [lindex $allocation 1]
    set owner [lindex $allocation 2]
    if {($name eq $appName) && ($owner eq $user)} {
      set port [lindex $allocation 0]
      break
    }
  }
  $portManager destroy

  if {$port eq ""} {
    return -code error "RingSourceMgr::getOrdererPort Unable to locate the event builder service"
  }
  return $port    
}

#------------------------------------------------------------------------------
## @brief Remove all sources from the registry
#
# @fn ::RingSourceMgr::clearSources
#
proc ::RingSourceMgr::clearSources {} {
  variable sourceDict
  set sourceDict [dict create]

}

#------------------------------------------------------------------------------
## @brief Set the handles associated with each source in the registry
#
# This is less severe than clearSources because it maintains all of the 
# information in the registry. The only thing that changes is that the
# file handle associated with each source is reset to its default value. 
# This will trigger the source to be started up the next transition from
# Halted to Active. It is actually the mechanism by which the EVBC callout
# bundle informs the RingSourceMgr to start new sources when it has its
# -restart option set.
#
proc ::RingSourceMgr::resetSources {} {
  variable sourceDict
  foreach source [dict keys $sourceDict] {
    dict set sourceDict $source fd ""
  }
}

#------------------------------------------------------------------------------
##
# @fn RingSourceMgr:startSource
#
#   Starts a ring buffer event source
#
# @param sourceRingUrl          - URL of the source ring.
# @param timestampExtractorLib  - Path to the timestamp extractor shared object.
# @param id                     - Source id used to associate event fragments
#                                 with an input queue.
# @param info                   - Long description used to identify the source
#                                 in the event orderer GUI.
# @param expectHeders True if body headers are expected and will be used to
#                    extract timestamp and ids.
# @param oneshot    If provided number of ends that result in exit.
# @param timeout    If provided, timeout in seconds after first end to wait for all ends
#                   in --oneshot mode.
# @note Event sources are subprocesses of us but not subprocesses of the
#       the event building pipeline.
#
# @returns file handle
proc ::RingSourceMgr::startSource {sourceRingUrl timestampExtractorLib id info
<<<<<<< HEAD
  {expectHeaders 0} {oneshot ""} {timeout ""} } {
=======
  {expectHeaders 0} {oneshot ""} {timeout ""} {offset 0} } {
>>>>>>> master

  set port [::RingSourceMgr::getOrdererPort]
  #  Construct the command we're going to run

  set ringSource [file join [InstallRoot::Where] bin ringFragmentSource]

  set switches [::RingSourceMgr::_computeRingSourceSwitches $port $sourceRingUrl \
    $timestampExtractorLib \
    $id \
    $info \
<<<<<<< HEAD
    $expectHeaders $oneshot $timeout]
=======
    $expectHeaders $oneshot $timeout $offset]
>>>>>>> master

  append ringSource $switches

  # Run the command in a pipeline that gets stderr/stdout and
  # set a fileevent on it so that we get output and errors and eof.
  # The trick with cat below ensures that we get both stderr and stdout.
  #
  set fd [open "| $ringSource |& cat" r]
  chan configure $fd -buffering line -blocking 0
  chan event $fd readable [list ::RingSourceMgr::_HandleDataSourceInput $fd $info $id]

  # Indicate to the end run monitor it will have an end run from this src:
    
  EndrunMon::incEndRunCount
  
  ::RingSourceMgr::addSource  \
    $sourceRingUrl $timestampExtractorLib $id $info  $expectHeaders \
    $oneshot $timeout
  dict set ::RingSourceMgr::sourceDict $sourceRingUrl fd $fd
  
  return $fd
}

#------------------------------------------------------------------------------
## @brief Begin run logic
#
# The logic within this is intended to prevent starting multiple 
# ringFragmentSources for the same event builder. When a source 
# is started, the handle associated with it is stored with the registry. 
# By checking whether the handle is already set, we can avoid starting 
# copies of the same client.
#
proc ::RingSourceMgr::onBegin {} {
  variable sourceDict
  dict for {source paramDict} $sourceDict {

  # find the handle associated with it.
    set fd [dict get $paramDict fd]
    # only start it if it is not already started.
    if {$fd eq ""} {
      set lib [dict get $paramDict tstamplib]
      set id [dict get $paramDict id]
      set info [dict get $paramDict info]
      set expectHeaders [dict get $paramDict expecthdrs]
<<<<<<< HEAD
      set fd [::RingSourceMgr::startSource $source $lib $id $info $expectHeaders]
=======
      set oneshot [dict get $paramDict oneshot]
      set timeout [dict get $paramDict timeout]
      set fd [::RingSourceMgr::startSource $source $lib $id $info \
                                           $expectHeaders $oneshot $timeout]
>>>>>>> master
      dict set sourceDict $source fd $fd
    }
  }
}
###########-------------------------------------------------------------#######
#                      BEGIN CALLOUT BUNDLE INTERFACE                         #



##
#  RingSourceMgr::attach
#    Called when the bundle is attached to the state manager
#    Initialize the event builder. 
#
# @param state - the current state.
#
proc ::RingSourceMgr::attach state {
}

##
#
#
proc ::RingSourceMgr::register {{beforeBundle {}}} {
  set sm [RunstateMachineSingleton %AUTO%]
  if {$beforeBundle eq {}} {
    $sm addCalloutBundle RingSourceMgr
  } else {
    $sm addCalloutBundle RingSourceMgr $beforeBundle
  }
  $sm destroy
}

#
#------------------------------------------------------------------------------
##
# EVBC::enter
#   Called when a new state is entered.
#   * Halted -> Active   invoke onBegin - we do this inleave to get the jump
#     on when the data sources start spewing data.
# @param from - state that we left.
# @param to   - State that we are entring.
#
proc ::RingSourceMgr::enter {from to} {
  if {$to eq "NotReady"} {
    ::RingSourceMgr::resetSources
  }
}


#------------------------------------------------------------------------------
##
# RingSourceMgr::leave
#   Called when a state is being left.
#   * Active -> Halted  invoke onEnd

#
# @param from - State we are leaving.
# @param to   - State we are about to enter.
#
proc ::RingSourceMgr::leave {from to} {
  if {($from eq "Halted") && ($to eq "Active")} {
    ::RingSourceMgr::_waitForEventBuilder
    ::RingSourceMgr::onBegin

    # give time to start up.
    after 100
  }
}

namespace eval ::RingSourceMgr {
  namespace export attach enter leave
}

############# PRIVATE HELPER PROCS ############################################

##
# @fn RingSourceMgr::_computeRingSourceSwitches
#
# @param port             the port of the orderer server
# @param url              url of the ring 
# @param tstampExtractor  path to tstamp extractor lib
<<<<<<< HEAD
# @param id               source id associated with source
=======
# @param ids              list of source ids associated with source
>>>>>>> master
# @param info             description of the source
# @param expectHeaders    boolean to specify --expectbodyheaders flag
# @param oneshot          --oneshot count "" if not using.
# @param timeout          --timeout seconds or "" if default.
<<<<<<< HEAD
# @returns string containing command line arguments to use
#
proc ::RingSourceMgr::_computeRingSourceSwitches {port url tstampExtractor id
  info expectHeaders oneshot timeout} {

  set switches ""
  append switches " --evbhost=localhost --evbport=$port"
  append switches " --info=$info --ids=$id --ring=$url"
=======
# @param offset           --offset dt in timestamp ticks.
# @returns string containing command line arguments to use
#
proc ::RingSourceMgr::_computeRingSourceSwitches {port url tstampExtractor ids
  info expectHeaders oneshot timeout offset} {

  set switches ""
  append switches " --evbhost=localhost --evbport=$port"
  append switches " --info=$info --ring=$url"
  foreach id $ids {
    append switches " --ids=$id"
  }
>>>>>>> master

  if {$tstampExtractor ne ""} {
    append switches " --timestampextractor=[file normalize $tstampExtractor]"
  }

  if {[string is true $expectHeaders]} {
    append switches " --expectbodyheaders"
  }
  
  if {$oneshot ne ""} {
    append switches " --oneshot=$oneshot"
  }
  if {$timeout ne ""} {
    append switches " --timeout=$timeout"
  }
<<<<<<< HEAD
=======
  if {$offset != 0} {
    append switches " --offset=$offset"
  }
>>>>>>> master

  return $switches
}

#------------------------------------------------------------------------------
##
# @fn [private] RingSourceMgr::_HandleDataSourceInput
#
#   Called as a file event when input is ready from a data source pipeline.
#   This may also be due to an event source exiting
#
# @param fd   - File descriptor open on the event source stdout/stderr,.
# @param info - Textual description of the event source.
# @param id   - Source id (numeric)
#
proc ::RingSourceMgr::_HandleDataSourceInput {fd info id} {
  set text "$info ($id) "
  if {[eof $fd]} {
    catch {close $fd} msg
    append text "exited: $msg"
    ::EndrunMon::decEndRunCount;           # One less end run to wait for.
    ::RingSourceMgr::_SourceDied  $fd;     # Do the book keeping for a dead source.
  } else {
    append text [gets $fd]
  }
  RingSourceMgr::_Output $text
}

##
# _SourceDied
#   Marks the fd in the sourceDict as empty.  Ths will make OnBegin
#   restart the source.
#
#   Unfortunately the only way to do any of this is a linear search of the
#   source dict for the source with the fd that closed
#
# @param fd - The fd that is being closed.
#

proc ::RingSourceMgr::_SourceDied {fd} {
  dict for {uri info} $::RingSourceMgr::sourceDict {
    set sourceFd [dict get $info fd]
    if {$sourceFd == $fd} {
      dict set ::RingSourceMgr::sourceDict $uri fd  ""
      break;          # No need to go further.
    }
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
proc ::RingSourceMgr::_Output msg {
  ReadoutGUIPanel::outputText "[clock format [clock seconds]] $msg"
}

# @fn RingSourceMgr::_waitForEventBuilder
#
#  Waits for the event builder to be ready to accept TCP/IP connections
#  By now it's assumed the event builder is visible to the port manager.
#
#  This essentially demands the event builder comes before it in the 
#  callout bundle list. For this reason, the EVBC::useEventBuilder actually
#  registers this immediately following the EVBC bundle.
#
proc ::RingSourceMgr::_waitForEventBuilder {} {

# Figure out which port the event builder is listening on:
  set ports [::portAllocator create %AUTO%]
  set name [::EVBC::getAppName]
  set port -1
  set allocations [$ports listPorts]
  foreach allocation $allocations {
    set appName [lindex $allocation 1]
    set user [lindex $allocation 2]
    if {($appName eq $name) && ($user eq $::tcl_platform(user))} {
      set port [lindex $allocation 0]
      break
    }
  }
  $ports destroy

  #  Try to connect every 100ms until success:

  while {[catch {socket localhost $port} fd]} {
    after 100
  }
  close $fd
}
