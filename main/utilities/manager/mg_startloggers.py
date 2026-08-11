#!/usr/bin/env python3

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014, 2026
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#	     FRIB
#	     Michigan State University
#	     East Lansing, MI 48824-1321


'''
This program replaces mg_startloggers.tcl.  We add the ability to specify the service
the manager has advertised.

@file mg_startloggers.py
@brief Start all applicable event loggers.
@author Ron Fox
'''

from nscldaq.manager_client import Logger, CONSTANTS
import sys

def usage() -> None:
    ''' Print program usage to stderr.'''
    print(f'''
Usage:
    $DAQBIN/mg_startloggers host user [service-name]

Where:
    host     - Is the host in which the manager is running.
    user     - Is the user that started the manager.
    sevice-name - is an optional service name the manager is
               advertising for its ReST server. IF not specified,
               this will default to {CONSTANTS.DEFAULT_MANAGER_REST_SERVICE}
               which is the default service the manager advetises.

Purpose:
    Start the applicable event loggers.  Note this normally is run
    from a sequence that responds to the BEGIN transition prior to 
    actually telling the readouts to start.

        ''', file = sys.stderr)



def main() -> int:
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        usage()
        return -1
    host = sys.argv[1]
    user = sys.argv[2]
    service= CONSTANTS.DEFAULT_MANAGER_REST_SERVICE
    
    if len(sys.argv) == 4:
        service = sys.argv[3]                # User supplied the service.

    
    try:
        api = Logger(host, user, service)
        api.start()
    except Exception as e:
        print(f'Failed to start the loggers: {e}', file=sys.stderr)
        return -1
     
    return 0
   
if __name__ == '__main__':
    sys.exit(main())