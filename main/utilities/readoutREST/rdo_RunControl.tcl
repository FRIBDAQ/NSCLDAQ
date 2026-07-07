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
# @file   rdo_RunControl.tcl
# @brief  Full RunControl UI with manager.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}

package require ReadoutRESTUI;           # Widgets we'll use.
package require ReadoutRESTClient;       # To get readout states.
package require loggerrestclient;        # Event log REST domains
package require programstatusclient;     # To query programs from manager.
package require stateclient;             # To query/control manager state.
package require kvclient;                # Title and run# are in kv store.
package require Tk


# Flag to the user this is deprecated in favor of rdo_PyRunControl:
# Issue #496 , #496 In case someone decides to install this again.

tk_messageBox -icon warning -title "Deprecated application" -type ok \
    -message {$DAQBIN/rdo_RunControl is deprecated in favor of $DAQBIN/rdo_PyRunControl for now youcan continue
but you should change your scripts gainst the day we pull this from the installation.}




#-------------------------------------------------------------------------------
#   User interface megawidgets.

##
# @class ReadoutStateTable
#    Provides a table of Readouts and their run/manager states
#
#  This is a ttk::treeview (with vertical scrollbar)
#  run as a table with the following
#  columns:
#     *   name   - program name.
#     *   host   - where it runs.
#     *   state  - internal state.
#     *   running- X if the program is actually running.
# This is a  view.  The model comes both from the manager and,  for
# running programs, the programs themselves
#
# OPTIONS
#    none
# METHODS
#    add    - Adds a name/host
#    delete - Remove a name/host\
#    exists - Query the existence of a program.
#    list   - Return the contents of the table as  a list of dicts.
#    setActive - Set active flag of a name/host.
#    setState  - Set the runstate of a name/host.
#
snit::widgetadaptor ReadoutStateTable {
    variable ids -array [list]
    constructor {args} {
        installhull using ttk::frame
        
        set columns [list name host state running]
        ttk::treeview $win.table -yscrollcommand [list $win.vscroll set] \
            -columns $columns -show headings \
            -displaycolumns $columns         \
            -selectmode none
        
        foreach c $columns h [list Name Host State Running] {
            $win.table heading $c -text $h
        }
        
        ttk::scrollbar $win.vscroll -command [list $win.table yview] \
            -orient vertical
        
        grid $win.table $win.vscroll -sticky nsew
        grid columnconfigure $win 0 -weight 1;          # Only tree expands.
          
        #  There are no options and configureList gets unhappy if
        #  we call it an none are defined.  The code below ensure we
        #  don't forget to configure if options are later added:
        
        if {[array names options] ne [list]} {
            $self configurelist $args
        }
    }
    #--------------------------------------------------------------------------
    # Public methods:
    #
    
    ##
    # add - adds a new item to the end of the table.
    #
    # @param name - program name (contents of name column).
    # @param host - contents of host column.
    # @param state - (optional) value to put in state column.
    # @param active - (optional) value to put in the  Running columns.
    #
    method add {name host {state -} {active -}} {
        set index $name@$host
        if {![$self exists $name $host]} {
            set active [expr {$active ? "X" : ""}]
            set element \
                [$win.table insert {} end -values [list $name $host $state $active]]
            set ids($index) $element
        } else {
            error "This program ($name@$host) already exists."
        }
        
    }
    ##
    # delete
    #    Removes a name/host combination from the table.
    #
    # @param name - name of the program.
    # @param host - host in which it runs.
    #
    method delete {name host} {
        set index $name@$host
        if {[$self exists $name $host]} {
            $win.table delete $ids($index)
            array unset ids $index
        } else {
            error "$index is not in the table."
        }
    }
    ##
    # exists
    #   Determines if a name/host combination is in the table.
    #   The assumption is that the ids array is properly maintained.
    #
    # @param name - name of the program.
    # @param host - host it runs in.
    # @return boolean - true if name@host is in the table.
    #
    method exists {name host} {
        set index $name@$host
        return [expr {[array names ids $index] ne ""}]
    }
    ##
    # list
    #   Lists the contents of the table.  The table contents are list of
    #   dicts...See below.
    # @return list of dicts  - Each dict represents one line of the table and
    #              contains the following key/value pairs:
    #              name  - program name.
    #              host  - host the program runs in.
    #              state - State of the program.
    #              active- Boolean true if the program is active according
    #                      to the table.
    # @note that state can be '-' indicating it's not been actually set.
    #       if that's the case with active, active is assumed to be false.
    # @note that the list should not be assumed to be correct as the underlying
    #       model can change at any time.  It's just the list as of the
    #       last time this view was updated.
    #
    method list {} {
        set result [list]
        foreach index [array names ids] {
            set data [$win.table item $ids($index) -values]
            set name [lindex $data 0]
            set host [lindex $data 1]
            set state [lindex $data 2]
            set active [expr {[lindex $data 3] eq "X"}]
            lappend result [dict create                                     \
                name $name host $host state $state active $active           \
            ]
        }
        
        return $result
    }
    ##
    # setActive
    #    Sets the activity state of a program.
    #
    # @param name   - program name.
    # @param host   - host the program runs on.
    # @param active - boolean flag with new value of the active state of name@host
    #
    method setActive {name host active} {
        set index $name@$host
        if {[$self exists $name $host]}  {
            set entry $ids($index)
            set data [$win.table item $entry -values]
            set active [expr {$active ? "X" : ""}]
            $win.table item $entry -values [lreplace $data 3 3 $active]
        } else {
            error "$index does not exist"
        }
    }
    ##
    # setState
    #   Sets a program's state field.
    #
    # @param name - name of the program.
    # @param host - host of the program.
    # @param state - String to put in the state field.
    #
    method setState {name host state} {
        set index $name@$host
        if {[$self exists $name $host]} {
            set data [$win.table item $ids($index) -values]
            $win.table item $ids($index) -value [                        \
                lreplace $data 2 2 $state                                \
            ]
        } else {
            error "$index does not exist."
        }
    }
}
##
# @class controls
#    Widget to control more than one Readout.  Consists of
#    - ParametersUI  (from ReadoutRESTUI)
#    - Control/State (from ReadouttRESTUI)
#    - Boot/Shutdown control section of the manager itself.
#  Note this is a view, it relies on controller and model logic.
#  Model comes from the manager and the various readout programs.
#
# OPTIONS:
#   -state -  manager state.
#   -readoutstate - List of states from the readouts.
#   -bootcommand  - command to call when we should boot.
#   -shutdowncommand - command to call when we should shutdown.
#
#  Delegated to ReadoutParameters subwidget
#
#   -title        - Current title.
#   -nexttitle    - Next title (being entered).
#   -titlecommand - Called when title should be changed.
#   -run          - Current run number value.
#   -nextrun      - Next run number (being entered).
#   -runcommand   - Command called when run number should be changed.
#   -parameterstate -> -state in ReadoutParameters subwidget.
#
#    Delegated to ReadoutState
#
#    -statecommand  - Called when state change button has been clicked.
#                     (-> -command.)
#
snit::widgetadaptor ReadoutManagerControl {
    component parameters
    component control
    
    # 'native' options:
    
    option -state -default *Unknown* -configuremethod _cfgState
    option -readoutstate -configuremethod _cfgRdoState 
    option -bootcommand [list]
    option -shutdowncommand [list]
    option -elapsed -default "*Unknown*"


    # Options delegated ton the parameters widget:
    
    delegate option -title        to parameters
    delegate option -nexttitle    to parameters
    delegate option -run          to parameters
    delegate option -nextrun      to parameters
    delegate option -titlecommand to parameters
    delegate option -runcommand   to parameters
    delegate option -parameterstate to parameters as -state
    
    
    # Delegated to ReadoutState:
    
    delegate option -statecommand  to control as -command
    
    ## constructor
    
    constructor args {
        installhull using ttk::frame
        
        # Generate the components.
        
        install parameters using ReadoutParameters $win.pars
        install control    using ReadoutState      $win.control
        
        #  Generate widgets that control/reflect the state machine in the manager
        
        ttk::labelframe $win.manager -borderwidth 3 -relief groove -text "Manager"
        ttk::label $win.manager.statelabel -text "State: "
        ttk::label $win.manager.state -textvariable [myvar options(-state)]
        ttk::button $win.manager.boot -text Boot -command [mymethod _dispatchBoot] \
            -state disabled
        ttk::button $win.manager.shutdown -text Shutdown \
            -command [mymethod _dispatchShutdown] \
            -state disabled
        
        ttk::label $win.manager.lelapsed -text "Elapsed:"
        ttk::label $win.manager.elapsed -textvariable [myvar options(-elapsed)]

        grid $win.manager.statelabel $win.manager.state -sticky w
        grid $win.manager.boot $win.manager.shutdown    -sticky w -padx 3
        grid $win.manager.lelapsed $win.manager.elapsed
        
        grid $parameters -columnspan 3
        grid $win.manager $control
        
        $self configurelist $args
        
        
    }
    
    #--------------------------------------------------------------------------
    #  Configuration management.
    
    ##
    # _cfgState
    #   Handle the -state option configuration:
    #      If the state is SHUTDOWN - Boot is enabled and  Shutdown not.
    #      otherwise Shutdowns is enabled and Boot is not.
    #
    # @param optname - name of the option being modified.
    # @param value   - new value.
    method _cfgState {optname value} {
        
        set options($optname) $value;     # This updates the state label.
        if {$value eq "SHUTDOWN"} {
            $win.manager.shutdown configure -state normal
            $win.manager.boot     configure -state normal
        } else {
            $win.manager.shutdown configure -state normal
            $win.manager.boot     configure -state disabled
        }
        
    }
    
    ##
    # _cfgRdoState
    #    Called when -readoutstate is configured.  We're handed a list of
    #    states, one for each reaodut program being controlled.  If all the
    #    states are the same, we use that state as the resulting state.
    #    if not, we use the state 'inconsistent'
    #
    #    Regardless, options(-readoutstate) is updated so cget is trivial.
    #
    # @param optname - name of the option being configured (-readoutstate).
    # @param value   - new value.
    #
    method _cfgRdoState {optname value} {
        set state [lindex $value 0]
        foreach s [lrange $value 1 end] {
            if {$s ne $state} {
                set state inconsistent
                break
            }
        }
        if {$state eq ""} {
            set state inconsistent;               # If there's no values.
        }
        #
        #  Turn off the parameters if the run is active.
        #
        if {$state eq "active"} {
            $self configure -parameterstate disabled
        } else {
            $self configure -parameterstate normal
        }
        set options($optname) $state
        $control configure -state $state
    }
        
    
    
    #---------------------------------------------------------------------------
    # Event handling.
    
    ##
    # _dispatch
    #   Dispatch a script.
    #
    # @param opt - option holding the script.
    #
    method _dispatch {opt} {
        set script $options($opt)
        if {$script ne ""} {
            uplevel #0 $script
        }
    }
    
    ##
    # _dispatchBoot
    #  Dispacth the boot button's script:
    #
    method _dispatchBoot {} {
        $self _dispatch -bootcommand
    }
    
    ##
    # _dispatchShutdown
    #
    # Dispatch the shutdown button's script.#
    method _dispatchShutdown {} {
        $self _dispatch -shutdowncommand
    }
}
##
# @class RecordingGUI
#    View for event recording status/control.  This is a checkbox.
#    it is usually paired with a run control controller which manages the updates.
#
snit::widgetadaptor RecordingGUI {
    option -recording  -default 0
    option -command -default [list]
    
    delegate option -state to button
    
    component button
    
    constructor {args} {
        installhull using ttk::frame
        install button using \
            ttk::checkbutton $win.state -onvalue 1 -offvalue 0 \
            -variable [myvar options(-recording)] -text Recording \
            -command [mymethod _dispatch]
        pack $win.state -fill both -expand 1
    }
    ##
    # _dispatch
    #   Dispatch the user's command with the state of the checkbutton as
    #  a parameter.
    method _dispatch {} {
        set command $options(-command)
        if {$command ne ""} {
            lappend command $options(-recording)
            uplevel #0 $command
        }
    }

}
##
# @class RecordingController
#    Interacts with the outside world to handle managing the recording state.
#  OPTIONS:
#    -view   - The RecordingGUI we're managing
#    -kvmodel  - kvstore client.
#    -statemodel - Sequencer/state model.
#  METHODS:
#     update - Update the state of the UI.
#  ACTIONS:
#     - Checkbutton changes control the recording state.
#     - State changes can have effect:
#       *  To BEGIN - deactivates the button.
#       *  out of BEGIN - if recording is active, increments the run from
#          what is was when BEGIN occured (this works with multiple clients I think).
#
#
snit::type RecordingController {
    option -view       -readonly 1
    option -kvmodel    -readonly 1
    option -statemodel -readonly 1
    option -recordingmodel -readonly 1
    
    variable LastState *unknown*
    variable LastRun   0
    
    constructor args {
        $self configurelist $args
        
        $options(-view) configure -command [mymethod _onButton]
        
        # update will update the GUI and variables properly.
        
        $self update
    }
    
    #-------------------------------------------------------------
    # Public methods
    
    ##
    # update
    #   Should be periodically called to refresh both the internal state
    #   the kvmodel and the view.
    #
    method update {} {
        
        # Handle state changes:
        
        set state [$options(-statemodel) currentState]
        set recording [$options(-recordingmodel) isRecording]
        
        # Transitioning into BEGIN?
        
        if {($LastState ne "BEGIN") && ($state eq "BEGIN")} {
            set LastRun [$options(-kvmodel) getValue run]
            $options(-view) configure -state disabled
        } elseif {($LastState eq "BEGIN" ) && ($state ne "BEGIN")} {
            # Transition out of active.
            
             $options(-view) configure -state normal
            if {$recording} {
                incr LastRun
                $options(-kvmodel) setValue run $LastRun
               
            }
        }
    
        set LastState $state
        
        # Update the checkbutton
        
        $options(-view) configure -recording $recording
        
    }
    #------------------------------------------------------------------
    # Event Handling
    
    ##
    # _onButton
    #    Called when the requested state of the checkbutton changes.
    #
    method _onButton {state} {
        $options(-recordingmodel) record $state
    }
}
    

##
# @class RunControlGUI
#   This is the view for the run control.  It consists of a top part that is a
#    ReadoutManagerControl and a bottom part that is a ttk::notebook.
#    The notebook has one page that is a ReadoutStateTable and then one page
#    for each readout program that contains a ReadoutStatistics entry for
#    each program.
# OPTIONS:
#   All ReadoutManagerControl options - delegated to that component.
# Methods:
#   All ReadoutStateTable methods delegate to that component.
#   updateStatistics - updates statistics page for a specific program/node
#                   if necessary generating a new page for that program/node.
#
snit::widgetadaptor RunControlGUI {
    component controls
    component notebook
    component summary
    component recording
    
    ## ReadoutManagerControls options:
    
    delegate option -state to controls
    delegate option -readoutstate to controls
    delegate option -bootcommand to controls
    delegate option -shutdowncommand to controls
    delegate option -title to controls
    delegate option -nexttitle to controls
    delegate option -titlecommand to controls
    delegate option -run  to controls
    delegate option -nextrun to controls
    delegate option -runcommand to controls
    delegate option -parameterstate to controls
    delegate option -statecommand to controls
    delegate option -elapsed to controls
    
    # Summary methods:
    
    delegate method add to summary
    delegate method delete to summary
    delegate method exists to summary
    delegate method list to summary
    delegate method setActive to summary
    delegate method setState to summary
    
    # Manages assigning, keeping track of and 
    
    variable statsIndex 0;     # USed to generate index commands.
    variable statsWidgets -array [list];   # Widgets indexed by program@host
    

    ## Constructor
    
    constructor {args} {
        installhull using ttk::frame
        
        install controls using ReadoutManagerControl $win.controls
        install recording using RecordingGUI $win.recording
        install notebook using ttk::notebook $win.notebook
        install summary using ReadoutStateTable $notebook.summary
        $notebook add $summary -text summary
        
        grid $controls $recording -sticky nsew
        grid $notebook -sticky nsew -columnspan 2
        
        
        
        $self configurelist $args
    }
    
    #--------------------------------------------------------------------------
    #  Public methods.
    
    ##
    # getRecording
    #    Retrieve the recording widget
    #
    method getRecording {} {
        return $recording
    }
    
    ##
    # updateStatistics
    #    - If necessary a new statistics tab/widget is created.
    #    - The statistics are then passed to that widget to be displayed.
    #
    # @param program - name of the program.
    # @param host    - Host the program runs in.
    # @param stats   - Statistics dict of the program@host.
    #
    method updateStatistics {program host stats} {
        set index $program@$host
        if {[array names statsWidgets $index] eq ""} {
            #
            #   Need to make a new tab:
            #
            set widget  $notebook.stats[incr statsIndex]
            ReadoutStatistics $widget
            $notebook add $widget -text $index
            set statsWidgets($index) $widget
        } else {
            set widget $statsWidgets($index)
        }
        $widget configure -data $stats
    } 
}
#------------------------------------------------------------------------------
# Controller classes.
#

##
# @class SystemStateTracker
#    This class interacts with a manager (model) to maintain the state
#    of the system in a view that supports a -state option.
#    Note that so we don't have a plethora of after's in the event qeueu
#    update is externally called at whatever rate or demand required by the
#    larger application:
#
#  OPTIONS
#    -view    - Widget with a -state option that we'll maintain.
#    -model   - StateClient object that can interact with the manager.
#  METHODS
#    update   - Fetches the state from the model (if possible) and
#               updates the view.
#
snit::type SystemStateTracker {
    option -view
    option -model
    
    constructor {args} {
        $self configurelist $args
    }
    
    #--------------------------------------------------------------------------
    # public methods.
    #
    
    ##
    # update
    #    Attempts to update the view from the model
    #    - currentState is called in the model.
    #    - The returned value is passed into the view.
    #
    # @return integer - 0 on success and 1 if currentState failed.
    #
    method update {} {
        
        set model $options(-model)
        set view  $options(-view)
        
        if {[catch {$model currentState} state]} {
            
            return 1;                       # state fetch failed.
        } else {
            
            $view configure -state $state
        }
    }
}
##
# @class MultiReadoutStateTracker
#    Controller that maintains the readout state for multiple Readout models.
#
# OPTIONS:
#    -view - an object that acepts the -readoutstate option.
#    -models - List of ReadoutRESTClient objects that we'll get our
#              data from.
# METHODS:
#    update - Updates the view from the models.
#
snit::type MultiReadoutStateTracker {
    option -view
    option -models
    
    constructor {args} {
        $self configurelist $args
    }
    
    #--------------------------------------------------------------------------
    # Public methods.
    
    ##
    # update
    #    Attempts to fetch the run state from the models we have.
    #    If any model cannot give us a state (e.g. the Readout is not running)
    #    We'll put in the pseudo state 'inconsistent'  This will mean the
    #    view will represent the state as inconsistent because:
    #    - Either all Readouts are done which results in inconsistent state
    #      due to unanimous consent.
    #    - Some Readouts are down which will cause state disagreement
    #      resulting in a display of inconsistent as the state.
    #
    #
    method update {} {
        set statelist [list]
        foreach model $options(-models) {
            if {[catch {$model getState} state]} {
                lappend statelist inconsistent
            } else {
                lappend statelist $state
            }
        }
        $options(-view) configure -readoutstate $statelist
    }
        
    
}
##
# @class MultiReadoutStatisticsTracker
#    Controller that maintains statistics in a view with multiple readout
#    programs.
# OPTIONS:
#    -view  - A view object that must support the updateStatistics method.
#    -models - list of ReadoutRESTCLient model objects from which the statistics
#              are fetched.
#    -programs - List of program names - corresponding to the models.
# METHODS:
#   udpate  - update the view from the models.
snit::type MultiReadoutStatisticsTracker {
    option -view
    option -models
    option -programs
    
    constructor {args} {
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    # public methods
    
    ##
    # update
    #    Update the view object from the models.  Note that if a model
    #    errors out (e.g. the Readout isn't running) we ignore that error
    #    at this time.
    #
    method update {} {
        foreach model $options(-models) name $options(-programs) {
            if {![catch {$model getStatistics} stats]} {
                $options(-view) updateStatistics $name [$model cget -host] $stats
            }
        }
    }
}
##
# @class ReadoutParameterTracker
#
#  Manages updates from the title and run key value store items and
#  the uneditable parmaters of a readout.
#
# OPTIONS
#   -view   - View that must support the -title, -run options.
#   -model  - Model which must support a getValue method to retrieve a
#             key value store item.
#
snit::type ReadoutParameterTracker {
    option -view
    option -model
    
    variable lastRun -1
    variable lastTitle ""
    
    constructor {args} {
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    # Public methods.
    
    ##
    # update
    #   Updates the view from the model.  Note that if the
    #   model fails (e.g. the manager isn't running we don't update).
    #
    # @return integer - 0 success 1 failure in update.
    #
    method update {} {
        return [catch {
            set model $options(-model)
            set view  $options(-view)
            set title [$model getValue title]
            set run   [$model getValue run]
            
            if {$title ne $lastTitle} {
                $view configure -title $title
                $view configure -nexttitle $title
                set lastTitle $title
            }
            
            if {$run != $lastRun} {
                $view configure -run $run
                $view configure -nextrun $run
                set lastRun $run
            }
        }]
    }
}
##
# @class SummaryTracker
#    Updates the readout summary view from the program states and models we
#    dynamically generate for those programs.
#
# OPTIONS:
#    -view   - an object that supports add, delete, setActive and setState
#             methods in the same calling convention that ReadoutStateTable does.
#    -model  - an object with a status method that returns lists of dicts that
#              are like the ones a ProgramClient object returns.
#    -readouts - List of program names for programs that are Readouts which also
#              must run their REST interface plugin as we'll be fetching their states.
#
# METHODS:
#    update   - Updates the view from data gotten from the model.
#
snit::type SummaryTracker {
    option -view
    option -model
    option -readouts
    
    constructor {args} {
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    # Utility methods
    
    ##
    # _filterReadouts
    #   Given the list o dicts containing program states, filters out those
    #   that are not readouts, returning only those that are.
    #   A readout program just needs to match a name in options(-readouts).
    #
    # @param states - list of program status dicts.
    # @return list of program status dicts - the name of each program in the
    #                 returned list is gauranteed to be in options(-readouts)
    #
    method _filterReadouts {states} {
        set result [list]
        foreach status $states {
            if {[dict get $status name] in $options(-readouts)} {
                lappend result $status
            }
        }
        return $result
    }
    ##
    # _getReadoutState
    #   Get the state of the readout running in the specified host/user pair
    #
    # @param host  - host running the program.
    # @param user  - user running the program.
    # @return string - state name.
    # @retval *unresponsive* indicates we could not fetch the state from the
    #                REST server.
    #
    method _getReadoutState {host user} {
        set client [ReadoutRESTClient %AUTO% -user $user -host $host]
	if {[array names ::env SERVICE_NAME] ne ""} {
	    $client configure -service $::env(SERVICE_NAME)
	}
        if {[catch {$client getState} state]} {
            set state *unresponsive*;    # Deal with e.g. exit after state get.
        }
        $client destroy
        return $state
    }
    ##
    # _updateReadoutStatus
    #   Updates the status of a program:
    #   - Generate a ReadoutRESTClient and, if possible, fetch the state.
    #   - If the program already exists, just update the state and status.
    #   - If the program does not exist add it.
    #
    # @param program - dict describing the program.
    #
    method _updateReadoutStatus {program} {

        set host [dict get $program host]
        set name [dict get $program name]
        set active [dict get $program active]
        
        set user [$options(-model) cget -user]
        
        if {$active} {
            set state [$self _getReadoutState $host $user]
        } else {
            set state *unresponsive*
        }
        
        set view $options(-view)
        
        if {[$view exists $name $host]} {
            $view setActive $name $host $active
            $view setState  $name $host $state
        } else {
            $view add $name $host $state $active
        }
        
    }
    ##
    # _markDeleted
    #   If a program is not in the list of status items it gets marked deleted
    #   Locate any program in the listed names and remove it.
    #
    # @param name - program name.
    #
    method _markDeleted {name} {
        set model $options(-view)
        set programs [$model list]
        foreach program $programs {
            if {[dict get $program name] eq $name} {
                $model delete [dict get $program name] [dict get $program host]
                break;     # Presumably there's only one.
            }
        }
    }
    ##
    # _deleteMovedHosts
    #    If any programs have moved to a new host, delete the old entry for
    #   them.  It's up to some other bit of code to make the new one.
    #
    # @param names  - Names of programs.
    # @param nodes  - Nodes in which each program is running.
    #
    method _deleteMovedHosts {names nodes} {
        set view $options(-view)
        
        # Note we query the model each time as the outer loop can change the
        #  list iterated over in the inner loop.
        
        foreach name $names node $nodes {
            foreach existing [$view list] {
                set oldName [dict get $existing name]
                set oldHost [dict get $existing host]
                if {($name eq $oldName) && ($node ne $oldHost)} {
                    $view delete $oldName $oldHost
                    break;                 #Presumably only one match is allowed.
                }
            }
        }
    }

    #--------------------------------------------------------------------------
    # Public methods
    
    ##
    # update
    #   This is actually quite complex considering some of the edge cases:
    #    - The manager does not respond to 'status' no update can occur.
    #    - Some programs have been removed from the manager (vs. the -readouts
    #      list); Those get marked with a state *deleted* and inactive.
    #    - Some program supposedly active does not respond to its state request
    #      That gets marked as '*unresponsive*'
    #    - If a program moves to a different host, we'll make a new one and
    #      delete the old one.
    #
    # @return integer - 0 success -1 failure  because status didn't work.
    #                   all other problems are caught and handled.
    #
    method update {} {
        catch {
            set view $options(-view)
            set programStates [dict get [$options(-model) status] programs]
            set programStates [$self _filterReadouts $programStates]
            
            set existingReadouts [list]
            set readoutHosts     [list]
            foreach program $programStates {
                lappend existingReadouts [dict get $program name]
                lappend readoutHosts     [dict get $program host]
                $self _updateReadoutStatus $program
            }
            #  Mark the programs that disappeared:
            
            foreach program $options(-readouts) {
                if {$program ni $existingReadouts} {
                    $self _markDeleted $program
                }
            }
            #  If hosts changed delete the old ones:
            
            $self _deleteMovedHosts $existingReadouts $readoutHosts
            
        }
    }
}

##
# @class RunControlActionHandler
#
#   The run control action handler handles all events in a RunOntrolGUI
#   object.  It is a Controller like object (in the MVC sense of the word)
#   That can respond to user interface events generated by the user.
# OPTIONS:
#   -view - the RunControlGUI view.  Must have -bootcommand, -shutdowncommand,
#           -titlecommand, -runcommand and -statecommand options.
#   -statemodel - The Manager StateClient that models the manager interface.
#   -kvmodel    - The Manager KvClient that models the key value store.
#
snit::type RunControlActionHandler {
    option -view -default [list] -configuremethod _cfgView
    option -statemodel
    option -kvmodel
    
    constructor {args} {
        $self configurelist $args
    }
    #-------------------------------------------------------------------------
    # Public methods.
    
    ##
    # update
    #   Does nothing except make this duck type compatible with the tracker
    #   controllers.
    #
    method update {} {}
    #-------------------------------------------------------------------------
    # Configuration management.
    
    ##
    # _cfgView
    #    Called to configure a new (or initial) view.
    #    -  If there's an existing view, we unconfigure our callbacks from it.
    #    -  We condigure our callbacks into the new view and
    #    -  Set the options array element with the new view so cget works.
    #
    # @param optname - name of the option to configure.
    # @param value   - new value (view) object base command.,
    # @note - we properly handle an empty value.
    method _cfgView {optname value} {
        set oldView $options($optname)
        if {$oldView ne ""} {
            $oldView configure -bootcommand ""
            $oldview configure -shutdowncommand ""
            $oldview configure -titlecommand ""
            $oldview configure -runcommand ""
            $oldview configure -statecommand ""
        }
        if {$value ne ""} {
            $value configure -bootcommand     [mymethod _onBoot]
            $value configure -shutdowncommand [mymethod _onShutdown]
            $value configure -titlecommand    [mymethod _onTitle]
            $value configure -runcommand      [mymethod _onRun]
            $value configure -statecommand    [mymethod _onStateChange]
        }
        
        set options($optname) $value
    
    }
    
    #-------------------------------------------------------------------------
    # Event handlers.
    
    ##
    # _onBoot
    #    Called to boot the manager.
    #
    method _onBoot {} {
        
        $options(-statemodel) transition BOOT
    }
    ##
    # _onShutdown
    #   Called to shutdown the system via the manager.
    #
    method _onShutdown {} {
        $options(-statemodel) transition SHUTDOWN
    }
    
    ##
    # _onTitle
    #   A new title is being set:
    # @param title  - value of the new title.
    #
    method _onTitle {title} {
        $options(-kvmodel) setValue title $title
    }
    ##
    # _onRun
    #    A new run number is being set.
    #
    # @param run - value of the new run number.
    #
    method _onRun {run} {
        if {(![string is integer -strict $run]) || ($run < 0)} {
            error "Run numbers must be integers >= 0"
        }
        $options(-kvmodel) setValue run $run
    }
    ##
    # _onStateChange
    #   Readout must change state.  This is actually a change in the
    #   manager's state:
    #  - init -> HWINIT
    #  - begin -> BEGIN
    #  - end   -> END
    #  - shutdown -> manager shutdown
    #
    # @param what - the transition in Readout(s) being attempted.
    #
    method _onStateChange {what} {
        set model $options(-statemodel)
        if {$what eq "init"} {
            $model transition HWINIT
        } elseif {$what eq "begin"} {
            $model transition BEGIN
        } elseif {$what eq "end"} {
            $model transition END
        } elseif {$what eq "shutdown" } {
            $self _onShutdown
        }
        #  For now ignore all others in the sake of expandability.
        
    }  
}
##
# RunController
#   Pulls together the controllers above into a single integrated controller
#   that only requires:
# OPTIONS:
#    -view         - A RunControlGUI view object.
#    -statemodel   - A StateClient model.
#    -programmodel - A manager program status model.
#    -kvmodel      - Manager key value store model.
#    -recordingcontroller - Controller for event recording.
#    -readouts     - A list of readout program names.
#    -refresh      - Refresh rate of the display in miliseconds.
#
#  @note that this controller will generate ReadoutRESTClient models for each
#  Readout program.
#  @note for the update part of this model to work we need to get back in the event
#        loop in a 'timely' manner.
#
#  This model has a couple of duties:
#   *   Provide periodic refresh of the readonly parts of the display from
#       the manager and readout programs.  It does this by encapsulating a number
#       of tracker controllers and updating them.  Note that all subcontrollers
#       have a -view option and an update method.
#   *   Encapsulating a RunControlActionHandler which is responsible for
#       responding to user interactions with the view object.
#
snit::type RunController {
    #  Option settings almost always require multiway forwarding to the components
    #  so delegation isn't directly possible but -configuremethods get the
    #  job done.
    
    option -view       -configuremethod _cfgView
    option -statemodel -configuremethod _cfgStateModel
    option -programmodel -configuremethod _cfgProgramModel
    option -kvmodel    -configuremethod _cfgKvModel
    option -recordingcontroller -default ""
    option -readouts   -configuremethod _cfgReadouts
    option -refresh  -default 1000;       # Default to 1 second update rate.
    
    component actionHandler
    component summaryTracker
    component readoutParameterTracker
    component multiReadoutStatisticsTracker
    component multiReadoutStateTracker
    component systemStateTracker
    
    
    variable afterid -1
    variable updateList [list]
    variable readoutModels [list]
    
    constructor {args} {
        
        install actionHandler using RunControlActionHandler %AUTO%
        install summaryTracker using SummaryTracker %AUTO%
        install readoutParameterTracker using ReadoutParameterTracker %AUTO%
        install multiReadoutStatisticsTracker using MultiReadoutStatisticsTracker %AUTO%
        install multiReadoutStateTracker using MultiReadoutStateTracker %AUTO%
        install systemStateTracker using SystemStateTracker %AUTO%
        
        set updateList [list                                        \
            $actionHandler $summaryTracker $readoutParameterTracker \
            $multiReadoutStatisticsTracker                          \
            $multiReadoutStateTracker                               \
            $systemStateTracker                                     \
        ]
        
        $self configurelist $args
        $self _update
        
    }
    destructor {
        after cancel $afterid
        foreach object $readoutModels {
            $object destroy
        }
        foreach object $updateList  {
            $object destroy 
        }
        foreach option [list -statemodel -kvmodel -programmodel] {
            set obj $options($option)
    
            if {$obj ne ""} {
                $obj destroy
            }
        }
        if {$options(-view) ne ""} {
            destroy $options(-view)
        }
        
    }
    #--------------------------------------------------------------------------
    # Private utilities

    ##
    # _canUpdate
    #   Ensures everything needed to update is defined.
    #
    # @return bool
    #
    method _canUpdate {} {
        foreach option [list -view -statemodel -programmodel -kvmodel] {
            if {$options($option) eq ""} {
                return 0
            }
        }
        return 1
    }
    ##
    # _findProgramHost (proc )
    #  Locate the host that a program lives on.
    #
    # @param program - program name.
    # @param programs - Program status definitions.
    # @return string  - note empty string if no match.
    #
    proc _findProgramHost {program programs} {
        foreach p $programs {
            if {[dict get $p name] eq $program} {
                return [dict get $p host]
            }
        }
        return ""
    }
    ##
    # _createReadoutModels
    #     Create the readout modules associated with -readouts into
    #     readoutModels (assumed to be empty)
    # 
    method _createReadoutModels {} {
       set status [ catch {;                        # In case the manager is dead...
            if {$options(-programmodel) ne ""} {
                set status [$options(-programmodel) status]
                set programs [dict get $status programs]
                foreach program $options(-readouts) {
                    set host [_findProgramHost $program $programs]
                    if {$host eq ""} {
                        # No match can't make all models so bail.
                        
                        return
                    } else {
			set newModel [ReadoutRESTClient %AUTO% \
                            -host $host -user [$options(-programmodel) cget -user] \
			]
			if {[array names ::env SERVICE_NAME] ne "" } {
			    $newModel configure -service $::env(SERVICE_NAME)
			}
                        lappend readoutModels $newModel
                    }
                }
            }
        } msg]
        # If we get through this all successfully, there will be as many models as
        # programs.
        if {$status} {
            puts "$msg $::errorInfo"
        }
    }
    ##
    # _computeReadoutModels
    #   Usually called either because the names of the readouts have changed
    #   or the program model has changed.
    #   - Destroy all the readout REST models.
    #   - Deconfigure them/names in all appropriate objects.
    #   - If there's a program state model, we recreate new program
    #      models and configure them into the appropriate objects.
    #      as well as the names.
    #
    method _computeReadoutModels {} {
        
        # Out with the old.
        
        $summaryTracker configure -readouts ""
        $multiReadoutStatisticsTracker configure -models [list] -programs [list]
        $multiReadoutStateTracker configure -models [list]
        foreach program $readoutModels {
            $readoutModels destroy
        }
        set readoutModels [list]
        
        # In with the new
        
        $self _createReadoutModels;    # Empty lists possibly.
        if {[llength $readoutModels] == [llength $options(-readouts)]} {
            $multiReadoutStatisticsTracker configure \
                -models $readoutModels -programs $options(-readouts)
            $multiReadoutStateTracker configure -models $readoutModels
            $summaryTracker configure -readouts $options(-readouts)
        }
        
        
    }
    
    #--------------------------------------------------------------------------
    #  configuration management
    
    ##
    # _cfgView
    #   -view configured.
    #     set the view option in all subcontrollers.  We don't destroy any prior
    #     view.  That's up to the manager of this object to do if they so wish.
    #
    # @param optname  - option name.
    # @param value    - New value.
    #
    method _cfgView {optname value} {
        set options($optname) $value;           # Make cget work.
        
        foreach object $updateList {
            $object configure -view $value
        }
    }
    ##
    # _cfgStateModel
    #   Updates the -statemodel and does that for every controller we manage
    #   that needs one.  The state model is the manager's state REST interface.
    #
    # @param optname - name of the option being configured.
    # @param value   - new value.
    #
    method _cfgStateModel {optname value} {
        set options($optname) $value
        
        $systemStateTracker configure -model      $value
        $actionHandler      configure -statemodel $value
        
    }
    ##
    # _cfgProgramModel
    #    Configure the program state model.  If there are readouts defined,
    #    this will cause a recomputation and resetting of the readoutModels
    #
    # @param optname - option name.
    # @param value   - new value of the option.
    #
    method _cfgProgramModel {optname value} {
        set options($optname) $value
        
        $summaryTracker configure -model $value
        $self _computeReadoutModels
    }
    ##
    # _cfgKvModel
    #   Updates the -kvmodel option value.  This is a model that provides
    #   access to the key value store.
    #
    # @param optname - option name.
    # @param value   - new value.
    #
    method _cfgKvModel {optname value} {
        set options($optname) $value
        
        $readoutParameterTracker configure -model $value
        $actionHandler configure -kvmodel $value
    }
    ##
    # _cfgReadouts
    #   Configures a new set of readout program names.  This requires
    #   a recomputation of the REST models of those programs and configuring
    #   them into the controllers that need them.
    #
    # @param optname - Name of option being configured.
    # @param value   - New value (list of program names).
    #
    method _cfgReadouts {optname value} {
        set options($optname) $value
        
        $self _computeReadoutModels ;   # Does the heavy lifting.
    
    }
        
    
    
    #--------------------------------------------------------------------------
    # Event handling
    #
    
    ##
    # _update
    #    If all of the things needed to run update are present update everything in
    #  updateList. Regardless, reschedule.
    #
    method _update {} {
    
        if {[$self _canUpdate]} {
            
            foreach object $updateList {
                $object update
            }
            # Now the elapsed run time:

            set stateAPI $options(-statemodel)
            if {[$stateAPI currentState] eq "BEGIN"} {
                set elapsedText [$stateAPI elapsedRunTime]
                $options(-view) configure -elapsed $elapsedText
            }
        }
        if {$options(-recordingcontroller) ne ""} {
            $options(-recordingcontroller) update
        }
        after $options(-refresh) $self _update
    }
}


    


#------------------------------------------------------------------------------
#  Utility procs:

##
# usage
#   Output an error message and program usage.
#
# @param msg - the error message.
#
proc usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "   \$DAQBIN/rdo_RunControl host user program..."
    puts stderr "Provides a graphical interface to control run state"
    puts stderr "Where:"
    puts stderr "   host  - is the host running the program manager"
    puts stderr "   user  - is the user the program manager is running under"
    puts stderr "   program... is a list of program names that are readouts."
    
    exit -1
    
}

#------------------------------------------------------------------------------
#  Entry

if {[llength $argv] < 3} {
    usage "Insufficient number of parameters"
}
set mgrhost [lindex $argv 0]
set mgruser [lindex $argv 1]
set programs [lrange $argv 2 end]


# Make the view.   That's easy:

RunControlGUI .gui
pack .gui -fill both -expand 1


# Make the models needed by the controller

StateClient state -host $mgrhost -user $mgruser
ProgramClient programs -host $mgrhost -user $mgruser
KvClient    kv -host $mgrhost -user $mgruser
LoggerRestClient logger -host $mgrhost -user $mgruser


# Make the controllers

RecordingController logcontrol -view [.gui getRecording] \
    -kvmodel kv -statemodel state -recordingmodel logger

RunController controller \
    -view .gui -statemodel state -programmodel programs -kvmodel kv  \
    -readouts $programs -recordingcontroller logcontrol

# Sit back and let the event loop handle the rest:

