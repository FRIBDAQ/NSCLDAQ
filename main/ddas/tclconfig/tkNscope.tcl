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
# @file tkNscope.tcl
# @brief  Quick and fast growing replacement for nscope
# @author Ron Fox <fox@nscl.msu.edu>
#

# Assume we're installedin bin figure out where the libs are so we can
# pull in the pixie16 package.

set libdir [file normalize [file join [file dirname [info script]] .. TclLibs]]
lappend auto_path $libdir

package require Tk
package require snit
package require pixie16

pixie16::init

set version V1.0

set moduleList [pixie16::inventory]
set defaultModule 0


#-----------------------------------------------
#  Megawidgets.
#

# same as bits below but there's a mask for every channel
  
snit::widgetadaptor ChannelBits {
    option -masks -default [lrepeat 16 0] -configuremethod _setBits
    option -labels -default [lrepeat 32 ""] -readonly 1
    
    constructor args {
        installhull using ttk::frame
        $self configurelist $args;   #need to get the labels early.
        
        for {set i 0} {$i < 32} {incr i} {
            set label [lindex $options(-labels) $i]
            ttk::label $win.l$i -text $label
            grid $win.l$i -row 0 -column $i -padx 3
        }
            
        # Now 16 rows of 32 checkbuttons; note we start in row 1.
        
        for {set row 1} {$row <=16 } {incr row} {        
            for {set col 0} {$col < 32} {incr col} {
                ttk::checkbutton $win.row${row}col$col -command [mymethod _toggle $row $col]
                grid $win.row${row}col$col -in $win -row $row -column $col
            }
        }
        
        # Need to do this again in case -masks was supplied.
        
        $self configurelist $args
    }
    method _setBits {opt newmasks} {
        # New set of mask values:
        
        if {[llength $newmasks] != 16} {
            error "There must be 16 masks in '$newmasks'"
        }
        for {set ch 0} {$ch < 16} {incr ch} {
            set row [expr {$ch + 1}];    # For widget name.
            set newmask [lindex $newmasks $ch]
            set oldmask [lindex $options($opt) $ch]
            for {set i 0} {$i < 32} {incr i} {
                set widget $win.row${row}col$i
                set bit [expr {1 << (31-$i)}]
                set sb [expr {$newmask & $bit}]
                set is [expr ($oldmask & $bit)]
            
                
                if {$sb != $is && [winfo exists $widget]} {
                    $widget invoke;  # Bring widget into line.
                }
            }
        }
        set options($opt) $newmasks
    }
    method _toggle {row col} {
        set maskno [expr {$row -1}]
        set bit    [expr 1 << (31 - $col)];   #mask of bit to flip.
        
        set current [lindex $options(-masks) $maskno]
        set current [expr {$current ^ $bit}];  # xor is a bit flip.
        set options(-masks) [lreplace $options(-masks) $maskno $maskno $current]
    }
}
snit::widgetadaptor Bits {
    option -mask -default 0 -configuremethod _setBits 
    option -labels -default [lrepeat 32 ""] -readonly 1
    
    # Assumes a 32 bit mask
    constructor args {
        installhull using ttk::frame
        $self configurelist $args
        set labels $options(-labels)
        for {set i 0} {$i < 32} {incr i} {
            ttk::label $win.l$i -text [lindex $labels $i]
            ttk::checkbutton $win.check$i -command [mymethod _updateMask $i] 
            if {[lindex $labels $i] eq ""} {
                $win.check$i configure -state disable
            }
            grid $win.l$i -row 0 -column $i -padx 3
            grid $win.check$i -row 1 -column $i -padx 3
        }
        $self configurelist $args;    # Catch any mask config.
    }
    method _setBits {opt newmask} {
        # Note that this can be called prior to the creation of the
        # checkbutton.
        
        for {set i 0} {$i < 32} {incr i} {
            set bit [expr {1 << (31 - $i)}];  # Bits top to bottom.
            set is [expr {$options($opt) & $bit}]
            set sb [expr {$newmask & $bit}]

            
            if {($is != $sb) && [winfo exists $win.check$i]} {

                $win.check$i invoke;   #Toggle to match
            }
        }
        set options($opt) $newmask
    }
    method _updateMask i {
        # Change state of mask bit i
        
        set bit [expr {1 << (31-$i)}]
        set mask $options(-mask)
        set mask [expr {$mask ^ $bit}];  # XOR is a bit flip
        set options(-mask) $mask
    }
}

snit::widgetadaptor ChannelLabels {
    constructor args {
        installhull using ttk::frame
        
        ttk::label $win.l -text {ch}
        grid $win.l -sticky nsew
        for {set i 0} {$i < 16} {incr i} {
            ttk::entry $win.l$i -width 2 
            $win.l$i insert 0 [format %2d $i]
            $win.l$i configure -state disabled
            grid $win.l$i -sticky nsew
        }
            
        
    }
}

snit::widgetadaptor ChoiceColumn {
    option -values -default [lrepeat 16 0] \
        -configuremethod _setColumnValues -cgetmethod _getColumnValues
    option -choices -default [list] \
        -configuremethod _setColumnChoices
    option -title -default ""
    option -width -readonly 1 -default 10
    
    constructor args {
        installhull using ttk::frame
        ttk::label $win.title -textvariable [myvar options(-title)]
        grid $win.title
        for {set i 0} {$i < 16} {incr i} {
            ttk::combobox $win.channel$i -values $options(-choices)
           
        }
        $self configurelist $args
        for {set i 0} {$i < 16} {incr i} {
            $win.channel$i configure -width $options(-width)
            grid $win.channel$i -sticky e
        }
            
        
    }
    method _setColumnValues {opt values} {
        if {[llength $values] != 16} {
            error "Must have 16 values"
        }
        for {set i 0} {$i < 16} {incr i} {
            set value [lindex $values $i]
        
            if {$value ni $options(-choices)} {
                error "$value needed to be one of '$options(-choices)'"
            }
            $win.channel$i set $value
        }
    }
    method _getColumnValues {opt} {
        set result [list]
        for {set i 0} {$i < 16} {incr i} {
            lappend result [$win.channel$i get]
        }
            
        return $result
    }
    method _setColumnChoices {opt values} {
        set options($opt) $values
        for {set i 0} {$i < 16} {incr i} {
            $win.channel$i configure -values $values
            $win.channel$i set [lindex $values 0]
        }
    }
    
}

snit::widgetadaptor ChannelColumn {
    option -values -default [lrepeat 16 0]  \
        -configuremethod _setColumnValues   \
        -cgetmethod      _getColumnValues
    option -title  -default ""
    
    constructor args {
        installhull using ttk::frame
        ttk::label $win.label -textvariable [myvar options(-title)]
        grid $win.label
        
        for {set chan 0} {$chan < 16} {incr chan} {
            set entry [ttk::entry $win.entry$chan]
            $entry insert 0  0
            grid $entry
        }
            
        $self configurelist $args
    }
    method _setColumnValues {opt val} {
        if {[llength $val] != 16} {
            error "Must have 16 channel values!!!"
        }
        for {set i 0} {$i< 16} {incr i} {
            $win.entry$i delete 0 end
            $win.entry$i insert 0 [lindex $val $i]
        }
    }
    method _getColumnValues {opt} {
        set result [list]
        for {set i 0} {$i < 16} {incr i} {
            lappend result [$win.entry$i get]
        }
        return $result
    }
    
}
##
#  Stuff at the bottom of a channel prompter
#
snit::widgetadaptor ChannelActions {
    option -module -default 0 \
        -configuremethod _selectModule -cgetmethod _getModule
    option -usedmodules -default 0 -configuremethod _setSpinmax
    option -loadaction ""
    option -setaction  ""
    option -newmoduleaction ""
    
    constructor args {
        installhull using ttk::frame

        ttk::label   $win.title -text {Module}
        
        ttk::spinbox $win.module -from 0 -to $options(-usedmodules) \
            -width 2 -command [mymethod _dispatch -newmoduleaction]
        $win.module set 0
        
        ttk::button $win.load -text {Load} -command [mymethod _dispatch -loadaction]
        ttk::button $win.set  -text {Set}  -command [mymethod _dispatch -setaction]
        
        $self configurelist $args
        
        # Layout:
        
        grid $win.title $win.module -sticky w
        grid $win.load  $win.set    -sticky w
    }
    method _selectModule {opt val} {
        if {($value < 0) || ($value > 15)} {
            error "$value is not a valid channel number"
        }
        $win.module set $value
        $self _dispatch -nwmoduleaction
    }
    method _getModule {opt} {
        return [$win.module get]
    }
    method _setSpinmax {opt value} {
        $win.module configure -from 0 -to $value
        set options($opt) $value
    }
    #  command dispatcher.
    
    method _dispatch what {
        if {$options($what) ne ""} {
            set command $options($what)
            lappend command [$win.module get]
            uplevel #0 $command
        }
    }
}
##
# Top level widget to manage decay times.

snit::widgetadaptor Tau {
    component column
    component actions
    delegate option -module to actions
    constructor args {
        installhull using toplevel
        wm title $win {Decay times}
        ChannelLabels $win.l
        install column using ChannelColumn $win.values -title {Decay times(us)}
        install actions using ChannelActions $win.actions -loadaction [mymethod _load] \
            -setaction [mymethod _set] -newmoduleaction [mymethod _load] \
            -usedmodules [llength $::moduleList]
        
        $self configurelist $args
        
        set module [$actions cget -module]
        $self _load $module
        
        grid $win.l $column -sticky nsew
        grid $actions -sticky w -columnspan 2
    }
    method _load module {
        set values [list]
        for {set i 0} {$i < 16} {incr i} {
            lappend values [pixie16::readchanpar $module $i TAU]
        }
        $column configure -values $values
    }
    method _set module {
        set values [$column cget -values]
        for {set i 0} {$i < 16} {incr i} {
            pixie16::writechanpar $module $i TAU [lindex $values $i]
        }
    }
}
##
# Top level widget to manage energy filter parameters.

snit::widgetadaptor Efilter {
    component actions
    component peaking
    component gap
    delegate option -module to actions
    
    constructor args {
        installhull using toplevel
        wm title $win {Energy filter}
        
        ChannelLabels $win.l
        install peaking using ChannelColumn $win.peaking -title {Peaking time(us)}
        install gap     using ChannelColumn $win.gap -title {Gap time(us)}
        install actions using ChannelActions $win.actions  \
            -loadaction [mymethod _load] \
            -setaction [mymethod _set]  \
            -newmoduleaction [mymethod _load] \
            -usedmodules [llength $::moduleList]
        $self configurelist $args
        
        # We need the slow filter range as well
        #
        ttk::label $win.filterlabel -text {Filter Range:}
        ttk::spinbox $win.filter -from 0 -to 4 -width 1
        
        grid $win.l $peaking $gap -sticky nswe
        grid $win.filterlabel  -sticky ew
        grid $win.filter -row 1 -column 1 -sticky w 
        ttk::label $win.filterfill -text {}
        grid $win.filterfill -row 1 -column 2 -sticky ew 
        grid $actions -sticky ew -columnspan 3
        
        $self _load [$actions cget -module]
    }
    method _load module {
        set peakings [list]
        set gaps     [list]
        for {set i 0} {$i < 16} {incr i} {
            lappend peakings [pixie16::readchanpar $module $i ENERGY_RISETIME]
            lappend gaps     [pixie16::readchanpar $module $i ENERGY_FLATTOP]
        }
        $peaking configure -values $peakings
        $gap     configure -values $gaps
        
        #  now the slow filter:
        
        $win.filter set [pixie16::readmodpar $module SLOW_FILTER_RANGE]
        
    }
    method _set module {
        set peakings [$peaking cget -values]
        set gaps    [$gap     cget -values]
        for {set i 0} {$i < 16} {incr i} {
            pixie16::writechanpar \
                $module $i ENERGY_RISETIME [lindex $peakings $i]
            pixie16::writechanpar \
                $module $i ENERGY_FLATTOP [lindex $gaps $i]
        }
        # Now set the slow filter range:
        
        pixie16::writemodpar $module SLOW_FILTER_RANGE [$win.filter get]
    }
}
##
# Analog
#   polarity (choice) DCOFFSET (values)  Gain (choice).
#
snit::widgetadaptor Analog {
    component action
    component polarity
    component dcoffset
    component gain
    delegate option -module to action
    
    typevariable gainbit [expr 1 << 14]
    typevariable polaritybit [expr 1 << 5]
    
    constructor args {
        installhull using toplevel
        wm title $win {Analog conditioning}
        
        ChannelLabels $win.l
        install polarity using ChoiceColumn $win.polarity \
            -choices {+ -} -title {Polarity} -width 3
        install dcoffset using ChannelColumn $win.dcoffset -title {DC Offset(V)}
        install gain using ChoiceColumn $win.gain \
            -choices {4.0 0.9} -width 5
        install action using ChannelActions $win.actions \
            -loadaction [mymethod _load] \
            -setaction [mymethod _set]  \
            -newmoduleaction [mymethod _load] \
            -usedmodules [llength $::moduleList]
        
        $self configurelist $args
        $self _load [$action cget -module]
        
        grid $win.l $polarity $dcoffset $gain -sticky nsew
        grid $action  -sticky ew -columnspan 4
    }
    method _load module {
        set offsets [list]
        set polarities [list]
        set gains      [list]
        
        for {set i 0} {$i < 16} {incr i} {
            lappend offsets [pixie16::readchanpar $module $i VOFFSET]
            set csra [expr {int([pixie16::readchanpar $module $i CHANNEL_CSRA])}]
            if {$csra & $polaritybit} {
                lappend polarities +
            } else {
                lappend polarities -
            }
            if {$csra & $gainbit} {
                lappend gains 4.0
            } else {
                lappend gains 0.9
            }
            
        }
        
        $polarity configure -values $polarities
        $dcoffset configure -values $offsets
        $gain    configure -values $gains
            
        
        
    }
    method _set module {
        set offsets    [$dcoffset cget -values]
        set polarities [$polarity cget -values]
        set gains      [$gain cget -values]
        
        for {set i 0} {$i < 16} {incr i} {
            set csra [expr {int([pixie16::readchanpar $module $i CHANNEL_CSRA])}]
            
            pixie16::writechanpar $module $i VOFFSET [lindex $offsets 0]
            
            # Clear the polarity and gain bits from the csr:
            
            set csra [expr {$csra & ~($gainbit | $polaritybit)}]
            
            # Figure out which bits to or back in (if any).
            
            if {[lindex $polarities $i] eq "+"} {
                set csra [expr {$csra | $polaritybit}]
            }
            if {[lindex $gains $i] eq "4.0"} {
                set csra [expr {$csra | $gainbit}]
            }
            pixie16::writechanpar $module $i CHANNEL_CSRA $csra
        }
            
        
    }
}

snit::widgetadaptor Baselines {
    component blcut
    component baseline
    component action
    delegate option -modules to action
    
    constructor args {
        installhull using toplevel
        wm title $win Baselines
        
        ChannelLabels $win.l
        install blcut using ChannelColumn $win.blcut -title {BLcut[us]}
        install baseline using ChannelColumn $win.baseine -title {Baseline[%]}
        install action using ChannelActions $win.actions \
            -loadaction [mymethod _load] \
            -setaction [mymethod _set]  \
            -newmoduleaction [mymethod _load] \
            -usedmodules [llength $::moduleList]
        
        $self configurelist $args
        $self _load [$action cget -module]
        
        grid $win.l $blcut $baseline -sticky nsew
        grid $action -sticky ew -columnspan 3
        
    }
    method _load module {
        set cuts [list]
        set bls  [list]
        for {set i 0} {$i < 16} {incr i} {
            lappend cuts [pixie16::readchanpar $module $i BLCUT]
            lappend bls  [pixie16::readchanpar $module $i BASELINE_PERCENT]
        }
        $blcut configure -values $cuts
        $baseline configure -values $bls
            
        
        
    }
    method _set module {
        set cuts [$blcut cget -values]
        set bls  [$baseline cget -values]
        for {set i 0} {$i < 16} {incr i} {
            pixie16::writechanpar $module $i BLCUT [lindex $cuts $i]
            pixie16::writechanpar $module $i BASELINE_PERCENT [lindex $bls $i]
        }
            
        
    }
}
snit::widgetadaptor TriggerFilter {
    component action
    component peaking
    component gap
    component thresh
    
    delegate option -modules to action
    constructor args {
        installhull using toplevel
        wm title $win {Trigger Filter}
        
        ChannelLabels $win.l
        install action using ChannelActions $win.actions  \
            -loadaction [mymethod _load] \
            -setaction [mymethod _set]  \
            -newmoduleaction [mymethod _load] \
            -usedmodules [llength $::moduleList]
        install peaking using ChannelColumn $win.peaking -title {TPeaking[us]}
        install gap     using ChannelColumn $win.gap -title {TGap[us]}
        install thresh  using ChannelColumn $win.thresh -title {Thresh. [ADC u]}
        
        $self configurelist $args
        $self _load [$action cget -module]
        
        grid $win.l $peaking $gap $thresh -sticky nsew
        grid $action -sticky ew -columnspan 4
    }
    method _load module {
        set tpeaking [list]
        set tgap    [list]
        set trthresh [list]
        for {set i 0} {$i < 16} {incr i} {
            lappend tpeaking [pixie16::readchanpar $module $i TRIGGER_RISETIME]
            lappend tgap     [pixie16::readchanpar $module $i TRIGGER_FLATTOP]
            lappend trthresh [pixie16::readchanpar $module $i TRIGGER_THRESHOLD]
        }
        $peaking configure -values $tpeaking
        $gap     configure -values $tgap
        $thresh  configure -values $trthresh
        
    }
    method _set module {
        set tpeaking [$peaking cget -values]
        set tgap     [$gap cget -values]
        set trthresh [$thresh cget -values]
        
        for {set i 0} {$i < 16} {incr i} {
            pixie16::writechanpar $module $i TRIGGER_RISETIME [lindex $tpeaking $i]
            pixie16::writechanpar $module $i TRIGGER_FLATTOP  [lindex $tgap $i]
            pixie16::writechanpar $module $i TRIGGER_THRESHOLD [lindex $trthresh $i]
        }
            
        
    }
}
snit::widgetadaptor CFD {
    component action
    component delay
    component scale
    component thresh
    component resetdelay
    
    delegate option -module to action
    
    constructor args {
        installhull using toplevel
        
        ChannelLabels $win.l
        install action using ChannelActions $win.action \
            -loadaction [mymethod _load] \
            -setaction [mymethod _set]  \
            -newmoduleaction [mymethod _load] \
            -usedmodules [llength $::moduleList]
        install delay using ChannelColumn $win.delay -title {CFD Delay[us]}
        install scale using ChannelColumn $win.scale -title {CFD Scale}
        install thresh using ChannelColumn $win.thr  -title {CFD Thresh. [ADC u]}
        install resetdelay using ChannelColumn $win.rdelay -title {CFD ResetDelay [us]}
        
        grid $win.l $delay $scale $thresh $resetdelay -sticky nsew
        grid $action -sticky ew -columnspan 4
        
        $self configurelist $args
        
        $self _load [$action cget -module]
    }
    method _load module {
        set del [list]
        set s   [list]
        set t   [list]
        
        for {set i 0} {$i < 16} {incr i} {
            lappend del [pixie16::readchanpar $module $i CFDDelay]
            lappend s   [pixie16::readchanpar $module $i CFDScale]
            lappend t   [pixie16::readchanpar $module $i CFDThresh]
            lappend rd  [pixie16::readchanpar $module $i ResetDelay]
        }
        
        $delay configure -values $del
        $scale configure -values $s
        $thresh configure -values $t
        $resetdelay configure -values $rd
    }
    method _set module {
        set del [$delay cget -values]
        set s   [$scale cget -values]
        set t   [$thresh cget -values]
        set rd  [$resetdelay cget -values]
        
        for {set i 0} {$i < 16} {incr i} {
            pixie16::writechanpar $module $i CFDDelay [lindex $del $i]
            pixie16::writechanpar $module $i CFDScale [lindex $s $i]
            pixie16::writechanpar $module $i CFDThresh [lindex $t $i]
            pixie16::writechanpar $module $i ResetDelay [lindex $rd $i]
        }
            
        
    }
}
snit::widgetadaptor PulseShape {
    component action
    component tracelen
    component tracedelay
    
    delegate option -module to action
    
    constructor args {
        installhull using toplevel
        wm title $win {Pulse shape}
        
        ChannelLabels $win.l
        install action using ChannelActions $win.actions \
            -loadaction [mymethod _load] \
            -setaction [mymethod _set]  \
            -newmoduleaction [mymethod _load] \
            -usedmodules [llength $::moduleList]
        install tracelen using ChannelColumn $win.len -title {Trace length [us]}
        install tracedelay using ChannelColumn $win.del -title {Trace delay [us]}
        
        $self configurelist $args
        $self _load [$action cget -module]
        
        grid $win.l $tracelen $tracedelay -sticky nsew
        grid $action -sticky ew -columnspan 3
    }
    method _load module {
        set lens [list]
        set dels [list]
        for {set i 0} {$i < 16} {incr i} {
            lappend lens [pixie16::readchanpar $module $i TRACE_LENGTH]
            lappend dels [pixie16::readchanpar $module $i TRACE_DELAY]
        }
        
        $tracelen configure -values $lens
        $tracedelay configure -values $dels
        
    }
    method _set module {
        set lens [$tracelen cget -values]
        set dels [$tracedelay cget -values]
        
        for {set i 0} {$i < 16} {incr i} {
            pixie16::writechanpar $module $i TRACE_LENGTH [lindex $lens $i]
            pixie16::writechanpar $module $i TRACE_DELAY [lindex $dels $i]
        }
            
        
    }
}
#
# module ac
#
# Module variables
#
snit::widgetadaptor Modvars {
    component actions
    delegate option -module to actions
    
    constructor args {
        installhull using toplevel
        install actions using ChannelActions $win.actions \
            -loadaction [mymethod _load] \
            -setaction [mymethod _set]  \
            -newmoduleaction [mymethod _load] \
            -usedmodules [llength $::moduleList]
        
        ttk::label $win.csrbl -text {Module CSRB (hex)}
        ttk::entry $win.csrb -width 10
        
        ttk::label $win.maxel -text {Maximum Events (dec)}
        ttk::entry $win.maxe  -width 6

        ttk::label $win.nmwmodl  -text {Slot Id}
        ttk::entry $win.nmwmod   -width 3
        
        ttk::label $win.modadrl -text {Module ID}
        ttk::entry $win.modadr  -width 3
        
        ttk::label $win.crateadrl -text {Crate address}
        ttk::entry $win.crateadr  -width 3
        
        grid $win.csrbl $win.csrb -sticky nsew
        grid $win.maxel $win.maxe -sticky nsew
        grid $win.nmwmodl $win.nmwmod -sticky nswe
        grid $win.modadrl $win.modadr -sticky nsew
        grid $win.crateadrl $win.crateadr -sticky nsew
        grid $actions -sticky w -columnspan 2
        
        $self configurelist $args
        $self _load [$actions cget -module]
        
    }
    method _load module {
        set csrb [pixie16::readmodpar $module MODULE_CSRB]
        $win.csrb delete 0 end
        $win.csrb insert end [format %x $csrb]
        
        $win.maxe delete 0 end
        $win.maxe insert end [pixie16::readmodpar $module MAX_EVENTS]

        $win.nmwmod delete 0 end
        $win.nmwmod insert end [pixie16::readmodpar $module SlotID]
        
        $win.modadr delete 0 end
        $win.modadr insert end [pixie16::readmodpar $module ModID]
        
        $win.crateadr delete 0 end
        $win.crateadr insert end [pixie16::readmodpar $module CrateID]
    }
    method _set module {
        pixie16::writemodpar $module MODULE_CSRB 0x[$win.csrb get]
        pixie16::writemodpar $module MAX_EVENTS [$win.maxe get]
        pixie16::writemodpar $module SlotID [$win.nmwmod get]
        pixie16::writemodpar $module ModID [$win.modadr get]
        pixie16::writemodpar $module CrateID [$win.crateadr get]
    }
}
snit::widgetadaptor ModCSRB {
    component actions
    component bits
    
    delegate option -module to actions
    variable bitnames -array {
       0 PUP
       4 DIR
       6 MSTR
       7 GFTSEL
       8 ETSEL
       10 INHIBITEna
       11 Mult.Crates
       12 Sort
       13 {Bckpl Fast trg}
    }    
    constructor args {
        installhull using toplevel
        wm title $win {Module CSRB}
        

        #  Figure out the names  - fill them in by bit number then
        #  reverse them because that's what's expected.
        
        set names [lrepeat 32 ""]
        
        foreach bitnum [array names bitnames] {

            set names [lreplace $names $bitnum $bitnum $bitnames($bitnum)]
        }
        set names [lreverse $names];   # High bit to low bit now.
        
        install bits using Bits $win.bits -labels $names
        install actions using ChannelActions $win.actions \
            -loadaction [mymethod _load] \
            -setaction [mymethod _set]  \
            -newmoduleaction [mymethod _load] \
            -usedmodules [llength $::moduleList]
        grid $bits -sticky nsew
        grid $actions -sticky ew
        $self configurelist $args
        $self _load [$actions cget -module]
    }
    method _load module {
        set csrb [pixie16::readmodpar $module MODULE_CSRB]
        $bits configure -mask $csrb
    }
    method _set module {
        set mask [$bits cget -mask]
        pixie16::writemodpar $module MODULE_CSRB $mask
    }
}

snit::widgetadaptor ChannelCSRA {
    component bits
    component actions
    delegate option -module to actions
    variable bitnames -array {
        0 FTRIG
        1 ETRIG
        2 ENABL
        3 EXTVL
        4 SYNCH
        5 "POL- "
        6 VENAB
        7 HSTEN
        8 TRCEN
        9 QDCEN
        10 CFDEN
        11 GBLTR
        12 SUMEN
        13 SLFTR
        14 NOATT
        15 PURCT
        16 PURIN
        17 ECUTE
        18 GRTRG
        19 CHVETO
        20 MVETOS
        21 EXCLK
    }
    constructor args {
        installhull using toplevel
        
        set names [lrepeat 32 ""]
        foreach idx [array  names bitnames] {
            set names [lreplace $names $idx $idx $bitnames($idx)]
        }
        set names [lreverse $names];    #top bit first.
        
        install bits using ChannelBits $win.bits -labels $names
        install actions using ChannelActions $win.actions \
            -loadaction [mymethod _load] \
            -setaction [mymethod _set]  \
            -newmoduleaction [mymethod _load] \
            -usedmodules [llength $::moduleList]
        $self configurelist $args    
        
        grid $bits -sticky nsew
        grid $actions -sticky ew
        
        $self _load [$actions cget -module]
    }
    method _load module {
        set masks [list]
        for {set i 0} {$i < 16} {incr i} {
            lappend masks [expr {int([pixie16::readchanpar $module $i CHANNEL_CSRA])}]
        }
        $bits configure -masks $masks    
    }
    method _set module {
        set masks [$bits cget -masks]
        for {set i 0} {$i < 16} {incr i} {
            pixie16::writechanpar $module $i CHANNEL_CSRA [lindex $masks $i]
        }
    }
}
#-----------------------------------------------
# Main widget control handlers:


##
# Boot modules in moduleList
#
proc bootModules {} {
    for {set i 0} {$i < [llength $::moduleList]} {incr i} {
        set d [lindex $::moduleList $i]
        set slot [lindex $d 0]
        set bits [lindex $d 3]
        set mhz  [lindex $d 4]
        puts "Booting $bits bit ${mhz}MHz module in physical slot $slot ($i)"
        pixie16::boot $i
    }
    
}
#----------------------------------------------
# Expert menu handlers

proc handleModVars {} {
    if {![winfo exists .modvars]} {
        Modvars .modvars
    }
}
proc handleModCSRB {} {
    if {![winfo exists .modcsrb]} {
        ModCSRB .modcsrb
    }
    
}
#----------------------------------------------
# UV menu handlers.
#
##
# handlePulseShape
#
proc handlePulseShape {} {
    if {![winfo exists .pulseshape]} {
        PulseShape .pulseshape
    }
}
##
# handleCFD
#
proc handleCFD {} {
    if {![winfo exists .cfd]} {
        CFD .cfd
    }
}
##
# handleTriggerFilter
#
proc handleTriggerFilter {} {
    if {![winfo exists .triggerfilter]} {
        TriggerFilter .triggerfilter
    }
}
##
# handleAnalog
#
proc handleAnalog {} {
    if {![winfo exists .analog]} {
        Analog .analog
    }
}
##
# handleTaus
#
proc handleTaus {} {
    if {![winfo exists .taus]} {
        Tau .taus
    }
    
}
proc handleEfilter {} {
    if {![winfo exists .efilter]} {
        Efilter .efilter
    }
}
proc handleBaselines {} {
    if {![winfo exists .baselines]} {
        Baselines .baselines
    }
}
proc handleChCSRA {} {
    if {![winfo exists .chcsra]} {
        ChannelCSRA .chcsra
    }
}
#-----------------------------------------------
# file menu handlers

##
# Prompt for and load a set file:
#
proc loadSetFile {} {
    set file [tk_getOpenFile -parent . -title {Set file} \
        -filetypes [list                                 \
          [list {Set Files} .set ]                       \
          [list {All Files} *]                           \
        ]                                                \
    ]
    if {$file ne ""} {
        puts "Loading $file"
        pixie16::restore $file
        wm title . "tkNscope - $file"
    }
}
    
##
# prompt and save set file.
#
proc saveSetFile {} {
    set file [tk_getSaveFile -parent . -title {Set file} \
        -filetypes [list                                 \
          [list {Set Files} .set ]                       \
          [list {All Files} *]                           \
        ]                                                \
    ]
    if {$file ne ""} {
        puts "Saving $file"
        pixie16::save $file
        wm title . "tkNscope - $file"
    }
}
    
#----------------------------------------------------
# Menu creation.
#
#  Create file menu:
#
proc setupFilemenu {parent} {
    set menu [menu $parent.file -tearoff 0]
    
    $menu add command -label Open...  -command loadSetFile
    $menu add command -label Save...  -command saveSetFile
    $menu add separator
    $menu add command -label Exit -command [list exit 0]
    
    return $menu
}

##
# create the UV menu whatever the hecck that is.
#
proc setupUVMenu {parent} {
    set menu [menu $parent.uv -tearoff 0]
    $menu add command -label {Analog Signal Conditioning} -command handleAnalog
    $menu add command -label {Baselines Setup} -command handleBaselines
    $menu add command -label {Energy Filter}   -command handleEfilter
    $menu add command -label {Trigger filter}  -command handleTriggerFilter
    $menu add command -label {CFD} -command handleCFD
    $menu add command -label {channel CSRA} -command handleChCSRA
    $menu add command -label {Multiplicity Coincidence} -state disabled
    $menu add command -label {Timing controls} -state disabled
    $menu add command -label {Pulse shape}  -command handlePulseShape
    $menu add command -label {Decay time}  -command handleTaus
    $menu add command -label {Histogramming} -state disabled
    
    return $menu
}
##
# Set up expert menu
#
proc setupExpertMenu parent  {
    set menu [menu $parent.expert -tearoff 0]
    $menu add command -label {Module Variables} -command handleModVars
    $menu add command -label {Module CSRB}  -command handleModCSRB
    $menu add command -label {Find Tau}     -state disabled
    $menu add command -label {Start Baseline Run} -state disabled
    
    return $menu
}
##
# Help menu
proc setupHelpMenu {parent} {
    set menu [menu $parent.help -tearoff 0]
    
    $menu add command -label "About..." -command [list tk_messageBox \
        -type ok -icon info -title "tkNscope about" \
        -message "tkNscope\nAuthor: Ron Fox\nVersion $::version" \
    ]
    
    return $menu
}
##
# Set up the menu
#
proc menusetup {} {
    menu .main -tearoff 0
    set fileMenu [setupFilemenu .main]
    .main add cascade -label File -menu $fileMenu
    
    set uvMenu [setupUVMenu .main]
    .main add cascade -label {UV Setup} -menu $uvMenu
    
    set expert [setupExpertMenu .main]
    .main add cascade -label {Expert} -menu $expert
    .main add cascade -label Scope -state disabled
    
    set help [setupHelpMenu .main]
    .main add cascade -label Help -menu $help
    
    . configure -menu .main
}

#----------------------------------------------------
# Entry point.
#
wm geometry . 400x100
menusetup
button .boot -command bootModules -text {Boot}
grid .boot

