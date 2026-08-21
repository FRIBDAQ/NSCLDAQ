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
@file lg_mgshift.py
@brief Shift manager creates, edits or lists shifts.
@note with the exception of list, this is inherently GUI so we won't conditionally import the Qt 
      as we did for e.g. lg_mkshift and similer.
'''
import sys

def usage() -> None:
    # Print program usage to stderr:
    
    print('Usage:', file = sys.stderr)
    print('    $DAQBIN lg_mgshift [verb [shiftname]]', file = sys.stderr)
    print('Manage this logbook shifts', file = sys.stderr)
    print('Valid Verbs:', file = sys.stderr)
    print('   help   - or an invalid verb, prints this message', file = sys.stderr)
    print('   create - Creates a new shift.  If the name in sot supplied, ', file = sys.stderr)
    print('            lg_mkshift in GUI mode is invoked. If a name is provided', file = sys.stderr)
    print('            it is created and you can graphically edit the members', file = sys.stderr)
    print(file = sys.stderr)
    print('   edit  - If a shiftname is provided, and exists you a graphical editor of members', file = sys.stderr)
    print('           is presented allowing you to edit the members.  If no shiftname is provided', file = sys.stderr)
    print('           you first select from the existing shifts via  GUI', file = sys.stderr)
    print(file = sys.stderr)
    print('   list  - If a shiftname is provided, the members of that shift are listed, otherwise', file = sys.stderr)
    print('           all shift and their members are listed', file = sys.stderr)
    print('           this is the only verb without a GUI', file = sys.stderr)
    print(file = sys.stderr)
    print('If no verb is given, a simple GUI listing the shifts is presented with a pair of buttons allowing', file = sys.stderr)
    print('you to either create a new shift or edit a selected shift.')
    


def main() -> int:
    usage()
    return 0


if __name__ == "__main__":
    sys.exit(main())