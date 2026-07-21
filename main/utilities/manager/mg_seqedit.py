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
        QWidget, QHBoxLayout, QVBoxLayout, QTableView, QStyle, QSpinBox)

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
        
class SequenceTable(QTableView):
    '''
        This is a table view and associated model
        that allows the display and limited editing of
        a sequence  A sequencde is an  orderd list of objects that each have
        *  step_number -  A floating point number that defines the position of the
                          step in the sequence (the ordering).
        *  program_name - the name of the program that will be run for that step.
        *  pre_delay    - ms to delay before running the program (int).
        *  post_delay   - ms to delay _after_ running the program (int).
        
        As you can imagine, the external representation of a sequence is, therefore
        a list of dicts containing the keys described above.  Internally, the
        tableview maintains its own StandardItemModel
        
        Attributes:
           steps    - The list of dicts described above.
           
        Methods of interest:
           append   - Add a new step at the end of the sequence.
           
        Autonomous actions:
        *   The selected item can be moved upwards in the sequence.  This will
            change its step number.
        *   The selected item can be moved downwards in the sequence.  This will change
            its step number.
            
        Note:
           The step numbers are not maintained in the model.  Instead, they are used
           to order the table and, assigned on retrieval.  This is much simpler
           than the scheme used by the Tcl editor.
        
        Since all actions are autonomous, no signals are emitted from the widget.
    '''
    def __init__(self, parent : QObject | None = None):
        '''
            A bit about the layout.  The main feature is a table view.
            To its right will be  a stack of buttons. These are labeled 
            top to bottom with the icons:
            * SP_ArrowUp - an up arrow, clicking it will move the selected step up.
            * Sp_ArrowDown - A down arrow, clicking it will move the selected step down.
            * SP_DialogCancelButton - a red x, clicking it will delete the selected item.
            
        '''

        super().__init__(parent)
        
        # The primary layout is, therefore, an HBOX layout with
        # a VBOX sublayout for the buttons:
        
        self._layout = QHBoxLayout()
        self.setLayout(self._layout)
        
        buttonLayout = QVBoxLayout()
        
        # The table at the left:
        
        self._table = QTableView(self)
        self._sequence = QStandardItemModel()
        self._initializeModel()                  # clear and set column headers.
        self._table.setModel(self._sequence)
        self._layout.addWidget(self._table)
        
        # The buttons at the right:
        
        self._moveup = QPushButton(self)
        self._setButtonIcon(self._moveup, 'SP_ArrowUp')
        buttonLayout.addWidget(self._moveup)
        
        self._movedown= QPushButton(self)
        self._setButtonIcon(self._movedown, 'SP_ArrowDown')
        buttonLayout.addWidget(self._movedown)
        
        self._delete = QPushButton(self)
        self._setButtonIcon(self._delete, 'SP_DialogCancelButton')
        buttonLayout.addWidget(self._delete)
        
        self._layout.addLayout(buttonLayout)
        
        # Hook in the buttons:
        
        self._moveup.clicked.connect(self._moveRowUp)
        self._movedown.clicked.connect(self._moveRowDown)
        self._delete.clicked.connect(self._deleteRow)
            
    # Implement attributes:
    
    def setSteps(self, steps : list[dict]) -> None:
        '''
            Clear the table view model and fill it with steps
            
            @param steps a list of dicts described in the class header.
        '''
        self._initializeModel()            # clear and reset the headers:
        
        for step in steps:
            self.append(step)
    
    def steps(self) -> list[dict]:
        '''
            Returns the contents of the model as a list of steps.
            Note that we assign step numbers:
            
        '''
        result = list()
        for row in range(self._sequence.rowCount()):
            step_no = (row+1)*10.0
            program = self._sequence.item(row, 0).text()
            pre_delay = float(self._sequence.item(row, 1).text())
            post_delay= float(self._sequence.item(row, 2).text())
            
            step = {'step_number' : step_no, 
                    'program_name': program, 
                    'pre_delay': pre_delay,
                    'post_delay': post_delay}
            result.append(step)
        
        return result
        
    
    def append(self, step : dict) -> None:
        '''
            Append a new step to the model:
            @param step - the step dict described in the class docstring.
        
        '''
        program = QStandardItem(step['program_name'])
        pre     = QStandardItem(str(step['pre_delay']))
        post    = QStandardItem(str(step['post_delay']))
        self._sequence.appendRow([program, pre, post])
        
    # Utility methods:
    
    def _initializeModel(self):
        #  Clear the model and set the column headers:
        
        self._sequence.clear()
        self._sequence.setHorizontalHeaderLabels(
            ['Program', 'Pre delay', 'Post Delay']
        )
        
    def _setButtonIcon(self, button : QPushButton, name : str) -> None:
        # Set the pixmp of the 'button' to the standard one  
        # designated by 'name'
        
        pixmap = getattr(QStyle.StandardPixmap, name)
        icon   = self.style().standardIcon(pixmap)
        button.setIcon(icon)
        
    # Private slots:
    
    def _moveRowUp(self) -> None:
        # Move the selected item up, if it's not the first item already:
        
        selection = self._table.selectedIndexes()
        if len(selection) > 0:
            row = selection[0].row()
            if row > 0:                          # Can only move non top row up.
                row_items = self._sequence.takeRow(row)
                row -= 1                         # insert in prior position.
                self._sequence.insertRow(row, row_items)
                self._table.selectRow(row)
    
    def _moveRowDown(self) -> None:
        # Move the selected item down. If it's the last item 
        # It's not moved.
        
        selection = self._table.selectedIndexes()  
        if len(selection) > 0:
            row = selection[0].row()
            if row < self._sequence.rowCount() -1:
                # not the last row.
                row_items = self._sequence.takeRow(row) 
                row += 1
                self._sequence.insertRow(row, row_items) 
                self._table.selectRow(row)
    
    def _deleteRow(self) -> None:
        # Delete the selected row:
        selection = self._table.selectedIndexes()  
        if len(selection) > 0:
            row = selection[0].row()
            self._sequence.takeRow(row)
    
class StepCreator(QWidget):
    ''''
        Compount widget that supports the creation of a step.
        A step consits of 
        * a program, selected from a list of valid program names,
        * A delay in ms before the program is run (can, of course be zero).
        * A delay in ms after the program is run (can of course be zero).
        
        So this widget is a strip of controls to define these 
        step elements and a button to request it be added to a sequence
        being edited (preumably in a SequenceTable widget).
        
        Attributes:
           programNames - the legal program names from which to select the:
           programName  - The selected program name.
           predelay     - ms in the pre-delay.
           postdelay    - ms in the post delay.
       
       Signals:
        add(name, pre, post)     - The add button was clicked.  the parameters in the widget are passed.
        
        
    '''
    add = pyqtSignal(str, int, int)
    
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # This is all a horizontal strip of widgets:
        
        self._layout = QHBoxLayout()
        self.setLayout(self._layout)
        
        self._addButton = QPushButton('Add Step', self)
        self._layout.addWidget(self._addButton)
        
        self._layout.addWidget(QLabel('Program Name:', self))
        self._program  = QComboBox(self)
        self._layout.addWidget(self._program)
        
        self._predelay  = self._addDelaySpinBox('Pre delay')
        self._postdelay = self._addDelaySpinBox('Post delay')
        
    # Utilities:
    
    def _addDelaySpinBox(self, label : str) -> QSpinBox:
        # Add a labeled spinbox suitable for setting delays.
        
        self._layout.addWidget(QLabel(label, self))
        spinbox = QSpinBox(self)
        spinbox.setRange(0, 1000*3600)  # An hour should be enough.
        spinbox.setSingleStep(100)      # 100ms steps. are grand.
        self._layout.addWidget(spinbox)
        
        
        return spinbox
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
    
    # Now a sequence table:
    
    seqtbl = SequenceTable()
    
    # Make a sequence and insert it:
    
    sequence = [
        {'program_name': 'setrun', 'pre_delay': 0, 'post_delay': 0},
        {'program_name': 'settitle', 'pre_delay': 100, 'post_delay': 150},
        {'program_name': 'begrun', 'pre_delay': 0, 'post_delay': 1000}
    ]
    seqtbl.setSteps(sequence)
    print(seqtbl.steps())
    seqtbl.show()
    
    add  = StepCreator()
    add.show()
    
    
    sys.exit(app.exec())
        
        
    