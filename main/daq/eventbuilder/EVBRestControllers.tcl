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
# @file EVBRestControllers.tcl
# @brief Bridges between the model of EVBRestClient and views of EVBRestUI
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}

package provide EVBRestControllers 1.0
package require snit
package require EVBRestUI
package require EVBRESTClient

##
#  Note that these controllers are, for the most part, trivial bundlings
#  of the model, an EVBRestClient, and a view from EVBRestUI.
    

##
# @class EVBInputStatsController
#
# OPTIONS:
#    -model - RestClient object.
#    -view  - InputStatsView object.
# METHODS:
#   update - updates the display.
#
snit::type EVBInputStatsController {
    option -model
    option -view
    
    constructor args {
        $self configurelist $args
    }
    
    method update {} {
        $options(-view) configure -inputstats [$options(-model) inputstats]
    }
}
##
# @class EVBQueueStatsController
#
#  Bridges between an EVBRestClient object and a QueueStatsView object.
#
# OPTIONS
#   -model  - EVBRestClient object.
#   -view   - QueueStatsView object.
# METHODS:
#   update - updates the view from the model.
#
snit::type EVBQueueStatsController {
    option -model
    option -view
    
    constructor args {
        $self configurelist $args
    }
    method update {} {
        $options(-view) configure -queuestats [$options(-model) queuestats]
    }
}
##
# @class EVBBarrierStatsController
#
#   Controller to mediate between an EVBRestClient object as a model and
#   a BarrierStatsView view.
#
# OPTIONS
#   -model   - EVBRestClient object.
#   -view    - BarrierStatsView objecdt.
# METHODS
#   update - updates the model from the view.
#
snit::type EVBBarrierStatsController {
    option -model
    option -view
    
    constructor args {
        $self configurelist $args
    }
    method update {} {
        $options(-view) configure -barrierstats [$options(-model) barrierstats]
    }
}
##
# @class EVBCompleteBarrierController
#   Controller to mediate between an EVBRestClient object as a model and
#   a CompletBarrierView view.
#
# OPTIONS
#   -model
#   -view
# METHODS
#   update
#
snit::type EVBCompleteBarrierController {
    option -model
    option -view
    
    constructor args {
        $self configurelist $args
    }
    method update {} {
        $options(-view) configure \
            -completebarrierdetails [$options(-model) completebarrierdetails]
    }
}
##
# @class EVBIncompleteBarrierController
#   Mediates between an EVBRestClient model and an IncompleteBarrierView
#   presentation.
#
# OPTIONS
#   -model
#   -view
# METHODS
#   update
#
snit::type EVBIncompleteBarrierController {
    option -model
    option -view
    
    constructor args {
        $self configurelist $args
    }
    method update {} {
        $options(-view) configure \
            -incompletedetails [$options(-model) incompletebarrierdetails]
    }
}
##
# @class EVBDataLateController
#    Mediates between an EVBRestClient model and a DataLateView presentation
#    of that data.
#
# OPTIONS
#   -model
#   -view
# METHODS
#    update
#
snit::type EVBDataLateController {
    option -model
    option -view
    
    constructor args {
        $self configurelist $args
    
    }
    method update {} {
        $options(-view) configure \
            -datalatestatistics [$options(-model) datalatestatistics]
    }
}
##
# @class EVBOutOfOrderController
#    Mediates between an EVBRestClient model and an OutOfOrderView presentation.
# OPTIONS
#   -model
#   -view
# METHODS
#   update
#
snit::type EVBOutOfOrderController {
    option -model
    option -view
    
    constructor args {
        $self configurelist $args
    }
    method update {} {
        $options(-view) configure -oostatistics [$options(-model) oostatistics]
    }
}
##
# @class EVBConnectionController
#  Mediates between an EVBRestClient model and a ConnectionView presentation
#
# OPTIONS
#   -model
#   -view
# METHODS
#   update
#
snit::type EVBConnectionController {
    option -model
    option -view
    
    constructor {args} {
        $self configurelist $args
    }
    
    method update {} {
        $options(-view) configure \
            -connections [dict get [$options(-model) connections] connections]
    }
}

##
# @class EVBFlowController
#    Mediate between an EVBRestClient model and a FlowControlVew presentation.
#
# OPTIONS
#    -model
#    -view
# METHODS
#    update
#
snit::type EVBFlowController {
    option -model
    option -view
    
    constructor {args} {
        $self configurelist $args
    }
    method update {} {
        $options(-view) configure -flowcontrol [$options(-model) flowcontrol]
    }
}