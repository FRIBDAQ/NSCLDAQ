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
# @file mg_readout_wizard.tcl
# @brief Wizard to create readout programs.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names env DAQTCLLIBS] ne ""} {
    lappend auto_path $env(DAQTCLLIBS)
}
package require Tk
package require snit

package require programs
package require sequence
package require containers
package require sqlite3

##
# Provides a wizard that creates all of the stuff to make a readout program work.
# This is not only the Readout program itself but the REST clients that
# are needed to start it.  Each Readout program named xxx
# will create the following programs.
#
#  -  xxx_readout the readout itself.
#  -  xxx_init - initializes the hardware.
#  -  xxx_setrun - sets the run number from the KV store.
#  -  xxx_settitle - sets the title from the KV Store.
#  -  xxx_beginrun - Starts the run.
#  -  xxx_endrun   - Ends the run.
#  -  xxx_shutdown - Shuts the readout down.
#
#  If the following sequences don't exist they are created
#
#   -  bootreadouts - triggered by the BOOT transition and xxx is added to that sequence.
#   -  initreadouts - triggered by the  HWINIT transition and xxx_init is added to that sequence.
#   -  beginreadouts- triggered by the BEGIN transition, xxx_setrun, xxx_settitle
#                     and xxx_beginrun are added to that sequence.
#   -  endreadouts  - Triggered by the END transtion, xxx_endrun is added to it.
#   -  shutdownreadouts - Triggered by SHUTDOWN< xxx_shutdown is added to it.
#
#  For each Readout we need the following:
#   *  Type of Readout - XIA/DDAS-12+, VMUSB, CCUSB, Customized
#   *  Container - can be none or any of the defined containers.
#   *  Host
#   *  Working directory.
#   *  IF customized, the location of the Readout program.
#   *  Source id  - for the event builder.
#   *  ring   - Name of ring buffer.
#   *  Rest service name (defaults to ReadoutREST)
#   IF XXUSBReadout:
#    ctlconfig, daqconfig
#   IF XIADDAS-12+:
#     sort host, sort ring, sort window, fifo threshold, buffersize, infinity clock,
#     clock multiplier, scaler readout period.
#   If Customized
#     Readout filename.
#     Any options the user wants to add.
#
#   Environment variables the program will have... in addition to those set up by
#   the container start script.
#
#  Note that prior to DDAS readout prior to V12 should use customized and select
#  $DAQBIN/DDASReadout configuring it manually using environment variables etc.
#

#==============================================================================
#   Utility widgets:
#

##
# @class NameValueList
#    Form to prompt for a set of name/value items.
#    These are not assumed to be ordered.
#
# OPTIONS
#   *  -items - List of name vale pairs.  It is legal for there to be no
#               value.
#
#
snit::widgetadaptor NameValueList {
      option -items -configuremethod _stockList  -cgetmethod _getList
      
      constructor args {
          installhull using ttk::frame
          
          # The list box and scrollbars.
          
          ttk::frame $win.list
          listbox    $win.list.listbox    \
              -xscrollcommand [list $win.list.hscroll set]     \
              -yscrollcommand [list $win.list.vscroll set]     \
              -selectmode single
          ttk::scrollbar $win.list.vscroll -orient vertical \
              -command [list $win.list.listbox yview] 
          ttk::scrollbar $win.list.hscroll -orient horizontal \
              -command [list $win.list.listbox xview]
          
          grid $win.list.listbox $win.list.vscroll -sticky nsew
          grid $win.list.hscroll              -sticky ew
          
          # The entries and add button.
          
          ttk::frame $win.entries
          ttk::entry $win.entries.name
          ttk::label $win.entries.namelabel -text Name
          ttk::entry $win.entries.value
          ttk::label $win.entries.valuelabel -text Value
          ttk::button $win.entries.add -text Add -command [mymethod _addItem]
          
          grid $win.entries.name $win.entries.namelabel   -padx 3 -sticky nsew
          grid $win.entries.value $win.entries.valuelabel -padx 3 -sticky nsew
          grid $win.entries.add -sticky w
          
          grid $win.list  -sticky nsew
          grid $win.entries -sticky nsew
          
          # The context menu which posts on <Button-3>
          
          menu $win.context -tearoff 0
          $win.context add command -label Delete -command [mymethod _deleteEntry]
          $win.context add command -label Edit   -command [mymethod _editEntry]
          
          bind $win.list.listbox <Button-3> [mymethod _contextMenu %x %y %X %Y]
          
          
          $self configurelist $args
      }
      #-------------------------------------------------------------------------
      #  Configuration methods.
      #
      
      ##
      # _stockList
      #   Called to configure the items.  Each item is a one or two element
      #   List.  If two elements we put quotations around the second element
      #   to ensure that the list is valid.
      #
      method _stockList {name value} {
           
           set w $win.list.listbox
           
           $w delete  0 end
           foreach item $value {
               set optname [lindex $item 0]
               set v ""
               set item [lrange $item 1 end]
               if {$item ne ""} {
                 set v "\"$item\""
               }
               $w insert end "$optname $v"
           }
           
      }
      ##
      # _getList
      #    Returns the value of the list.
      #
      method _getList {name} {
           set w $win.list.listbox
           
           set contents [$w get 0 end]
           set result [list]
           foreach entry $contents {
              if {[llength $entry] == 1} {
                  lappend result $entry
              } else {
                 set pair [list [lindex $entry 0] [lrange $entry 1 end]]
                 lappend result $pair
              }
           }
            
           
           
           return $result
      }
      #------------------------------------------------------------------------
      # Event handling.
      
      ##
      # _addItem
      #   Called when the add button is clicked.  There must be a name
      #   if there is a value then a two element list is added to the list box.
      #
      method _addItem {} {
           set l $win.list.listbox
           set n $win.entries.name
           set v $win.entries.value
           
           set name [string trim [$n get]]
           set value [string trim [$v get]]
           
           
           if {$name ne ""} {
               set item [list $name]
               if {$value ne ""} {
                   append item " "  $value
               }
               $l insert end $item
           }
           $n delete 0 end
           $v delete 0 end
      }
      
      ##
      # _contextMenu
      #   In response to a right click.
      #   - If the mouse is over an entry, select it and pop up the
      #     menu.
      #   -  Otherwise do notyhing
      #
      # @param x - window relative pointer x.
      # @param y - window relative pointer y.
      # @param X,Y - screen relative pointer x/y
      #
      method _contextMenu {x y X Y} {
           set l $win.list.listbox
           set index [$l nearest $y]
           
           $l selection clear 0 end
           $l selection set $index $index
           $win.context post $X $Y
      }
      ##
      # _deleteEntry
      #    Delete the selected element...and unpoast the context menu.
      #
      method _deleteEntry {} {
           set l $win.list.listbox
           set selIndex [$l curselection]
           if {$selIndex ne ""} {
              $l delete $selIndex $selIndex
           }
           $win.context unpost
      }
      ##
      # _editEntry
      #   Edits the selected entry.  The entry is loaded into the
      #   Name, and value entries and then deleted from the list.
      #   Note that the menu is unposted.
      method _editEntry {} {
           set l $win.list.listbox
           
           set selindex [$l curselection]
           if {$selindex ne ""} {
               set selection [$l get $selindex]
               $win.entries.name delete 0 end
               $win.entries.value delete 0 end
               
               $win.entries.name  insert 0 [lindex $selection 0]
               $win.entries.value insert 0 {*}[lrange $selection 1 end]
               
               $l delete $selindex $selindex
           }
           $win.context unpost
      }
}
      
##
# OrderedValueList
#
#   As the name implies, this widget provides a mechanism to enter, and edit
#   a list of values where order important.
#
#  OPTIONS
#  *  -values list of values in the listbox (set/get).
#
snit::widgetadaptor OrderedValueList {
    option -values -default [list] -configuremethod _cfgValues \
           -cgetmethod _cgetValues
    
    constructor args {
        installhull using ttk::frame
        
        # List box and scrollbars.
        
        ttk::frame $win.list
        listbox    $win.list.listbox   -selectmode single    \
            -xscrollcommand [list $win.list.hscroll set]     \
            -yscrollcommand [list $win.list.vscroll set]
        ttk::scrollbar $win.list.vscroll -orient vertical  \
            -command [list $win.list.listbox yview]
        ttk::scrollbar $win.list.hscroll -orient horizontal \
            -command [list $win.list.listbox xview]
        
        grid $win.list.listbox $win.list.vscroll -sticky nsew
        grid $win.list.hscroll                   -stick new
        
        #entry and buttons:
        
        ttk::frame $win.entry
        ttk::entry $win.entry.entry
        ttk::label $win.entry.label -text Value
        ttk::button $win.entry.add -text Add  -command [mymethod _addItem]
        ttk::button $win.entry.replace -text Replace -command [mymethod _replaceItem]
        
        grid $win.entry.entry $win.entry.label -sticky nsew
        grid $win.entry.add $win.entry.replace -sticky nsw
        
        grid $win.list
        grid $win.entry
        
        $self configurelist $args
        
        # context menu (right click on item).
        #  Allows repositioning and deletion:
        
        menu $win.context -tearoff 0
        $win.context add command -label Delete -command [mymethod _delete]
        $win.context add command -label {Move up} -command [mymethod _moveUp]
        $win.context add command -label {Move down} -command [mymethod _moveDown]
        
        bind  $win.list.listbox <Button-3> [mymethod _contextMenu %x %y %X %Y]
    }
    #--------------------------------------------------------------------------
    # Configuration handling:
    
    ##
    # _cfgValues
    #     New list of values
    # @param name -configuration option name.
    # @param value - New values.
    #
    method _cfgValues {name value} {
        set l $win.list.listbox
        $l delete 0 end
        foreach item $value {
            $l insert end $item
        }
    }
    ##
    # _cgetValues
    #    Return the list of values in the list box.
    #
    # @param name - configuration option name.
    #
    method _cgetValues {name} {
        
        return [$win.list.listbox get 0 end]
    }
    #-------------------------------------------------------------------------
    # Event handling.
    
    ##
    # _addItem
    #    Add an item from the text entry box to the end of the list.
    #
    method _addItem {} {
        set item [string trim [$win.entry.entry get]]
        if {$item ne ""} {
           $win.list.listbox insert end $item
           $win.entry.entry delete 0 end
        }
    }
    ##
    # _replaceItem
    #    Replace the selected item with the entry.
    #    - There must be a new value.
    #    - There must be a selection.
    #
    method _replaceItem {} {
        # Get the value it must be nonempty:
        
        set value [string trim [$win.entry.entry get]]
        if {$value eq ""} {
            tk_messageBox -icon info -type ok -title "No value" \
                -message {Need a value to replace} -parent $win
            return
        }
        #  Get the selection - it too must be non empty:
        
        set sel [$win.list.listbox curselection]
        if {$sel eq ""} {
             tk_messageBox  -icon info -type ok -title "No selection" \
                 -message {No item selected for replacement} \
                 -parent $win
                    
             return
        }
        $win.list.listbox delete $sel
        $win.list.listbox insert $sel $value
        
    }
    ##
    # _contextMenu
    #   Select the item under the mouse and provide a pop up menu to
    #   manipuate that item.
    #
    # @param x,y - window relative pointer coords.
    # @param X,Y - screen relativel pointer coords.
    #
    method _contextMenu {x y X Y} {
       set l $win.list.listbox
       set index [$l nearest $y]
       
       $l selection clear 0 end
       $l selection set $index $index
       
       $win.context post $X $Y 
    }
    ##
    # _delete
    #    Delete the selected item, if there is one.
    #
    method _delete {} {
        set l $win.list.listbox
        set sel [$l curselection]
        if {$sel ne ""} {
           $l delete $sel $sel
        }
    }
    ##
    # _moveUp
    #   Move the selected item up one slot in the list box.
    #
    method _moveUp {} {
       set l $win.list.listbox
       
       set sel [$l curselection]
       if {$sel ne "" && $sel > 0} {
          set text [$l get $sel $sel]
          $l delete $sel
          $l insert [incr sel -1] $text
          
       }
    }
    ##
    # _moveDown
    #     Move the selected item down one slot in the list box.
    #
    method _moveDown {} {
       set l $win.list.listbox
       
       set sel [$l curselection]
       set n   [$l size]
       if {$sel ne "" && $sel < ($n-1)} {
            set text [$l get $sel $sel]
            $l delete $sel
            $l insert [incr sel]  $text
       }
       
    }
    
}

 #==============================================================================
 #     The Main GUI has several components:
 
 ##
 # @class rdo::CommonAttributes
 #
 #  Form to allow the input of common attributes;
 #
 # *   - -type -  one of: {ddas, vmusb, ccusb, custom}
 # *   - -name - Readout name.
 # *   - -container  If not an empty string the container that will run the readout.
 # *   - -host    Host in which the Readout will run.
 # *   - -directory - working directory in which the Readout will run.
 # *   - -sourceid - Event builder source id.
 # *   - -ring  - output ring buffer.
 # *   - -service - REST service name used to control the Readout.
 # *   - -manageruser - user that will run manager.
 #
 #  Other OPTIONS:
 #    -containers - containers to select between.
 #    -typeselectcommand - script when the radio buttons holding the Readout Type
 #        change.
 #
 snit::widgetadaptor rdo::CommonAttributes {
    option -type   -default XIA
    option -name
    option -containers  -default [list] -configuremethod _configContainers;       # Containers to chose between
    option -container;        # Selected container.
    option -host
    option -directory
    option -sourceid -default 0
    option -ring -default $::tcl_platform(user)
    option -service -default ReadoutREST
    option -typeselectcommand -default [list]
    option -manageruser -default $::tcl_platform(user)
    
    variable daqtypes [list XIA VMUSB CCUSB Custom]
    
    variable fields -array [list]
    
    constructor args {
        installhull using ttk::frame
        
        #  Name of this:
        
        ttk::labelframe $win.name -text {Readout name}
        ttk::entry      $win.name.name -textvariable [myvar options(-name)] \
            -width 32
        ttk::label      $win.name.label -text {Readout Name}
        grid $win.name.name $win.name.label
        set fields(name) $win.name.label
        
        #  The Readout types
        
        ttk::labelframe $win.types -text {Readout Type}
        set radios [list]
        foreach type $daqtypes {
            lappend radios [ttk::radiobutton $win.types.[string tolower $type] \
                -variable [myvar options(-type)] -value $type -text $type \
                -command [mymethod _dispatchType]]
        }
        set options(-type) [lindex $daqtypes 0]
        grid {*}$radios
        
        #  Container and host:
        
        ttk::labelframe $win.container -text {Container}
        ttk::combobox $win.container.container \
            -values [list] \
            -textvariable [myvar options(-container)]
        ttk::entry $win.container.host -textvariable [myvar options(-host)]
        ttk::label $win.container.hostlabel -text Host
        grid $win.container.container  $win.container.host $win.container.hostlabel
        set fields(host) $win.container.hostlabel
        
        # directory, sourceid
        
        ttk::labelframe $win.dir -text {Directory}
        ttk::label $win.dir.label -text Dir: 
        ttk::entry $win.dir.dir -textvariable [myvar options(-directory)]
        ttk::button $win.dir.browse -text {Browse...} -command [mymethod _browsedir]
        grid $win.dir.label $win.dir.dir $win.dir.browse -sticky ew
        set fields(directory) $win.dir.label
        
    
        ttk::labelframe $win.sid -text {Source Id}
        ttk::spinbox $win.sid.sid \
            -from 0 -to 100000 -increment 1 \
            -textvariable [myvar options(-sourceid)]
        ttk::label $win.sid.label -text {Source Id}
        grid $win.sid.sid $win.sid.label -sticky ew
        
        
        # ring and REST service.
        
        ttk::labelframe $win.ring -text {Output Ring}
        ttk::entry $win.ring.ring -textvariable [myvar options(-ring)]
        ttk::label $win.ring.label -text {Ring name}
        grid $win.ring.ring $win.ring.label -sticky ew
        set fields(ring) $win.ring.label
        
        ttk::labelframe $win.service -text {REST service name}
        ttk::entry $win.service.name -textvariable [myvar options(-service)]
        ttk::label $win.service.label -text {Service}
        ttk::entry $win.service.user -textvariable [myvar options(-manageruser)]
        ttk::label $win.service.userlabel -text {User running manager}
        grid $win.service.name $win.service.label -sticky ew
        grid $win.service.user $win.service.userlabel
        set fields(service) $win.service.label
        set fields(manageruser) $win.service.userlabel
        
        #  Grid the top level frames:
        
        grid $win.name  -columnspan 2 -sticky ew
        grid $win.types $win.container -sticky ew
        grid $win.dir $win.sid -sticky ew
        grid $win.ring $win.service -sticky ew
        
         
        $self configurelist $args 
        
    }
    #--------------------------------------------------------------------------
    #  Public methods:
    
    ##
    # highlightField
    #   sets the backgroun of the specified field to red.
    #   The field names are those in the fields array
    #
    method highlightField name {
        if {[array names fields $name] ne ""} {
            $fields($name) configure -foreground red
        } else {
            error "Invalid field name"
        }
    }
    ##
    # resetHighlights
    #   Turns  off the highlight on all fields.
    #
    method resetHighlights {} {
        foreach f [array names fields] {
            $fields($f) configure -foreground black
        }
    }
    #--------------------------------------------------------------------------
    #
    # Configuration methods
    
    
    ##
    # _configContainers
    #    Configure the containers in the $win.container.container combobox.
    #
    # @param name - name of configuration option
    # @param value - new value
    #
    method _configContainers {name value} {
        set options($name) $value
        $win.container.container configure -value $value
    }
    #-------------------------------------------------------------------------
    #  Event handling
    
    
    ##
    # _dispatchType
    #   Called when the type changes.
    #   The user script, if it exists is called with the new type as a parameter.
    #
    method _dispatchType {} {
        set userscript $options(-typeselectcommand)
        if {$userscript ne ""} {
            lappend userscript $options(-type)
            uplevel $userscript
        }
        
    }
    ##
    # _browsedir
    #   Browse for the current directory string.
    #
    method _browsedir {} {
        set dir [tk_chooseDirectory -parent $win -title {Choose directory}]
        
        # Only change if one was chosen:
        #
        if {$dir ne ""} {
            $self configure -directory $dir
        }
    }
 }
 ##
 # @class rdo::XIAAttributes
 #
 #   XIA readouts for version 11.4 experimental and later have
 #   The following attributes (class options).
 #
 #  -  -sorthost   - host in which the sorter runs.
 #  -  -sortring   - Ringbuffer that gets the sorted output.
 #  -  -sortwindow - Sorter sliding window.
 #  -  -fifothreshod - FIFO Threshold
 #  -  -buffersize  - Readout program's buffersize (clump).
 #  -  -infinityclock (bool true if infinity clock should be used).
 #  -  -clockmultiplier - Multiply external clock by this to get ns.
 #  -  -scalerperiod - Seconds between scaler readouts.
 #
 snit::widgetadaptor rdo::XIAAttributes {
    option -sorthost   -default [info hostname]
    option -sortring   -default $::tcl_platform(user)_sort
    option -sortwindow -default 5
    option -fifothreshold -default [expr 8192*10]
    option -buffersize    -default 16384
    option -infinityclock  -default 0
    option -clockmultiplier -default 1
    option -scalerperiod    -default 2
    
    variable fields -array [list]
    
    constructor args {
        installhull using ttk::frame
        #  Title:
        
        ttk::label $win.title -text {XIA Readout Attributes}
        
        #  Sort host entry and label:
        
        ttk::labelframe $win.host -text {Sort host}
        ttk::entry $win.host.host -textvariable [myvar options(-sorthost)]
        ttk::label $win.host.label -text {Sort Host}
        grid $win.host.host $win.host.label -sticky ew
        set fields(sorthost) $win.host.label
        
        # Sort ring
        
        ttk::labelframe $win.ring -text {Sort output ring}
        ttk::entry $win.ring.ring -textvariable [myvar options(-sortring)]
        ttk::label $win.ring.label -text {Output Ring}
        grid $win.ring.ring $win.ring.label -sticky ew
        set fields(sortring) $win.ring.label
        
        #  sort window
        
        ttk::labelframe $win.sortwin -text {Sort window}
        ttk::spinbox $win.sortwin.window \
            -from 1 -to 1000 -increment 1 \
            -textvariable [myvar options(-sortwindow)]
        ttk::label $win.sortwin.label -text {Seconds}
        grid $win.sortwin.window $win.sortwin.label -sticky ew
        set fields(sortwindow) $win.sortwin.label
        
        #  FIFO Threshold
        
        ttk::labelframe $win.fifo -text {Readout Threshold}
        ttk::spinbox $win.fifo.fifo \
            -from 1024 -to [expr 128*1024] -increment 512 \
            -textvariable [myvar options(-fifothreshold)]
        ttk::label $win.fifo.label -text {FIFO Threshold}
        grid $win.fifo.fifo $win.fifo.label -sticky ew
        set fields(fifothreshold) $win.fifo.label
 
        # Buffer size
        
        ttk::labelframe $win.bsize -text {Buffer Size}
        ttk::spinbox    $win.bsize.bsize -from 8192 -to [expr 1024*1024] -increment 1024 \
            -textvariable [myvar options(-buffersize)]
        ttk::label $win.bsize.label -text {Readout Buffersize}
        grid $win.bsize.bsize $win.bsize.label -sticky ew
        set fields(buffersize) $win.bsize.label
        
        #clock parameters
        
        ttk::labelframe $win.clock -text {Clock parameters}
        ttk::checkbutton $win.clock.infinity -text Infinity \
             -onvalue 1 -offvalue 0 -variable [myvar options(-infinityclock)]
        ttk::entry $win.clock.multiplier -textvariable [myvar options(-clockmultiplier)]
        ttk::label $win.clock.mlabel -text {Multiplier}
        grid $win.clock.infinity $win.clock.multiplier $win.clock.mlabel -sticky ew
        
        #scaler period:
        
        ttk::labelframe $win.scaler -text {Scalers}
        ttk::spinbox $win.scaler.period -from 2 -to 3600 -increment 1 \
            -textvariable [myvar options(-scalerperiod)]
        ttk::label $win.scaler.label -text {Readout period}
        grid $win.scaler.period $win.scaler.label -sticky ew
        
        grid $win.title -sticky w
        grid $win.host $win.ring -sticky ew
        grid $win.sortwin $win.fifo -sticky ew
        grid $win.bsize $win.clock -sticky ew
        grid $win.scaler -columnspan 2 -sticky ew
        
        
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    # public methods
    
    ##
    # highlightField
    #   Set the foreground of the specified field identifier to red.
    #
    # @param name - Name of the field - index in fields array.
    #
    method highlightField {name} {
        if {[array names fields $name] ne ""} {
            $fields($name) configure -foreground red
        } else {
            error "Invalid field name '$name'"
        }
    }
    method resetHighlights {} {
        foreach field [array names fields] {
            $fields($field) configure -foreground black
        }
    }
     
 }
##
# @class XXUSBAttributes
#   Prompting form for attributes of an XXUSBReadout.  For this we need:
#
#  -  -byserial  - True if we want a specific serial number
#  -  -serialstring - Serial number string.
#  -  -daqconfig   - DAQ configuration file.
#  -  -usectlserver - Enable control configuration
#  -  -ctlconfig   - Control config file.
#  -  -port        - ctlconfig port.
#  -  -enablelogging - True to turn on logging.
#  -  -logfile     - Log file name.
#  -  -logverbosity - Verbosity of logfile.
#  -   -extracttimestamps - If timestamp extraction is to be done
#  -  -timestamplib - Path to timestamp extraction library.
#
snit::widgetadaptor rdo::XXUSBAttributes {
    option -byserial -default 0 -configuremethod _setSerialVisibility
    option -serialstring
    
    option -daqconfig
    
    option -usectlserver -default 0 -configuremethod _setCtlServerVisibility
    option -ctlconfig
    option -port -default 1024
    
    option -enablelogging -default 0 -configuremethod _setLoggingVisibility
    option -logfile
    option -logverbosity -default 0
    
    option -extracttimestamps -default 1 -configuremethod _setTsLibVisibility
    option -timestamplib -default ""
    
    variable fields -array [list]
    
    constructor args {
        installhull using ttk::frame
        ttk::label $win.title -text {XXUSBReadout attributes}
        
        #  Device selection:
        
        ttk::labelframe $win.selection -text {Device selection}
        ttk::checkbutton $win.selection.byserial -onvalue 1 -offvalue 0 \
            -variable [myvar options(-byserial)] -text {By serial} -command [mymethod _controlSerial]
        ttk::entry $win.selection.serial -textvariable [myvar options(-serialstring)]
        ttk::label $win.selection.label -text {Serial string}
        set fields(serialstring) $win.selection.label
        grid $win.selection.byserial $win.selection.serial $win.selection.label
        
        # DAQ Config file.
        
        ttk::labelframe $win.daqconfig -text {DAQ configuration}
        ttk::label $win.daqconfig.label -text {daqconfig}
        ttk::entry $win.daqconfig.config -textvariable [myvar options(-daqconfig)]
        ttk::button $win.daqconfig.browse -text {Browse...} -command [mymethod _browseDAQFile]
        ttk::checkbutton $win.daqconfig.usets  \
            -onvalue 1 -offvalue 0 -command [mymethod _toggleTslibEntry] \
            -variable [myvar options(-extracttimestamps)]
        ttk::label $win.daqconfig.tslabel -text {Extract Timestamps}
        ttk::entry $win.daqconfig.tslib -textvariable [myvar options(-timestamplib)]
        ttk::button  $win.daqconfig.browsetslib -text Browse... -command [mymethod _browseTSLib]
        grid $win.daqconfig.label $win.daqconfig.config $win.daqconfig.browse   \
            $win.daqconfig.usets $win.daqconfig.tslabel $win.daqconfig.tslib \
            $win.daqconfig.browsetslib \
            -sticky ew
        set fields(daqconfig) $win.daqconfig.label
        set fields(tslib) $win.daqconfig.tslabel
        
        # Control configuration :
        
        ttk::labelframe $win.ctlconfig -text {Control Configuration}
        ttk::checkbutton $win.ctlconfig.enable -text {Use Control Server} \
            -onvalue 1 -offvalue 0 -variable [myvar options(-usectlserver)] \
            -command [mymethod _controlOptions]
        ttk::entry $win.ctlconfig.ctlconfig \
            -textvariable [myvar options(-ctlconfig)]
        ttk::label $win.ctlconfig.filelbl -text {Config File}
        ttk::button $win.ctlconfig.browse \
            -text Browse... -command [mymethod _browseCtlConfig]
        ttk::spinbox $win.ctlconfig.port -textvariable [myvar options(-port)] \
            -from 1024 -to 65535 -increment 1
        ttk::label $win.ctlconfig.plabel -text {Server Port}
        grid $win.ctlconfig.enable -sticky w
        grid $win.ctlconfig.ctlconfig $win.ctlconfig.filelbl \
             $win.ctlconfig.browse $win.ctlconfig.port $win.ctlconfig.plabel \
             -sticky ew
        set fields(ctlconfig) $win.ctlconfig.filelbl
        set fields(ctlport)    $win.ctlconfig.plabel
        
        
        # Logging configuration:
        
        ttk::labelframe $win.logging -text {Logging configuration}
        ttk::checkbutton $win.logging.enable -text Enable -onvalue 1 -offvalue 0 \
             -variable [myvar options(-enablelogging)] \
             -command [mymethod _loggingOptions]
        ttk::entry $win.logging.file -textvariable [myvar options(-logfile)]
        ttk::label $win.logging.label -text {Log File}
        ttk::button $win.logging.browse -text {Browse...} \
             -command [mymethod _browseLogfile]
        ttk::spinbox $win.logging.level -textvariable [myvar options(-logverbosity)] \
         -from 0 -to 3 -increment 1
        ttk::label $win.logging.levellabel -text {Verbosity level}
        grid $win.logging.enable -sticky w
        grid $win.logging.file $win.logging.label $win.logging.browse -sticky ew
        grid $win.logging.level $win.logging.levellabel -sticky w
        set fields(logfile) $win.logging.label
        
        grid $win.title     -sticky w
        grid $win.selection -sticky ew
        grid $win.daqconfig -sticky ew
        grid $win.ctlconfig -sticky ew
        grid $win.logging  -sticky ew
        
        $self configurelist $args
        $self _controlSerial
        $self _controlOptions
        $self _loggingOptions
        $self _toggleTslibEntry
    }
    #---------------------------------------------------------------------------
    # public methods
    
    ##
    # highlightField
    #   Set the foreground of the specified field identifier to red.
    #
    # @param name - Name of the field - index in fields array.
    #
    method highlightField {name} {
        if {[array names fields $name] ne ""} {
            $fields($name) configure -foreground red
        } else {
            error "Invalid field name '$name'"
        }
    }
    method resetHighlights {} {
        foreach field [array names fields] {
            $fields($field) configure -foreground black
        }
    }
     
    #---------------------------------------------------------------------------
    # Configuration handling.
    
    ##
    # _setSerialVisibility
    #   Configure the -byserial flag and visibility of associated fields.
    #
    method _setSerialVisibility {name value} {
        set options($name) $value
        $self _controlSerial
    }
    
    ##
    #  _setCtlServerVisibility
    #    Configure the flag that uses/does not use control server,
    #    just set the options array and invoke _controlOptions.
    #
    method _setCtlServerVisibility {name value} {
        set options($name) $value
        
        $self _controlOptions
    }
    ##
    # _setLoggingVisibility
    #    Configure the flag that enables/disables logging. This enables
    #    disables some widgets too
    #
    method _setLoggingVisibility {name value} {
        set options($name) $value
        $self _loggingOptions
    }
    ##
    #  _setTsLibVisibility
    #
    method _setTsLibVisibility {name value} {
        set options($name) $value
        $self _toggleTslibEntry
    }
    
    #---------------------------------------------------------------------------
    # Event handlers:
    
    ##
    # _toggleTslibEntry
    #   Called when the tslib checkbstton changes state.
    #   If it is checked, enable the associated entry and browse button
    #   otherise disable them.
    #
    method _toggleTslibEntry {} {
        if {$options(-extracttimestamps)  } {
            set state normal
        } else {
            set state disabled
        }
        $win.daqconfig.tslib configure -state $state
        $win.daqconfig.browsetslib configure -state $state
    }
    ##
    # _browseTSLib
    #   Browse the filesystem to set the timestamp library.
    #
    method _browseTSLib {} {
        set result [tk_getOpenFile -parent $win -title "Timstamp Extractor"  \
            -filetype [list                                                   \
                {{Shared object}   .so }                                      \
                {{All files}       *   }                                      \
            ]                                                                 \
        ]
        if {$result ne ""}  {
            $self configure -timestamplib $result
        }
    }
        
    
    
    ##
    #  _browseDAQfile
    #    Select a daqconfig file:
    #
    method _browseDAQFile {} {
        $self _browseForOption -daqconfig
    }
    ##
    # _browseCtlConfig
    #   Browse for the control config file
    #
    method _browseCtlConfig {} {
        $self _browseForOption -ctlconfig
    }
    ##
    # _browseLogfile
    #    Browse for a logfile.
    #
    method _browseLogfile {} {
        set result [tk_getSaveFile -title Logfile -parent $win \
            -filetypes [list                                     \
                {{Log files}  .log}                              \
                {{All files}   *  }                              \
            ]                                                    \
        ]
        if {$result ne ""} {
            $self configure -logfile $result
        }
    }
    ##
    # _controlSerial
    #    Handle the state of the serial string entry depending on
    #    the state of options(-byserial)
    #
    method _controlSerial {} {
        
        if {$options(-byserial)} {
            set state normal
        } else {
            set state disabled
        }
        $win.selection.serial configure -state $state
    }
    
    
    ##
    # _controlOptions
    #   Called when options(-usectlserver) changes
    #
    method _controlOptions {} {
        if {$options(-usectlserver)} {
            set state normal
        } else {
            set state disable
        }
        foreach subwidget [list ctlconfig browse port plabel] {
            $win.ctlconfig.$subwidget config -state $state
        }
            
        
    }
    ##
    # _loggingOptions
    #    Enable/disable logging option inputs.
    #
    method _loggingOptions {} {
        if {$options(-enablelogging)} {
            set state normal
        } else {
            set state disabled
        }
        $win.logging.file configure -state $state
        $win.logging.browse configure -state $state
        $win.logging.level configure -state $state
    }
    #---------------------------------------------------------------------------
    # Utils
    
    method _browseForOption {name} {
       set file [tk_getOpenFile -parent $win -title "Config file" \
            -filetypes [list                                           \
                    {{Tcl Scripts} .tcl}                               \
                    {{All Files}    *}                                  \
        ]]
        if {$file ne ""} {
            $self configure $name $file
        }
    }
}

##
# CustomAttributes
#    The custom attributes that are required are:
#
#  -program - the program that will be run.
#
#  -options - option value pairs for command line options.
#  -environment - Environment variables and their values.
#  -parameters - Parameter following options on the command line.
#
#  These are presented as listboxes that can be edited.
#
#
snit::widgetadaptor rdo::CustomAttributes {
    option -program -default [list]
    
    component programOptions
    component programEnvironment
    component programParameters

    
    delegate option -options to programOptions as -items
    delegate option -environment to programEnvironment as -items
    delegate option -parameters  to programParameters as -values
    
    variable fields -array [list]
    constructor args {
        installhull using ttk::frame
        
        ttk::labelframe $win.program -text {Readout Program}
        ttk::label      $win.program.label -text {Program}
        ttk::entry      $win.program.entry -textvariable [myvar options(-program)]
        ttk::button     $win.program.browse -text {Browse...} -command [mymethod _browseFile]
        set fields(program) $win.program.label
        
        grid $win.program.label $win.program.entry $win.program.browse -sticky ew
        
        ttk::labelframe $win.parameters -text {Parameters and environment}
        ttk::label      $win.parameters.note \
            -text {NOTE: General options will set options and environment variables as well} \
            -foreground DarkOrange3
    
        ttk::labelframe $win.parameters.options -text {Program Options}
        install programOptions using NameValueList   \
            $win.parameters.options.options 
        grid $win.parameters.options.options -sticky nsew
    
        ttk::labelframe $win.parameters.parameters -text {Program parameters}
        install programParameters using OrderedValueList \
            $win.parameters.parameters.parameters
        grid $win.parameters.parameters.parameters -sticky new
        
        ttk::labelframe $win.parameters.environment -text {Environmnent}
        install programEnvironment using NameValueList   \
            $win.parameters.environment.environment
        grid $win.parameters.environment.environment -sticky nsew
        
        grid $win.parameters.note -columnspan 3
        grid $win.parameters.options \
            $win.parameters.parameters \
            $win.parameters.environment -sticky nsew
        grid $win.program -sticky ew
        grid $win.parameters -sticky ew
        
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    # public methods
    
    ##
    # highlightField
    #   Set the foreground of the specified field identifier to red.
    #
    # @param name - Name of the field - index in fields array.
    #
    method highlightField {name} {
        if {[array names fields $name] ne ""} {
            $fields($name) configure -foreground red
        } else {
            error "Invalid field name '$name'"
        }
    }
    method resetHighlights {} {
        foreach field [array names fields] {
            $fields($field) configure -foreground black
        }
    }
    
    #-- private methods:

    ##
    #   browse for the readout file:
    #
    method _browseFile {} {
	set program [tk_getOpenFile -parent $win -title "Readout File"]
	if {$program ne "" } {
	    $win.program.entry delete 0 end
	    $win.program.entry insert 0 $program
	}
    }
} 

#----------------------------------------------------------------------------

##
# @class rdo::ReadoutWizard
#    The UI for the program wizard is a paned window of which at most
#    two panes are visible at any given time.  The rdo::CommonAttributes
#    widget is always visible and then, depending on the type selected,
#    the pane that corresponds to the detailed options for that readout
#    type is visible.
#
snit::widgetadaptor rdo::ReadoutWizard {
    component common
    component xia
    component usb
    component custom
    
    
    #  Delegate the common attributes back out to the component
    
    delegate option -name       to common
    delegate option -readouttype to common as -type
    delegate option -containers to common
    delegate option -container  to common
    delegate option -host       to common
    delegate option -cwd        to common as -directory
    delegate option -sourceid   to common
    delegate option -ring       to common
    delegate option -restservice to common as -service
    delegate option -manageruser to common
    
    
    #  XIA options:
    
    delegate option -sorthost to xia
    delegate option -sortring to xia
    delegate option -sortwindow to xia
    delegate option -fifothreshold to xia
    delegate option -buffersize to xia
    delegate option -infinityclock to xia
    delegate option -clockmultiplier to xia
    delegate option -scalerperiod to xia
    
    #  XXUSB attributes:
    
    delegate option -byserial    to usb
    delegate option -serialstring to usb
    delegate option -daqconfig   to usb
    delegate option -usectlserver to  usb
    delegate option -ctlconfig   to usb
    delegate option -ctlport     to usb as -port
    delegate option -enablelogging to usb
    delegate option -logfile     to usb
    delegate option -logverbosity to usb
    delegate option -extracttimestamps to usb
    delegate option -timestamplib to usb
    
    #  Custom attributes:
    
    delegate option -program   to custom
    delegate option -options   to custom
    delegate option -environment to custom
    delegate option -parameters to custom
    
    # Re-expose the individual highlightField and resetHighlight methods:
    
    delegate method highlightCommonField to common as highlightField
    delegate method resetCommonHighlights to common as resetHighlights
    
    delegate method highlightXIAField to xia as highlightField
    delegate method resetXIAHighlights to xia as resetHighlights
    
    delegate method highlightUSBField to usb as highlightField
    delegate method resetUSBHighlights to usb as resetHighlights
    
    delegate method highlightCustomField to custom as highlightField
    delegate method resetCustomHighlights to custom as resetHighlights
    
    #  The paned window is the catch all method handler:
    
    delegate method * to hull
    delegate option * to hull
    
    #  Array which says which component to show depending on the
    #  type:
    
    variable shownPane -array [list              \
        XIA    xia                               \
        VMUSB  usb                               \
        CCUSB  usb                               \
        Custom custom                            \
    ]
    
    constructor args {
        installhull using panedwindow -orient vertical
        
        # The top pane is the common attributes:
        
        install common using rdo::CommonAttributes $win.common \
            -typeselectcommand [mymethod _displayDetailsPane]
        $hull add $common
        
        # The other panes are hidden initially until we figure out our
        # Type and then we unhide the appropriate pane:
        
        
        # XIA.
        
        install xia using rdo::XIAAttributes $win.xia
        $hull add $xia
        $hull paneconfigure $xia -hide 1
        
        # XXUSBB:
        
        install  usb using rdo::XXUSBAttributes $win.xxusb
        $hull add $usb
        $hull paneconfigure $usb -hide 1
        
        #  Custom:
        
        install custom using rdo::CustomAttributes $win.custom
        $hull add $custom
        $hull paneconfigure $custom -hide 1

        $self configurelist $args

        
        #  show the right pane:
        
        $self _displayDetailsPane
    }
    #--------------------------------------------------------------------------
    #  utilties
    
    ##
    # _displayDetailsPane
    #
    #   Pulls the current type of readout and displays its pane:
    #
    method _displayDetailsPane args {
        set rdoType [$common cget -type]
        set paneComponent $shownPane($rdoType)
        set paneWindow [set $paneComponent]
        
        foreach pane [array names shownPane] {
            set paneName $shownPane($pane)
            if {$paneName eq $paneComponent} {
                $hull paneconfigure $paneWindow -hide 0
            } else {
                set window [set $paneName]
                $hull paneconfigure $window -hide 1
            }
        }
    }
}
#-------------------------------------------------------------------------------

##
# @class rdo::ReadoutPrompter
#   Wrapt the readout wizard in action buttons:
#
snit::widgetadaptor rdo::ReadoutPrompter {
    option -okcommand     -default [list]
    option -cancelcommand -default [list]
    
    component form
    delegate method * to form
    delegate option * to form
    
    constructor args {
        installhull using ttk::frame
        
        install form using rdo::ReadoutWizard $win.form
        
        ttk::frame $win.action
        ttk::button $win.action.ok -text Ok \
            -command [mymethod _dispatch -okcommand]
        ttk::button $win.action.cancel -text Cancel \
            -command [mymethod _dispatch -cancelcommand]
        
        grid $win.action.ok $win.action.cancel
        grid $win.form -sticky nsew
        grid $win.action -sticky ew
        
        
        
        $self configurelist $args
    }
    #---------------------------------------------------------------------------
    # Event handling
    
    
    ##
    # _dispatch
    #   Dispatch to the script in the option passed in.
    #
    # @param name - name of the option holding the script.
    #
    method _dispatch name {
        set command $options($name)
        if {$command ne ""} {
            uplevel #0 $command
        }
    }
}
#------------------------------------------------------------------------------
#  The wizard itself.

##
# Usage
#   Provide program usage:
#  @param msg - error message.
#
proc usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "    mg_readout_wizard  config-database"
    puts stderr "Where:"
    puts stderr "   config-database - is a configuration database"
    exit -1
    
}
#----------------------------------------------------------------------------
#  Common attributes
#
#

##
# getCommonAttributes
#   Returns a dict of the common attributes
#
# @param widget - the wizard widget.
# @return a dict containing the following keys:
#   *  type       - type of readout.
#   *  name       - name of readout (used to derive program and squence defs).
#   *  container  - Name of container the software should run in.
#   *  host       - host in which the program should run.
#   *  directory  - Working directory in which the software should run.
#   *  sourceid   - Source id to emit.
#   *  ring       - output ring buffer.
#   *  service    - Rest service name.
#   *  manageruser - User running the manager.
#   
proc getCommonAttributes {widget} {
    dict create type [$widget cget -readouttype]    name [$widget cget -name] \
        container [$widget cget -container]  host [$widget cget -host] \
        directory [$widget cget -cwd]  sourceid [$widget cget -sourceid] \
        ring [$widget cget -ring]            service [$widget cget -restservice] \
        manageruser [$widget cget -manageruser]
}
##
# checkCommonMandatories
#   Returns a list of missing common fields (Hopefully empty).
#  
#
# @param atttributes
#
proc checkCommonMandatories {attributes} {
    set mandatory [list name host directory ring service manageruser]
    set missing [list]
    foreach key $mandatory {
        if {[dict get $attributes $key] eq ""} {
            lappend missing $key
        }
    }
    return $missing
}
#-----------------------------------------------------------------------------
#  Common utilities that span program types.
    
##
# makeRunControlProgram
#    Create a run control program.
#
# @param dbcmd - database command.
# @param name - program name.
# @param executable - executable path.
# @param params - Program parameterization (need some common elements).
# @Param args  - (optional) program after the host/user arguments.
#
proc makeRunControlProgram {dbcmd name executable params args} {

    set parameters [list [dict get $params host] [dict get $params manageruser] {*}$args]
    
    set env [list [list SERVICE_NAME [dict get $params service]]]


    
    program::add $dbcmd $name $executable Transitory [dict get $params host] \
        [dict create                           \
            parameters $parameters             \
            environment $env                   \
            container [dict get $params container] \
        ]
}

##
# ensureSequencesExist
#   If any sequences are missing, create them:
# @param db - database command.
#
proc ensureSequencesExist {db} {
    # List of sequences required and their triggers:
    
    set required [list  \
        [list bootreadouts BOOT]  \
        [list initreadouts HWINIT] \
        [list beginreadouts BEGIN] \
        [list endreadouts END] \
        [list shutdownreadouts SHUTDOWN] \
    ]
    set existing [sequence::listSequences $db]
    
    # Turn the list of existing sequences into a list of names we can lindex:
    #
    set existingnames [list]
    foreach seq $existing {
        lappend existingnames [dict get $seq name]
    }
    
    #  Now make the ones that don't yet 
    
    foreach seq $required {
        if {[lsearch -exact $existingnames [lindex $seq 0]] == -1} {
            sequence::add $db [lindex $seq 0] [lindex $seq 1]
        }
    }
}
##
# makeRunControlPrograms
#    Make all of the programs needed to control a readout over REST:
#   These are
#   *    name_settitle  - Sets the title.
#   *    name_setrun    - sets the run number.
#   *    name_begin     - Begin a run.
#   *    name_init      - initialize hardware.
#   *    name_end       - end a run.
#   *    name_shutdown  - push exit.
#
# @param dbcmd - The database command.
# @param name - the name of the readout program - will be used as the base
#               name for the programs made.
# @param parameters - the program parameterization.
#
proc makeRunControlPrograms {dbcmd name parameters} {

    set kvparams $parameters
    set readoutName ${name}_readout
    dict set kvparams name ${name}_readout
    makeRunControlProgram \
        $dbcmd ${name}_settitle [file join \$DAQBIN rdo_titleFromKv] $kvparams $readoutName
    makeRunControlProgram \
        $dbcmd ${name}_setrun [file join \$DAQBIN rdo_runFromKv] $kvparams $readoutName
    makeRunControlProgram \
        $dbcmd ${name}_beginrun [file join \$DAQBIN rdo_control] $parameters begin
    makeRunControlProgram \
        $dbcmd ${name}_init [file join \$DAQBIN rdo_control] $parameters init
    makeRunControlProgram \
        $dbcmd ${name}_endrun [file join \$DAQBIN rdo_control] $parameters end
    makeRunControlProgram \
        $dbcmd ${name}_shutdown [file join \$DAQBIN rdo_control] $parameters shutdown
}

##
# addProgramsToSequences
#  Adds the programs needed to do a Readout to the appropriate sequences.
#  The sequences are assumed to already exist.
#   @param db - database command.
#   @param name - Readout name (program base name).
#
proc addProgramsToSequences {db name} {
    # List of suffixes/sequences - in order of add
    
    set suffixes [list                          \
        [list _readout bootreadouts]            \
        [list _init     initreadouts]           \
        [list _setrun   beginreadouts]          \
        [list _settitle beginreadouts]          \
        [list _beginrun beginreadouts]          \
        [list _endrun   endreadouts]            \
        [list _shutdown shutdownreadouts]      \
    ]
    foreach program $suffixes {
        set fullname ${name}[lindex $program 0]
        sequence::addStep $db [lindex $program 1] $fullname
    }
}
#-----------------------------------------------------------------------------
#  Build XIA readout.


##
# getXIAAttributes
#   Given the wizard widget produce a dict of the XIA specific attributes.
#
# @param widget - the wizard widget.
# @return dict containing the keys:
#   *   sorthost - the host in which the DDASSort program runs.
#   *   sortring - the ring to which DDASSort writes.
#   *   sortwindow - The sliding time window over which the sort is done.
#   *   fifothreshold  - the FIFO threshold used by Readout to trigger readout.
#   *   buffersize - The size of the readout buffer.
#   *   infinityclock - (bool) true if infinity clock is enabled.
#   *   clockmultiplier - Clock multiplication factor used in external clock mode.
#   *   scalerperiod - Scaler readout period.
#
proc getXIAAttributes {widget} {
    set result [dict create]
    foreach \
        opt [list -sorthost -sortring -sortwindow -fifothreshold -buffersize\
               -infinityclock -clockmultiplier -scalerperiod]    \
        key [list sorthost sortring sortwindow fifothreshold buffersize \
             infinityclock clockmultiplier scalerperiod] {
        dict set result $key [$widget cget $opt]
    }
    return $result
    
}

##
# checkXIAAttributes
#    Return a list of any mandatory XIA attributes that are missing:
#
# @param params - dict of parameters (see getXIAAttributes)
# @return list  - List of misssing mandatory fields.
# @note we've cleverly chosen our dict keys so that the mandatory ones
#      match the field names that can be highlighted.
#
proc checkXIAAttributes {params} {
    set mandatory [list sorthost sortring sortwindow fifothreshold buffersize]
    set result [list]
    
    foreach field $mandatory {
        if {[dict get $params $field] eq ""} {
            lappend result $field
        }
    }
    
    return $result
}
##
#  makeXIAReadoutOptions
#   Given the parameterization creates the list of options that will be passed
#   to ddasReadout.
# @param params  - program parameterization.
# @return list - of option/value pairs.  Note that simple flags are singlets.
#
proc makeXIAReadoutOptions {params} {
    set result [list]
    
    foreach \
        key [list host sourceid ring sorthost sortring sortwindow fifothreshold \
             buffersize  clockmultiplier scalerperiod directory]   \
        optname [list -readouthost -sourceid -readoutring -sorthost -sortring \
                 -window -fifothreshold -buffersize -clockmultiplier -scalerseconds -cratedir] {
        lappend result [list $optname [dict get $params $key]]
    }
    if {[dict get $params infinityclock]} {
        lappend result [list -infinity on]
    }
    lappend result \
        [list -init-script [file join \$DAQSHARE scripts rest_init_script.tcl]]
    
    return $result  
}
##
# makeXIAEnvironment
#    Return environment variables needed for XIA reaodouts.
#  @param params  - parameter dict.
#  @return list - pairs of name/value.  We create:
#     *    RDOREST_KEEPSTDIN = 1 so that ddasReadout doesn't think we're going to exit.
#     *    SERVICE_NAME = params[service]
#
proc makeXIAEnvironment {params} {
    return [list                                                \
        [list RDOREST_KEEPSTDIN 1]                              \
        [list SERVICE_NAME [dict get $params service]]          \
    ]       
}


##
# makeXIAReadout
#    Do what's needed to make an XIA readout.  For the most part
#    this means gathering the XIA specific commands,
#     forming the readout information.
#     Creating the appropriate program and squence entries.
#
# @param widget - the wizard form.
# @param commonAttributes - attributes from the common section,.
# @param dbcmd - database command.
# @return int - 1 - failure retry 0 done.
proc makeXIAReadout {widget commonAttributes dbcmd} {
    set xiaattributes [getXIAAttributes $widget]
    set missing [checkXIAAttributes  $xiaattributes]
    if {[llength $missing] != 0} {
        foreach field $missing {
            $widget highlightXIAField $field
        }
        tk_messageBox -icon error -type ok -title "Missing XIA" \
            -message "The following mandatory XIA specific parameters are missing: $missing"
        return 1;                       #Let the user fix this.
    }
    set parameters [dict merge $commonAttributes $xiaattributes]
    
    # Make the main program
    
    set exe [file join {$DAQBIN} ddasReadout]
    set opts [makeXIAReadoutOptions $parameters]
    set env  [makeXIAEnvironment $parameters]
    set name [dict get $parameters name]_readout
    program::add $dbcmd $name $exe Critical [dict get $parameters host] \
        [dict create                                                 \
            options $opts                                            \
            directory [dict get $parameters directory]              \
            container [dict get $parameters container]              \
            service   [dict get $parameters service]                \
            environment $env
        ]

    # Make the ancillary programs.
    
    set name [dict get $parameters name]
    makeRunControlPrograms $dbcmd $name $parameters
    
    # Add the programs to the sequences.
    
    addProgramsToSequences $dbcmd $name
    
    return 0
}
#------------------------------------------------------------------------------
#  Build XXUSB Readouts


##
# getUSBAttributes
#   Fetch the XXUSBReadout parameterization out of the USB part of the
#   wizard widget:
#
# @param widget - wizard form object.
# @return dict containing the following keys:
#   *   byserial - boolean true if a specific serial string is used to locate the XXUSB.
#   *   serialstring - the serial string to use if byserial
#   *   daqconfig    - The DAQ config file path.
#   *   usectlserver - True if the control server will be used.
#   *   ctlconfig    - path to the control configuration file.
#   *   ctlport      - Control server port.
#   *   enablelogging - True if logging is enabled.
#   *   logfile       - path to logfile.
#   *   logverbosity  - Log file verbosity.
#   *   usetimestamps - A timestamp extraction library will be provided.
#   *   tslib         - The timestamp library.
#
proc getUSBAttributes {widget} {
    set result [dict create]
    foreach \
    key [list byserial serialstring daqconfig usectlserver ctlconfig ctlport \
         enablelogging logfile logverbosity usetimestamps tslib] \
    opt [list -byserial -serialstring -daqconfig -usectlserver -ctlconfig \
         -ctlport -enablelogging -logfile -logverbosity  \
         -extracttimestamps -timestamplib] {
        dict set result $key [$widget cget $opt]
    }
    return $result
}
##
#  checkUSBAttributes
#  Check the USB attributes to see if all of the necessary bits and pieces are
#  nonempty.  Note that in the case of XXUSBreadout, there are some fields that
#  are conditionally mandatory, for exampl, the serialstring field is only
#  mandatory if byserial is true.
#
# @param attributes - the USB attributes dictionary.
# @return list      - list of the missing fields.
#
proc checkUSBAttributes {attributes} {
    set unconditionals [list daqconfig]
    set serial         [list serialstring]
    set ctlserver      [list ctlconfig ctlport]
    set logging        [list logfile]
    
    # Now compute that actual required strings:
    
    set mandatory $unconditionals
    if {[dict get $attributes byserial]} {
        set mandatory [concat $mandatory $serial]
    }
    if {[dict get $attributes usectlserver]} {
        set mandatory [concat $mandatory $ctlserver]
    }
    if {[dict get $attributes enablelogging]} {
        set mandatory [concat $mandatory $logging]
    }
    if {[dict get $attributes usetimestamps]} {
        lappend mandatory tslib
    }
    set result [list]
    
    foreach field $mandatory {
        if {[dict get $attributes $field] eq ""} {
            lappend result $field
        }
    }
    
    return $result
}
##
# makeXXUSBReadoutOptions
#    Given the program parameterization, make the options that will be passsed
#    to the XXUSBReadout program.
#  @param params - the parameterization of the readout from the wizard.
#  @return list  - List of options.  If the list entry is a 2 element sublist it's
#                   [list option value] if a single element the optin is a flag.
#
proc makeXXUSBReadoutOptions {params} {
    set result [list]
    
    # Many parameters have direct mappings to options:
    # This list is key/value of those that do:
    # These are al lmandatory options.
    
    set optionMapping [list                         \
       [list  ring     --ring]                      \
       [list  daqconfig --daqconfig]                \
    ]
    foreach opt $optionMapping {
        lappend result [list [lindex $opt 1] [dict get $params [lindex $opt 0]]]
    }
    #   set the init script to enable the REST server:
    
    lappend result \
        [list --init-script [file join {$DAQSHARE} scripts rest_init_script.tcl]]
    
    #  some options are conditional on the true values of a parameter:
    
    if {[dict get $params byserial]} {
        lappend result [list --serialno [dict get $params serialstring]]
    }
    
    if  {[dict get $params usectlserver]} {
        lappend result [list --ctlconfig [dict get $params ctlconfig]]
        lappend result [list --port [dict get $params ctlport]]
    } else {
        lappend result [list --ctlconfig /dev/null]
    }
    
    if {[dict get $params enablelogging]} {
        lappend result [list --log [dict get $params logfile]]
        lappend result [list --debug [dict get $params logverbosity]]
    }
    
    if {[dict get $params usetimestamps]}  {
        lappend result [list --timestamplib [dict get $params tslib]]
    }
    
    return $result
}


##
# makeUSBReadout
#   The only difference between CCUSB and VMUSB readouts is the actual
#   program name. so that gets passed in:
#
# @param widget          - the wizard form widget.
# @param commonAttributes- the common attributes.
# @param program         -  The program (assumed in $DAQBIN)
# @param db              - databvase command
# @return int 1 - if need to re-prompt, 0 if done.
#
proc makeUSBReadout {widget commonAttributes program dbcmd} {
    set usbAttributes [getUSBAttributes $widget]
    set missing [checkUSBAttributes $usbAttributes]
    if {[llength $missing] > 0} {
        foreach field $missing {
            $widget highlightUSBField $field
        }
        tk_messageBox -icon error -type ok -title "Mixxing USB" \
            -message "The following mandatory XXUSB parameters are missing: $missing"
        return 1
    }
    set parameters [dict merge $commonAttributes $usbAttributes]
    
    set name [dict get $parameters name]
    
    #  Create the readout  program
    
    set exe [file join {$DAQBIN} $program]
    set opts [makeXXUSBReadoutOptions $parameters]
    
    program::add $dbcmd ${name}_readout $exe Critical [dict get $parameters host] \
        [dict create                                                \
            options $opts                                           \
            directory [dict get $parameters directory]              \
            container [dict get $parameters container]               \
            service   [dict get $parameters service]                \
            environment [list [list SERVICE_NAME [dict get $parameters service]]] \
        ]
    #  Make the ancillary programs:
    
    makeRunControlPrograms $dbcmd $name $parameters
    
    # and add all to appropriate sequences:
    
    addProgramsToSequences $dbcmd $name
    
    
    return 0
}
#------------------------------------------------------------------------------
#  Custom readouts:

##
# getCustomAttributes
#   Return a dict with the custom attributes:
# @param widget - the wizard widget.
# @return dict with the keys:
#  *    program - path to the Readout program.
#  *    options - Command line options as a list of option or option/value pairs.
#  *    environment - environment name or name/value pairs.
#  *    parameters - list of command parameters
#
proc getCustomAttributes {widget} {
    set result [dict create]
    
    foreach \
        key [list program options environment parameters] \
        opt [list -program -options -environment -parameters] {
        dict set result $key [$widget cget $opt]
    }
    
    return $result
}  
##
# checkCustomAttibutes
#   @param attributes - the readout attributes.
#   @return list - of missing required attributes.
#
proc checkCustomAttributes {attributes} {
    set required [list program]
    set result [list]
    
    foreach key $required {
        if {[dict get $attributes $key] eq ""} {
            lappend result $key
        }
    }
    
    return $result
}
##
# makeCustomOptions
#   These are just the options in the parameters withan --init-script
#   --sourceid and --ring added from the common options.
#
# @param parameters - the parameterization dict.
# @return list - list of optinos and their values if they have them.
#
proc makeCustomOptions parameters {
    set result [dict get $parameters options]
    
    lappend result [list --sourceid [dict get $parameters sourceid]]
    lappend result [list --ring [dict get $parameters ring]]
    lappend result [list --init-script [file join {$DAQSHARE} scripts rest_init_script.tcl]]
    
    return $result                
}
##
# makeCustomParams
#   These are just the parameters key of the dict.
#
# @param parameters - the parameteization of the program
# @return list (possibly empty ) of parameters.
#
proc makeCustomParams {parameters} {
    return [dict get $parameters parameters]
}
##
# makeCustomEnv
#   This is the value  of the environment key of the dict with
#   SERVICE_NAME appended to it.
# @param parameters - program parameterization.
# @return list of env/values.
#
proc makeCustomEnv {parameters} {
    set  result [dict get $parameters environment]
    lappend result [list SERVICE_NAME [dict get $parameters service]]
    return $result
}

##
# makeCustomReadout
#   Pull the parameters for a custom readout and
#   make that into a program + ancillary stuff.
#
# @param widget - the wizard form.
# @param commonAttributes - the common attributes.
# @param database command
# @return int - 0 all ok 1 retry form entry.
proc makeCustomReadout {widget commonAttributes dbcmd} {
    set customAttributes [getCustomAttributes $widget]
    set missing [checkCustomAttributes $customAttributes]
    
    if {[llength $missing] > 0} {
        foreach field $missing {
            $widget highlightCustomField $field
        }
        tk_messageBox -icon error -type ok -title "Missing custom fields" \
            -message "Required custom fields missing: $missing"
        return 1
    }
    
    set parameters [dict merge $commonAttributes $customAttributes]
    
    # We make some assumptions about custom readouts:
    #  - they are based on a Tcl interpreter/
    #  - They support a --sourceid option.
    #  - They support a --ring option
    #  - They support an --init-script options.
    #  - They support the command set and eventloop that allow the --init-script
    #    to run the TCLHTTPD server to provide the REST interface.
    #
    #  If your readout does not meet these pre-requisites, then
    #  you'll most likely need your own run control bits and will have to
    #  construct this thing using individual programs and sequence entries anyway.
    #
    
    set exe [dict get $parameters program]
    set name [dict get $parameters name]
    set opts [makeCustomOptions $parameters]
    set params [makeCustomParams $parameters]
    set env [makeCustomEnv $parameters]
    
    program::add $dbcmd ${name}_readout $exe Critical [dict get $parameters host] \
        [dict create                                           \
            options $opts  environment $env parameters $params  \
            directory [dict get $parameters directory]        \
            container [dict get $parameters container]         \
            service   [dict get $parameters service]           \
        ]
    
    makeRunControlPrograms $dbcmd $name $parameters
    addProgramsToSequences $dbcmd $name
    
    return 0
}
    



#------------------------------------------------------------------------------
#  Action handlers:
#

##
# makeReadout
#   Ok was plinked - so create the new readout and its sattelite programs.
#
#  @param form - the formt that describes the Readout.
# @param  db   - Database command that accesses the configuration.
#
#   On success, we're going to exit. On handled errors, we'll be
#   leaving the form up for the user to try again.
#
proc makeReadout {form dbcmd} {
    
    # Reset any highlighted field labels back to normal and let validation
    # re-highlight any appropriate ones.
        
    $form resetCommonHighlights
    $form resetXIAHighlights
    $form resetUSBHighlights
    $form resetCustomHighlights
    
    #  The creation of the Readout results in:
    #  The creation of the specific program itself
    #  The creation of program helpers that need to be run on state changes.
    #  The creation and stockage of statechange triggered sequences:
    
    
    #  Get the common attributes and highlight any missing ones with an error.
    
    set attributes [getCommonAttributes $form]
    set missingCommonMandatories [checkCommonMandatories $attributes]
    if {[llength $missingCommonMandatories] != 0} {
        foreach f $missingCommonMandatories {
            $form highlightCommonField $f
        }
        tk_messageBox -icon error -type ok -title {Missing common} \
            -message "The following mandatory fields are empty:  $missingCommonMandatories"
        
        return
    }
    #  What we do next depends on the readout type...at least we know that
    #  the required common parameters are present.
    
    set rdotype [dict get $attributes type]
    if {$rdotype eq "XIA"} {
        if {[makeXIAReadout $form $attributes  $dbcmd]}  {
            return
        }
    } elseif {$rdotype eq "VMUSB"} {
        if {[makeUSBReadout $form $attributes VMUSBReadout $dbcmd]} {
            return
        }
        
    } elseif {$rdotype eq "CCUSB"} {
        if {[makeUSBReadout $form $attributes CCUSBReadout $dbcmd]} {
            return
        }
        
    } elseif {$rdotype eq "Custom"} {
        if {[makeCustomReadout $form $attributes $dbcmd]} {
            return
        }
    } else {
        tk_messageBox -icon error -type ok -title {Bad readout type} \
            -message "Invalid readout type: $rdotype"
        return;                    # Maybe given another chance???
    }
    
    exit 0
}

######## ENTRY
##
# Ensure we have a database file and open it:

# Support incremental/interactive testing;
# If you want to incrementally/interactively test the widgets, fire up
#  tclsh or wish,
#  %  set norun 1;   # or anything
#  %  source mg_readout_wizard.tcl
#
# and the main program won't run so you can test.
    


if {[info globals norun] eq ""} {

if {[llength $argv] != 1} {
    usage {Incorrect number of command line parameters}
}

sqlite3 db $argv

#  If necessary make the sequences

ensureSequencesExist db

rdo::ReadoutPrompter .prompt  -okcommand [list makeReadout .prompt db] -cancelcommand exit
pack .prompt

# Stock the containers list

set containerDefs [container::listDefinitions db]
set containers [list]
foreach c $containerDefs {
    lappend containers [dict get $c name]
}
.prompt configure -containers $containers

}
