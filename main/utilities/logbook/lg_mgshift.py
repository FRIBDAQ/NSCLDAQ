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
import os
import subprocess
from collections.abc import Iterable

import logbookadmin
from nscldaq.LogBook import LogBook
from nscldaq.LogBook.LogBookUIUtilities import ShiftEditor
from nscldaq.mg_configutils import OkDialog

from PyQt6.QtWidgets import (
    QApplication, QDialog, QWidget, QComboBox, QLabel, QVBoxLayout, QHBoxLayout, QPushButton, QLineEdit
)

from PyQt6.QtCore import Qt


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
    

class ShiftEditorDialog(OkDialog):
    def __init__(self, parent : None | QWidget = None):
        super().__init__(ShiftEditor(), parent = parent)

class ShiftChooser(QWidget):
    # Widget that provides a choice of shifts
    
    def __init__(self, parent : QWidget  | None = None):
        super().__init__(parent)
        self._layout = QVBoxLayout()
        self._layout.addWidget(QLabel('Choose Shift: ', self))
        self.setLayout(self._layout)
        
        self._shifts = QComboBox(self)
        self._layout.addWidget(self._shifts)
    
    def setShifts(self, shifts : Iterable[str]) -> None:
        for shift in shifts:
            self._shifts.addItem(shift)
    
    def shift(self) -> str:
        return self._shifts.currentText()

class ShiftChooserDialog(OkDialog):
    def __init__(self, parent : None |QWidget = None):
        super().__init__(ShiftChooser(), parent = parent)
        

class WhatToDo(QDialog):
    # THis is the default prompter... What should the user do?
    # edit an existing shift, chosen from a shift chooser or 
    # create a new shift?  THis is a funky dialog.
    # because it can be dual purpose.
    
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        #  The selection part:
        
        selection = QHBoxLayout()
        self._shift = ShiftChooser(self)
        selection.addWidget(self._shift)
        self._editbutton =QPushButton('Edit...', self)
        selection.addWidget(self._editbutton, 0, Qt.AlignmentFlag.AlignBottom)
        
        self._layout.addLayout(selection)
        
        # The New... part:
        
        creation = QHBoxLayout()
        creation.addWidget(QLabel('Shift Name:', self))
        self._newname = QLineEdit(self)
        creation.addWidget(self._newname)
        self._newbutton = QPushButton('New...', self)
        creation.addWidget(self._newbutton)
        
        self._layout.addLayout(creation)
        
        # Hook in the buttons:
        
        self._editbutton.clicked.connect(lambda: self._ok('edit'))
        self._newbutton.clicked.connect(lambda: self._ok('new'))
        
    # Public methods:
    
    def setShifts(self, shifts : Iterable[str]) -> None:
        self._shift.setShifts(shifts)
    def editShift(self) -> str:
        return self._shift.shift()
    
    def newShift(self) -> str:
        return self._newname.text()
    
    def action(self) -> str:
        return self._action
    
    # Private/internal slots:
    
    def _ok(self, action : str) -> None:
        #  An ok action was clicked:
        
        self._action = action
        self.accept()

    
        
def editShiftGui(name : str) -> list[LogBook.Person] | None:
    #  Pop up a dialog to edit the named shift with the 
    #  left and right hand people properly loaded as well as the shiftname
    # Note that its up to the caller to make actual database changes.
     
    _app = QApplication.instance()    # Culd come from default...
    if not _app:
        _app = QApplication(sys.argv)   
        
    prompt = ShiftEditorDialog()
    
    # Load out the prompt:
    
    current_members = logbookadmin.listShiftMembers(name)
    current_names    = [(p.lastname, p.firstname, p.salutation) for p in current_members]
    all_people      = logbookadmin.listPeople()
    current_nonmembers = [p for p in all_people if (p.lastname, p.firstname, p.salutation) not in current_names]
    
    wa = prompt.workarea()
    wa.setName(name)
    
    wa.editor().setMembers(current_members)
    wa.editor().setNonMembers(current_nonmembers)
    
    if prompt.exec() == QDialog.DialogCode.Accepted:
        return prompt.workarea().editor().members()
    else:
        return None     # Rejected.
    
def promptForShift() -> str | None:
    # Pop up a dialog with a list of shifts to choose from. We'll use a combobox.   
    
    _app = QApplication(sys.argv)
    widget = ShiftChooserDialog()
    widget.workarea().setShifts(logbookadmin.listShifts())
    
    if widget.exec() == QDialog.DialogCode.Accepted:
        return widget.workarea().shift()
    else:
        return None
        

def create_shift(shift : str | None) -> int:
    # Handle the 'create' verb.  shift is the new shiftname or 
    # None if not provided.
    
    if not shift:
        # Run $DAQBIN/lg_mkshift
        
        if 'DAQBIN'  in os.environ:
            command = f'{os.environ["DAQBIN"]}/lg_mkshift'
            
            return subprocess.call(command)
        else:
            print(
                'The DAQBIN enviromment variable is not defined, use daqsetup.bash in a vesion of FRIB/NSCLDAQ to set it up',
                file = sys.stderr
            )
            return -1
    else:
        # Make sure this is a new shift:
        
        if shift in logbookadmin.listShifts():
            print(f'{shift} is already a shift, use "edit" to modify it.')
            return -1
        
        # Make the empty shift and let the user edit it:
        
        logbookadmin.createShift(shift, [])
        members = editShiftGui(shift)     # None if cancel. and
        if members:                       # No point if the list is empty.
            logbookadmin.addMembersToShift(shift,members)

    return 0
        
def edit_shift(name : str | None) -> int:
    if not name:
        name = promptForShift()
        if not name:
            return 0                      # Cancelled.

    # Since the shift could have been typed in we need to be sure it's real:
    
   
    if name not in logbookadmin.listShifts():
        print(f'{name} is is not a shift!', file=sys.stderr)
        return -1
    
    members = editShiftGui(name)
    if members is None:
        return 0                          # rejected. 
    
    member_names = [(p.lastname, p.firstname, p.salutation) for p in members]
    prior_members = logbookadmin.listShiftMembers(name)
    
    # Remove prior members no longer in the shift:
    
    for p in prior_members:
        if (p.lastname, p.firstname, p.salutation) not in member_names:
            logbookadmin.removeMemberFromShift(name, p)
    
    # Add members that are not already in the shift:
    
    member_names = [(p.lastname, p.firstname, p.salutation) for p in prior_members]
    new_members = []
    for m in members:
        if (m.lastname, m.firstname, m.salutation) not in member_names:
            new_members.append(m)
    
    logbookadmin.addMembersToShift(name, new_members)
    
    return 0

def list_shift(name : str) -> int:
    #   List the named shift:
    
    if name not in logbookadmin.listShifts():
        print(f"There is no shift named {name}")
        return -1
    
    members = logbookadmin.listShiftMembers(name)
    print(f"{name} shift  members:")
    for member in members:
        print(f'{member.salutation} {member.firstname} {member.lastname}')
    return 0

def gui() -> int:
    # No verb so, prompt for new or shift to edit.
    # then do it.
    
    _app = QApplication(sys.argv)
    action_selection = WhatToDo()
    action_selection.setShifts(logbookadmin.listShifts())
    
    if action_selection.exec() == QDialog.DialogCode.Accepted:
        action = action_selection.action()
        if action == 'edit':
            edit_shift(action_selection.editShift())
        elif action == 'new':
            create_shift(action_selection.newShift())
        else:
            return 0       # Don't really know what else to do.
    else:
        return 0        # They killed the dialog 
    
def main() -> int:
    
    # Get the parameters..
    
    verb = None
    shiftname = None
    
    if len(sys.argv) >= 2:
        verb = sys.argv[1]
    if len(sys.argv) >=3:
        shiftname = sys.argv[2]
    if len (sys.argv) > 3:
        usage()
        return -1
    
    match verb:
        case 'help':
            usage()
            return 0 
        case 'create':
            return create_shift(shiftname)
        case 'edit':
            return edit_shift(shiftname)
        case 'list':
            if shiftname:
                return list_shift(shiftname)
            else:
                for name in logbookadmin.listShifts():
                    list_shift(name)
                    print('---------------------------------------------')
            return 0
        case None:
            return gui()
            
        case _:
            # Illegal/unsupported verb.
            print("Invalid command verb:")
            usage()
            return 0
                 
    


if __name__ == "__main__":
    sys.exit(main())