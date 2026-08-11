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
Ask the manager server to shutdown.
@file mg_shutdown.py
@brief Shutdown the manager server.
@author Ron Fox
'''
import sys
from nscldaq.manager_client import State, CONSTANTS


def usage() -> None:
    '''  print program usage to stderr.  '''
    print(f'''
Usage:
    $DAQBIN/mg_shutdown host user [service]
Request the manager server for the FRIB/NSCLDAQ managed experiment
environment to shutdown.

Where:
    host  - The host in which the server is running.
    user  - The username that started the server.
    service - optional service name the server advertisedf for its
            ReST service.  Defaults to {CONSTANTS.DEFAULT_MANAGER_REST_SERVICE}
            if omitted.
    ''', file=sys.stderr)

def main() -> int:
    ''' entry point '''
    
    if len(sys.argv) < 3:
        usage()
        return -1
    host = sys.argv[1]
    user = sys.argv[2]
    if len(sys.argv) == 4:
        service = sys.argv[3]
    elif len(sys.argv) == 3:
        service = CONSTANTS.DEFAULT_MANAGER_REST_SERVICE
    else:                              # Too many parameters.
        usage()
        return -1
    
    api = State(host, user, service)
    result = api.shutdown()
    if result['status'] != 'OK':
        print(f'Failed to shutdown the server: {result["message"]}')
        return -1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
    

