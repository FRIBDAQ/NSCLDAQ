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
This program provides a mechanism to allow users to edit
the managed environment state machine.  Unless you know what
you are doing and are prepared to build a custom UI to
control your experiment, this is not recommended.  Editing the
state machine means adding and removing states as well as defining
legals state transitions.  Legal state transitions are defined



'''

import sqlite3
import sys

from nscldaq import mg_database
from nscldaq.mg_configutils import SaveDialog
from PyQt6.QtCore import QObject, pyqtSignal
from PyQt6.QtWidgets import (
    QApplication,
    QComboBox,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QListWidget,
    QListWidgetItem,
    QPushButton,
    QVBoxLayout,
    QWidget,
)


class StateEditor(QWidget):
    '''
        This widget provides an editor for the state machine.
        Specifically one can add and remove states and add and remove precursor and 
        successors to a selected state.
        
        The visual representation is three listboxes the middle one, lists the states.
        The left box, predecessor states to the selected state, the right one, successor states
        reachable from the selected state.
        
        Buttons below the three listboxes allow predecessors, states, or successors to be removed.
        A line edit and button below the state list box allows new states to be defined.  A ComboBox and button
        below the successor and prececessor states allows a new successor/predecessor state to be defined.
        
        Attributes:
          stateMachine - Allows the entire state machine to be loaded/fetched.
          
        Key methods:
          newState    - Add a new state to the internal model of the state machine
          newTransition - Add a new transition to the internal model of the state machine
          removeState  - Remove a new state from the internal state machine model
          removeTransition - Remove a transition from the internal state machine model.
          
        Signals:
            addState(str) - A new state was added
            deleteState(str) - A state was removed.
            addTransition(str, str) - A new transition was added, parameters are from/to.
            removeTransition(str, str) - A transition was deleted, parameters are from/to.
            
        Note, the signals don't automatically run the methods associated with them.  That's left
        for the signal handler to do...as after all, databgase manipulations to do that may fail.
        
        Note that the state machine is defined as a list of dicts.  The dicts have keys:

            * name - name of the state.
            * precursors - List of names of the legal precursor states.
            * successors - list of names of the legal successor states

            A precursor state is one with a legal transition to the state in 'name'.
            A successor state is one that 'name' can transition to.
        
    '''
    addState         = pyqtSignal(str)
    deleteState      = pyqtSignal(str)
    addTransition    = pyqtSignal(str, str)
    deleteTransition = pyqtSignal(str, str)
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # the main layout is left to right for each of the
        # prececessor, state and successor regions.
        
        self._layout = QHBoxLayout(self)  
        
        self._layout.addLayout(self._createPrecursorWidgets())
        self._layout.addLayout(self._createStateWidgets())
        self._layout.addLayout(self._createSuccessorWidgets())
        
        self.setLayout(self._layout)
        
        self._stateModel = list()                   # Start with an empty model.
        
        
        # Connect Signals to internal slots:
        
        #   Clicks in the state list:
        self._stateList.itemClicked.connect(self._setPrecursors)
        self._stateList.itemClicked.connect(self._setSuccessors)
        self._stateList.itemClicked.connect(self._loadStateLabel)
        
        # Clicks in the precursor and successor lists:
        
        self._precursorWidgets['list'].itemClicked.connect(self._selectPrecursor)
        self._successorWidgets['list'].itemClicked.connect(self._selectSuccessor)
        
        # Set up the button slots for the delete and add buttons on the
        # state  list widget.
    
        self._deleteState.clicked.connect(self._signalDeleteState)
        self._addState.clicked.connect(self._signalAddState)
        
        # Set up button slots below the precursor widgets to work properly.
        
        self._precursorWidgets['add'].clicked.connect(self._signalAddPrecursor)
        self._precursorWidgets['delete'].clicked.connect(self._signalDeletePrecursor)
        
        # Set up the button slots for the successor widgets to work correctly.
        
        self._successorWidgets['add'].clicked.connect(self._signalAddSuccessor)
        self._successorWidgets['delete'].clicked.connect(self._signalDeleteSuccessor)
    
    # Attributes implementations.
    
    def stateMachine(self) -> list[dict]:
        ''' 
        @return list[dict] - return the internal model of the state machine.
           See the class docstrings for the form of this list of dicts.
        '''
        return self._stateModel
    
    def setStateMachine(self, stateMachine : list[dict]) -> None:
        '''
        Set the contents of the internal model of the state machine.
        
        @param stateMachine - the new internal model of the state machine to use.
            This is a list of state definitions as described in the class docstrings.
        '''
        self._stateModel = stateMachine

        # Now load the state list, and signal the current row was clicked  to 
        # Load the precursor and successor list boxes.
        # Clear the selected labels in all list boxes.
        
        self._stockListBox(self._stateList, self._enumerateStates())
    
    
    # The public mthods needed to edit the internal model.
    
    def newState(self, state: str) -> None:
        '''
            Add a new state to the model. 
            @param state -name of the new state.
            @throws ValueError if the state aready exists.
        '''
        
        if state  in self._enumerateStates():
            raise ValueError(f'There is already a state named {state}')
        
        self._stateModel.append( {
            'name': state, 'precursors':  list(), 'successors': list()
        })
        # Update the list box too:
        
        self._stockListBox(self._stateList, self._enumerateStates())
    
    def removeState(self, state : str) -> None:
        '''
            Remove a state from the model.
            @param state -name of the state to remove.
            @throw ValueError if the state does not exist.
        '''
        if state not in self._enumerateStates():
            raise ValueError(f'There is no state named {state}')
        
        info = self._findState(state)
        self._stateModel.remove(info)
        
        # Refresh the state list:
        self._stockListBox(self._stateList, self._enumerateStates())
    
    def newTransition(self, initial : str, final : str) -> None:
        '''
            Adds a new transition to the model.  This adds
            * final to the successor states of initial and
            * initial to the precursor states of final.
            
            @param initial  - name of the initial state of the transition.
            @param final    - name of the final state of the transition.
            @throw ValueError if initial or final are not valid states
                              * initial is already a precursor for final
                              * final is already a successor for initial.
        '''
        states = self._enumerateStates();
        if initial not in states :
            raise ValueError(f'{initial} is not a defined state')
        if final not in states:
            raise ValueError(f'{final} is not a defined state')
        
        initial_info = self._findState(initial)
        final_info   = self._findState(final)
        
        if final in initial_info['successors']:
            raise ValueError(f'{final} is already a successor state for {initial}')
        if initial in final_info['precursors']:
            raise ValueError(f'{initial} is already a precursor to {final}')
        
        # Update the data and load the listboxes for the precursor/successors:
        
        initial_info['successors'].append(final)
        final_info['precursors'].append(initial)
        
        # If the current state is the initial state, load the successor list box.
        # and the combobox
        
        current_state= self._stateList.currentItem().text()
        if initial == current_state:
            self._stockListBox(self._successorWidgets['list'], initial_info['successors'])
            self._stockComboBox(
                self._successorWidgets['statelist'], 
                [x for x in states if x not in initial_info['successors']])
        
        # If the current state is the final state, load the precursor listbox.
        # and the combobox.
        
        if final == current_state:
            self._stockListBox(self._precursorWidgets['list'], final_info['precursors'])
            self._stockComboBox(
                self._precursorWidgets['statelist'], 
                [x for x in states if x not in final_info['precursors']])
    
    def removeTransition(self, initial, final) -> None:
        '''
        Removes a transition from the model and updates the view accordingly.  This
        removes the initial state from the precursors of the final state and removes
        the final state from the successors of the initial state:
        
        @param initial - initial state of the transition to remove.
        @param final   -  final state of the trnsition to remove.
        @raise ValueError if either initial or final are not valid states or:
                         initial is _not_ a precursor to to final, or
                         final is _not_ a successor to initial
        '''
        states = self._enumerateStates();
        if initial not in states :
            raise ValueError(f'{initial} is not a defined state')
        if final not in states:
            raise ValueError(f'{final} is not a defined state')
        
        initial_info = self._findState(initial)
        final_info   = self._findState(final)
        
        if final not in initial_info['successors'] and initial not in final_info['precursors']:
            raise ValueError(f'{final} -> {initial} is not a known state transition')
        
        
        # Update the model:
        
        initial_info['successors'].remove(final)
        final_info['precursors'].remove(initial)
        
        # Now update the appropriate list of states and combo boxes:
        
        current_state = self._stateList.currentItem().text()
        if initial == current_state:
            # sucessors needs updating:
            
            self._stockListBox(self._successorWidgets['list'], initial_info['successors']) 
            self._stockComboBox(
                self._successorWidgets['statelist'], 
                [x for x in states if x not in initial_info['successors']])
        
        if final == current_state:
            self._stockListBox(self._precursorWidgets['list'], final_info['precursors'])
            self._stockComboBox(
                self._precursorWidgets['statelist'], 
                [x for x in states if x not in final_info['precursors']])
            
    # Internal slots:
    
    
    #  These methods handle clicks in the state list:
    
    def _loadStateLabel(self, item : QListWidgetItem) -> None:
        #  Load the state label with the selecte item:
        
        self._selectedState.setText(item.text())
    
    def _setPrecursors(self, item : QListWidgetItem) -> None:
        self._setTransitionStates(item, 'precursors', self._precursorWidgets)
        
    def _setSuccessors(self, item :QListWidgetItem) -> None:
        self._setTransitionStates(item, 'successors', self._successorWidgets)
    
    def _setTransitionStates(self, item, which, whichWidgets):
        #  Set the appropriate transition region
        #  which - is which key in the state machine to use to get the states
        #  whichWidgets is the map of widgets to stock:
        # This is commmon code for setPrecursors and setSuccessors.
        
        current_state = item.text()
        state_info    = self._findState(current_state)
        transitions    = state_info[which]
        self._stockListBox(whichWidgets['list'], transitions)
        
        # Figure out which states belong in the combobox:
        # All states but the ones alread in precursors;
        
        states = self._enumerateStates()
        selectable_states = [x for x in states if x not in transitions]
        self._stockComboBox(whichWidgets['statelist'], selectable_states)
   
    #  Methods handle clicks in the precursor/successor list.
    #  Too small to be worth factoring  into common code.
    
    def _selectPrecursor(self, item :  QListWidgetItem) -> None:
        self._precursorWidgets['selected'].setText(item.text())
        
    def _selectSuccessor(self, item : QListWidgetItem) -> None:
        self._successorWidgets['selected'].setText(item.text())
    
    # Methods to handle the buttons under the state list:
    
    def _signalAddState(self) -> None:
        # Add button clicked to emit the signal requires the text edit to be non empty:
        
        newState = self._newStateName.text()
        if newState.strip():
            self.addState.emit(newState)
        
    def _signalDeleteState(self) -> None:
        # The delete button was clicked  Pass the current item in the
        # list to the signal:
        
        self.deleteState.emit(self._stateList.currentItem().text())
    
    # Methods to handle clicks inthe precursor buttons.
    
    def _signalAddPrecursor(self) -> None:
        # Get the combobox as From and the selecte state as to and signal
        # addTransition:
        
        from_state = self._precursorWidgets['statelist'].currentText()
        to_state   = self._selectedState.text()
        
        self.addTransition.emit(from_state, to_state)
        
    def _signalDeletePrecursor(self) -> None:
        # The selected precursor is the from state and the selected state the to state
        # to signal deleteTransition:
        
        from_state = self._precursorWidgets['selected'].text()
        to_state   = self._selectedState.text()
        
        self.deleteTransition.emit(from_state, to_state)
    
    # Methods to handle clicks inthe successor buttons:
    
    def _signalAddSuccessor(self) -> None:
        # Now the succesor is the to tate:
        
        from_state = self._selectedState.text()
        to_state   = self._successorWidgets['statelist'].currentText()
        self.addTransition.emit(from_state, to_state)
        
    def _signalDeleteSuccessor(self) -> None:
        
        from_state = self._selectedState.text()
        to_state   = self._successorWidgets['selected'].text()
        self.deleteTransition.emit(from_state, to_state)
    #  GUI layout utilities:
    
    def _createPrecursorWidgets(self) -> QVBoxLayout:
        #  Create the widgets for the precursors.
        
        layout = QVBoxLayout()
        self._precursorWidgets = dict()
        self._createTransitionWidgets('Precursor States', layout,  self._precursorWidgets)
        return layout

    def _createStateWidgets(self) -> QVBoxLayout:
        # Create the widgets for the state:
        
        layout =  QVBoxLayout()            # mostly vertical box.
        layout.addWidget(QLabel('States:', self))
        self._stateList = QListWidget(self)
        layout.addWidget(self._stateList)
        
        # The deletion area contains a label with the selected state
        # and a button next to it to remove that state:
        
        deleteLayout = QHBoxLayout()
        
        self._selectedState = QLabel('', self)
        deleteLayout.addWidget(self._selectedState)
        
        self._deleteState = QPushButton('Delete', self)
        deleteLayout.addWidget(self._deleteState)
        
        layout.addLayout(deleteLayout)
        
        # The add area contains a text edit into which the
        # user can type the name of a new state and 
        # click the 'Add' button next to it to add the state.
        
        addLayout = QHBoxLayout()
        
        self._newStateName = QLineEdit(self)
        addLayout.addWidget(self._newStateName)
        
        self._addState = QPushButton('Add', self)
        addLayout.addWidget(self._addState)
        
        layout.addLayout(addLayout)
        
        return layout

    def _createSuccessorWidgets(self) -> QVBoxLayout:
        # Create the widgets for the successors./
        layout = QVBoxLayout()
        self._successorWidgets = dict()
        self._createTransitionWidgets('Successor States', layout, self._successorWidgets)
        return layout
    
    def _createTransitionWidgets(self, name: str, layout : QVBoxLayout, widgetMap : dict[str, QWidget]) -> None:
        #
        #  Create the widgets for a transition.  widgetMap is a map that will, when we're done
        #  have the following keys:
        #   * 'list'  - The listwidget containing the state names.
        #   * 'selected' - label widget for the selected state.
        #   * 'delete' - The delete pushbutton.
        #   * 'statelist' - The combobox with the list of valid states.
        #   * 'add'    - The add transition combobox.
        # The other parameters are:
        #  name - Text for a label at the top of the area.
        #  layout - A layout into which the widgets are inserted. This is assumed to be a
        #           vertical box layout.

        layout.addWidget(QLabel(name, self))
        widgetMap['list'] = QListWidget(self)
        layout.addWidget(widgetMap['list'])
        
        # Label and delete button for the selected state:
        
        deleteLayout = QHBoxLayout()
        widgetMap['selected'] = QLabel('', self)
        deleteLayout.addWidget(widgetMap['selected'])
        
        widgetMap['delete'] = QPushButton('Delete', self)
        deleteLayout.addWidget(widgetMap['delete'])
        
        layout.addLayout(deleteLayout)
        
        # The combobox state selector and 
        # add button:
        
        addLayout = QHBoxLayout()
        widgetMap['statelist'] = QComboBox(self)
        addLayout.addWidget(widgetMap['statelist'])
        
        widgetMap['add'] = QPushButton('Add', self)
        addLayout.addWidget(widgetMap['add'])
        
        layout.addLayout(addLayout)
    
    # Utility methods:
    
    def _findState(self, name : str) -> dict[str, list[str]]:
        # Locate the dict with the specified name.
        
        return [x for x in self._stateModel if x['name'] == name][0]
    
    def _stockListBox(self, widget : QListWidget, items : list[str]) -> None:
        # Stock a list box widget with the items passesd in.
        # The first item is seleted and 'clicked'.
        widget.clear()
        widget.addItems(items)
        
        
        if len(items):
            widget.setCurrentRow(0)
            widget.itemClicked.emit(widget.item(0))
    
    def _stockComboBox(self, widget : QComboBox, items) -> None:
        # Stock the items that can be selected in a combobox:
        
        widget.clear()
        widget.addItems(items)
        
    def _enumerateStates(self) -> list[str]:
        return   [x['name'] for x in self._stateModel]      

class StateEditorDialog(SaveDialog):
    '''
        This is a dialog that has the state editor as the
        work area.  
    '''
    def __init__(self, parent : QWidget | None = None):
        super().__init__(StateEditor(), parent)


class StateEditorController(QObject):
    ''''
        This class provides the controller for runnning
        a StateEditor Dialog.
    
    '''
    def __init__(self, dbfile : str, view : StateEditorDialog, parent : QObject| None = None) :
        super().__init__(parent)
        
        # Save the file and view.
        # create an sqlite3 connection on the file and stock the view..
        
        self._config = dbfile
        self._db     = sqlite3.Connection(self._config)
        self._dialog = view
        self._workarea = view.workarea()
        
        self._computeStateMachine()
        # Hook into the signals we have to process:
        
        self._dialog.accepted.connect(self._processStateMachine)   # Done update the db.
        
        self._workarea.addState.connect(self._newState)
        self._workarea.deleteState.connect(self._stateDeleted)
        self._workarea.addTransition.connect(self._newTransition)
        self._workarea.deleteTransition.connect(self._deleteTransition)
        
    def _computeStateMachine(self):
        # Compute the state machine for he dialog.
        # We need to build the list of dicts descsribed in the docstring heading for 
        # StateEditor.
        dbapi = mg_database.Sequence(self._db)
        states = dbapi.listStates()
        db_info = list()
        for state in states:
            state_info = {'name': state}
            # Find the precursors and sucessor states:
            state_info['precursors'] = dbapi.legalFromStates(state)
            state_info['successors'] = dbapi.legalSuccessorStates(state)
            db_info.append(state_info)
        self._workarea.setStateMachine(db_info)
    
    # Slots:
    
    def _newState(self, name : str) -> None:
        # New state added:
        
        self._workarea.newState(name)

    def _stateDeleted(self, name: str) -> None:
        # State deleted.
        
        self._workarea.removeState(name)
        
    def _newTransition(self, pred: str, succ : str) -> None:
        # New transition added:
        self._workarea.newTransition (pred, succ)
        
    def _deleteTransition(self, pred: str, succ: str) -> None:
        # Transition deleted:
        self._workarea.removeTransition(pred, succ)
          
    def _processStateMachine(self):
        # The user accepted the dialog.  Update the state machine
        # to match the current image of it.
            
        state_machine = self._workarea.stateMachine()
        api           = mg_database.Sequence(self._db)
        
        # This whole thing should be rolled back
        # if there are exceptions:
        
        self._db.execute('''SAVEPOINT updating_statemachine''', tuple())
        
        try:
            
            my_states = [x['name'] for x in state_machine]
            db_states = api.listStates()
            
            # Add any new states:
            
            for ms in my_states:
                if ms not in db_states:
                    api.addState(ms)         # New state.
                    
            # reconcile the transitions and ensure they are the same.
            
            for state in state_machine:
                self._reconcilePredecessors(api, state)
                self._reconcileSuccessors(api, state)
                
        
            # Delete any obsolete states.
            
           
            for dbs in db_states:
                if dbs not in my_states:
                    api.deleteState(dbs)     # Obsolete state.
            
                
                
            
        except Exception:
            # Roll back the operation if there's any error.
            self._db.execute('''ROLLBACK TO SAVEPOINT updating_statemachine''', tuple())
            raise
        # Commit the changes:
        # Some methods of sequence commit so we ignore errors below:
        try: 
            self._db.execute('''RELEASE SAVEPOINT updating_statemachine''', tuple())
        except Exception:
            pass
    def _reconcilePredecessors(self, api : mg_database.Sequence, state: dict) -> None:
        # Make the database valid precursors match those in the dialog.
        # state the dict of state name, precursors and successors:
        
        name = state['name']
        my_precursors = state['precursors']
        db_precursors = api.legalFromStates(name)
        
        # Remove obsolete transitions:
        
        for dbp in db_precursors:
            if dbp not in my_precursors:
                api.removeTransition(dbp, name)
        
        # add in any new transitions:
        
        for myp in my_precursors:
            if myp not in db_precursors:
                api.addTransition(myp, name)

    def _reconcileSuccessors(self, api : mg_database.Sequence, state : dict) -> None:
        # Make the database valid successors match the dialog's:
        # state the dict of sttae name, precursors and successors
        
        name = state['name']
        my_successors = state['successors']
        db_successors = api.legalSuccessorStates(name)
        
        # Remove obsolete transitions:
        
        for dbs  in db_successors:
            if dbs not in my_successors:
                api.removeTransition(name, dbs)
            
        # Add in new transitions:
        
        for mys in my_successors:
            if mys not in db_successors:
                api.addTransition(name, mys)

def usage():
    ''' Print program usage to stderr '''
    print('''
Usage:
    $DAQBIN/mg_stateedit config-path
Where:
    config-path - is the filesystem path to the configuration database file.
    
    ''', file = sys.stderr)

def main() -> int:
    ''' Program entry point'''
    if len(sys.argv) != 2:
        usage()
        return -1
    
    dbfile = sys.argv[1]
    
    #  Create the application, the dialog
    # the controller and run this all:
    
    _app = QApplication(sys.argv)
    win = StateEditorDialog()
    _controller = StateEditorController(dbfile, win)
    
    win.exec()
    
    return 0

if __name__ == '__main__':
    sys.exit(main())
        
        