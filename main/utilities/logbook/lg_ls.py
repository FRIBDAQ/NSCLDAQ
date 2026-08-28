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

@file lg_ls.py
@brief Report wha tthe currently selected logbook file is.
@author Ron Fox
'''

import sys

from nscldaq.LogBook import logbookadmin


# Program entry point
def main() -> int:
    current = logbookadmin.currentLogBookFile()
    if current:
        print(current)
    else:
        print('There is no current logbook file. use $DAQBIN/lg_current to select one')

    return 0

if __name__ == '__main__':
    sys.exit(main())
