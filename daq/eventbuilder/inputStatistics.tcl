#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

package require Tk
package require snit
package provide EVB::inputStatistics 1.0


#
# Define ::EVB::inputStatistics namespace.
#
namespace eval EVB {
    namespace eval inputStatistics {}
}

##
# @file inputStatistics.tcl
# @author Ron Fox
# @brief  Input file statistics display widgets.
#
# This file defines  megawidgets that can be used to display the
# event builder input stage statistics.  The package defines
# three widgets:
#   EVB::inputStatistics::summaryDisplay - summarize the data.
#   EVB::inputStatistics::queueDisplay   - Displays the data from one queue.
#   EVB::inputStatistics::statusDisplay  - Glues the previous widgets together
#                                          into a single page.
#------------------------------------------------------------------------------

##
# @class EVB::inputStatistics::summaryDisplay
#
# This snit::widget provides a display of the statistics
# summary data.  Specifically:
#    - The oldest timestamp in flight
#    - The newest timestamp in flight.
#    - The The number of input sources that have received data.
#    
#   These are laid out as follows:
#
# \verbatim
#   +-----------------------------------------------------+
#   | Total number of queued fragments: <fragmentcount>   |
#   | Oldest timestamp: <stamp>  Newest timestamp <stamp> |
#   +-----------------------------------------------------+
#
# \endverbatim
#
# OPTIONS:
#    - -fragments  Provides the total number of fragments.
#    - -oldest     Provides the oldest timestamp.
#    - -newest     Provides the newest timestamp.
#
# @note that all other options are delegated to the frame.
# @note all methods are also delegated to the frame.
#
snit::widget ::EVB::inputStatistics::summaryDisplay {
    component innerHull
    
    option -fragments  -default 0
    option -oldest     -default 0
    option -newest     -default 0
    
    
    delegate option * to innerHull
    delegate method * to innerHull
    
    constructor args {
        $self configurelist $args
        
        install innerHull using ttk::frame $win.hull
        
        #
        #  Define the widgets:
        #
        ttk::label $innerHull.fraglabel -text "Queued Fragments: "
        ttk::label $innerHull.oldlabel  -text "Oldest Timestamp: "
        ttk::label $innerHull.newlabel  -text "Newest Timestamp: "
        
        ttk::label $innerHull.fragments -textvariable ${selfns}::options(-fragments)
        ttk::label $innerHull.oldest    -textvariable ${selfns}::options(-oldest)
        ttk::label $innerHull.newest    -textvariable ${selfns}::options(-newest)
        
        # Layout the widgets
        #
        
        grid x $innerHull.fraglabel $innerHull.fragments
        
        grid $innerHull.oldlabel $innerHull.oldest $innerHull.newlabel $innerHull.newest
        grid $innerHull -sticky nsew
    }
}
##
# @class EVB::inputStatistics::queueDisplay
#
#    Create a display of the statistics for a single queue:
#
# OPTIONS:
#   - -id    - Queue source identifier.
#   - -depth - Queue depth
#   - -oldest - Timestamp at frontof queue.
#
# DELEGATIONS:
#   - all other options -> hull
#   - all methods -> hull.
# LAYOUT:
#  \verbatim
#
#   +------------------------------------------------+
#   |               Source ID: <id>                  |
#   |  Fragments: <depth>   Oldest: <timestamp>      |
#   +------------------------------------------------+
#
#  \endverbatim
#
snit::widget ::EVB::inputStatistics::queueDisplay {

    
    option -id     -default -1
    option -depth  -default 0
    option -oldest -default 0
    
    delegate option * to innerHull
    delegate method * to innerHull
    
    ##
    # constructor
    #
    #   process the args, build the UI.
    #
    # @param args - configuration option key/value pairs
    #
    constructor args {

        
        install innerHull using ttk::frame $win.hull
        set innerHull $win.hull
        
        # construct the UI elements.
        
        ttk::label $innerHull.idlabel    -text {Source ID: }
        ttk::label $innerHull.depthlabel -text {Fragments: }
        ttk::label $innerHull.oldlabel   -text {Oldest:    }
        
        ttk::label $innerHull.id     -textvariable ${selfns}::options(-id)
        ttk::label $innerHull.depth  -textvariable ${selfns}::options(-depth)
        ttk::label $innerHull.oldest -textvariable ${selfns}::options(-oldest)
        
        $self configurelist $args
        
        # Lay them out.
        
        grid  x $innerHull.idlabel  $innerHull.id
        grid $innerHull.depthlabel $innerHull.depth $innerHull.oldlabel $innerHull.oldest
        grid $innerHull -sticky nsew
    }
}
##
# @class EVB::inputStatistics::statusDisplay
#
#  This widget builds a full status display from a summary and zero or
#  more queue displays.  It is up to the caller to add and remove widgets
#  The widgets themselves are organized in a grid with  -columns across.
#  of queue displays with the summary centered over the top of them.
#  Each queue display is identified by its queue id.  Methods allow you to
#  - add a new queue element
#  - remove an existing queue element
#  - list the queue element displays that are already present.
#
#
#  OPTIONS:
#    - -fragments - current number of in flight fragments.
#    - -oldest    - Oldest timestamp.
#    - -newest    - Newest timestamp.
#    - -columns   - Columns in the grid of queue status info.
# METHODS:
#   - addDataSource    - adds a new data source, returns the widget.
#   - removeDataSource - Removes  data source from the grid..
#   - updateDataSource - Updates a data source display configuration
#   - getDataSource    - Gets the widget associated with  a data source.
#   - listSources      - Lists the ids of all displayed data sources.
#
# @note Data sources are laid out left to right top to bottom in
#       increasing order of their source ids.
#
snit::widget EVB::inputStatistics::statusDisplay {
    component summary;            # Summary widget.
    component queues;             # Frame containing individual queue statuses.
    
    option -columns  -default 1 -configuremethod configColumns
    
    delegate option -fragments to summary
    delegate option -oldest    to summary
    delegate option -newest    to summary
    
    delegate option * to queues
    
    variable sourceWidgets -array [list]

    #-----------------------------------------------------------------------
    #
    # Canonicals:
    
    ##
    # constructor
    #    Builds two frames.  The first frame contains a summary widget,
    #    the second frame is blank for now.  The bottom one will be used
    #    to grid in queue status objects as they are added.
    #
    # @param args - Option value pairs to configure the wdidges and us.
    #
    constructor args {
        #
        # Construct the components.
        #
        ttk::frame $win.summaryFrame
        install summary using ::EVB::inputStatistics::summaryDisplay \
            $win.summaryFrame.summary
        set summary $win.summaryFrame.summary
        
        install qeueues using ttk::frame $win.queueFrame -relief ridge -borderwidth 2
        set queues $win.queueFrame
        

        
        # Configure the component.
        
        $self configurelist $args;     # Configure us and the components
        
        # Layout the UI components.
        
        grid $win.summaryFrame.summary -sticky ew
        grid $win.summaryFrame         -sticky new
        grid $win.queueFrame           -sticky nsew
    }

    #------------------------------------------------------------------------
    #
    #  Option processing methods.
    
    ##
    #  configColumns
    #
    #   Called when the number of columns is changing.
    #   - Check the column is a legal integer parameter.
    #   - Check the column is >0
    #   - update options(-columns)
    #   - relayout the grid of data.
    #
    # @param option - Name of option being modified (-columns).
    # @param value  - Proposed new value.
    #
    method configColumns {option value} {
        if {![string is integer -strict $value]} {
            error "$option must be an integer"
        }
        if {$value <= 0} {
            error "$option must have at least one column"
        }
        
        set options($option) $value
        
        $self _LayoutQueueStatusGrid
    }
    #------------------------------------------------------------------------
    #  Public methods:
    #
    
    ##
    # addDataSource
    #
    #    Adds a new data source to the set of data source widgets displayed in
    #    the queues frame.
    #
    # @param id - Id of the source.
    # @param args - option/value list for the widget creation.
    #
    method addDataSource {id args} {
        
        # must be a new data source:
        
        if {$id in [array names sourceWidgets]} {
            error "Data source $id already has a widget on display"
        }
        # Create the new widget.  The new widget name will be
        # $win.queues.source$id .
        
        set newWidget [::EVB::inputStatistics::queueDisplay $queues.source$id \
                       -id $id -relief groove -borderwidth 2 {*}$args]
        
        #  figure out where it goes.  It's ranking is how many sources
        #  have smaller ids.
        #
        
        set ranking 0
        set sources [lsort -integer [array names sourceWidgets]];  # Sorted sources.
        #
        #  When this loop is done, sources only has the indices of higher rank.
        #  furthemore ranking says how many are in front of us.
        foreach sourceId $sources {
            if {$sourceId > $id} {
                break
            }
            incr ranking
            set sources [lrange $sources 1 end]
        }
        
        
        #  Tell the grid to forget the sources at higher rank than us:
        
        $self _forgetSources $sources
        
        # Add us to the widget array and prepend us tothe sources list:

        set sourceWidgets($id) $newWidget
        set sources [concat $id $sources]
        $self _regridSources $ranking $sources       
        
    }
    
    ##
    # removeDataSource
    #
    #  Remove an existing data source from plot.  All widgets after it in the
    #  gridding order get moved up to take its spot.
    #
    # @param id - Source id to remove.
    #
    method removeDataSource id {
        # Ensure the source is in the widget set:
        
        set sourceList [lsort -integer [array names sourceWidgets]]
        if {$id ni $sourceList} {
            error "Data source $id does not have a widget to remove."
        }
        # figure out the rank of this widget and remove
        # all the widgets that won't be moving from the front of the  list.
        #
        set rank [lsearch -exact $sourceList $id]
        set removeEnd [expr {$rank -1}];    # Index of last guy to remove..
        #
        #  If we are the first item nothing is removed:
        #
        if {$removeEnd >= 0} {
            set sourceList [lrange $sourceList 0 $removeEnd]
        }
        #  Forget the widgets remaining in sourceList, destroy our widget
        #  and remove it from the list too:
        
        $self _forgetSources $sourceList

        set sourceList [lrange 1 end]
        destroy $sourceWidgets($id)
        unset sourceWidgets($id)
        
        $self _regridSources $ranking $sourceList         
        
    }
    
    ##
    # updateDataSource
    #
    #  Updates the values in a data source.  The parameters are actually
    #  just passed to a reconfig of the widget.
    #
    # @param id   - Id of the data source.
    # @param args - Configuration option pairs.
    #
    method  updateDataSource {id args} {
        # Ensure this is a known source:
        
        if {$id ni [array names sourceWidgets]} {
            error "$id is not a data source we know anythingn about"
        }
        $sourceWidgets($id) configure {*}$args
    }
    
    ##
    # getDataSource
    #
    #  Return the widget associated with a data source.  This allows you to
    #  directly manipulate the apperance of that widget.  Do not destroy it under
    #  penalty of likel irrational behavior.
    #
    # @param id - source id.
    #
    method getDataSource id {
        # Ensure this is a known source:
        
        if {$id ni [array names sourceWidgets]} {
            error "$id is not a data source we know anythingn about"
        }
        return $sourceWidgets($id)        
    }
    
    ##
    # listSources
    #  List the data sources that are known to this 
    #  Widget manager.
    #
    #  @return list of integers.
    #
    method listSources {} {
        return [array names sourceWidgets]
    }
    #-------------------------------------------------------------------------
    #
    # Private methods (all start with _)
    
    
    ##
    # _regridSources
    # Regrid a list of widgets.
    #
    # @param ranking - where they belong in ranking
    # @param sourceListist - List of source Ids.
    #
    method _regridSources {ranking sourceList} {
        # now we just need to figure out where to start re-gridding widgets:
        
        set row [expr {int($ranking/$options(-columns))}]
        set column [expr {$ranking % $options(-columns)}]
       
        # And re-grid them:
        
        foreach source $sourceList {
            grid $sourceWidgets($source) -row $row -column $column
            incr column
            if {$column >= $options(-columns)} {
                incr row
                set column 0
            }
        }        
    }
    ##
    # _forgetSources
    # Forget the widgets that correspond to a set of sources:
    #
    # @param sourceList - list of data sources.
    #
    method _forgetSources sourceList {
        foreach source $sourceList {
            grid forget $sourceWidgets($source)
        }    
    }
    ##
    # _LayoutQueueStatusGrid
    #
    # Normally called when the grid configuration changtes.
    # forget all the sources and relayout:
    #
    method _LayoutQueueStatusGrid {} {
        set sourceList [lsort -integer [array names  sourceWidgets]]
        $self _forgetSources $sourceList
        $self _regridSources 0 $sourceList
    }
}