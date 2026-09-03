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
@file lg_lspeople.py
@brief Print a tabular lsting of peopl in the logbook.
@author Ron Fox

'''

import sys
import tabulate
from nscldaq.LogBook import logbookadmin

def usage() -> None:
    print('Usage:', file=sys.stderr)
    print('   $DAQBIN/lg_lspeople', file=sys.stderr)


def main() -> int:
    if logbookadmin.currentLogBook() is None:
        print('No current logbook selected, use $DAQBIN/lg_current to select one', file=sys.stderr)
    if len(sys.argv) != 1:
        usage()
        return -1
    
    people = logbookadmin.listPeople()
    names = [(p.salutation,  p.firstname, p.lastname) for p in people]
    print(tabulate.tabulate(names, headers=['Salutation', 'First Name', 'Last Name'], tablefmt='plain'))
    return 0

if __name__ == '__main__':
    
    sys.exit(main())