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
  This program sets the current logbook for the lg_* utilties.
  the user can either specify a logbook on the command line, in which case
  that one is used,  or not, in which case we popup a file selection dialog
  to choose it.
'''
import sys
from   nscldaq import logbookadmin
def usage() -> None:
    # Print the program usage to stderr
    
    print('Usage:', file=sys.stderr)
    print('   $DAQBIN/lg_current [logbook_file]', file=sys.stderr)
    print('Sets the current logbook', file=sys.stderr)
    print('Where:', file=sys.stderr)
    print('   logbook_file - if provided is the name of the logbook file', file=sys.stderr)
    print('                  if not provided it is graphicaly prompted for', file=sys.stderr)

def prompt_for_logbook() -> str | None:
    # We don't import the Qt stuff until  here:
    
    from PyQt6.QtWidgets import QApplication, QFileDialog
    app = QApplication(sys.argv)
    
    file, _ = QFileDialog.getOpenFileName(
        None, 'Choose Logbook File', '.', 
        'LogBook Files (*.logbook);;Sqlite database (*.db, *.sqlite);;All Files (*)',
        '*.logbook'
    )
    if file:
        return file
    else:
        return None     # No selection.

def main() -> int:
    if len(sys.argv) > 2:
        usage()
        return -1
    if len(sys.argv) == 2:
        logbook_file = sys.argv[1]
    else:
        logbook_file = prompt_for_logbook()

    if not logbook_file:
        return 0
    
    logbookadmin.setCurrentLogBook(logbook_file)
    return 0
    
if __name__ == "__main__":
    sys.exit(main())

