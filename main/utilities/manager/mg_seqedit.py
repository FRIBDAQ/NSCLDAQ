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
        QWidget, QHBoxLayout, QVBoxLayout, QTableView, QStyle, QSpinBox,
        QDialog, QDialogButtonBox, QApplication, QMessageBox)

from PyQt6.QtGui import  QStandardItemModel, QStandardItem
from PyQt6.QtCore import QModelIndex, QObject, pyqtSignal

import sys
import sqlite3
import pathlib
from nscldaq.mg_database import Sequence, Program

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
        deleteSequence(str) - The delete selected button was clicked
    '''
    editSequence = pyqtSignal(str)
    createSequence = pyqtSignal(str, str)
    deleteSequence = pyqtSignal(str)
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
        self._deleteSelected = QPushButton('Delete Selected')
        self._layout.addWidget(self._deleteSelected)
        
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
        self._deleteSelected.clicked.connect(self._deleteRelay)
    
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
    
    def _deleteRelay(self) -> None:
        # Emit the deleteSequence signal with the name of the selected sequence.
        
        selected = self._seqview.selectedIndexes()
        if len(selected) > 0:
            seqname = self._seqlist.itemFromIndex(selected[0]).text()
            self.deleteSequence.emit(seqname)
            
            
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
           programName  - The selected program name. (readonly)
           predelay     - ms in the pre-delay. (readonly)
           postdelay    - ms in the post delay. (readonly)
       
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
        
        
        self._addButton.clicked.connect(self._relayAdd)
    # Define attributes:
    
    def programNames(self) -> list[str]:
        ''' @return list[str] names of programs in the combobox.'''
        
        return [self._program.itemText(row) for row in range(self._program.count())]
    
    def setProgramNames(self, names: list[str]) -> None:
        self._program.clear()
        for name in names:
            self._program.addItem(name)
    
    def programName(self) -> str:
        '''@return str return the value of the program name combobox.'''
        
        return self._program.currentText()
    
    def predelay(self) -> int:
        ''' @return int - value of the predelay spinbox'''
        
        return self._predelay.value()
    
    def postdelay(self) -> int:
        ''' @return int - value of he postdelay spinbox. '''
        return self._postdelay.value()
    
    
    #  INternal privatge slots:
    
    def _relayAdd(self) -> None:
        
        #emit the add signal:
            
        self.add.emit(self.programName(), self.predelay(), self.postdelay())   
    
    # Utilities:
    
    def _addDelaySpinBox(self, label : str) -> QSpinBox:
        # Add a labeled spinbox suitable for setting delays.
        
        self._layout.addWidget(QLabel(label, self))
        spinbox = QSpinBox(self)
        spinbox.setRange(0, 1000*3600)  # An hour should be enough.
        spinbox.setSingleStep(100)      # 100ms steps. are grand.
        self._layout.addWidget(spinbox)
               
        return spinbox

class SequenceEditor(QWidget):
    '''
        Compound widget to support editing a 
        sequence.  So this is basically a SequenceTable stacked on top
        of a StepCreator with the autonomous handling of the 
        Add Step button in the step creator working
        to add a step to the sequence table.
        
        We also add some decoration:  
        sequence name and the trigger state.  To make our life easier,
        the attributes are:
        
        name - Sequence name. 
        states - triggers states to select from.
        trigger - The selected trigger.
        sequence - The SequencdeTable widget from which the steps can be fetched
               (readonly)
        creator - The StepCreator object (readonly).
               
        No signals are emitted, though normally this all gets embedded in a 
        SequenceEditorDialog so the external controller code knows if/when
        to save te sequence.
        
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        # Step and trigger are laid out horizontally:
        
        decorators = QHBoxLayout()
        decorators.addWidget(QLabel('Sequence name: ', self))
        self._name = QLineEdit(self)
        decorators.addWidget(self._name)
        
        decorators.addWidget(QLabel('Trigger Stte', self))
        self._trigger = QComboBox(self)
        decorators.addWidget(self._trigger)
        
        self._layout.addLayout(decorators)
        
        
        # The meat of the widget.
        
        self._sequence = SequenceTable(self)
        self._layout.addWidget(self._sequence)
        
        self._stepmaker = StepCreator(self)
        self._layout.addWidget(self._stepmaker)
        
        # Connect the add in stepmaker to an internal slot to
        # add the step to the table:
        
        self._stepmaker.add.connect(self._addStep)
    
    
    #  Attribute implementations.
    
    def name(self) -> str:
        ''' @return str - name of the sequence '''
        return self._name.text()
    def setName(self, name : str) -> None:
        '''
        @param name - new name for the sequence.
        '''
        self._name.setText(name)
        
    def states(self) -> list[str]:
        ''' @return list[str] - list of states'''
        return [self._trigger.itemText(c) for c in range(self._trigger.count())]
    
    def setStates(self, states: list[str]) -> None:
        '''  
            @param states - list of state names to populate the trigger selection combobox.
        '''
        self._trigger.clear()
        for state in states:
            self._trigger.addItem(state)
    
    def trigger(self) -> str:
        ''' @return str - current trigger state'''    
        return self._trigger.currentText()
    def setTrigger(self, state: str) -> None:
        ''' @param state - a state from the comboboxl (set states  value)
             @throws ValueError if state is not in the combobox.
        
        '''
        if state not in self.states():
            raise ValueError(f'{state} is not a valid trigger state')
        self._trigger.setCurrentText(state)
        
    
    def sequence(self) -> SequenceTable:
        ''' @return SequenceTable - the table with the sequence steps.'''
        return self._sequence
    
    def creator(self) -> StepCreator:
        ''' @return StepCreator - the step creation subwidget.'''
        
        return self._stepmaker
    
    # Internal, private slots:
    
    def _addStep(self, name : str, pre : int, post: int) -> None:
        # Handle the Add Step button click:
        
        step = {
            'program_name': name, 'pre_delay': pre, 'post_delay': post
        }
        self._sequence.append(step)

class SequenceEditorDialog(QDialog): 
    '''
        This is a dialog with a work area that consists of a SequenceEditor
        the workarea method returns the SequencdeEditor object it contains.
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        self._workarea = SequenceEditor(self)
        self._layout.addWidget(self._workarea)
        
        self._buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Save | QDialogButtonBox.StandardButton.Cancel,
            self
        )
        self._layout.addWidget(self._buttons)
        
        # Hook in the buttons.
        
        self._buttons.accepted.connect(self.accept)
        self._buttons.rejected.connect(self.reject)
    
    def workarea(self) -> SequenceEditor:
        ''' @return the SequenceEditor widget that is the workarea: '''
        
        return self._workarea

class SequenceEditController(QObject):
    ''''
        This controller mediates between the sequence editor views 
        and a manager configuration database.
    '''  
    def __init__(self, view : SequenceSelector, dbfile : str, parent : QObject | None = None):
        '''
            @param view - the top level view of the controller. Note that  the
                  controller may spin off SequenceEditorDialog windows too.
            @param dbfile - The path to the configuration database file.
            @param parent - parent object of the controlle.r
        '''
        super().__init__(parent)
        self._view = view
        self._config_file = dbfile
        self._db = sqlite3.Connection(dbfile)
    
        # Populate the view:
        
        self._stock_view()
        
        # Connect view signals:
        # We're going to see about channelling the 
        # Handling of the dialog to mostly common code:
        
        self._view.editSequence.connect(self._editExistingSequence)
        self._view.createSequence.connect(self._createNewSequence)
        self._view.deleteSequence.connect(self._deleteSequence)
    
    # Private slots:
    
    def _editExistingSequence(self, name : str) -> None:
        # Pop up a dialog for editing an existing sequence.
        # Note that the trigger state has to come from the database,
        # though the valid triggers come from the view.
        
        seq_api = Sequence(self._db)
        seq_def = [x for x in seq_api.list() if x['name'] == name][0]
        dialog = self._stockCommonDialogElements(name, seq_def['trigger'])
        self._stockSequenceDefinition(dialog, seq_def)
        
        self._processDialog(dialog)
    
    def _createNewSequence(self, name : str, trigger : str) -> None:
        #  Pop up a dialog to create a brand new sequence:
        
        dialog = self._stockCommonDialogElements(name, trigger)
        self._processDialog(dialog)
    
    def _deleteSequence(self, name : str) -> None:
        # Delete the named sequence.
        
        seq_api = Sequence(self._db)
        seq_api.deleteSequence(name)
        self._stock_view()
        
    def _acceptSequence(self, dialog : SequenceEditorDialog) -> None:
        # This slot is called when the sequence editing dialog 
        # completes with the user accepting the edited sequence.
        # If the sequencde exists we first need to delete it.
        # Either way, we fish the stuff we need out of the dialog
        # and make a new one.  Note that if the sequence name is blank
        # we pop up a message box can't exec again because
        # dialog does not like recursive execs
        #
        seq_api  = Sequence(self._db)
        workarea = dialog.workarea()
        seq_name = workarea.name()
        if not seq_name.strip():
            QMessageBox.warning(dialog,
                                'Missing sequence name', 
                                'To make a sequence you must give it a name')
            return
        
        # If the sequence exists we need to delete it.
        
        if seq_api.exists(seq_name):
            seq_api.deleteSequence(seq_name)
        
        # Now we know we can insert the new sequence definition:
        
        trigger = workarea.trigger()
        sequence_widget = workarea.sequence()
        step_dictlist = sequence_widget.steps()
        
        # Need to impedance match the steps to what Sequence.add expects:
        
        steps = list()
        for step in step_dictlist:
            steps.append(
                [step['program_name'], step['pre_delay'], step['post_delay']]
            )
    
        seq_api.add(seq_name, trigger, steps)   # Step #s get assigned here too.
        

    #  Utilities:
    
    def _stock_view(self) -> None:
        # Fill the view:
        
        seq_api = Sequence(self._db)
        states = seq_api.listStates()
        
        self._view.setStates(states)
        
        seq_names = [x['name'] for x in seq_api.list()]
        self._view.setSequenceNames(seq_names)
            
    
    def _stockCommonDialogElements(self, name : str, trigger: str) -> SequenceEditorDialog:
        # This utility creates a SequenceEditorDialog and stocsk the elements
        # that are present no matter if the dialog is creating a new sequence
        # or editing an old one.
        
        dialog = SequenceEditorDialog(self._view)
        workarea = dialog.workarea()      # This is the SequencEditor objecft:
        workarea.setStates(self._view.states())
        workarea.setName(name)
        workarea.setTrigger(trigger)
        
        
        # for the work area, we need to set the program names
        
        creator = workarea.creator()    # StepCreator object.
        program_api = Program(self._db)
        program_names = [x['name'] for x in program_api.list()]
        creator.setProgramNames(program_names)
        
        return dialog
        
    def _stockSequenceDefinition(self, dialog: SequenceEditorDialog, definition : dict) -> None:
        #
        #  This is called when setting up the dialog to edit an existing sequence.
        #  We need to stock the SequenceTable element with the sequence steps.
        #
        workarea = dialog.workarea()   # SequenceEditor
        seqTable = workarea.sequence() # SequenceTable.
        
        
        # Marshall the steps into the form expected by
        # The table, for this we need a dict of program_id -> name
        programapi = Program(self._db)
        program_list = programapi.list()
        
        
        steps = list()       # List of program dicts.
        for step in definition['steps']:
            program = step[0]
            pre  = step[1]
            post = step[2]
            
            number = step[4]
            
            step_dict = {
                'step_number' : number,
                'program_name': program,
                'pre_delay':    pre,
                'post_delay':   post
            }
            steps.append(step_dict)
            
        seqTable.setSteps(steps)
        
        
    def _processDialog(self, dialog: SequenceEditorDialog) -> None:
        
        # We're going to connect the accepted signal to a 
        # Lambda that passses the dialog to the 
        # acceptance handler so it can fish stuff out of the
        # dialog:
        
        dialog.accepted.connect(lambda :self._acceptSequence(dialog))
        
        dialog.exec()
        self._stock_view()    # Update the list of sequences
        
        

def usage() -> None:
    '''
    Print the program usage.
    '''
    print('''
Usage:
    $DAQBIN/mg_seqedit config-path
Where:
    config-path - is the filesystem path to the configuration database file.
          ''', file=sys.stderr)
    
def main() -> int:
    '''
    The entry point for the program.  We need a databas filepath
    '''
    if len(sys.argv) != 2:
        usage()
        return -1
    
    # Let's make sure the config-path exist:
    
    config_path = pathlib.Path(sys.argv[1])
    if not config_path.exists() :
        print('No such file: ', str(config_path))
        usage()
        return -1
    
    #  Make the user interface and attach a controller to it:
    
    app = QApplication(sys.argv)
    win = SequenceSelector()
    
    _controller = SequenceEditController(win, str(config_path))
    
    win.show()
    return app.exec()
    

if __name__ == '__main__':
    
    sys.exit(main())
        
    