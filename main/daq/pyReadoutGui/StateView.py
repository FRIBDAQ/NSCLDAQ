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
@file StateView.py
@brief Provides widgets for handling states and their transitions.
@author Ron Fox
'''

from PyQt6.QtWidgets import (QPushButton, QLabel, QCheckBox, QSpinBox, QStackedWidget,
    QHBoxLayout, QVBoxLayout, QWidget)
from PyQt6.QtCore     import pyqtSignal, Qt

from typing import ClassVar



class QStateButtons(QWidget):
    '''
        Given the state, displays the correct buttons to handle state transitions:
        
        - "Not Ready"  Displays the 'Start' push button.
        - "Starting"   Displays no buttons but the widget has a wait cursor.
        - 'Halted'     Displays a Begin button.
        - 'Active'     Displays an End button and a Pause button.
        - 'Paused'     Displays an End and a Resume Button
        
        Unlike the Tcl version of this, we can use QStackedWidget and widget hiding
        rather than ghosting and re-labeling.  This allows us to provide fixed signal mapping
        for each button.
        
        Speaking of which, the signals we provide are:
        
        start    - Request a start of the system.
        begin    - Request a start run.
        end      - request an end run.
        pause    - request a pause run.
        resume   - request a resume run.
        
        
        Attributes:
           state - the current state.
    '''
    # Define signals:
    
    start = pyqtSignal()
    begin = pyqtSignal()
    end   = pyqtSignal()
    pause = pyqtSignal()
    resume = pyqtSignal()
    
    
    # Tables indexed by state determining which button index is
    # visible for each stacked widget:

    leftIndex : ClassVar[dict[str, int]] = {
        'Not Ready' : 1,       # Start button
        'Starting'  : 0,       # No button.
        'Halted'    : 2,       # begin button.
        'Active'    : 3,       # End button.
        'Paused'    : 3,       # End button.
    }    
    rightIndex : ClassVar[dict[str, int]] = {
        'Not Ready' : 0,        # No button.
        'Starting'  : 0,        # no button
        'Halted'    : 0,        # No button.
        'Active'    : 1,        # Pause button.
        'Paused'    : 2,        # Resume button.
    }
    
    def __init__(self, parent : QWidget | None = None):
        '''
        @param parent - if given, the parant of this widget.
        '''
        
        super().__init__(parent)
        
        self._layout = QHBoxLayout()    # Buttons are side by side.
        self.setLayout(self._layout)

        # Two widget stacks.
        # Index 0 of each widget stack is a hidden push button.

        self._leftButtons = QStackedWidget(self)
        self._rightButtons= QStackedWidget(self)
        
        self._layout.addWidget(self._leftButtons)
        self._layout.addWidget(self._rightButtons)

        # Fill in the left button stack:
        
        self._leftButtons.addWidget(QPushButton('Hidden', self._leftButtons))
        self._leftButtons.widget(0).hide()
        
        self._leftButtons.addWidget(QPushButton('Start', self._leftButtons))
        self._leftButtons.widget(1).clicked.connect(self.start)
        
        self._leftButtons.addWidget(QPushButton('Begin', self._leftButtons))
        self._leftButtons.widget(2).clicked.connect(self.begin)
        
        self._leftButtons.addWidget(QPushButton('End', self._leftButtons))
        self._leftButtons.widget(3).clicked.connect(self.end)
        
        #  Now the right buttons:
        
        self._rightButtons.addWidget(QPushButton('Hidden', self._rightButtons))
        self._rightButtons.widget(0).hide()
        
        self._rightButtons.addWidget(QPushButton('Pause', self._rightButtons))
        self._rightButtons.widget(1).clicked.connect(self.pause)
        
        self._rightButtons.addWidget(QPushButton('Resume', self._rightButtons))
        self._rightButtons.widget(2).clicked.connect(self.resume)
    
        self._state = None   
        self._normalCursor = self.cursor()
    
    # Attributes:
    
    def state(self) -> str | None:
        '''@return str - the state name.  or None if it's never been set.'''    
        return self._state

    def setState(self, state : str) -> None:
        '''
            @param state : str - State the buttons shouild take on.
                            Raises 
        '''
        if state not in self.leftIndex.keys(): 
            raise ValueError(f'{state} is not a valid state name.')
        
        self._leftButtons.setCurrentIndex(self.leftIndex[state])
        self._rightButtons.setCurrentIndex(self.rightIndex[state])
        
        
        # If the state is 'Starting' set the cursor to a wait indicator else
        # set it to the standard one.
        
        if state == 'Starting':
            self.setCursor(Qt.CursorShape.WaitCursor)
        else:
            self.setCursor(self._normalCursor)
            
        # Reassert the hidden state of the index 0 widgets that can  get 
        # unhidden by being displayed:
        
        self._leftButtons.widget(0).hide()
        self._rightButtons.widget(0).hide()
        
# test code.
if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication
    from PyQt6.QtCore    import QTimer
    import sys
    from nscldaq.readoutgui import StateMachine
    
    def updateState() -> None:
        win.setState(StateMachine.ReadoutStateMachine.instance().state())
    
    def setState(name : str) -> None:
        StateMachine.ReadoutStateMachine.instance().transition(name)
    def startingToHalted() -> None:
        # timer triggered transition -> Halted.
        setState('Halted')
        updateState()
    
    def startToHalted() -> None:
        # transition to starting then schedule a transtion to halted one second later.
        setState('Starting')
        updateState()
        timer.start()
    
    def beginToActive() -> None:
        setState('Active')
        updateState()
        
    def activeToHalted() -> None:
        setState('Halted')
        updateState()
        
    def activeToPaused() -> None:
        setState('Paused')
        updateState()
        
    def pausedToActive() -> None:
        setState('Active')
        updateState()
        
        
    app = QApplication(sys.argv)
    win = QStateButtons()
    
    updateState()
    
    # Set signal handlers to walk the state machine through its paces.
    # Note we're going to put a delay in between Starting and Halted
    # to test the cursor handling.
    
    win.start.connect(startToHalted)
    win.begin.connect(beginToActive)
    win.end.connect(activeToHalted)
    win.pause.connect(activeToPaused)
    win.resume.connect(pausedToActive)
    
    timer = QTimer(win)
    timer.setInterval(1000)   # a second.
    timer.setSingleShot(True)
    timer.timeout.connect(startingToHalted)
    
    win.show()
    
    sys.exit(app.exec())
    