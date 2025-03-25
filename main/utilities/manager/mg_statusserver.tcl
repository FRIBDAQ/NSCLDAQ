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
# @file   mg_statusserver.tcl
# @brief  REST server for /Status domain
# @author Ron Fox <fox@nscl.msu.edu>
#

lappend auto_path $::env(DAQTCLLIBS)
package require json::write
package require sqlite3
package require sequence
package require restutils

# Register a handler for the /Status domain:

Url_PrefixInstall /State [list stateHandler]


proc _TransitionEnded {db mgr how} {
    
    set ::Done $how
}
set tty [open info.log w]
fconfigure $tty -buffering line


##
# _PrepareManagerShutdown
#   - Called for /shtudown suffix.
#     *   Require POST
#     *   Require username.
#     *   If not shutdown, make stransition to shutdown state.
#     *   Return a status OK and no message field.
#     *   Schedule an exit in one second to ensure the
#         returned value has been sent.
#
# @param sock - the socket that defines the state of the connection.
#
proc _PrepareManagerShutdown {sock} {
    #  Require post or error.
    
    if {[GetRequestType $sock] ne "POST"} {
        Doc_Error $sock ".../shutdown must be invoked via a POST method"
        return;
    }
    # Require the user was supplied:
    
    set postdata [GetPostedData $sock]
    if {![dict exists $postdata user]} {
        Doc_Error $sock ".../shutdown requires an invoking user be sent"
        return
    }
    #   If the current state is not already SHUTDOWN do a synchronous
    #   shutdown to cleanup all programs;
    
    if {[::sequence::currentState db] ne "SHUTDOWN"} {
        ::sequence::transition db SHUTDOWN [list _TransitionEnded]
        vwait ::Done
    }
    
    ##  Return the success and schedule exit to hapen in one second.
    
    Httpd_ReturnData $sock application/json [json::write object   \
        status [json::write string OK]                            \
        message [json::write string ""]                           \
    ]
    after 1000 exit
}


##
# stateHandler
#   Handles domain requests in the /State domain.
#
# @param sock - socket connected back to the client.
# @param suffix - The rest of the URI :
#                 /status - return current state
#                 /allowed - return allowed next states.
#                 /transition - post a request to transition.
#                 /shutdown - shuts down the server.
#                 /elapsed  -Elapsed run time to display.
proc stateHandler {sock suffix} {

    # These make reading our JSON by a user easier.

    json::write indented 1
    json::write align    1
    
    if {$suffix eq "/status"} {
        
        set state [::sequence::currentState db]
        
        
        Httpd_ReturnData $sock application/json [json::write object   \
            status [json::write string OK]                            \
            message [json::write string ""]                           \
            state   [json::write string $state]                       \
        ]
    } elseif {$suffix eq "/allowed"} {
        
        set next [::sequence::listLegalNextStates db]
        
        
        set resultStrings [list]
        foreach state $next {
            lappend resultStrings [json::write string $state]
        }
        Httpd_ReturnData $sock application/json [json::write object    \
            status [json::write string OK]                             \
            message [json::write string ""]                            \
            states [json::write array {*}$resultStrings]                  \
        ]
    } elseif {$suffix eq "/elapsed"} {
        set state [::sequence::currentState db]

        # IF the state is BEGIN then sequence::runSeconds gives what we need:

        if {$state eq "BEGIN"} {
            if {[catch {::sequence::runSeconds db} secs]} {
                set elapsedText "Failed: $secs"
            } else {
                set hrs [expr {int($secs / 3600)}]
                set minsec [expr {$secs % 3600}]
                set min [expr {int ($minsec/60)}]
                set sec [expr {$minsec % 60}]
                set elapsedText [format "%d:%02d:%02d" $hrs $min $sec]

            }
            Httpd_ReturnData $sock application/json [json::write object \
                status [json::write string OK]                          \
                elapsed [json::write string $elapsedText]               \
            ]
        } else {
            Httpd_ReturnData $sock application/json [json::write object \
                status [json::write string OK]                          \
                elapsed [json::write string *Inactive*]                 \
            ]
        }
    } elseif {$suffix eq "/transition"} {
        
        
        
        if {[GetRequestType $sock] ne "POST"} {
            Doc_Error $sock ".../transition must be invoked via a POST method"
            return;                          # In case Doc_Error returns.
        } else {
            
            set postdata [GetPostedData $sock]
            
            if {(![dict exists $postdata user]) || (![dict exists $postdata state])} {
                Doc_Error $sock "Both 'user' and 'state' are necessary POST parameters with ../transition"
                return
            }
            set user [dict get $postdata user]
            set state [dict get $postdata state]
            
            # TODO: Check that user has the role they claim to have:
            
            #  Try to set the new state:

            # Only do a transition if the state changes:

            if {[::sequence::currentState db] eq $state} {
                 Httpd_ReturnData $sock application/json [json::write object \
                    status [json::write string OK]                          \
                    state  [json::write string $state]                   \
                    completed [json::write string OK]
                ]
                return

            }
            
            
            set status [catch \
                {::sequence::transition db $state [list _TransitionEnded]} msg]
            if {$status} {
                # Failed:
                
                
                ErrorReturn $sock "$msg : $::errorInfo "
            } else {
                # Success
                vwait ::Done
                
                set newstate [::sequence::currentState db]
                
                Httpd_ReturnData $sock application/json [json::write object \
                    status [json::write string OK]                          \
                    state  [json::write string $newstate]                   \
                    completed [json::write string $::Done]
                ]
            }
            
        }
    } elseif {$suffix eq "/shutdown"} {;             # Shutdown the manager:
        _PrepareManagerShutdown $sock;             # Make ready to shutdown.
        
        
    } else {
        ErrorReturn $sock "$suffix subcommand not implemented"
    }
}
set dbFile $::env(DAQ_EXPCONFIG)
sqlite3 db $dbFile