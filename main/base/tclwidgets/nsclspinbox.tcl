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
# @file nsclspinbox.tcl
# @brief Spinbox that fires commands on <FocusOut> and Enter key.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide nsclspinbox 1.0

package require Tk
package require snit



snit::widgetadaptor nscl::spinbox {
    delegate option * to hull
    delegate method * to hull
    
    constructor args {
        installhull using ttk::spinbox
        $self configurelist $args
        
        bind $win <Key-Return> {eval [%W cget -command]}
        bind $win <FocusOut>   {eval [%W cget -command]}
    }
}