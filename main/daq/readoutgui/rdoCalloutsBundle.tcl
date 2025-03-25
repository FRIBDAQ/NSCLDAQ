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
# @file rdoCalloutsBundle.tcl
# @brief Callback bundle that handles ReadoutCallouts.tcl
# @author Ron Fox <fox@nscl.msu.edu>


package provide rdoCalloutsBundle 1.0
package require RunstateMachine
package require ReadoutGUIPanel
package require ExpFileSystem

##
#  Provides a RunstateMachine callback bundle that implements the old
#  ReadoutCallouts capability of the 10.x and earlier readout gui.
#  - When loaded we look for ReadoutCallouts.tcl and source it at global level
#  - If a script is found, we register ourselves with the RunstateMachineSingleton
#    state machine so that we are called at appropriate times.
#  - On transitions, by inspecting the state, we figure out which proc to
#    invoke from the ReadoutCallouts.tcl script.
#
#  The original ReadoutCallouts.tcl interface provides the following callouts which are
#  called as described below..left means called from [left] entered means
#  called from [enter]:
#
#  * OnBegin - called when Halted is left for Active, (tested)
#  * OnEnd   - called when Halted is entered from Active or Paused (Tested)
#  * OnPause - Called when Paused is entered.  (Tested)
#  * OnResume - Called when Paused is left for Active
#  * OnStart  - Called when Halted is entered from Starting (Tested)
#
#  In addition, the following new callbacks are implemented:
#
#  * OnFail     - Called when NotReady is entered from any other state. (tested)
#  * OnPrecheck - Hooks into the precheckTransitionForErrors bundle.
#  * OnEnslaving- Called when we are enslaving a slave GUI.
#  * OnEnslaved - Called when we are being enslaved by a master GUI.
#  * OnFreeing  - Called when we are freeing a slave GUI.
#  * OnEmancipation - Called when we are being freed by a master GUI.
#  * OnSlaveConnectionLost - Called when as a master we lose connection to a slave.
#


#
#

#-------------------------------------------------------------------------------
#
# Establish the bundle and an local data
#

namespace eval ::rdoCallouts {
    variable stateMachine ""
    namespace export attach enter leave
    namespace export precheckTransitionForErrors
    namespace export remotecontrol
    namespace ensemble create
    variable  runNumber 0;               # Memorized at run start.
}
#-------------------------------------------------------------------------------
#
# Implement the bundle methods:
#
proc ::rdoCallouts::attach {state} {
    
}

##
# ::rdoCallouts::enter
#
#   Called by the state machine when a new state is entered.
#
# @param from - prior state
# @param to   - New state.
# 
proc ::rdoCallouts::enter {from to} {
    
    # Starting -> Halted : OnStarting
    
    if {($from eq "Starting")  &&  ($to eq "Halted")} {
        if {[info procs ::OnStart] ne ""} {
            uplevel #0 ::OnStart
        }
        if {[info procs ::OnReady] ne ""} {
            uplevel #0 ::OnReady
        }
        return
    }
    
    # Any -> NotReady : OnFail
    
    if {($from ne "NotReady") && ($to eq "NotReady") } {
        if {[info procs ::OnFail] ne ""} {
            uplevel #0 ::OnFail
            return
        }
    }
    
    # Starting -> Halted : OnReady
    
    if {($from eq "Starting") && ($to eq "Halted")} {
        
    }
    
    # {Active, Paused} -> Halted : OnEnd
    
    if {($from in [list Active Paused]) && ($to eq "Halted")} {
        if {[info procs ::OnEnd] ne ""} {
            uplevel #0 ::OnEnd $rdoCallouts::runNumber
        }
    }
    # ->Paused : OnPaused
    
    if {$to eq "Paused"} {
        if {[info procs ::OnPause] ne ""} {
            uplevel #0 ::OnPause $rdoCallouts::runNumber
        }
    }
}
##
#  ::rdoCallouts::leave
#
#   Called by the state machine when a state is being left.
#   The current state is still the oldstate.
#
#  @param from - The current state (the one being left).
#  @param to   - The new state about to be entered.
# 
proc ::rdoCallouts::leave {from to} {

    #  Halted -> Active : OnBegin
    
    if {($from eq "Halted") &&($to eq "Active")} {
        if {[info procs ::OnBegin] ne ""} {
            set rdoCallouts::runNumber [::ReadoutGUIPanel::getRun]
            uplevel #0 ::OnBegin $rdoCallouts::runNumber
        }
    }
    # Paused -> Active : OnResume
    if {($from eq "Paused") && ($to eq "Active")} {
        if {[info procs ::OnResume] ne ""} {
            uplevel #0 ::OnResume $rdoCallouts::runNumber
        }
    }
}

##
# ::rdoCallouts::precheckTransitionForErrors
#    If OnPrecheck exists in the global namespace,
#    we relay to it.
#    Note that:
#   -   If the proc returns an error we capture the messag and
#       return it as expected by the BundleManager (Prechecks are
#       only allowed to report errors, not throw them).
#    -  If the proc returns a string that too is returned.
#
proc ::rdoCallouts::precheckTransitionForErrors {from to} {
    set msg "";               # in case it's not defined.
    if {[info commands ::OnPrecheck] ne ""} {
        catch {::OnPrecheck $from $to} msg;  #return value if no error.
    }
    return $msg
}
##
# ::remotecontrol
#   Called to handle a remote control change.  The first
#   Parameter is the action that occured, the second is the
#   host that is involved. Note that the first parameter is actually,
#   as luck? design?  would have it the name of the global proc
#   we'll call (if it's defined).
# @param method to call.
# @param host  involved host.
# @return - whatever the proc returns (though that's actually ignored
#           in this implementation.
# @throws - Whatever the proc throws.
#
proc ::rdoCallouts::remotecontrol {what host} {
    set result ""
    
    if {[info commands ::${what}] ne ""} {
        set result [::${what} $host]
    }
    
    return $result
}
#------------------------------------------------------------------------------
#  Other methods in the namespace:


##
# ::rdoCallouts::reload
#
#   If there is a ReadoutCallouts.tcl file in ~, ~/experiment/current or [pwd]
#   source it in at the global level.
#
proc ::rdoCallouts::reload {} {
    # Kill off old ReadoutCallout procs...yes this is not always enough but...
    # The catch is needed because the callout ma not be defined.
    
    foreach callout [list OnBegin OnEnd OnStart OnPause OnResume OnFail OnStart OnReady] {
        catch {rename $callout {}}
    }

    # Look for an load the new file:
    # daqdev/NSCLDAQ#1029  To honor the fact that EVENTS can move the
    # stagearea we need to get the stagaearea from the configuration as that's
    # already folded that in:
    
    set stagearea [ExpFileSystem::getStageArea]
    set expcurrent [file join $stagearea experiment current]
    
    foreach directory [list ~ $expcurrent [pwd]] {
        set candidate [file join $directory ReadoutCallouts.tcl]
        if {[file readable $candidate]} {
            uplevel #0 source $candidate
        }
    }
};    #Locate and reload the rdo callouts script.

#------------------------------------------------------------------------------
#  Auto register the bundle with the singleton.
#


set ::rdoCallouts::stateMachine [RunstateMachineSingleton %AUTO%]
if {"rdoCallouts" ni [$::rdoCallouts::stateMachine listCalloutBundles]} {
    $::rdoCallouts::stateMachine addCalloutBundle rdoCallouts
}

# Load ReadoutCallouts if it exists in the appropriate dirs.

::rdoCallouts::reload
