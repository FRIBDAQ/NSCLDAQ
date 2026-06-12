#!/usr/env python3
'''
  This program provides command line control over
  a Readout program over the ReST interface. It replaces
  rdo_control.tcl
  
Copy/pasted from that program:


##
# Usage:
#    rdo_control host user subcommand ?...?
#  Where:
#     host - is the host on which Readout is running.
#     user - is the user running the Readout.
#     subcommand is what we want the Readout to do.
#     the rest are subcommand specific parameters.
#
#  subcommands (and their parameters) are:
#    - begin  - begin a run.
#    - end    - End a run.
#    - init   - Initialize hardware.
#    - shutdown - Shutdown the program.
#    - setRun n - Set a new run number.
#    - setTitle title words - set a new title.
#    - getRun   - Get the run number (to stdout).
#    - getTitle - Get title (to stdout)
#    - getState - Get state of Readout.
#    - getStatistics - formats statistics nicely to stdout.
#

@file rdo_control.py
@brief control readouts over ReST interface.
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
from   nscldaq.readoutREST import readoutRestClient
import tabulate


##  Utility functions:

def usage():
    global dispatch_table
    # Print the program usage on stderr.
    # Note we use the short help and keys in the dispatch_table below
    
    print("Usage:", file=sys.stderr)
    print("   $DAQBIN/rdo_control host user subcommand ...", file=sys.stderr)
    print("Control a Readout program via its ReST server", file=sys.stderr)
    print("Where:", file=sys.stderr)
    print("    host - is the host that's running the Readout we're controlling", file=sys.stderr)
    print("    user - is the user running the readout we're controlling", file=sys.stderr)
    print("    subcommand - is a subcommand that describes what we want to tell the Readout to do", file=sys.stderr)
    print("Subcommands accepted are:", file=sys.stderr)
    print("Note that the port is gotten byt one of the following:", file=sys.stderr)
    print("Translating the environment variable SERVICE_NAME which must be", file=sys.stderr)
    print("the name of the service advertised by the Readout's ReST service", file=sys.stderr)
    print("If SERVICE_NAME does not exist, we default to the service name 'ReadoutREST'", file=sys.stderr)
    for key, value in dispatch_table.items(): 
        print(f"    {key}  - {value[1]}", file=sys.stderr)
    
def make_client(host, user):
    # Create the ReadoutRestClient object.  See the usage
    # text for a description of how the service name is determined.
    # Or just look at the next line :-P
    service_name = os.environ['SERVICE_NAME'] if 'SERVICE_NAME' in os.environ.keys() else 'ReadoutREST'
    
    return readoutRestClient.ReadoutClient(host, service_name, user)

## Subcommand executor functions.


#   functions to control the run:

def begin(argv):
    # Executor function to request a begin run.
    client = make_client(sys.argv[1], sys.argv[2])
    return client.begin()

def init(argv):
    client = make_client(sys.argv[1], sys.argv[2])
    return client.init()

def end(argv):
    client = make_client(sys.argv[1], sys.argv[2])
    return client.end()

def shutdown(argv):
    client = make_client(sys.argv[1], sys.argv[2])
    return client.shutdown()


# Run number functions:

def setRun(argv):
    if len(argv) != 5:
        print('The setRun command requires a run number argument', file=sys.stderr)
        usage()
        exit(-1)
    try:
        run_num = int(argv[4])
    except ValueError as e:
        print(f'Invalid run number: {e}', file=sys.stderr)
        exit(-1)
    
    # Now do the operation:
    
    client = make_client(sys.argv[1], sys.argv[2])
    return client.setRunNumber(run_num)

def getRun(argv):
    client= make_client(sys.argv[1], sys.argv[2])
    json  = client.getRunNumber()
    if json['status'] == 'OK':
        print(json['run'])
    return json 

## Title functions:

def setTitle(argv):
    #  We're a bit more generous than the Tcl version.,
    #  All word after setTitle are joined separated by " "
    #  to form the title. 
    # We do, however, require at least one word;
    
    if len(argv) < 5:
        print('The setTitle command requires title argument(s)', file=sys.stderr)
        usage()
        exit(-1)
    
    client= make_client(sys.argv[1], sys.argv[2])
    return client.setTitle(' '.join(argv[4:]))
def getTitle(argv):
    client= make_client(sys.argv[1], sys.argv[2])
    json  = client.getTitle()
    if json['status'] == 'OK':
        print(json['title'])
    return json  

# Miscellanous methods:

def getRunState(argv):
    client= make_client(sys.argv[1], sys.argv[2])
    json  = client.getState()
    if json['status'] == 'OK':
        print(json['state'])
    return json 

def getStatistics(self):
    client= make_client(sys.argv[1], sys.argv[2])
    json  = client.getStatistics()
    if json['status'] == 'OK':
        # We can mechanically construct everything:
        
        titles = [t for t in json['cumulative'].keys()]   # could use either.
        titles.insert(0, '')
        
        perrun = [v for v in json['perRun'].values()]
        perrun.insert(0, 'Per Run')
        
        cumulative = [v for v in json['cumulative'].values()]
        cumulative.insert(0, 'cumulative')
        lines = [perrun, cumulative]
        print(tabulate.tabulate(lines, headers=titles, tablefmt='grid'))
        
    return json 
        
## Function dispatch table:
#   This is a hash keyed on the subcommand name and
#   with values a two element tuple containing  in order,
#   the function reference and a short help string.
# Note that all functions must accept the sys.argv
#   argv which contains at least through the subcommand.
# The service functions are expected to make use of the function 'make_client' to make their
# ReST client object  it handles all the gymnastics needed to figure out the
# service name/port
#
dispatch_table = {
    'begin': (begin, 'Attempt to start a run'),
    'init' : (init,  'Initialize the Readout program'),
    'end'  : (end,   'End an active run'),
    'shutdown': (shutdown, 'Request Readout exit NOTE: this is honored even if a run is active so be careful!!!'),
    'setRun' : (setRun, 'Set the run number to the next commane line parameter'),
    'getRun' : (getRun, 'Print the run number on stdout.'),
    'setTitle' : (setTitle, 'Set a new title from the remaining command line words'),
    'getTitle': (getTitle, 'Print the title on stdout'),
    'getState' : (getRunState, 'Print the Run state to stdout'),
    'getStatistics' : (getStatistics, 'Print run event statistics'),
}

##-------------------- Entry point ## -----------------

## We need at least the host user and subcommand, note that some subcommands need
#  more.  Note argv[0] is the program name.

if len(sys.argv) < 4:
    usage()
    sys.exit(-1)
host = sys.argv[1]
user = sys.argv[2]
subcommand = sys.argv[3]


if subcommand not in dispatch_table.keys():
    print(f'Unrecognized subcommand {subcommand}', file=sys.stderr)
    usage()
    sys.exit(-1)

response = dispatch_table[subcommand][0](sys.argv)
if response['status'] != 'OK':
    print(f'Error: {response["message"]}', file=sys.stderr)