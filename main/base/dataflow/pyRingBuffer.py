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
from nscldaq import RingBuffer
from nscldaq import RingMaster


##
#  The handlers for each command:
#

# Create a new ringbuffer.
def create(cmd) :
    print('create subcommand')
    print(cmd)

# Format a ring.
def format(cmd):
    print('format command')
    print(cmd)
    
# Delete a ring
def delete(cmd):
    print('delete command')
    print(cmd)
    
# Print the status:
def status(cmd):
    print('status command')
    print(cmd)  

def list(cmd):
    print('list command')
    print(cmd)

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
parser_status.add_argument('-H', '--host', default='localhost', type=str)
parser_status.add_argument('-a', '--all', action='store_true')
parser_status.add_argument('-u', '--users')
parser_status.set_defaults(func=status)

parser_list = command.add_parser('list', help='simple list of ringbuffer names')
parser_list.add_argument('-H', '--host', default='localhost')
parser_list.set_defaults(func=list)

#  Parse the arguments and dispatch the parse to the
# subcommand handlers.  Each handler knows what to expect
# in its namespace.

cmd = parser.parse_args()
cmd.func(cmd)

