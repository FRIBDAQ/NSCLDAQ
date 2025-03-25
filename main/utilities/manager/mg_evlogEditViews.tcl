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
# @file   mg_evlogEditViews.tcl
# @brief  Views used by event log configuration editors.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide evLogEditViews  1.0
package require Tk
package require snit

namespace eval evLogEditViews {
    ##
    # loggerEq
    #    @param l1 - dict defining one longger.
    #    @param l2 - dict defining a second logger.
    #    @return bool - true if the two loggers are functionally identical
    #                by functionally identical we mean that only the values of
    #                'the keys that matter' are the same.
    #
    proc loggerEq {l1 l2} {
        return [expr {
            ([dict get $l1 daqroot] eq [dict get $l2 daqroot])       &&
            ([dict get $l1 ring]  eq [dict get $l2 ring])            &&
            ([dict get $l1 host] eq [dict get $l2 host])            &&
            ([dict get $l1 destination] eq [dict get $l2 destination]) &&
            ([dict get $l1 container] eq [dict get $l2 container])    &&
            ([dict get $l1 partial] == [dict get $l2 partial])        &&
            ([dict get $l1 critical] == [dict get $l2 critical])      &&
            ([dict get $l1 enabled] == [dict get $l2 enabled])
        }]
    }
}


##
# @class evLogEditLogger
#    Provides an editor for a single logger.
#
# OPTIONS
#    *  -containers   - Containers that can be chosen fromm.
#    *  -savelabel    - Label for the save command.
#    *  -command      - command script for the button.
#
# METHODS:
#    * load          - Load the values of the editor.
#    * get           - Get the values of the editor.
#    * selectContainer - Select a container from the containers list.
#
# PRESENTATION
#    entries for the root, ring, destination and host.
#    dropdown with containers and checkbuttons for critical, enabled, and partial
#
#   +-------------------------------------------------------------------+
#   |  DAQRoot: [ ] Browse...  Source [ ] Destination [ ] Browse...     |
#   |  Host:  [ ]    Container: [ ]                                     |
#   |  [ ] Partial [ ] Critical [ ] Enabled                             |
#   +-------------------------------------------------------------------+
#
snit::widgetadaptor evlogEditLogger {
    option -containers -default [list] -configuremethod _cfgContainers 
    option -savelabel -default Save
    option -command [list]
    
    #  Checkbutton variables:
    
    variable partial  0
    variable critical 1
    variable enabled  1
    
    constructor {args} {
        installhull using ttk::frame
        
        #  Top row:
        #    The root cluster of widgets:
        
        ttk::label $win.rootl -text "DAQRoot: "
        ttk::entry $win.root
        ttk::button $win.rootb -text Browse... \
            -command [mymethod _browseDir $win.root]
        
        #   Data source cluster of widgets
        
        ttk::label $win.srcl -text Source: 
        ttk::entry $win.source
        
        #     Destination cluster:
        
        ttk::label $win.destl  -text Dest:
        ttk::entry $win.dest
        ttk::button $win.destb -text Browse... \
            -command [mymethod _browseDir $win.dest]
        
        grid $win.rootl $win.root $win.rootb      \
             $win.srcl  $win.source  x            \
             $win.destl $win.dest $win.destb  -sticky w
        
        #  Second row
        #     Host cluster:
        
        ttk::label $win.hostl   -text Host:
        ttk::entry $win.host
        
        #    Container cluster:
        
        ttk::label $win.contl -text Container:
        ttk::combobox $win.cont
        
        grid $win.hostl $win.host x    \
             $win.contl $win.cont  -sticky w
        
        #   bottom row of checkboxes.
        
        ttk::checkbutton $win.partial -text Partial -variable [myvar partial]
        ttk::checkbutton $win.critical -text Critical -variable [myvar critical]
        ttk::checkbutton $win.enabled -text Enabled -variable [myvar enabled]
        grid $win.partial $win.critical $win.enabled
        
        #  The save button.
        
        ttk::button $win.save -textvariable [myvar options(-savelabel)] \
            -command [mymethod _dispatchCommand]

        grid $win.save
        
        $self configurelist $args
        
    }
    #--------------------------------------------------------------------------
    # Private utilities
    
    #   _setEntry
    #      Set the contents of an entry.
    # @param entry - an entry widget spec.
    # @param value - the value.
    #
    proc _setEntry {entry value} {
        $entry delete 0 end
        $entry insert end $value
    }
    
    
    #---------------------------------------------------------------------------
    #  Configuration management
    
    ##
    #  _cfgContainers
    #    Configure the values in the container names combobox.
    #    Note that if the client wants to allow containerless specifiction,
    #    they should provide the first entry as an empty string.
    #
    # @param optname - name of the option being modified.
    # @param value   - List of allowed container names.
    #
    method _cfgContainers {optname value} {
        $win.cont configure -values $value
        set options($optname) $value
    }
    
    
    #-------------------------------------------------------------------------
    # Event handling.
    
    ##
    # _browseDir
    #   Browse for a directory that is loaded into the associated entry.
    #   Note the current directory, if there is one, is used for -initialdir.
    #
    #  Since we may have differing containers with differing mappings,
    #  we set -mustexist false on the dialog.
    #
    # @param entry - entry widget to load when a successful choice is made.
    #
    method _browseDir {entry} {
        set dir [$entry get]
        if {$dir eq "" } {
            set dir [pwd];              # use current if none yet.
        }
        set result [tk_chooseDirectory \
            -initialdir $dir -mustexist 0 -parent $win -title "Choose directory"
        ]
        if {$result ne ""} {
            _setEntry $entry $result
        }
    }
    ##
    # _dispatchCommand
    #   Dispatch to the user's script.  The user's script will have the
    #   widget name appended to it
    #
    method _dispatchCommand {} {
        set script $options(-command)
        if {$script ne ""} {
            lappend script $win
            uplevel #0 $script
        }
    }
    #------------------------------------------------------------------------
    # Public methods.
    
    ##
    # load
    #    Load the editor with an event logger definition.
    #
    # @param logger  The logger definition.  This in in the form of one of the
    #                dict elements that comes back from eventlog::listLoggers.
    #
    method load {logger} {
        #  This unloading of raw values from the dict ensures that if the dict
        # is improperly formatted the editor is not corrupted:
        
        set root [dict get $logger daqroot]
        set ring [dict get $logger ring]
        set host [dict get $logger host]
        set ispartial [dict get $logger partial]
        set dest [dict get $logger destination]
        set iscritical [dict get $logger critical]
        set isenabled [dict get $logger enabled]
        set container [dict get $logger container]
        
        set cindex [lsearch -exact $options(-containers) $container]
        if {$cindex == -1} {
            error "The value of '$container' is not in the list of legal containers"
        }
        
        #  We could pull everything we need so load the editor:
        
        _setEntry $win.root $root
        _setEntry $win.source $ring
        _setEntry $win.dest $dest
        _setEntry $win.host $host
        $win.cont current $cindex
        set partial $ispartial
        set critical $iscritical
        set enabled $isenabled
        
    }
    ##
    # get
    #   Get the value of the editor.
    #
    # @return dict with the following key/value pairs:
    #        - daqroot - DAQ Root directory of the logger.
    #        - ring    - URI of the data source ring buffer.
    #        - host    - Name/IP address of the host in which the logger runs.
    #        - partial - Boolean indicating if  the logger is a partial logger.
    #        - destination - Destination directory to which events are logged.
    #        - critical - Boolean that's true if the logger is critical.
    #        - enabled  - True if the logger is currenly enabled.
    #        - container- Name of the container.
    #
    method get {} {
        return [dict create                                          \
            daqroot [$win.root get]         ring [$win.source get]      \
            host [$win.host get]         partial $partial            \
            destination [$win.dest get]  critical $critical          \
            enabled $enabled             container [$win.cont get]  \
        ]
    }
    ##
    # selectContainer
    #    Choose one of the containers as the currently selected one.
    #    This is normally done after containers are described on a new
    #    widget so that a default container is selected.
    #
    # @param name - container name requested.
    #
    method selectContainer {name} {
        set index [lsearch -exact $options(-containers) $name]
        if {$index == -1} {
            error "There is no container named '$name'"
        } else {
            $win.cont current $index
        }
    } 
}

##
# @class evlogListView
#    Provides a view for listing event loggers and performing actions
#    on them.
#
# OPTIONS
#    -data - List of event log definitions such as you might get from
#            e.g. eventlog::listLoggers.  Note that the id and container_id
#            are ignored and also not needed.
#    -contextmenu - context menu definition.  This consists of
#             text elements that are put in a context menu.
#    -command - Script executed when a context menu entry is selected.
#             The script receives the menu command text and definition of the
#             item the menu was invoked on.
# METHODS:
#    highlight - Renders the specified item in reverse video coloring.
#    unhighlight - Removes the highlight from the a specified item.
#    unhighlightall - removes the highlight from all items.
# PRESENTATION:
#    The data are presented as a tree with the following columns:
#    *  DAQRoot - the DAQ installation version used.
#    *  source  - URL of the ring buffer data is taken from.
#    *  host    - The Host on which the logger runs.
#    *  destination - the destination in whichthe data are logged.
#    * container- Name of the container (or blank if none) in which the
#                  logger run.s
#    *  partial - X if the logger is partial
#    *  critical- X if the logger is critical
#    *  enabled - X if the logger is enabled.
#
snit::widgetadaptor evlogListView {
    option -data -default [list] -configuremethod _cfgData
    option -contextmenu -default [list] -configuremethod _cfgMenu
    option -command -default [list]
    
    variable contextMenu "";       # If not empty the context menu.
    
    constructor args {
        
        installhull using ttk::treeview
        
        set columns [list root source host destination container partial critical embedded]
        set titles   [list DAQRoot source host destination container P C E]
        
        ttk::treeview $win.t -yscrollcommand [list $win.vsb set]    \
            -columns $columns -show headings -selectmode none
        
        foreach c $columns h $titles {
            $win.t heading $c -text $h
            if {[string length $h] == 1} {
                $win.t column $c -width 15 -stretch 0
            }
        }
        
        ttk::scrollbar $win.vsb -command [list $win.t yview] -orient vertical
        
        #  Make an item tag and bind the button-3 event to post any 
        #  context menu:
        
        $win.t tag add item [list];         # All items.
        $win.t tag bind item <ButtonPress-3> [mymethod _postContextMenu %X %Y %x %y]
        $win.t tag add highlighted [list];  # Highlighted items.
        
        grid $win.t $win.vsb -sticky nsew
        grid columnconfigure $win 0 -weight 1
        grid columnconfigure $win 1 -weight 0
        
        $self configurelist $args
    }
    #----------------------------------------------------------------------
    # Public methods:
    #
    
    ##
    # highlight
    #   Given the definition of a logger highlights that element.
    #   -   Attempts to highlight nonexistent defs are silently ignored.
    #   -   Any existing highlights remain highlighted.
    #   -   This is done by adding the element to the selection and tagging it
    #       with the highlight tag (so that unhighlight's search space is smaller).
    #
    # @param def - definition to highlight.
    #
    method highlight {def} {
        foreach entry [$win.t children {}] {
            set itemdef [_valuesToDict [$win.t item $entry -values]]
            if {[_loggerEq $def $itemdef]} {
                $win.t selection add $entry
                $win.t tag add highlight $entry
                break;                # Assume there can be only one.
            }
        }
    }
    ##
    # unhighlight
    #    Remove the highlight from a logger.
    #
    # @param def  - logger definition
    #
    method unhighlight {def} {
        foreach entry [$win.t tag has highlight] {
            set itemdef [_valuesToDict [$win.t item $entry -values]]
            if {[_loggerEq $def $itemdef]} {
                $win.t selection remove $entry
                $win.t tag remove highlight $entry
                break;      # There can be only one match.
            }
        }
    }
    ##
    # unhighlightall
    #    Remove the highlight from all elements.
    #
    method unhighlightall {} {
        set items [$win.t tag has highlight]
        $win.t selection remove $items
        $win.t tag remove highlight $items
    }
    
    #----------------------------------------------------------------------
    #  Utility methods/procs
    #
    
    ##
    # _boolValue
    #   Takes a dict and a key and returns "" if the value is boolean false and
    #   X otherwise.
    #
    # @param d   - dictionary value.
    # @param k   - Key name.
    # @return string - "" or "X"
    proc _boolValue {d k} {
        set value [dict get $d $k]
        
        if {$value} {
            return "X"
        } else {
            return " "
        }
    }
    ##
    # _textToBool
    #    Takes a text value and turns it into a bool
    #
    # @param value
    # @return bool
    #
    proc _textToBool {value} {
        if {$value eq "X"} {
            return 1
        } else {
            return 0
        }
    }
    ##
    # _valuesToDict
    #   Turn the values list into a dict that defines the
    #   logger.
    # @param data   - Data from the -values e.g. from a list.
    # @return dict definining the logger.
    #
    proc _valuesToDict {data} {
        set def [dict create                                          \
                daqroot [lindex $data 0]      ring [lindex $data 1] \
                host [lindex $data 2]     \
                destination [lindex $data 3]    container [lindex $data 4] \
                partial [_textToBool [lindex $data 5]]                     \
                critical [_textToBool [lindex $data 6]]                     \
                enabled [_textToBool [lindex $data 7]]                     \
             ]
        return $def
    }
    ##
    # _loggerEq
    #    @param l1 - dict defining one longger.
    #    @param l2 - dict defining a second logger.
    #    @return bool - true if the two loggers are functionally identical
    #                by functionally identical we mean that only the values of
    #                'the keys that matter' are the same.
    #
    proc _loggerEq {l1 l2} {
        return [evLogEditViews::loggerEq $l1 $l2]
    }
        
    
    
    
    ##
    # _addLogger
    #   Adds a single logger to the end of the list of loggers.
    #  The logger is tagged with the item ta so that it's got the right click
    #  bound to it.
    #
    # @param logger  - The dict containing the logger definition.
    #
    method _addLogger {logger} {
        
        set values [list                                             \
            [dict get $logger daqroot]  [dict get $logger ring]      \
            [dict get $logger host]     \
            [dict get $logger destination] [dict get $logger container] \
        ]
        lappend values [_boolValue $logger partial]
        lappend values [_boolValue $logger critical]
        lappend values [_boolValue $logger enabled]
        
        $win.t insert {} end -values $values -tags [list item]
    }
    ##
    # _createContextMenuIfNecessary
    #    If the conextMenu variable is an empty string and
    #    options(-contextmenu) is a nonempty list, create the new context
    #    menu.  The elements of the option(-contextmenu) list are labels for
    #    command entries in that menu and all commands route to _dispatchCommand.
    #    which will dispatch to the user's -command script.
    #
    # @param x - window coordinates of pointer.
    # @param y - window coordinates of pointer.
    # @note the window coordinates are passed to the menu action method so that
    #       the element the context menu was created under can be retrieved and passed
    #       to the user's script.
    # @note because of the above we actually have to rebuild the context menu each time...that is
    #     it's always necessary
    #
    method _createContextMenuIfNecessary {x y} {
        destroy $contextMenu
        set contextMenu ""
        if {($contextMenu eq "") && ([llength $options(-contextmenu)] > 0) } {
            set contextMenu [menu $win.context -tearoff 0]
            foreach item $options(-contextmenu) {
                $contextMenu add command \
                    -label $item -command [mymethod _dispatchMenuItem $item $x $y]
            }
        }
    }
    
    #----------------------------------------------------------------------
    # Configuration handling
    #
    
    ##
    # _cfgData
    #   Stock the treeview with the data in the list.
    # @param optname - name of the option being modified.
    # @param value   - new value.
    #
    method _cfgData {optname value} {
        $win.t delete [$win.t children {}]
        foreach logger $value {
            $self _addLogger $logger
        }
        
        set options($optname) $value
    }
    ##
    # _cfgMenu
    #   Configure the menu contents.. This just involves destroying
    #   and emptying out the contextMenu string so that the next
    #   posting will regenerate the menu:
    #
    # @param optname - name of the option being modified.
    # @param value   - new value.
    #
    method _cfgMenu {optname value } {
        set options($optname) $value
        destroy $contextMenu
        set contextMenu ""
    }
    
    #-----------------------------------------------------------------------
    # Event handling.
    #
    
    ##
    # _postContextMenu
    #    If there's an existing context menu post it at the cursor position.
    #    If there isn't a context menu, generate it and post it.
    #
    # @param x  - Root window x coordinate of pointer.
    # @param y  - Root window y coordinate of pointer.
    # @param wx - Window coordinates of pointer.
    # @param wy - Window coordinates of pointer.'
    #
    #
    method _postContextMenu {x y wx wy} {
        
        $self _createContextMenuIfNecessary $wx $wy
        if {$contextMenu ne ""} {
            $contextMenu post $x $y
        }
    }
    ##
    # _dispatchMenuItem
    #     Called when a context menu item is clicked. If the user has a
    #     -command script the menu item label is lappended to it along with the
    #     dict describing the item the pointer was over when the context menu
    #     was posted and the script is called at global level.
    #
    # @param item  - the text of the label of the menu item clicked.k
    # @param x     - X window Position of the item the menu was posted on.
    # @param y     - Y Window position of the item the menu was posted on.
    #
    method _dispatchMenuItem {item x y} {
        set command $options(-command)
        if {$command ne ""} {
            set el [$win.t identify item $x $y]
            set data [$win.t item $el -values]
            set def [_valuesToDict $data]
            lappend command $item $def
            uplevel #0 $command
        }
    }
}
##
# @class evlogEditorView
#    The view of a full event logger editor.
#
#  OPTIONS:
#     *  -data  - the current event log definition data from e.g
#          eventlog::listLoggers
#     *  -savecommand - the command to execute when it's time to save
#          the current set of definitions.
#     *  -cancelcommand - the command to ex
#     *  -containers - list of legal containers.
#
# PRESENTATION:
#   The top part is a list of the event loggers defined by -data.
#   The bottom part is an editor that allows eventlog definitions to be
#   added or changed.
#   At the very bottom are "Save" and "Cancel" buttons.
#   A context menu on the logger list provides the ability to start a new definition,
#   initiate an edit of an existing item or delete an existing item.
#
snit::widgetadaptor evlogEditorView {
    
    option -savecommand
    option -cancelcommand
    
    component loggerList
    component editor
    
    delegate option -data to loggerList
    delegate option -containers to editor
    
    
    # The use of the editor is modal.  It can either edit an existing
    # item or create a new one.  The mode keeps track of what to do
    # when the save button is clicked and has the values "create" or "edit"
    # It also determines the label that will be given to the editor's save
    # button.  "Create" or "Modify.
    #
    
    variable mode create
    variable editorSaveLabels -array [list      \
        create  Create         edit  Modify      \
    ]
    variable editing ""
    
    typevariable defaultEditorValue [dict create      \
        daqroot   [file normalize $::env(DAQROOT)]      \
        host      [exec hostname]                     \
        ring      ""                                 \
        destination ""                                \
        container   ""                               \
        partial  0 critical 1 enabled 1              \
    ]
    
    constructor {args} {
        installhull using ttk::frame
        
        install loggerList using evlogListView  $win.list    \
            -contextmenu [list New Edit Delete]    \
            -command [mymethod _onContextMenu]
        install editor using evlogEditLogger $win.edit  \
            -command [mymethod _onEditSaved] \
            -savelabel $editorSaveLabels($mode) -containers [list ""]
        set actions [ttk::frame $win.actions -relief groove -borderwidth 3]
        ttk::button $actions.save -text Save -command [mymethod _onSave]
        ttk::button $actions.cancel -text Cancel \
            -command [mymethod _onCancel]
        
        grid $loggerList  -sticky nsew
        grid $editor -sticky nsew
        grid $actions.save $actions.cancel -sticky w -padx 5
        grid $actions -sticky nsew
        
        $editor load $defaultEditorValue
        
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    #   Private utilities:
    #
    ##
    # _checkDef
    #   Ensure there are no blank items in a def that must have no blank items.
    # @param def - logger definition.
    #
    proc _checkDef {def} {
        
        if {
            ([dict get $def daqroot]   eq "")                   ||
            ([dict get $def ring] eq "")                         ||
            ([dict get $def host] eq "")                       ||
            ([dict get $def destination] eq "")        
        } {
            error "Some items in the editor are blank that must not be."
        }
    }
    
    ##
    # _replaceItem
    #   Replace a definition in a list with another definitions.
    #
    # @param defs  - list of definitions to operate on.
    # @param editing - Item to replace.
    # @param newDef - What to replace it with.
    # @return list of dicts -modified list.
    # @note if there's no match the new item is appended to the list of defintions.
    #
    proc _replaceItem {defs editing newdef} {
        set index 0
        set replaced 0
        foreach item $defs {
            if {[::evLogEditViews::loggerEq $item $editing]} {
                set defs [lreplace $defs $index $index $newdef]
                set replaced 1
                break
            } else {
                incr index
            }
        }
        if {!$replaced} {
            lappend defs $newdef
        }
        
        return $defs
    }
    
    ##
    # _startNewItem
    #    Begin editing a new item:
    #    - set the edit to a default value,
    #    - set the mode to edit.
    #    - Set the editor save button accordingly.
    #    - unhighlight all list entries.
    #
    method _startNewItem {} {
            $editor load $defaultEditorValue
            set mode edit
            $editor configure -savelabel $editorSaveLabels($mode)
            $loggerList unhighlightall
    }
    ##
    # _startEdit
    #    Begin editing an existing definition.
    #    - highlight the entry being edited.
    #    - set mode and editor button label appropriately.
    #    - load the editor with the definition.
    # @param def  - starting point of edit.
    #
    method _startEdit {def} {
        set editing $def
        $editor load $def
        set mode edit
        $editor configure -savelabel $editorSaveLabels($mode)
        $loggerList unhighlightall
        $loggerList highlight $def
    }
    ##
    # _deleteItem
    #    Delete an item (if we can find it)
    #    - Find the item to delete in our data.
    #    - Remove it.
    #    - unhighlight everything.
    #    - reset -data.
    #
    # @param def - definition of the item to delete.
    # @note the editor is left alone as is the editing mode.  This has implications
    #       on the save operation.
    #
    method _deleteItem {def} {
        set newdefs [list]
        foreach item [$loggerList cget -data] {
            if {![::evLogEditViews::loggerEq $def $item]} {
                lappend newdefs $item
            }
        }
        $loggerList configure -data $newdefs;    #also gets rid of all highlights etc.
    }
    
    
    #--------------------------------------------------------------------------
    #  Event handling.
    
    ##
    # _onContextMenu
    #   Called when a context menu entry has been clicked.
    #   We use the name of the menu entry to dispatch to the specific handler.
    #
    #  @param item  text on the itme.
    #  @param def   definition under the pointer when the menu was posted
    #
    method _onContextMenu {item def} {
        if {$item eq "New"} {
            $self _startNewItem 
        } elseif {$item eq "Edit"} {
            $self _startEdit $def
        } elseif {$item eq "Delete"} {
            $self _deleteItem $def
        } else {
            error "BUGBUG - Context menu returned an illegal menu entry '$item'"
        }
    }
    ##
    # _onEditSaved
    #    Called in response to the save button clicked in the editor.
    #    -  Pull the definition from the editor.
    #    -  If the mode is create append it to the current data.
    #    -  If the mode is edit locate the item being edited.  If found,
    #       replace it with new data.
    #    - Regardless:
    #      - unhighlight all list entries.
    #      - Set the mode to create.
    #      - Set the button label appropriately
    #      - load the default.
    #
    # @param ed - Editor widget.
    #
    method _onEditSaved {ed} {
        set newDef [$ed get]
        _checkDef $newDef;       # Make sure we have all the chunks.
        set defs [$loggerList cget -data]
        if {$mode eq "create"} {
            lappend defs $newDef
            $loggerList configure -data  $defs
        } elseif {$mode eq "edit"} {
            set defs [_replaceItem $defs $editing $newDef]
            $loggerList configure -data $defs
        } else {
            error "Invalid mode: '$mode'"
        }
        
        set mode create
        $ed load $defaultEditorValue
        $ed configure -savelabel $editorSaveLabels($mode)
    }
    ##
    # _onSave
    #   The botton Save button is clicked.   THe client's -savecommand
    #   is called with the data from the listing as a parameter.
    #
    #  *  If the -savecommand value is empty, this event is ignroed.
    #  *  The script is called at global level.
    #
    method _onSave {} {
        set script $options(-savecommand)
        if {$script ne ""} {
            lappend script [$loggerList cget -data]
            uplevel #0 $script
        }
    }
    ##
    # _onCancel
    #    Script called when the cancel button is clicked. Normally,
    #    the caller would reload the initial data in this case.
    #
    #   *  If the -savecommand is empty this event is ignored.
    #   *  The script is run at the global level.
    #
    method _onCancel {} {
        set script $options(-cancelcommand)
        if {$script ne ""} {
            uplevel #0 $script
        }
    }
}
