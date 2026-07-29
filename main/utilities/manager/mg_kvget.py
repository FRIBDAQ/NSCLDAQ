#! /usr/bin/env python3
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


''''
    Return the value of a single key of the key value store.  This requires
    a running manager server.
    
    @file mg_kvget.py
    @brief fetch a kv store value from the server.
    @author Ron Fox.
'''


import nscldaq.manager_client
import sys


def usage():
    ''' Print the program usage to stderr'''
    print('''
$DAQBIN/mg_kvget host user key

Output the value of a key in the key/value store to stdout.
WHere:
    host - the host in which the manager server is running
    user - User that started the manager server.
    key  - The key to output.
          ''', file = sys.stderr)



def main() -> int:
    ''' Entry point'''
    if len(sys.argv) != 4:
        usage()
        return -1
    
    host = sys.argv[1]
    user = sys.argv[2]
    key  = sys.argv[3]
    
    client = nscldaq.manager_client.KVStore(host, user)
    
    try:
        print(client.value(key))
        
    except RuntimeError as e:
        print(e, file=sys.stderr)
        return -1
    
    return 0
if __name__ == "__main__":
    sys.exit(main())