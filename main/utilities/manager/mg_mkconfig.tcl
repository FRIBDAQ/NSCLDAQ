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
# @file mg_mkconfig.tcl
# @brief Program to make a configuration database.
# @author Ron Fox <fox@nscl.msu.edu>
#
package require sqlite3

##
#  mg_mkconfig.tcl
#    Usage:
#       $DAQBIN/mg_mkconfig  database-path
#    Where
#       database-path is the file system path to a database to make.
# @note This is being written so that running it on an existing configuration
#       database is mostly harmless.  
#

##
# Usage
#   Print an error message and the usage above to stderr before exiting
#   with a failure code.
#
# @param msg - the messsage to exit with.
#
proc Usage {msg} {
    puts stderr $msg
    puts stderr "Usage"
    puts stderr "   \$DAQBIN/mg_mkconfig database-path"
    puts stderr "Where:"
    puts stderr "   database-path is filename into which the datbase is created."
    exit -1
}


#------------------------------------------------------------------------------
#  The database schema is divided into several subschema.  To hide information
#  from the main part of the script, we'll have a proc for each subschema:
#  Each of these procs creates the schema for that part of the containe and
#  takes a single parameter:  the database command.


##
# Containers table:
#
proc containers {db} {
    $db eval {
        CREATE TABLE IF NOT EXISTS container  (
            id         INTEGER PRIMARY KEY,
            container  TEXT,
            image_path TEXT,
            init_script TEXT
        )
        
    }
    
    $db eval {
        CREATE TABLE IF NOT EXISTS bindpoint (
            id            INTEGER PRIMARY KEY,
            container_id  INTEGER,    -- FK to container
            path          TEXT,
            mountpoint    TEXT DEFAULT NULL
        )
    }
}


##
# Programs tables
#
proc programs {db} {
    $db eval {
        CREATE TABLE IF NOT EXISTS program_type (
            id                INTEGER PRIMARY KEY,
            type              TEXT
        )
    }
    ##
    #  Only stock the table if it's not already stocked:
    #
    set count [$db eval {
        SELECT COUNT(*) FROM program_type
    } ]
    
    if {$count == 0} {
        $db eval {
            INSERT INTO program_type (type)
                VALUES ('Transitory'), ('Critical'), ('Persistent')
        }
    }
    
    $db eval {
        CREATE TABLE IF NOT EXISTS program (
            id           INTEGER PRIMARY KEY,
            name         TEXT,      -- Name used to refer to the program.
            path         TEXT,
            type_id      INTEGER, -- FK to program_type
            host         TEXT,
            directory    TEXT,
            container_id INTEGER, -- FK to container
            initscript   TEXT,
            service      TEXT
        )
    }
    
    $db eval {
            CREATE TABLE IF NOT EXISTS program_option (
                id          INTEGER PRIMARY KEY,
                program_id  INTEGER,  -- FK to program
                option      TEXT,
                value       TEXT
            )
    }
    
    $db eval {
        CREATE TABLE IF NOT EXISTS program_parameter (
            id             INTEGER PRIMARY KEY,
            program_id     INTEGER,   -- FK to program.
            parameter      TEXT
        )
    }
    $db eval {
            CREATE TABLE IF NOT EXISTS program_environment (
                id         INTEGER PRIMARY KEY,
                program_id INTEGER,
                name       TEXT,
                value      TEXT
            )
    }
}
##
# Sequences tables
#   Note - we set the lasttransition table initially to
#          contain the SHUTDOWN value since we start up needing to be
#          booted up.

## - utility to translate a state name:

proc stateId {db name} {
    set id [$db eval {SELECT id FROM transition_name WHERE name = $name}]
    if {[llength $id] != 1} {
        error "state_transition $name got invalid stateId result: $id"
    }
    return $id
}

proc sequences {db} {
    #
    #  Array indexed by from state showing the legal to states.
    #
    array set legalTransition [list                                   \
        BOOT [list SHUTDOWN HWINIT]                                   \
        SHUTDOWN [list SHUTDOWN BOOT]                                 \
        HWINIT [list SHUTDOWN BEGIN]                                  \
        BEGIN  [list SHUTDOWN END]                                    \
        END    [list SHUTDOWN BEGIN HWINIT]                           \

    ]
    
    $db eval {
        CREATE TABLE IF NOT EXISTS sequence (
            id                INTEGER PRIMARY KEY,
            name              TEXT,
            transition_id     INTEGER -- FK to transition triggering seq.
                
            
        )
    }
    
    $db eval {
        CREATE TABLE IF NOT EXISTS transition_name (
            id        INTEGER PRIMARY KEY,
            name      TEXT
        )
    }
    
    
    $db eval {
        CREATE TABLE IF NOT EXISTS legal_transition (
            id              INTEGER PRIMARY KEY,
            from_id         INTEGER,   -- FK in to transition_name
            to_id           INTEGER    -- Also FK into transition_name
        )
    }
    $db eval {
        CREATE TABLE IF NOT EXISTS last_transition (
            state           INTEGER     -- FK to transition_name
        )
    }
    
    $db eval {
        CREATE TABLE IF NOT EXISTS step (
            id                       INTEGER PRIMARY KEY,
            sequence_id              INTEGER, -- fk to sequence
            step                     REAL,
            program_id               INTEGER, -- fk to program table.
            predelay                 INTEGER DEFAULT 0,
            postdelay                INTEGER DEFAULT 0
        )
    }
    
    # Stock transition_name if it does not already have anything:
    
    if {[$db eval {SELECT COUNT(*) FROM transition_name}] == 0} {
        $db eval {
            INSERT INTO transition_name (name)
                VALUES ('BOOT'), ('SHUTDOWN'), ('HWINIT'),
                       ('BEGIN'), ('END')
        }
    }
    
    
    #
    #  Start out with one sequence (at least) named run_state:
    #
    if {[$db eval {SELECT COUNT(*) FROM sequence WHERE name='run_state'}] == 0} {
        $db eval {
            INSERT INTO sequence (name) VALUES ('run_state')
        }
    }
    # Stock the legal_transition
    if {[$db eval {SELECT COUNT(*) FROM legal_transition}] == 0} {
        foreach from [array names legalTransition] {
            set fromId [stateId $db $from]
            foreach to $legalTransition($from) {
                set toId [stateId $db $to]
                $db eval {INSERT INTO legal_transition
                    ( from_id, to_id)
                    VALUES ($fromId, $toId)
                }
            }
        }
    }
    if {[$db eval {SELECT COUNT(*) FROM last_transition }] == 0} {
        set sid [stateId $db SHUTDOWN]
        $db eval {
            INSERT INTO last_transition (state) VALUES ($sid)
        }
    }

    #  Transition log table:

    $db eval {
        CREATE TABLE IF NOT EXISTS transition_log  (
            id            INTEGER PRIMARY KEY,
            transition_id INTEGER,  -- FK to transition_name
            timestamp     INTEGER,
            success       INTEGER
        )
    }
}

#  Event logging database:
#

proc event_log {db} {
    $db eval {
        CREATE TABLE IF NOT EXISTS logger (
            id                 INTEGER PRIMARY KEY,
            daqroot            TEXT,
            ring               TEXT,
            host               TEXT,
            partial            INTEGER DEFAULT 0,
            destination        TEXT,
            critical           INTEGER DEFAULT 1,
            enabled            INTEGER DEFAULT 1,
            container_id       INTEGER DEFAULT NULL -- FK to container tbl.
        )
    }
    
    $db eval {
        CREATE TABLE IF NOT EXISTS recording (
            state     INTEGER
        )
    }
    if {[$db eval {SELECT COUNT(*) FROM recording}] == 0} {
        $db eval {INSERT INTO recording (state) VALUES (0)}
    }
}

##
# Key value store
#   - have to figure out what we need there.
#
proc kvstore {db} {
    $db eval {
        CREATE TABLE IF NOT EXISTS kvstore (
            id        INTEGER PRIMARY KEY,
            keyname   TEXT,
            value     TEXT
        )
    }
    #  Create mandatory keys if they don't already exist:
    
    set n [$db eval {
        SELECT COUNT(*) FROM kvstore WHERE keyname='title'
    }]
    if {$n == 0} {
        $db eval {
            INSERT INTO kvstore (keyname, value) VALUES ('title', 'Set a new title')
        }
    }
    set n [$db eval {
        SELECT COUNT(*) FROM kvstore WHERE keyname='run'
    }]
    if {$n == 0} {
        $db eval {
            INSERT INTO kvstore (keyname, value) VALUES ('run', '0')
        }
    }
}

##
# usersandroles
#    Tables of users, roles and role membership.
#
# @param db - database.
#
proc usersandroles {db} {
    
    # User names.
    
    $db eval {
        CREATE TABLE IF NOT EXISTS users (
            id        INTEGER PRIMARY KEY,
            username  TEXT
        )
    }
    #  Role names
    
    $db eval {
        CREATE TABLE IF NOT EXISTS roles (
            id       INTEGER PRIMARY KEY,
            role     TEXT
        )
    }
    #  Join table between the users and the roles they have.
    
    $db eval {
        CREATE TABLE IF NOT EXISTS user_roles (
            user_id    INTEGER,
            role_id    INTEGER
        )
    }
}

#-------------------------------------------------------------------------------
# Entry point
#

if {[llength $argv] != 1} {
    Usage "Incorrect number of command line parameters"
}

set dbname [lindex $argv 0]
sqlite3 db $dbname -create 1

foreach schema [list containers programs sequences event_log kvstore usersandroles] {
    $schema db
}

db close