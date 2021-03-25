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
# @file  mg_authedit.tcl
# @brief Editor for authorization database.
# @author Ron Fox <fox@nscl.msu.edu>
#

set libs [file normalize [file join  [file dirname [info script]] .. TclLibs]]
lappend auto_path $libs      

package require Tk
package require auth
package require snit
package require selectablelist 
package require dialogwrapper

#-----------------------------------------------------------------------------
#  Widgets

##
# @class UsersAndRoles
#    This is a megawidget that is a treeview that shows a list of the users and,
#    if a user is expanded, the roles that user currently has.
#
# OPTIONS
#    -data   - Results of auth::listall  that sets the contents of the treeview
#   -onuserb3    - Script for right button click in a user.
#   -ongrantb3   - Script for right button in a granted role for a user.
#
# METHODS:
#    addUser   - Add a new user to the tree.
#    grant     - Grant a role to a user.
#    revoke    - Revoke a role from a user.
#
snit::widgetadaptor UsersAndRoles {
    component tree
    
    option -data -default [list] -configuremethod _loadData -cgetmethod _cgetData
    option -onuserb3
    option -ongrantb3
    
    constructor args {
        installhull using ttk::frame
        
        install tree using ttk::treeview $win.tree  \
            -columns [list role]  -show [list tree headings] \
            -displaycolumns #all -selectmode browse \
            -yscrollcommand [list $win.ysb set]
        ttk::scrollbar $win.ysb -command [list $tree yview] -orient vertical
        foreach title [list "User name" "Role"] column [list #0  role] {
            $tree heading $column -text $title
        }
    
        
        grid $tree $win.ysb -sticky nsew
        
        # Make user and role tags and bind b3 to the dispatchers
        # for the -onxxxxb3 scripts.
        #
        
        $tree tag add user [list]
        $tree tag add role [list]
        
        $tree tag bind user <Button-3> [mymethod _dispatchUserMb3 %x %y]
        
        $self configurelist $args
    }
    #----------------------------------------------------------------------
    # Configuration management methods
    
    ##
    # _loadData
    #   Loads new data into the tree.
    #
    # @param optname - option name (ignored).
    # @param value   - dict from e.g. auth::listAll.
    #
    method _loadData {optname value} {
        $tree delete [$tree children {}];    # Clear the tree.
        
        # We'll alphabetize both the keys (users) and the roles.
        
        foreach user [lsort -increasing [dict keys $value]] {
            set userId [$tree insert {} end -text $user -tags user]
            set roles [dict get $value $user]
            foreach role [lsort -increasing $roles] {
                $tree insert $userId  end -values [list $role] -tags role
            }
        }        
    }
    ##
    # _cgetData
    #   Returns the data from the treeview in the form of the same dict
    #   that created it.
    #
    # @param optname  - option name - ignored.
    method _cgetData {optname} {
        set result [dict create]
        set userIds [$tree children {}];     # These are entry ids of users.
        foreach id $userIds {
            set username [$tree item $id -text]
            dict set result $username [list]
            set roleIds [$tree children $id];   # Ids of granted roles
            foreach rid $roleIds {
                dict lappend result $username [$tree item $rid -values]
            }
        }
        return $result
    }
    #---------------------------------------------------------------------
    # Event handling
    
    ##
    # _dispatchRoleMb3
    #    Dispatches a context menu callback on a right click on a user.
    #
    # @param x,y - window coordinates of the click.
    method _dispatchUserMb3 {x y} {
        
        set userScript $options(-onuserb3)
        if {$userScript ne ""} {
            set id [$tree identify item $x $y]
            set user [$tree item $id -text]
            lappend userScript $user
            uplevel #0 $userScript
        }
    }
}
    

    

    

