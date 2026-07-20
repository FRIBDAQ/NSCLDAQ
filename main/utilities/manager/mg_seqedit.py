#!/usr/bin/env  python3
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
Thie file contains a program to edit sequences in the managed
experiment environment. A squence is an ordered list of programs
that are started as a result of a state transition.  Sequences
are how the managed environment gets work done.

There are some wizards that create and pre-populate sequences 
(such as the readout wizard and the eventlog wizard), but there
are cases where a user may want to explicitly describe some set of
actions that trigger as a result of a state transition.
'''


from PyQt6.QtWidgets import (QListView, QLabel, QLineEdit, QComboBox, QPushButton, 
        QWidget, QHBoxLayout, QVBoxLayout)

from PyQt6.QtGui import  QStandardItemModel, QStandardItem
from PyQt6.QtCore import QModelIndex, QObject, pyqtSignal


class SequenceSelector(QWidget):
    '''
        This widget provides a list of sequences
        and the controls needed to create a new
        sequence or edit an existing one.
        
    Attributes:
        sequenceNames - The names of the currently defined sequences
        states        - The valid state names that can trigger a state transition.
        triggerState  - The current trigger state (must be in states).
        newSequence   - The name of a new sequence
    
    Signals:
        editSequence(str) - Requested that an existing sequence be edited, 
                            the parameter is the name of an existing sequence.
        createSequence(str, str) - Requests the creation of a new sequence
                           the parameters are, in order, the name of the sequence
                           and the name of the trigger state.
    '''
    editSequence = pyqtSignal(str)
    createSequence = pyqtSignal(str, str)
    def __init__(self, parent : QObject | None  = None):
        super().__init__(parent)
        
        # We'll have a list view that will use a model that lists the
        # names of the sequences. The model will be stocked using the
        # setSequenceNames attribute setter.
    
        self._layout  = QVBoxLayout(self)          # The two sections are stacked.
        
        self._seqlist = QStandardItemModel(self)  # Model for the sequences.
        self._seqview = QListView(self)        # Displays seqlist.
        self._seqview.setModel(self._seqlist)
        self._layout.addWidget(self._seqview)
        
        # The bottom is a bit more complex
        
        newlayout = QHBoxLayout()
        newlayout.addWidget(QLabel('New Sequence: ', self))
        self._newname = QLineEdit(self)
        newlayout.addWidget(self._newname)
        
        newlayout.addWidget(QLabel('TriggerState', self))
        self._trigger = QComboBox(self)
        newlayout.addWidget(self._trigger)
        
        self._layout.addLayout(newlayout)
        
        self._new = QPushButton('New...', self)
        self._layout.addWidget(self._new)
        
        # Set up the internal signal handling:
        
        self._seqview.doubleClicked.connect(self._editRelay)
        self._new.clicked.connect(self._newRelay)
    
    # Internal slots:
    
    def _editRelay(self, index : QModelIndex) -> None:
        # Figure out which item in the listbox was
        # double clicked and emit an editSequence signal for it.
        
        seqname = self._seqlist.itemFromIndex(index).text()
        self.editSequence.emit(seqname)
    
    def _newRelay(self) -> None:
        # Marshall the name of the new signal and the trigger state.
        # Only emit a newSequence signal if the new sequence line edit is
        # not blank.
        
        seq_name = self.newSequence()
        if seq_name.strip():
            trigger_state = self.triggerState()
            self.createSequence.emit(seq_name, trigger_state)
    # Attribute definitions:
    
    def  setSequenceNames(self, names : list[str]) -> None:
        '''
            @param names - names of the sequences that will be in the list.
        '''
        self._seqlist.clear()
        for name in names:
            self._seqlist.appendRow(QStandardItem(name))
    
    def sequenceNames(self) -> list[str] :
        '''
            @return list [str] - the names of the sequences in the box.
        '''
        result = list()
        for row in range(self._seqlist.rowCount()):
            result.append(self._seqlist.item(row).text())
    
        return result
    
    def setStates(self, states: list[str]) -> None:
        '''
        @param states - the states with which to populate 
             trigger combobox.
        '''
        self._trigger.clear()
        for state in states:
            self._trigger.addItem(state)
    
    def states(self) -> list[str]:
        '''
        @return list[str] - list of states in the trigger combobox:
        '''
        result = list()
        for index in range(self._trigger.count()):
            result.append(self._trigger.itemText(index))
        return result
        
    
    def setTriggerState(self, state : str) -> None:
        '''
        @param state - new value for the trigger state combobox.
        @raise ValueError - if the state is not in the combobox.
        
        
        '''
        if state not in self.states():
            raise ValueError(f'{state} is not a valid state name')
        self._trigger.setCurrentText(state)
            
    def triggerState(self) -> str:
        '''
        @return str - the current selected trigger state:
        '''
        return self._trigger.currentText()
    
    def setNewSequence(self, name : str) -> None:
        ''''
            @param name - name to shove in the new sequence text edit.
        '''   
        self._newname.setText(name)
    def newSequence(self) -> str:
        '''
            @return  str - the value of the new sequence text edit.
        '''
        return self._newname.text()
        
        
# Test code for noow.

if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication
    import sys
    
    
    def edit(name):
        print('edit seq', name)
        
    def create(name, trigger):
        print('create seq', name, 'triggered on', trigger)
    
    app = QApplication(sys.argv)
    
    win = SequenceSelector()
    win.show()
    
    # Test the sequenceName attribute:
    
    win.setSequenceNames(['seq1', 'seq2', 'seq3', 'last'])
    print(win.sequenceNames())
    
    win.setStates(['SHUTDOWN', 'BOOT', 'HWINIT', 'BEGIN', 'END'])
    print(win.states())
    
    win.setTriggerState('BOOT')
    print(win.triggerState())
    
    win.setNewSequence('new')
    print(win.newSequence())
    
    # Test signals:
    
    win.editSequence.connect(edit)
    win.createSequence.connect(create)
    
    sys.exit(app.exec())
        
        
    