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

from PyQt6.QtWidgets import (QListWidget, QListWidgetItem, 
            QLabel, QLineEdit, QPushButton, QComboBox,
            QVBoxLayout, QHBoxLayout, QWidget)
from PyQt6.QtCore import QObject


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
          addState    - Add a new state to the internal model of the state machine
          addTransition - Add a new transition to the internal model of the state machine
          removeState  - Remove a new state from the internal state machine model
          removeTransition - Remove a transition from the internal state machine model.
          
        Signals:
            addState(str) - A new state was added
            deleteState(str) - A state was removed.
            addTransition(str, str) - A new transition was added, parameters are from/to.
            deleteTransition(str, str) - A transition was deleted, parameters are from/to.
            
        Note, the signals don't automatically run the methods associated with them.  That's left
        for the signal handler to do...as after all, databgase manipulations to do that may fail.
        
        Note that the state machine is defined as a list of dicts.  The dicts have keys:

            * name - name of the state.
            * precursors - List of names of the legal precursor states.
            * successors - list of names of the legal successor states

            A precursor state is one with a legal transition to the state in 'name'.
            A successor state is one that 'name' can transition to.
        
    '''
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
        
        self._stateList.itemClicked.connect(self._setPrecursors)
        self._stateList.itemClicked.connect(self._setSuccessors)
        self._stateList.itemClicked.connect(self._loadStateLabel)
    
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
# Test code for now:

if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication
    import sys

    app = QApplication (sys.argv)
    win = StateEditor()
    
    dummy_statemachine = [
        {'name': 'SHUTDOWN', 'precursors': ['SHUTDOWN', 'BOOT', 'HWINIT'], 'successors' : ['BOOT',]}, 
        {'name': 'BOOT',     'precursors' : ['SHUTDOWN',], 'successors': ['HWINIT', 'SHUTDOWN'] },
        {'name': "HWINIT",   'precursors' : ['BOOT',], 'successors' : ['SHUTDOWN']},
    ]
    
    win.setStateMachine(dummy_statemachine)
    
    win.show()
    sys.exit(app.exec())
        
        