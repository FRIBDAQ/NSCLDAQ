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
from collections.abc import Iterable

from LogBookUIUtilities import ShiftChooser
from nscldaq.LogBook import LogBook, logbookadmin
from PyQt6.QtCore import Qt, QTimer, pyqtSignal
from PyQt6.QtWidgets import (
    QApplication,
    QHBoxLayout,
    QLabel,
    QListWidget,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

UPDATE_SECONDS : int = 1

def usage() -> None:
    print('Usage', file=sys.stderr)
    print('   $DAQBIN/lg_selshift [shift-name]', file=sys.stderr)
    print('Where:', file=sys.stderr)
    print('    shift-name is the name of the new shift to be mae current', file=sys.stderr)
    print('    If shift-name is not supplied this is a GUI shift selector that also shows the current shift', file=sys.stderr)
    print('    and its members, updated periodically (in case another terminal changes the current shift)')

class ShiftMemberList(QListWidget):
    # Lists the members of a shift.
    
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
    
    def setShift(self, shift : LogBook.Shift | None) -> None:
        self.clear()
        for member in shift.members:
            member_text = f'{member.salutation} {member.firstname} {member.lastname}'
            self.addItem(member_text)
            

class CurrentShift(QWidget):
    # Display the current shift  and
    # its members.
    
    def __init__(self, parent : None | QWidget = None):
        super().__init__(parent)
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        # Shiftname:
        
        shiftnameLayout = QHBoxLayout()
        shiftnameLayout.addWidget(QLabel('Current Shift: ', self))
        self._shiftname = QLabel('<Not Set>', self)
        shiftnameLayout.addWidget(self._shiftname)
        
        self._layout.addLayout(shiftnameLayout)
        
        # The Shift members:
        
        self._layout.addWidget(QLabel('Members:', self))
        self._members = ShiftMemberList(self)
        self._layout.addWidget(self._members)
        
    def setShift(self, shift : LogBook.Shift | None) -> None:
        self._members.setShift(shift)
        name = "<Not set>" if not shift else shift.name
        self._shiftname.setText(name)
            

class ShiftSelection(QWidget):
    # Combine a ShiftChooser with a member list.
    # 
    shiftselected =  pyqtSignal(str)    # Combo box changed
    shiftchosen    =  pyqtSignal(str)    # click of apply.
    
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        
        # THe combobox and list are side by side with an
        # Apply button below:
        
        self._layout  = QVBoxLayout()
        self.setLayout(self._layout)
        
        selectorLayout = QHBoxLayout()
        self._selection = ShiftChooser(self)
        selectorLayout.addWidget(self._selection, 0, Qt.AlignmentFlag.AlignTop)
        self._selectedMembers = ShiftMemberList(self)
        selectorLayout.addWidget(self._selectedMembers)
        
        self._layout.addLayout(selectorLayout)
        
        self._applybutton = QPushButton('Apply', self)
        self._layout.addWidget(self._applybutton)
        
        #  Relay the signals as needed.
        
        self._selection.shiftselected.connect(self.shiftselected)   # Straight relay.
        self._applybutton.clicked.connect(self._apply)
    
    # Attributes:
    
    def shift(self)  -> str:
        # Selected shift name:
        
        return self._selection.shift()
        
    def setShifts(self, shiftnames : Iterable[str]) -> None:
        
        self._selection.setShifts(shiftnames)
    
    def setShift(self, shift : LogBook.Shift) -> None:
        self._selectedMembers.setShift(shift)
    
    # Local/private slots:
    
    def _apply(self) -> None:
        self.shiftchosen.emit(self.shift())
    
    
class MainWidget(QWidget):
    # Top is CurrentShift.
    # Bottom is ShiftSelection.
    # These widgets are fetcahble via methods:
    
    def __init__(self, parent : QWidget | None = None) -> None:
        super().__init__(parent)
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        self._current = CurrentShift(self)
        self._layout.addWidget(self._current)
        
        self._selector  = ShiftSelection(self)
        self._layout.addWidget(self._selector)
        
    # Let our client get the subwidgets:
    
    def current(self) -> CurrentShift:
        return self._current
    def selector(self) -> ShiftSelection:
        return self._selector
    

def updateCurrentShift(currentWidget : CurrentShift) -> None:
    # Uupdate the contents of the current shift widget from the
    # actual logbook:
    
   
    shift  = logbookadmin.currentShift()    
    currentWidget.setShift(shift)

def updateSelection(selector : ShiftSelection, newshift : str) -> None:
    # Update the selector's shift members as the combobox changes selection:
    
    selector.setShift(logbookadmin.getShift(newshift))

def Gui() -> int:
    app = QApplication(sys.argv)
    widget = MainWidget()
    widget.show()
    
    # Set up a timer to maintain the current shift:
    
    update = QTimer(widget)
    update.setSingleShot(False)
    update.setInterval(UPDATE_SECONDS * 1000)
    update.timeout.connect(lambda: updateCurrentShift(widget.current()))
    update.start()
    
    # Stock the known shifts:
    # Set the members of the 'selected' shift.
    selector = widget.selector()
    shiftnames = logbookadmin.listShifts()
    selector.setShifts(shiftnames)
    selector.setShift(logbookadmin.getShift(shiftnames[0]))
    
    # Set up to track selection changes in the shift combobox:
    
    selector.shiftselected.connect(lambda newshift: updateSelection(selector, newshift))
    
    # Handle the user setting a new current shift:
    
    selector.shiftchosen.connect(logbookadmin.setCurrentShift)
    
    return app.exec()
        
    

def main() -> int:
    if len(sys.argv) > 2:
        usage()
        return -1
    if len(sys.argv) == 2:
        shiftname = sys.argv[1]
        if shiftname in logbookadmin.listShifts():
            logbookadmin.setCurrentShift(shiftname)
            return 0
        else:
            print(f'{shiftname} is not the name of a shift', file=sys.stderr)
            return -1
    else:
        return Gui()
        
if __name__ == "__main__":
    sys.exit(main())