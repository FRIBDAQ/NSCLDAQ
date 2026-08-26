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
@file lg_kvstore.py
@brief command level access to the key value store.
@author Ron Fox
'''
import sys
from nscldaq.LogBook import logbookadmin
from nscldaq.LogBook import LogBook

def usage():
    # Print the program usage:
    print('Usage:', file=sys.stderr)
    print('    $DAQBIN/lg_kvstore exists key', file=sys.stderr)
    print('    $DAQBIN/lg_kvsotre get key', file=sys.stderr)
    print('    $DAQBIN/lg_kvstore set key value', file=sys.stderr)
    print('    $DAQBIN/lg_kvstgore crate key value', file=sys.stderr)
    print('    $DAQBIN/lg_kvstore list', file=sys.stderr)
    print('Where:', file=sys.stderr)
    print('    key - is a key in the key value store of the current logbook', file=sys.stderr)
    print('    value - is a value to be assigned to "key"', file=sys.stderr)
    print('The verbs:', file=sys.stderr)
    print('    "exists" - tests for the existence "key". Nothing is printed, ', file=sys.stderr)
    print ('              a status of 1 indicates the key exists, 0 it does not', file=sys.stderr)
    print('    "get"   - Prints the value of "key" to stdout', file=sys.stderr)
    print('    "set"   - Sets a new value for "key" to "value". If "key" does not exist,', file=sys.stderr)
    print('              it is created', file=sys.stderr)
    print('    "create" - Creates a new "key" assigning it "value".  It is an error for the', file=sys.stderr)
    print('               "key" to already exist', file=sys.stderr)
    print('     "list"  - Lists all keys and values in the key/value stores in tabular form', file=sys.stderr)

def testExistence() -> int:
    # Handle the 'exists' verb:
    if len(sys.argv) != 3:
        usage()
        return -1
    key = sys.argv[2]
    try:
        _value = logbookadmin.kvGet(key)
        return 1
    except LogBook.error:
        return 0

def main() -> int:
    
    if logbookadmin.currentLogBook() is None:
        print('You must select the current logbook using $DAQBIN/lg_current', file=sys.stderr)
        return -1
    
    # Need at least a verb
    
    if len(sys.argv) < 2:
        usage()
        return -1
    
    verb = sys.argv[1]
    match verb:
        case 'exists':
            return testExistence()
        case _:
            usage()
            return -1
    return 0

if __name__ == '__main__':
    sys.exit(main())
