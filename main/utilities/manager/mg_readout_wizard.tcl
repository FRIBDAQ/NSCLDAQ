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

##
# Provides a wizard that creates all of the stuff to make a readout program work.
# This is not only the Readout program itself but the REST clients that
# are needed to start it.  Each Readout program named xxx
# will create the following programs.
#
#  -  xxx the readout itself.
#  -  xxx_init - initializes the hardware.
#  -  xxx_setrun - sets the run number from the KV store.
#  -  xxx_settitle - sets the title from the KV Store.
#  -  xxx_beginrun - Starts the run.
#  -  xxx_endrun   - Ends the run.
#  -  xxx_shutdown - Shuts the readout down.
#
#  If the following sequences don't exist they are created
#
#   -  bootreadout - triggered by the BOOT transition and xxx is added to that sequence.
#   -  initreadout - triggered by the  HWINIT transition and xxx_init is added to that sequence.
#   -  beginreadout- triggered by the BEGIN transition, xxx_setrun, xxx_settitle
#                     and xxx_beginrun are added to that sequence.
#   -  endreadout  - Triggered by the END transtion, xxx_endrun is added to it.
#   -  shutdownreadout - Triggered by SHUTDOWN< xxx_shutdown is added to it.
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
#     Any options the user wants to add.
#
#   Environment variables the program will have... in addition to those set up by
#   the container start script.
#
#  Note that prior to DDAS readout prior to V12 should use customized and select
#  $DAQBIN/DDASReadout configuring it manually using environment variables etc.
#


 #==============================================================================
 #     The Main GUI has several components:
 
 ##
 # @class rdo::CommonAttributes
 #
 #  Form to allow the input of common attributes;
 #
 # *   - -type -  one of: {ddas, vmusb, ccusb, custom}
 # *   - -container  If not an empty string the container that will run the readout.
 # *   - -host    Host in which the Readout will run.
 # *   - -directory - working directory in which the Readout will run.
 # *   - -sourceid - Event builder source id.
 # *   - -ring  - output ring buffer.
 # *   - -service - REST service name used to control the Readout.
 #
 #  Other OPTIONS:
 #    -containers - containers to select between.
 #    -typeselectcommand - script when the radio buttons holding the Readout Type
 #        change.
 #
 snit::widgetadaptor rdo::CommonAttributes {
    option -type
    option -containers  -default [list] -configuremethod _configContainers;       # Containers to chose between
    option -container;        # Selected container.
    option -host
    option -directory
    option -sourceid -default 0
    option -ring -default $::tcl_platform(user)
    option -service -default ReadoutREST
    option -typeselectcommand -default [list]
    
    variable daqtypes [list XIA VMUSB CCUSB Custom]
    
    constructor args {
        installhull using ttk::frame
        
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
        
        # directory, sourceid
        
        ttk::labelframe $win.dir -text {Directory}
        ttk::entry $win.dir.dir -textvariable [myvar options(-directory)]
        ttk::button $win.dir.browse -text {Browse...} -command [mymethod _browsedir]
        grid $win.dir.dir $win.dir.browse -sticky ew
    
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
        
        ttk::labelframe $win.service -text {REST service name}
        ttk::entry $win.service.name -textvariable [myvar options(-service)]
        ttk::label $win.service.label -text {Service}
        grid $win.service.name $win.service.label -sticky ew
        
        #  Grid the top level frames:
        
        grid $win.types $win.container -sticky ew
        grid $win.dir $win.sid -sticky ew
        grid $win.ring $win.service -sticky ew
        
         
        $self configurelist $args 
        
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
    
    constructor args {
        installhull using ttk::frame
        #  Title:
        
        ttk::label $win.title -text {XIA Readout Attributes}
        
        #  Sort host entry and label:
        
        ttk::labelframe $win.host -text {Sort host}
        ttk::entry $win.host.host -textvariable [myvar options(-sorthost)]
        ttk::label $win.host.label -text {Sort Host}
        grid $win.host.host $win.host.label -sticky ew
        
        # Sort ring
        
        ttk::labelframe $win.ring -text {Sort output ring}
        ttk::entry $win.ring.ring -textvariable [myvar options(-sortring)]
        ttk::label $win.ring.label -text {Output Ring}
        grid $win.ring.ring $win.ring.label -sticky ew
        
        #  sort window
        
        ttk::labelframe $win.sortwin -text {Sort window}
        ttk::spinbox $win.sortwin.window \
            -from 1 -to 1000 -increment 1 \
            -textvariable [myvar options(-sortwindow)]
        ttk::label $win.sortwin.label -text {Seconds}
        grid $win.sortwin.window $win.sortwin.label -sticky ew
        
        #  FIFO Threshold
        
        ttk::labelframe $win.fifo -text {Readout Threshold}
        ttk::spinbox $win.fifo.fifo \
            -from 1024 -to [expr 128*1024] -increment 512 \
            -textvariable [myvar options(-fifothreshold)]
        ttk::label $win.fifo.label -text {FIFO Threshold}
        grid $win.fifo.fifo $win.fifo.label -sticky ew
 
        # Buffer size
        
        ttk::labelframe $win.bsize -text {Buffer Size}
        ttk::spinbox    $win.bsize.bsize -from 8192 -to [expr 1024*1024] -increment 1024 \
            -textvariable [myvar options(-buffersize)]
        ttk::label $win.bsize.label -text {Readout Buffersize}
        grid $win.bsize.bsize $win.bsize.label -sticky ew
        
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
#
snit::widgetadaptor rdo::XXUSBAttributes {
    option -byserial -default 0
    option -serialstring
    
    option -daqconfig
    
    option -usectlserver -default 0 -configuremethod _setCtlServerVisibility
    option -ctlconfig
    option -port -default 1024
    
    option -enablelogging -default 0
    option -logfile
    option -logverbosity -default 0
    
    constructor args {
        installhull using ttk::frame
        ttk::label $win.title -text {XXUSBReadout attributes}
        
        # DAQ Config file.
        
        ttk::labelframe $win.daqconfig -text {DAQ configuration}
        ttk::entry $win.daqconfig.config -textvariable [myvar options(-daqconfig)]
        ttk::button $win.daqconfig.browse -text {Browse...} -command [mymethod _browseDAQFile]
        grid $win.daqconfig.config $win.daqconfig.browse -sticky ew
        
        # Control configuration :
        
        ttk::labelframe $win.ctlconfig -text {Control Configuration}
        ttk::checkbutton $win.ctlconfig.enable -text {Use Control Server} \
            -onvalue 1 -offvalue 0 -variable [myvar options(-usectlserver)] \
            -command [mymethod _controlOptions]
        ttk::entry $win.ctlconfig.ctlconfig \
            -textvariable [myvar options(-ctlconfig)] 
        ttk::button $win.ctlconfig.browse \
            -text Browse... -command [mymethod _browseCtlConfig]
        ttk::spinbox $win.ctlconfig.port -textvariable [myvar options(-port)] \
            -from 1024 -to 65535 -increment 1
        ttk::label $win.ctlconfig.plabel -text {Server Port}
        grid $win.ctlconfig.enable -sticky w
        grid $win.ctlconfig.ctlconfig $win.ctlconfig.browse $win.ctlconfig.port $win.ctlconfig.plabel -sticky ew
        
        grid $win.title     -sticky w
        grid $win.daqconfig -sticky ew
        grid $win.ctlconfig -sticky ew
        
        
        $self configurelist $args
        $self _controlOptions
    }
    #---------------------------------------------------------------------------
    # Configuration handling.
    
    
    
    ##
    #  _useCtlServer
    #    Configure the flag that uses/does not use control server,
    #    just set the options array and invoke _controlOptions.
    #
    method _useCtlServer {name value} {
        set options($name) $value
        
        $self _controlOptions
    }
    #---------------------------------------------------------------------------
    # Event handlers:
    
    ##
    #  _browseDAQfile
    #    Select a daqconfig file:
    #
    method _browseDAQFile {} {
        set file [tk_getOpenFile -parent $win -title "DAQ config file" \
            -filetypes [list                                           \
                    {{Tcl Scripts} .tcl}                               \
                    {{All Files}    *}                                  \
        ]]
        if {$file ne ""} {
            $self configure -daqconfig $file
        }
    }
    ##
    # _browseCtlConfig
    #   Browse for the control config file
    #
    method _browseCtlConfig {} {
        set file [tk_getOpenFile -parent $win -title "DAQ config file" \
            -filetypes [list                                           \
                    {{Tcl Scripts} .tcl}                               \
                    {{All Files}    *}                                  \
        ]]
        if {$file ne ""} {
            $self configure -ctlconfig $file
        }
    }
    
    ##
    # _coontrolOptions
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
}

