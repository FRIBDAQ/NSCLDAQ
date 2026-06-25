'''
  Provide common utilities for readout ReST client programs.
  
  @file rdo_utils.py
  @brief  Useful utility functions factored out of Readout ReST clients.
  @author Ron Fox
'''
from nscldaq.manager_client import Programs
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

def getReadoutHost(mgr_host: str, user: str, service: str, name: str) -> str:
    '''
        Get the host in which the named program is running:
        @param mgr_host - where the manager is running.
        @param user     - user running the DAQ
        @param service  - mgr service.
        @param name     - name of program to look up.
    '''
    client = Programs(mgr_host, user, service)
    info = client.status()
    if info['status'] != 'OK':
        raise RuntimeError('Failed to fetch program status from server', info['message'])    

    for program in info['programs']:
        if program['name'] == name:
            return program['host']
    
    # Not found:
    
    raise IndexError(f'There is no program named {name}')
