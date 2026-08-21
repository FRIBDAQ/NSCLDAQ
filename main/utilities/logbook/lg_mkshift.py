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
    @file lg_mkshift.py
    @brief - create a new shift and, optionally, add members to it.
    @author Ron Fox
'''
from nscldaq.LogBook import logbookadmin, LogBook

import sys


def usage() -> None:
    print('Usage:', file=sys.stderr)
    print('    $DAQBIN/lg_mkshift [shiftname]', file=sys.stderr)
    print('Where:', file=sys.stderr)
    print('  shiftname - is the optional name of a shift')
    print('If the shiftname is omitted, a GUI prompter allows you to', file=sys.stderr)
    print('supply the shfit name and a set of people that will be initially added to the shift', file=sys.stderr)
    print('If the shiftname is supplied, an empty shift is created.  The members can be', file=sys.stderr)
    print('managed with $DAQBIN/lg_mgshift', file=sys.stderr)

def prompt() -> tuple[str | None, list[LogBook.Person] | None]:
    # These imports are here so they only happen if I'm using pyqt6:
    
    from PyQt6.QtWidgets import (QApplication, QDialog, QDialogButtonBox, QLabel, QLineEdit,
                                 QHBoxLayout, QVBoxLayout, QMessageBox)
    from nscldaq.LogBook.LogBookUIUtilities import ShiftMemberEditor
    
    class ShiftCreationDialog(QDialog):
        def __init__(self):
            super().__init__(parent = None)
            
            self._layout = QVBoxLayout()
            self.setLayout(self._layout)
            
            # Top bit is the prompt for the shiftname and line edit for it:
            
            namelayout = QHBoxLayout()
            namelayout.addWidget(QLabel('Shift Name: ', self))
            self._name = QLineEdit(self)
            namelayout.addWidget(self._name)
            
            self._layout.addLayout(namelayout)
            
            #  Now the shift editor:
            
            self._memberEditor = ShiftMemberEditor(self)
            self._layout.addWidget(self._memberEditor)
            
            # The buttons and their connections:
            
            self._buttons = QDialogButtonBox(
                QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel,
                self
            )
            self._layout.addWidget(self._buttons)
            self._buttons.accepted.connect(self.accept)
            self._buttons.rejected.connect(self.reject)
            
        # Get the name:
        
        def name(self) -> str:
            '''
                @return str - the shfit name.
            '''
            
            return self._name.text()
        
        
        
        def setPeople(self, people : list[LogBook.Person]) -> None:
            '''
                @param people : list[LogBook.Person] - People known to the logbook.
            '''
            
            self._memberEditor.setNonMembers(people)
        def getMembers(self) -> list[LogBook.Person]:
            '''
            @return list[LogBook.Person] - List of the people on the shift.
            '''
            return self._memberEditor.members()
    
    # Now we have a prompter dialog, we can start it up.  Note that we
    # require a non-empty shift name:
    
    _app = QApplication(sys.argv)
    prompt = ShiftCreationDialog()
    prompt.setPeople(logbookadmin.listPeople())
    
    while True:
        if prompt.exec() ==  QDialog.DialogCode.Accepted:
            name = prompt.name()
            if name.strip():  # Require a name.
                return (name, prompt.getMembers()) 
            else:
                QMessageBox.warning(prompt, 'Missing data', 'The shift must have a non blank name')
        else:
            return (None, None)

def main() -> int:
    if not logbookadmin.currentLogBook():
        print('You must use $DAQBIN/lg_current to establish the current logbook', file=sys.stderr)
        return -1
    
    if len(sys.argv) > 2:
        usage()
        return -1
    
    members : list[LogBook.Person] = []
    
    if len(sys.argv) == 2:
        shiftname = sys.argv[1]
    else:
        (shiftname, members) = prompt()
        
    # If there's a valid shiftname, create it.
    
    if shiftname:
        if shiftname in logbookadmin.listShifts():
            print(f'{shiftname} is already a shift.  Use $DAQBIN/lg_mgshift to manage its members')
            return -1
        try:
            logbookadmin.createShift(shiftname, members)
        except LogBook.err as e:
            print(f'Unable to create the shift {shiftname} : {e}')
            return -1
        
    return 0

if __name__ == "__main__":
    sys.exit(main())
