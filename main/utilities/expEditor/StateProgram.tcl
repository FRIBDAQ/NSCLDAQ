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
# @file StateProgram.tcl
# @brief Encapsulate state program data and a GUI element.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide stateProgram 1.1
package require snit
package require Tk
package require stateProgramData
package require daqObject
package require img::png
package require Label

##
# @class StateProgram
#   Wraps a DaqObject that has the icon for a state program with a
#   StateProgramdata object that has the data.   Appropriate command
#   and option forwarding are done to make this all hang together.
#

snit::type StateProgram {
    component data
    component gui
    component Label
 
    delegate option -provider  to data
    delegate option -changecmd to data
    
    delegate option -canvas   to gui
    
    # Expose all but clone (which we have to handle)
    # to the world:
    
    delegate method getProperties to data
    delegate method addSink       to data
    delegate method clearSinks    to data
    delegate method rmSink        to data
    delegate method getSinks      to data
    
    delegate method addtag        to gui
    delegate method rmtag         to gui
    delegate method tags          to gui
    delegate method getPosition   to gui
    delegate method getId         to gui
    delegate method size          to gui
    delegate method bind          to gui

    variable inRingObj ""
    variable outRingObj ""
    
    ##
    # typeconstructor
    #   Create the image that will be used as the object's icon:
    
    typeconstructor {
        image create photo StateProgramIcon -format png \
            -file [file join [file dirname [info script]] program.png] 
    }
    
    ##
    # constructor
    #   Construct the object. This just means constructing the components.
    #
    constructor args {
        install data using StateProgramData %AUTO%
        install gui  using DaqObject %AUTO% -image StateProgramIcon
        
        $self configurelist $args
    }
    
    ##
    # destructor
    #
    destructor {
        if {$data ne ""} {
            $data destroy
        }
        if {$gui ne ""} {
            $gui  destroy
        }
        if {$Label ne ""} {
            $Label destroy
        }
    }
    #---------------------------------------------------------------------------
    # Private methods
    
    ##
    # _replaceData
    #   Used in a clone operation to replace the data with the data to be copied.
    #
    # @param new data object.
    #
    method _replaceData newData {
        $data destroy
        set data $newData
    }
    ##
    # _getProperty
    #   Return the value of a named property from an object that supports
    #   the property list API.
    #
    # @param propName  - Name of the properties.
    #
    method _getProperty {object propName} {
        set props [$object getProperties]
        set prop  [$props find $propName]
        return [$prop cget -value]
        
    }
 
    ##
    # _setProperty
    #   sets a named propert on an object.
    #
    # @param object   - object to modify.
    # @param propName - Name of the property.
    # @param value    - new property vale.
    #
    method _setProperty {object propName value} {
        set props [$object getProperties]
        set prop  [$props find $propName]
        $prop configure -value $value
        
    }
    ##
    # _getUri
    #    Return the URI associated with a ring buffer object.
    #
    # @param ring - the ring buffer object.
    #
    method _getUri ring {
        set host     [$self _getProperty $ring host]
        set ringName [$self _getProperty $ring name]
        
        return tcp://$host/$ringName
    }
    ##
    # _getName
    #   Return the name of a ringbuffer.
    #
    # @param ring - ring buffer object.
    #
    method _getName ring {
        return [$self _getProperty $ring name]
        
    }
    ##
    # _setOutRingInfo
    #   Set information about our output ring from an output ring item
    #
    # @param ring - the ring attached to us as an output ring.
    # @note  This program must run in the same host as the output ring.
    #        We'll remind the user of this fact if our host and the ring's
    #        host differ, and set our host to the ring's host.
    #
    method _setOutRingInfo {ring} {
        $self _setProperty $self {Output Ring} [$self _getName $ring]
        
        set myHost [$self _getProperty $self host]
        set ringHost [$self _getProperty $ring host]
        
        if {$myHost ne $ringHost} {
            set myName [$self _getProperty $self name]
            tk_messageBox \
                -title "Host mismatch" -type ok -icon info \
                -message "State programs must live in the same host as their output rings.  \
Therefore the host that '$myName' runs in is being changed to '$ringHost'"
            $self _setProperty $self host $ringHost
            
        }
    }
    ##
    # _setInRingInfo
    #   Set information about our input ring from an input ring item:
    #
    # @param ring - the ring attached to us as an input ring.
    #
    method _setInRingInfo {ring} {
        $self _setProperty $self {Input Ring} [$self _getUri $ring]
    }
    ##
    # _disableHostProperty
    #
    #   Turn off editing of the host property.  This is done when we have an
    #   output ring connection.  In that case the output ring forces us ot live
    #   in the same ost as that ring due to the way in which the API for
    #   producers works.
    #
    method _disableHostProperty {} {
        set props [$data getProperties]
        set prop  [$props find "host"]
        $prop configure -editable 0
    }
    ##
    # _enableHostProperty
    #
    #   Allow editing of the host property.  Ths is required when the object doe
    #   not have an output ring.  In that case there's no restriction on
    #   which host can run the program.
    #
    method _enableHostProperty {} {
        set props [$data getProperties]
        set prop  [$props find "host"]
        $prop configure -editable 1
    }
    #---------------------------------------------------------------------------
    # Public methods
    
    ##
    # clone
    #   Create/return a copy of self
    #
    method clone {} {
        set newObject [StateProgram %AUTO%]
        $newObject _replaceData [$data clone]
        
        return $newObject
    }
    ##
    # type
    #   Return the object type.
    #
    method type {} {
        return state-program
    }
    ##
    # connect
    #   Called when the program is connected to something.
    #
    #  @param direction - from if we are the source of the connection and to if
    #                     we are the sink end of the connection.
    #  @param object    - Who we are being connected to.
    #
    method connect {direction object} {
        if {[$object type] ne "ring"} {
            error "State programs can only be connected to rings."
        }
        if {$direction eq "from"} {
            $self _setOutRingInfo $object
            set outRingObj $object
            
            $self _disableHostProperty
            
        } elseif {$direction eq "to" } {
            $self _setInRingInfo $object
            set inRingObj $object
        } else {
            error "Invalid direction on connect"
        }
        return 1

    }
    ##
    # disconnect
    #   Called when the program is disconnected from an object
    #
    #  @param object
    #
    method disconnect object {
        
        set objUri  [$self _getUri $object]
        set objName [$self _getName $object]
        
        set sink [$self _getProperty $self {Output Ring}]
        set source [$self _getProperty $self {Input Ring}]
        
        # Take appropriate action depending on whether this is the from or to obj.
        
        if {$object eq $outRingObj} {
            $self _setProperty $self {Output Ring} {}
            set outRingObj ""
            
            $self _enableHostProperty
            
        } elseif {$object eq $inRingObj} {
            $self _setProperty $self {Input Ring} {}
            set inRingObj ""
        } else {
            error "$objName is not connected to [$self _getProperty $self name]"
        }
    }
    ##
    # isConnectable
    #   Determine if the object can accept the specified connection type:
    #
    # @param direction - from or to indicating whether we are a source or sink.
    # @return boolean true if the requested connection direction is acceptable.
    #
    method isConnectable direction {
        if {$direction eq "from" } {
            set obj $outRingObj
        } elseif {$direction eq "to"} {
            set obj $inRingObj
        } else {
            error "Ivalid connection direction: $direction"
        }
        # Connections are allowed if there is no current connection in that
        # direction:
        
        return [expr {$obj eq ""}]
    }
    ##
    # connectionPropertyChanged
    #   Called when the properties of an object we are connected to changed.
    #   In our case this allows us to update the information for our input
    #   and output rings (those are the only things we can have connections
    #   to).
    #
    # @param obj - the object whose properties changed.
    #
    method connectionPropertyChanged obj {
        if {$obj eq $inRingObj} {
            $self _setInRingInfo $obj
        } elseif {$obj eq $outRingObj} {
            $self _setOutRingInfo $obj
        } else {
            error "connectionPropertyChanged - $obj is not connected to us"
        }
    }
    ##
    # propertyChanged
    #   The label may have changed
    #   If the name or host are not "" we'll set the label to
    #   name@host else we'll set it to "".
    #
    method propertyChanged {} {
        set properties [$data getProperties]
        set nameProp [$properties find name]
        set hostProp [$properties find host]
        
        set name [$nameProp cget -value]
        set host [$hostProp cget -value]
        
        if {($name ne "") || ($host ne "") } {
            $Label configure -text $name@$host
        } else  {
            $Label configure -text ""
        }
    }
    
    ##
    # drawat
    #    This draws the GUI at a specified location.
    #    - Tell the GUI to run its drawat.. this may be what makes the image
    #      on the canvas the first time
    #    - If we've not yet installed the label object install it
    #    - Invoke propertyChanged to set the label's text.
    #
    # @param x    - Desired x postion of the object.
    # @param y    - Desired y position of the object.
    #
    method drawat {x y} {
        $gui drawat $x $y
        if {$Label eq ""} {
            install Label using Label %AUTO%                      \
                -canvas [$gui cget -canvas] -tag $self -id [$gui getId]
            $self propertyChanged
        } else {
            $Label move $x $y
        }
    }
    ##
    #  movto
    #   Move the GUI to the specified coordinates and drag the
    #   label along with it:
    #
    # @param x,y  - new position for the object.
    #
    method moveto {x y} {

        $gui moveto $x $y
        $Label move $x $y
    }
    ##
    # moveby
    #   Move the object and label by dx,dy.  Note that we had hoped to just move
    #   the tag but cant' because the gui object maintains inner stat that tells
    #   it where it is.
    #
    # @param dx,dy - amount to move the object in x,y
    #
    method moveby {dx dy} {
        $gui moveby $dx $dy
        $options(-canvas) move $self $dx $dy;        # Moves the label.
    }
    ##
    # gui
    #   @return the graphical representation object.
    #
    method gui {} {
        return $gui
    }
    ##
    # data
    #  @param[in] newData (optional) - if present, replaces the data object.
    #  @return object - the data object prior to the data call (e.g. if
    #                   the newData parameter is supplied, you'll get the
    #                   *previous* data).
    #
    method data {{newData ""}} {
        set oldData $data
        if {$newData ne ""} {
            install data using set newData
        }
        return $oldData
    }
    ##
    # setLabelText
    #    Sets the label text.  If the Label component has not yet been
    #    installed, this is an error.
    # @param str - new label text string.
    #
    method setLabelText str {
        if {$Label eq ""} {
            error "StateProgram::setLabelText - Label component has not yet been installed."
        } else {
            $Label configure -text $str
        }
    }
}
##
# @class ReadoutObject
#     Bundle of A readout program (data) and the display stuff associated with it.
#     Note that this is just a StateProgram with:
#     *  data replaced by a ReadoutProgram.
#     *  the graphical representation replaced by readout.png.
#
snit::type ReadoutObject {
    component StateObject
    
    delegate option * to StateObject
    delegate method * to StateObject
    
    typevariable icon
    
    ##
    # typeconstructor
    #   Create and save (in icon) the graphical representation of the object.
    #
    typeconstructor {
        set here [file dirname [info script]]
        set icon [image create photo -format png -file [file join $here readout.png]]
    }
    
    ##
    # constructor
    #   - construct the base class.
    #   - replace the data object.
    #   - replace the graphical representation of the base class's gui object.
    #   - Run the configuration options.
    constructor args {
        install StateObject using StateProgram %AUTO%
        [$StateObject data [ReadoutProgram %AUTO%]] destroy;   # replace and kill old data
        [$StateObject gui] configure -image $icon
        
        $self configurelist $args
    }
    destructor {
        if {$StateObject ne ""} {
            $StateObject destroy
        }
    }
    ##
    # clone
    #   Create/return a copy of self
    #
    method clone {} {
        set newObject [ReadoutObject %AUTO%]
        $newObject _replaceData [[$self data] clone]
        
        return $newObject
    }
}
##
# @class EventLogObject
#   Wrapper of a StateProgram with:
#   *  Data replaced by an EventLogProgram.
#   *  graphical representation replaced with  eventlog.png.
#
snit::type EventLogObject {
    component StateObject
    
    delegate option * to StateObject
    delegate method * to StateObject
    
    typevariable icon
    
    ##
    # typeconstructor
    #   Sets 'icon' to be the eventlog.png file.
    #
    typeconstructor {
        set here [file dirname [info script]]
        set icon [image create photo -format png -file [file join $here eventlog.png]]
    }
    ##
    # constructor
    #   - construct the base class.
    #   - replace the data object.
    #   - replace the graphical representation of the base class objec.
    #   - process configuration options.
    #
    constructor args {
        install StateObject using StateProgram %AUTO%
        [$StateObject data [EventLogProgram %AUTO%]] destroy
        [$StateObject gui] configure -image $icon
        
        $self configurelist $args
    }
    ##
    # destructor:
    #
    destructor {
        $StateObject destroy
    }
    ##
    # clone
    #   Create/return a copy of self
    #
    method clone {} {
        set newObject [EventLogObject %AUTO%]
        $newObject _replaceData [[$self data] clone]
        
        return $newObject
    }
}

