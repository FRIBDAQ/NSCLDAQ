#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL) ir the Tcl BSD derived license  The terms of these licenses
#     are described at:
#
#     GPL:  http://www.gnu.org/licenses/gpl.txt
#     Tcl:  http://www.tcl.tk/softare/tcltk/license.html
#     Start with the second paragraph under the Tcl/Tk License terms
#     as ownership is solely by Board of Trustees at Michigan State University. 
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

#  Implements a 'meter' megawidget.  A meter is a
#  box with a needle that goes up and down between
#  two possible limits.
#
# This is drawn in a canvas as follows:
#    +-------+
#    |       |
#    |  <----|
#    | ...
#    +-------+
#
#
# OPTIONS:
#    -from        - Value represented by the lower limit of the meter.  (dynamic)
#    -to          - Value represented by the upper limit of the meter.  (dynamic)
#    -height      - Height of the meter.                                (static)
#    -width       - Width of the meter.                                 (static)
#    -variable    - Variable the meter will track.                      (dynamic)
#    -majorticks  - Interval between major (labelled) ticks.            (dynamic)
#    -minorticks  - Number of minor ticks drawn between major ticks.    (dynamic)
#    -log         - True if should be log scale                         (dynamic).
#
# Methods:
#    set value    - Set the meter to a specific value (if -variable is defined it is modified).
#    get          - Returns the current value of the meter.

package provide meter 1.0
package require Tk
package require snit
package require bindDown

namespace eval controlwidget {
    namespace export meter
}
snit::widget controlwidget::meter {
    option -from        -1.0
    option -to          1.0
    option -height     {2i}
    option -width      {1.5i}
    option -variable   {}
    option -majorticks 1.0
    option -minorticks 4
    option -log        false
    
    variable constructing   1

    variable needleId       {}
    variable topY           {}
    variable bottomY        {}
    variable valueRange     {}
    variable needleLeft     {}
    variable meterLeft      {}
    variable majorlength

    variable tickIds        {}
    variable lastValue       0
    
    variable decadeLow       0;       # e.g. 1 -10... this is the low end exponent.
    variable decadeHigh      1;        # e.g. 10-100.

    variable fontList

    # Construct the widget:

    constructor args {
        $self configurelist $args

        set valueRange [expr 1.0*($options(-to) - $options(-from))]

	# In order to get the font info, we need to create an invisible
	# label so we can query the default font.. we'll accept that
	# but ensure that the font size is 10.

	label $win.hidden
	set fontList [$win.hidden cget -font]
	set fontList [font actual $fontList]
	set fontList [lreplace $fontList 1 1 10];    # Force size to 10pt.

        # Create the canvas and draw the meter into the canvas.
        # The needle is drawn at 1/2 of the rectangle height.
        # 3/4 width.
        # We'll store the resulting size back in the options asn
        # pixels since their much easier to work with:

        canvas $win.c   \
            -width $options(-width)   \
            -height $options(-height) \
            -background white


        set options(-height) [$win.c cget -height]
        set options(-width)  [$win.c cget -width]

        # In order to support label we need to create a left margin
        # the margin will be 8chars worth of 8's  in the font we've used
        # and a top/bottom margin of 5pt.. the assumption is that the labels
        # will be drawn in 10pt font.

	set leftmargin [font measure $fontList 88888888]

        set leftmargin [$win.c canvasx $leftmargin]
        set vmargin    [$win.c canvasy 5p]

        # Compute the coordinates of the rectangle and the top/bottom limits
        # (for scaling the arrow position).

        set meterLeft  $leftmargin
        set topY       $vmargin
        set meterRight $options(-width)
        set bottomY    [expr $options(-height) - $vmargin]


        # draw the frame of the meter as a rectangle:

        $win.c create rectangle $meterLeft $topY $meterRight $bottomY

        # figure out how to put the needle in the middle of the
        # height of the meter allowing 1/4 of the meter for ticks.
        #

        set needleWidth   [expr 3*($meterRight - $meterLeft)/4]
        set needleHeight  [$self computeHeight   \
                             [expr ($options(-to) + $options(-from))/2]]
        set needleLeft   [expr $options(-width) - $needleWidth]

        set needleId [$win.c create line $needleLeft $needleHeight      \
                                         $options(-width) $needleHeight \
                                        -arrow first]

        grid $win.c -sticky nsew


        $self drawTicks

        if {$options(-variable) ne ""} {
           trace add variable ::$options(-variable) write [mymethod variableChanged]
            $self needleTo [set ::$options(-variable)]
        }
	bindDown $win $win

        set constructing 0
    }

    #-------------------------------------------------------------------------------
    # public methods
    #

    # Set a new value for the meter... this moves the pointer to a new value.
    # if a variable is tracing the meter, it is changed
    #
    method set newValue {
        if {$options(-variable) ne ""} {
            set ::$options(-variable) $newValue;      # This updates meter too.
        } else {
            $self needleTo $newValue
        }
    }

    # Get the last meter value.
    #
    method get {} {
        return $lastValue
    }

    #-------------------------------------------------------------------------------
    # 'private' methods.

    # trace on -variable being modified.

    method variableChanged {name1 name2 op} {

        $self needleTo [set ::$options(-variable)]
    }

    # Set a new position for the needle:

    method needleTo newCoords {
        set lastValue $newCoords

        set height [$self computeHeight $newCoords]
        $win.c coords $needleId $needleLeft $height $options(-width) $height
    }

    # Compute the correct height of the needle given
    # A new coordinate value for it in needle units:

    method computeHeight needleCoords {
        if {$options(-log)} {
            return [$self computeLogHeight  $needleCoords]
        } else {
            return [$self computeLinearHeight $needleCoords]
        }
    }

    #  Compute the needle height if the scale is log.

    method computeLogHeight needleCoords {
        $self computeDecades
        #
        #  The following protect against range errors as well as
        #  negative/0 values:
        #
        if {$needleCoords < $decadeLow} {
            set needleCoords $decadeLow
        }
        if {$needleCoords > $decadeHigh} {
            set needleCoords $decadeHigh
        }
        
        #  Now it should be safe to do the logs:
        #  the scaling is just linear in log coords:
        
        set valueRange [expr log10($decadeHigh) - log10($decadeLow)]
        set value      [expr log10($needleCoords) - log10($decadeLow)]
        
        set pixelRange [expr 1.0*($bottomY - $topY)]
        
        set height [expr $value*$pixelRange/$valueRange]
        return [expr $bottomY - $height]
        
    }

    #  Compute the needle height if the scale is linear
    #
    method  computeLinearHeight needleCoords {

        #
        # Peg the needle to the limits:
        #
        if {$needleCoords > $options(-to)}  {
            return $topY
        }
        if {$needleCoords < $options(-from)} {
            return $bottomY
        }
        set pixelRange [expr 1.0*($bottomY - $topY)]

        # Transform the coordinates:

        set height [expr ($needleCoords - $options(-from))*$pixelRange/$valueRange]
        return [expr $bottomY - $height]
    }

    # Draw the tick marks on the meter face.  The major ticks are
    # labelled, while the minor ticks are just some length.
    # Major ticks extend from the meter left edge to 1/5 the width of the meter
    # while minor ticks extend from the meter left edge to 1/10 the width of the meter.
    # Tick labels are drawn at x coordinate 0.
    #
    method drawTicks {} {
	
	if {!$options(-log)} {
	    $self drawLinearTicks
	} else {
	    $self drawLogTicks
	}
    }
    #
    #  Draw the ticks for a log scale.
    #
    method drawLogTicks {} {
        set decades     [$self computeDecades];       # Range of meter...
        set majorRight  [$self getMajorRight];        # Right end coordinate of major tick.
        set minorRight  [$self getMinorRight];        # Right end coord of minor tick.

        
        #  Major ticks are easy.. they are at the decades.
        
        set range [expr $topY - $bottomY]
        set interval [expr $range/([llength $decades] -1) ];  # Space decades evenly.
        
        set pos   $bottomY
        set tickIds
        foreach decade $decades {
            lappend tickIds [$win.c create text $meterLeft $pos -text $decade -anchor e -font $fontList]
            lappend tickIds [$win.c create line $meterLeft $pos  $majorRight $pos]
            #
            #  Now the minor ticks... we draw for 1-9. of them in log spacing.
            #
            foreach mant [list 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0] {
                set ht [expr $pos + $interval*log10($mant)]
                lappend tickIds [$win.c create line $meterLeft $ht $minorRight $ht]
            }
            set pos [expr $pos + $interval]
        }
    }
    #
    #  Draw the ticks for a linear scale:
    #
    method drawLinearTicks {} {
        set first $options(-from)
        set last  $options(-to)
        set major $options(-majorticks)


        # minor ticks are just given in terms of the # ticks between majors so:

        set minor [expr 1.0*$major/($options(-minorticks)+1)]

        # Figure out the right most coordinates of the tick lines.

	set majorRight [$self getMajorRight]
	set minorRight [$self getMinorRight]
       # the for loop is done the way it is in order to reduce
       # the cumulative roundoff error from repetitive summing.
       #


        set majorIndex 0
        for {set m $first} {$m <= $last} {set m [expr $first + $majorIndex*$major]} {
            # Draw a major tick label and the tick mark itself
            # major ticks are formatted in engineering notation (%.1e).

            set label [format %.1e $m]
            set height [$self computeHeight $m]
            lappend tickIds [$win.c create text  $meterLeft $height -text $label -anchor e -font $fontList]
            lappend tickIds [$win.c create line  $meterLeft $height $majorRight $height]
            for {set i 1} {$i <=  $options(-minorticks)} {incr i} {
                set minorH [expr $m + 1.0*$i*$minor]
                set minorH [$self computeHeight $minorH]
                lappend tickIds [$win.c create line $meterLeft $minorH $minorRight $minorH]
            }
            incr majorIndex
        }
    }
    #
    #  Erase the Tick ids from the meter:
    #
    method eraseTicks {} {
        foreach id $tickIds {
            $win.c delete $id
        }
    }
    #
    #     Compute the right x coordinate  of the major ticks:
    #
    method getMajorRight {} {
	set majorlength [expr ($options(-width) - $meterLeft)/5]
	set majorRight  [expr $meterLeft + $majorlength]
	
	return $majorRight
    }
    #
    #    Compute the right x coordinate of the minor ticks:
    #
    method getMinorRight {} {
        set minorlength [expr  $majorlength/2]        
        set minorRight  [expr $meterLeft + $minorlength]
	return $minorRight
    }

    # compute the decades in the plot.  This is also where we will complain if the
    # range covers 0 or a negative range as for now we only support positive log scales.
    # Returns a list of the decades e.g. {1.0e-9 1.0e-08 1.0e-7}  that cover the range.
    # The low decade truncates.  The high one is a ceil.
    #
    
    method computeDecades {} {
	set low $options(-from)
        
	if {$low <= 0.0} {
	    error "Log scale with negative or zero -from value is not supported"
	}
	set high $options(-to)
	if {$high <= 0.0} {
	    error "Log scale with negative or zero -to value no"
        }
        #
        set lowDecade  [expr log10($low)]
        if {$lowDecade < 0} {
            set lowDecade [expr $lowDecade - 0.5]
        }
        set lowDecade [expr int($lowDecade)]
        
        set result     [format "1.0e%02d" $lowDecade]
        set highDecade [expr log10($high)];               # Don't truncate...
        while {$lowDecade < $highDecade} {
            incr lowDecade
            lappend result [format "1.0e%02d" $lowDecade]
        }
        set decadeLow  [lindex $result 0]
        set decadeHigh [lindex $result end]
        return $result
    }

    #------------------------ Configuration handlers for dynamic options  ----
    #    -from        - Value represented by the lower limit of the meter.  (dynamic)
    #    -to          - Value represented by the upper limit of the meter.  (dynamic)
    #    -variable    - Variable the meter will track.                      (dynamic)
    #    -majorticks  - Interval between major (labelled) ticks.            (dynamic)
    #    -minorticks  - Number of minor ticks drawn between major ticks.    (dynamic)


    # Handle configure -from
    # Need to set the stuff needed to scale the meter positions and reset the meter position.
    # Need to redraw ticks as well.
    #
    onconfigure -from value {
        set options(-from) $value
        if {![winfo exists $win.c]} return;     # Still constructing.
        $self eraseTicks
        set   valueRange [expr $options(-to) - $value]
        $self drawTicks

        $self needleTo $lastValue
    }
    # Handle configure -to
    # As for -from but -to is modified.
    #
    onconfigure -to value {
        set options(-to) $value
        if {![winfo exists $win.c]} return;     # Still constructing.
        $self eraseTicks
        set valueRange [expr $value - $options(-from)]
        $self drawTicks
        $self needleTo $lastValue
    }
    #
    #  Handle configure -log
    #  Set the log flag accordingly and then redraw the ticks and value:
    #  Note that we must check the -from/-to and figure out the first decade
    #  and the last decade.
    #
    onconfigure -log value {
	variable constructing
	#  No change return.
        
	if {$value == $options(-log)}  return;    # short cut exit.
	
        # require booleanness.
	
	if {![string is boolean $value]} {
	    error "meter.tcl - value of -log flag must be a boolean"
	}
	#  Set the new value and update the meter:
	
	set options(-log) $value
        if {!$constructing} {
            $self computeDecades
            $self eraseTicks
            $self drawTicks
            $self needleTo $lastValue
        }
    }

    # Handle a change in major ticks.. we just need to set the option and redraw the ticks.
    #
    onconfigure -majorticks value {
        set options(-majorticks) $value
        if {![winfo exists $win.c]} return;     # Still constructing.
        $self eraseTicks
        $self drawTicks
    }
    # same but for minor ticks...
    #
    onconfigure -minorticks value {
        set options(-minorticks) $value
        if {![winfo exists $win.c]} return;     # Still constructing.
        $self eraseTicks
        $self drawTicks
    }
    #  Configure the variable for the meter.
    #  Any prior variable must have its trace removed.
    #  The new variable gets a trace established and the meter position
    #  is updated from it.
    #  Note that if the new variable is "" then the meter will have
    #  no variable associated with it.

    onconfigure -variable name {

        # Could be still constructing in which case
        # $win.c does not exist:

        if {![winfo exists $win.c]} {
            set options(-variable) $name
            return;
        }

        # Remove any old traces


        if {$options(-variable) ne ""} {
            trace remove variable ::$options(-variable) write [mymethod variableChanged]
        }

        # Set new trace if appropriate and update value.

        set options(-variable) $name
        if {$options(-variable) ne ""} {
            trace add variable ::$options(-variable) write [mymethod variableChanged]
            $self needleTo [set ::$options(-variable)]
        }

    }
}

