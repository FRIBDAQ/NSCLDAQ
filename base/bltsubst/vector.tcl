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

##
# @file vector.tcl
# @brief Provides a subset of blt::vector

package provide blt::vector 1.0
package require snit

namespace eval blt {
    
}

##
# @class blt::vector
#
#   This snit object provides a subset of the capabilities of
#   the blt::vector package.
#
#  OPTIONS:
#  -tracewrite - Script to execute if the vector is modified.
#                script is passed a single parameter, the vector object name.
#                The following methods fire  traces:
#                *   edelete
#                *   append
#                *   setelement
# METHODS:
#
#   create   - Create a new vector
#   delete  - Delete an element 
#   append  - Append to a vector.
#   length  - Get the length of a vector
#
# The following methods are used to compensate for syntactical sugar that
# BLT can offer that pure Tcl can't at this time:
#
#   index      - Return the element at a specific index.
#   setelement - Set a specific element of the array.
#   range      - Return a range of elements as a Tcl list.
#
snit::type blt::vector {
    option -tracewrite [list]

    variable contents [list];   #contains the vector contents tcl list format.

    #--------------------------------------------------------------------------
    #
    # Public methods:
    
    ##
    # constructor
    #
    #  Construct a new vector
    #
    # @param args - Option/value pairs for initial configuration.
    #
    constructor args {
        $self configurelist $args
        
    }
    
    ##
    # delete
    #
    #  Deletes elements from the array
    #
    # @param args - list of element numbers to remove.
    #               note that the final compaction happens after all elements are
    #               deleted.
    #
    method delete args {
        set indices $args
        
        #  Sort indices in decending order so that we don't have to worry
        #  about delete order:
        
        set indices [lsort -decreasing -integer $indices]
        
        foreach index $indices {
            set contents [lreplace $contents $index $index]
        }
        $self _trace
    }
    ##
    # append
    #   append one or more elements to the end of the array.
    #
    # @param args - The elements to append.
    #
    method append args {
        foreach element $args {
            lappend contents $element
        }
        $self _trace
    }
    ##
    # length
    #
    #   Return the number of elements in the array.
    #
    # @return integer - Count of the elements.
    #
    method length {} {
        return [llength $contents]
    }
    ##
    # index
    #
    #  Return an element at the specified index
    #
    # @param i - The index.
    #
    method index i {
        if {$i < [llength $contents]} {
            return [lindex $contents $i]
        } else {
            error "$i is not in the array length($contents)"
        }
    }
    ##
    # set
    #   Sets an element to a value.
    #
    # @param i - index into the array of the item to replace.
    # @param value - New value for the element.
    #
    method set {i value} {
        set contents [lreplace $contents $i $i $value]
        $self _trace
    }
    ##
    # range
    #
    #  Return a subset of the elements as a TclList:
    #
    # @param first - First index, defaults to 0.
    # @param last  - Last index, defaults to end.
    #
    method range {{first 0} {last end}} {
        return [lrange $contents $first $last]
    }


    #--------------------------------------------------------------------------
    #
    #  Private methods
    #
    
    
    
    ##
    # _trace
    #
    #   Called by operations that fire the trace.
    #   If the option(-tracewrite) is nonempty it is dispatched to at the
    #   global level passing our name to it.
    #
    method _trace {} {
        set script $options(-tracewrite)

        if {$script ne ""} {
            uplevel #0 $script $self
        }
        
    }

}