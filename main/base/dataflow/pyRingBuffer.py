#!/usr/bin/env python3
'''
  This script replaces rinbuffer.tcl  with a python equivalent.
  From the original Tcl script:
  
  Usage
 ringbuffer create ?--datasize=n? ?--maxconsumers=n?   name
 ringbuffer format ?--maxconsumers=n?                  name
 ringbuffer delete                                     name
 ringbuffer status ?--host=hostname? ?--all? ?--user=user1,..?  ?name?
 ringbuffer list   ?--host=hostname?
 
 * ringbuffer create creats a new ringbuffer or complains if it already exists.
 * ringbuffer format formats an existing ringbuffer - use wisely.
 * ringbuffer delete deletes a ringbuffer. The ringmaster will kill off any
              clients (consumers and producers) of the ring.
 * ringubffer status will provide the list of ring buffers and current statistics.   
              This is presented via the tabulate module.
 * ringbuffer list  just lists the names of the ringbuffers.
'''


import tabulate
import argparse
import psutil
import getpass
from sys import stderr
from pathlib import Path
from nscldaq import RingBuffer
from nscldaq import RingMaster


## 
# Utility methods:

# Filter the rings by those owned by specific users:
# This assumes Linux because the assumption is
# Ringbuffers are files in /dev/shm and
# stat can get their owners.

def _filter_listing(listing, accepted_users):
    result = []
    for ring in listing:
        file_name = '/dev/shm/' + ring['name']
        path = Path(file_name)
        if path.owner() in accepted_users:
            result.append(ring)
    return result
        

#  Turn all the data size values in to kb...
# they come from the ringmaster in bytes.

def _sizes_to_kb(listing):
    result = []
    for ring in listing :
        kbring = ring
        #  The non consumer items:
        for key in ['size', 'free', 'minget', 'maxget']:
            kbring[key] = int(kbring[key]/1024)
        # Now the consumer items:
        for consumer in kbring['consumers']:
            consumer['backlog'] = int(consumer['backlog']/1024)
        result.append(kbring)
    return result

#  For each consumer add a key: command
#  Which  is the tail of the command represented by the pid
#  If the process goes out of existence during this
#  it is replaced with '-exited-'
def _add_consumer_names(listing):
    for ring in listing:
        for consumer in ring['consumers']:
            try:
                process = psutil.Process(consumer['pid'])
                command = Path(process.cmdline()[0])
                consumer['command'] = command.name
            except:
                consumer['command'] = '-exited-'
    
# Turn the data from the listing into data
# Suitable for the Python tabulate module.
#
def _make_tabulate_data(listing) :
    result = []
    for ring in listing:
        name = ring['name']
        size = ring['size']
        free = ring['free']
        consumers = ring['maxconsumers']
        ppid = ring['producer']
        maxget = ring['maxget']
        minget= ring['minget']
        result.append([name, size, free, consumers, ppid, maxget, minget, '-', '-'])
        for consumer in ring['consumers']:
            result.append(['-', '-', '-', '-', '-', '-', '-', consumer['pid'], consumer['backlog']])
            result.append(['-', '-', '-', '-', '-', '-', '-', f'({consumer["command"]})',  ''])
    return result
##
#  The handlers for each command:
#

# Create a new ringbuffer.
def create(cmd) :
    ringname = cmd.ring
    datasize = cmd.datasize
    max_consumers = cmd.maxconsumers
    RingBuffer.create(ringname, databytes=datasize*1024, maxconsumers=max_consumers)
    

# Format a ring.
def format(cmd):
    ringname = cmd.ring
    max_consumers = cmd.maxconsumers   
    RingBuffer.format(ringname, maxconsumers=max_consumers)
 
    
# Delete a ring
def delete(cmd):
    ring = cmd.ring
    RingBuffer.remove(ring)
    
# Print the status:
def status(cmd):
    # First we're not allowed both --all and a 
    # --users list.
    
    if cmd.all and (cmd.users is not None):
        print("The --all and --users options are mutually exclusive!", file=stderr)
        exit(-1)
    # Get the whole list and then figure out what
    # the filtering is:
    
    listing = RingMaster.usage(host=cmd.host)
    
    # turn our sizes into Kb.
    
    # If not all, then either filter on --users or 
    # the current username:
    
    if not cmd.all:
        if cmd.users is None:
            users = [getpass.getuser(),]
        else:
            users = cmd.users.split(',')
        listing = _filter_listing(listing, users)
    # turn the data sizes into units of Kbytes
    # and add the command for the consumers:
    
    listing = _sizes_to_kb(listing)
    _add_consumer_names(listing)
    
    # Make the data to be used for the
    # tabular output:
     
    table_data = _make_tabulate_data(listing)
    print(tabulate.tabulate(
        table_data, 
        headers=['name', 'size(k)', 'free(k)',  'max consumers', 'producer', 'maxget(k)', 'minget(k)', 'client', 'backlog(k)'],
        tablefmt='github'
    ))
    

def list(cmd):
    host = cmd.host
    status = RingMaster.usage(host=host)
    
    result = [x['name'] for x in status]
    result.sort()
    for name in result:
        print(name)


#  Set up the command line parser:
parser = argparse.ArgumentParser(
    prog="ringbuffer v2.0",
    description="Manipulate FRIBDAQ ringbuffers."
)

# The create subcommand:

command = parser.add_subparsers()
parser_create = command.add_parser('create', help='Create a new ringbuffer')
parser_create.add_argument('ring', help='Name of the ring to create')
parser_create.add_argument('-d', '--datasize', type=int, default=8192, help='size of ring buffer data in Kb')
parser_create.add_argument('-c', '--maxconsumers', type=int, default=100, help='maximum number of consumers')
parser_create.set_defaults(func=create)


# The format subcommand:

parser_format = command.add_parser('format', help='Reformat an existing ringbuffer')
parser_format.add_argument('ring', help='name of the ring to reformat')
parser_format.add_argument('-c', '--maxconsumers', type=int, default=100, help='Maximum number of consumers')
parser_format.set_defaults(func=format)

#  The delete subcommand:

parser_delete = command.add_parser('delete', help='Delete an existing ringbuffer')
parser_delete.add_argument('ring', help='name of the ring to delete')
parser_delete.set_defaults(func=delete)

#  The status subcommand:

parser_status = command.add_parser('status', help='Rigbuffer listing with statistics')
parser_status.add_argument('-H', '--host', default='localhost', type=str, help='Select host to list')
parser_status.add_argument('-a', '--all', action='store_true', help='Display rings owned by everyone')
parser_status.add_argument('-u', '--users', default=None, help='Display rings only owned by the comma separated users')
parser_status.set_defaults(func=status)

parser_list = command.add_parser('list', help='simple list of ringbuffer names')
parser_list.add_argument('-H', '--host', default='localhost', help='Specify host in which to list.')
parser_list.set_defaults(func=list)

#  Parse the arguments and dispatch the parse to the
# subcommand handlers.  Each handler knows what to expect
# in its namespace.

cmd = parser.parse_args()
cmd.func(cmd)

