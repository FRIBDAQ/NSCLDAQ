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
@file lg_selshift.py
@brief Select the current shift.
@author Ron Fox
'''
import sys

def usage() -> None:
    print('Usage', file=sys.stderr)
    print('   $DAQBIN/lg_selshift [shift-name]', file=sys.stderr)
    print('Where:', file=sys.stderr)
    print('    shift-name is the name of the new shift to be mae current', file=sys.stderr)
    print('    If shift-name is not supplied this is a GUI shift selector that also shows the current shift', file=sys.stderr)
    print('    and its members, updated periodically (in case another terminal changes the current shift)')

def main() -> int:
    if len(sys.argv) > 2:
        usage()
        return -1

if __name__ == "__main__":
    sys.exit(main())