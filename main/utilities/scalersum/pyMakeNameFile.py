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
@file pyMakeNameFile.py
@brief Make a name file from a pyScaler toml definition file.
'''

import tomllib
import sys


def usage() -> None:
    # Print the program usage on stderr.
    
    print('Usage', file=sys.stderr)
    print('   $DAQBIN/pyMakeNameFile toml-def-file namefile', file=sys.stderr)
    print('Where:', file=sys.stderr)
    print('    toml-def-file - is the path to a TOML scaler definition file for pyScaler', file=sys.stderr)
    print('    namefile      - is the  path to a file into which will be written a scalersum namefile.')


def main() -> int:
    if len(sys.argv) != 3:
        usage()
        return -1
    
    infile  = sys.argv[1]
    outfile = sys.argv[2]
    
    # Parse the toml
    
    with open(infile, 'rb') as f:
        config = tomllib.load(f)
    
    # We only care about the data source items
    
    with open(outfile, 'w') as nf:
        
        for definition in config['datasource'].values():
            scalers = definition['scalers']
            sourceid = 0
            if 'sourceid' in definition:
                sourceid = definition['sourceid']
                
        for chan,name in enumerate(scalers):
            print(f'{sourceid} {chan} 32 {name}', file=nf)        
    
    return 0


if __name__ == '__main__':
    sys.exit(main())