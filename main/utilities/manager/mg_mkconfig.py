#!/bin/env python3
'''
  Replacement for mg_mkconfi.tcl - create the initial database configuration for a managed 
  environment experiment.
  
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

from nscldaq.mg_database import make_schema
import sqlite3
import sys


usage_text = '''
Usage:
    $DAQBIN/mg_mkconfig database-path

Create a managed experiment configuration database.

Where:
    database-path is the path to the databse file that will be created.
'''
def Usage() :
    global usage_text
    print(usage_text, file= sys.stderr)
# Need a database file name
if len(sys.argv) != 2:
    Usage()
    sys.exit(-1)

def main():    
    connection = sqlite3.connect(sys.argv[1])
    make_schema(connection)
    
if __name__ == '__main__':
    main()
    sys.exit(0)
    

