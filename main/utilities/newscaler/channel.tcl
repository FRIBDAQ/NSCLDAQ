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
# @file channel.tcl
# @brief Encapuslate a scaler channel.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide channel 1.0
package require snit

##
# @class channel
#   Represents a scaler channel.
#
# OPTIONS:
#   -incremental - True if the channel updates are increments.
#   -width       - Num of bits used to mask off update values.
#   -lowlim      - Low alarm limit.
#   -hilim       - High alarm limit.
#   -name        - Name of the channel.

#
# METHODS:
#    total    - Return the total counts.
#    rate     - Return the count rates.
#    update   - Update the counters.
#    clear    - Clear the counters.
#    alarming - Return the channels alarm status.
#
snit::type Channel {
    option -incremental 1
    option -width       32
    option -lowlim      ""
    option -hilim       ""
    option -name        ""
    option -source    0
    
    variable total 0
    variable rate  0
    variable elapsed 0
    variable sumOfSquares 0
    variable samples      0
    variable overflowCount 0
    
    # The data below is needed to handle wraps in non-incremental mode
    
    variable lastUpdate   0

    
    ##
    # total
    # @return - the current scaler totals.
    #
    method total {} {
        return $total
    }
    ##
    # rate
    #   @return - the intantaneous scaler rate.
    #
    method rate {} {
        return $rate
    }
    ##
    # update
    #   Updates the value of the scalers.  What actually happends depends
    #   on
    #   * State of the -incremental flag.
    #   * value of the -width parameter.
    #
    # @param counts - counts from the scaler (integer).
    # @param dt     - floating point seconds over which these counts accumulated.
    #
    method update {counts dt} {
        
        set mask [expr {(1 << $options(-width)) - 1}]
        set counts [expr {$mask & $counts}]
        
        if {$options(-incremental)} {
            
            # Incremental scalers:
            
            incr total $counts
            set rate [expr {double($counts)/$dt}]
        } else {
            
            # Correct counts for any single wrap.  It's just
            # not possible to correct for multiple wraps:
            
            if {$counts < $lastUpdate} {
		        incr overflowCount
            }   
            set lastUpdate $counts
            incr counts [expr $overflowCount * (1 << $options(-width))]; # Adjust for all historical oveflows.
                
            # non incremental scaler:
            
            set rate [expr {double($counts - $total)/$dt}]
            set total $counts

            
        }
        set elapsed [expr {$elapsed + $dt}];      # dt could be non-integer.
        set sumOfSquares [expr {$sumOfSquares + $rate*$rate}]
        incr samples
    }
    ##
    # clear
    #   Clears the scaler.
    #
    method clear {} {
        set total 0
        set rate 0
        set elapsed 0
        set sumOfSquares 0
        set samples 0
	set lastUpdate 0
	set overflowCount 0
    }
    ##
    # alarming
    #   Return information about the alarm state of the channel.
    # @note - alarm limits that are not set have the value ""
    # @note - The assumption is that only one alarm condition
    #         can exist at a time, though it is possible to set the
    #         options otherwise (e.g. -low > -high).
    # @return string in the set {ok, high, low}  
    method alarming {} {
        if {($options(-hilim) ne "") && ($rate > $options(-hilim))} {
            return high
        } elseif {($options(-lowlim) ne "") && ($rate < $options(-lowlim))} {
            return low
        } else {
            return "ok"
        }
    }
    ##
    # mean
    #   Get the average rate over the elapsed time since the last clear.
    #
    # @return double.
    #
    method mean {} {
        if {$elapsed == 0} {
            return 0.0;                                # No elapsed time -> 0
        } else {
            return [expr {double($total)/$elapsed}];   # double forces real result.
        }
    }
    ##
    # stddev
    #   Get the standard deviation of the rate over the elapsed time since
    #   the last clear.
    #
    # @return double
    #
    method stddev {} {
        if {$samples < 2} {
            return 0.0
        } else {
            set mean [$self mean]
            
            return [expr {sqrt(double($sumOfSquares)/($samples-1) - $mean*$mean)}]
        }
        return 0.0
    }
    ##
    #  elapsed
    #    Return the elapsed time
    #  @return float - the elapsed time.
    #
    method elapsed {} {
        return $elapsed
    }
}