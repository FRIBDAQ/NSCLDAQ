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
 This program starts the experiment manager server.

@file mg_config.py
@brief Starter for the experiment manager server.
@author Ron Fox
'''

import pathlib
import sys
import os
import subprocess



def usage() -> None:
    ''' output program usage to stderr'''
    print('''
Usage:
    $DAQBIN/mg_startManager [config_file_path]
Where:
    config_file_path  - is the optional path to the configuration database
                        file. 
Notes:
*   If the DAQ_EXPCONFIG evironment variable is defined, it is used as the
    configuration file path.  In that case, it is an error for the config_file_path
    to be supplied.
    
*   The default service advertised by the manager will be 'DAQManager' but if
    the environment variable 'DAQ_MGRSERVICE' is define  that will be used instead.
    This is not recommended.
        ''', file = sys.stderr)

def main() -> int:
    '''  program entry point. '''
    service = 'DAQManager'    # Default servicename.
    if 'DAQ_MGRSERVICE' in os.environ:
        service = os.environ['DAQ_MGRSERVICE']
        
    # Figure out the database file:
    # The environment variable overrides the inline file.
    
    if 'DAQ_EXPCONFIG' in os.environ:
        if len(sys.argv) != 1:
            print(
                '''You cannot supply the database file if 'DAQ_EXPCONFIG' is defined''', 
                file=sys.stderr
            )
            usage()
            return -1
    elif len(sys.argv) == 2:
        os.environ['DAQ_EXPCONFIG'] = sys.argv[1]
    else:
        usage()
        return -1

if __name__ == '__main__':
    sys.exit(main())