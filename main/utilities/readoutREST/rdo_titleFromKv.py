#!/usr/bin/env python3

'''
    little program sets the run title from the 'title' key in the
    Key value data store.
    
    @file rdo_titleFromKv.py
    @brief set readout title from the key value store.
    @author Ron Fox.

'''


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

import sys
import os
from nscldaq.manager_client import  KVStore
from nscldaq.manager_client import CONSTANTS as MGR_CONSTS
from nscldaq.readoutREST.readoutRestClient import ReadoutClient
from nscldaq.readoutREST.rdo_utils import getReadoutHost, CONSTANTS
def usage() -> None:
    '''
        Print program usage on stderr.
    '''
    
    print('Usage',  file=sys.stderr)
    print('   $DAQBIN/rdo_titleFromKv host user program_name [service]', file=sys.stderr)
    print('Where:', file=sys.stderr)
    print('   host  - Is the host running the manager server', file=sys.stderr)
    print('   user  - Is the user running the manager server', file=sys.stderr)
    print('   program_name - the name of the Readout program whose run number will be set.', file=sys.stderr)
    print('   service - if provided is the service on which the manager server is running', file=sys.stderr)
    print(f'             Defaults to "{MGR_CONSTS.DEFAULT_MANAGER_REST_SERVICE}" If not given', file=sys.stderr)
    print('Note:', file=sys.stderr)
    print(f'   The Readout program is assumed to be advertising the "{CONSTANTS.DEFAULT_READOUT_REST_SERVICE}" service unless', file=sys.stderr)
    print('   The "SERVICE_NAME" environament variable is defined, in which case', file=sys.stderr)
    print('   the value of that environment variable is used instead.', file=sys.stderr)
    


def getTitle(mgr_host : str, user: str, service: str) -> str:
    '''
        Get the run number from the experiment's key value store:
        
        @param mgr_host - host in which the manager is running.
        @param user     - user running the manager.
        @param service  - manager service name.
    '''
    client = KVStore(mgr_host, user, service)
    return client.title()
    
def main():
    # Marshall the parameters:
    
    # First the mandatory ones:
    
    if len(sys.argv) < 4:
        usage()
        sys.exit(-1)
    mgr_host = sys.argv[1]
    mgr_user = sys.argv[2]
    readout_name = sys.argv[3]
    
    # Now the optional one:
    
    mgr_service = MGR_CONSTS.DEFAULT_MANAGER_REST_SERVICE
    if len(sys.argv) == 5:
        mgr_service = sys.argv[4]
    elif len(sys.argv) > 5:
        usage()
        sys.exit(-1)
        
    # Now figure out the Readout service:
    
    readout_service = CONSTANTS.DEFAULT_READOUT_REST_SERVICE
    if 'SERVICE_NAME' in os.environ:
        readout_service = os.environ['SERVICE_NAME']
        
    # Get the host in which the Readout program is running (user will be the same as the manager)
    # and get the run number we need to set:
    
    readout_host = getReadoutHost(mgr_host, mgr_user, mgr_service, readout_name)
    title          = getTitle(mgr_host, mgr_user, mgr_service)
    
    # Now we have everything we need Readout was run by the manager
    # so it uses the same username:
    
    client = ReadoutClient(readout_host, readout_service, mgr_user)
    client.setTitle(title)
    exit(0)


if __name__ == "__main__":
    main()