'''
    Provides two classes:
    
    RunStateModel - Provides a data model for the state of a run. 
    RunState      - Provides status and control over the run state.
    
    The controller for thie MVC pattern is external so that it could
    be plugged into more than one mechanism of control/status.
    
    
'''

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

from collections import namedtuple
from PyQt6.QtCore import QObject, pyqtSignal
from PyQt6.QtWidgets import QWidget, QPushButton, QLabel, QHBoxLayout, QVBoxLayout


Constants = namedtuple('Constants', ['MANAGER_STATES', 'READOUT_STATES'])
CONSTANTS = Constants(
    MANAGER_STATES=('SHUTDOWN', 'BOOT', 'HWINIT', 'BEGIN', 'END'),
    READOUT_STATES =('idle', 'active', 'inconsistent')
)


class RunStateModel(QObject):
    ''''
        Provides data and signals related to the system and readout states.
        The experiment system, as a whole is a state machinge. Its transitions
        can drive changes in the state of Readout state machines.  We capture
        both types of states. 
        
        The experiment system states (captured in CONSTANTS.MANAGER_STATES):
        
        SHUTDOWN - nothing is running.
        BOOT     - Persistent processes are running
        HWINIT   - hardware has been initialized.
        BEGIN    - Data taking is being attempted.
        END      - Data taking is not being attempted.
        
        The Readout programs have a simpler set of states
        (capturedin CONSTANTS.READOUT_STATES):
        idle     - Data is not being collected.
        active   - Data is being collected.
        
        The experiment may have serveral Readouts from which events are built.
        We therefore allow for an additional Readout collective state 
        'inconsistent' which means that not all Readouts have the same state.
        
    Attributes:
        managerState - Should be the current manager state.  Setting requires a set of valid next states
        legalTransitions (readonly) - the state transitions allowed from managerState
        readoutState - The readout state.
        
    Signals:
        managerStateChange(str, set[str]) - new manager state entered.
        readoutStateChange(str)            -new Readout state  entered.
    
    Note:
        Next state validation (that is if a new state is loaded was it allowed?) is done by
        an external controller.  The model here is just used to informt the view.
    '''
    managerStateChange = pyqtSignal(str, set)
    readoutStateChange = pyqtSignal(str)
    
    
    def __init__(self, 
        managerInitial : str = 'SHUTDOWN', managerAllowed : str = 'BOOT',
        readoutInitial: str = 'idle', 
        parent : QObject |None = None ):
        '''
            @param managerInitial - manager initial state, if not in CONSTANTS.MANAGER_STATES, 
                  ValueError is raised.
            @param managerAllowed - initial set of next states allowed all elements are also
                  validated against CONSTANTS.MANAGER_STATES
            @param readoutInitial - Initial state of readouts. Validated against CONSTANTS.READOUT_STATES.
            @param parent    - Parent of this object (None if not supplied).
            
            The default values for the initial states and allowed next states are of a system
            that's not yet been booted.
        '''
        super().__init__(parent)
        
        # Validate the parameters:
        
        self._validateState(
            managerInitial, CONSTANTS.MANAGER_STATES, 'Initial state {1} invalid must be one of {2} '
        )
        self._validateStateSet(
            managerAllowed, CONSTANTS.MANAGER_STATES, 
            'The next state {1} is invalid it must be one of {2} ' 
        )
        self._validateState(
            readoutInitial, CONSTANTS.READOUT_STATES,
            'Initial Readout state; {1} is invalid, must be one of {2}'
        )
        
        # Now that we know they're all good, save them:
        
        self._managerState       = managerInitial
        self._managerTransitions = managerAllowed
        self._readoutState       = readoutInitial
    
    
    # Attribute implementations:
    
    def managerState(self) -> str:
        ''' @return str - the current manager state'''
        
        return self._managerState
    def setManagerState(self, proposed: str, nextStates: set[str]) -> None:
        '''
            @param proposed - the proposed next state.
            @param nextStates - The next allowed states
            
            @note proposed and the states in nextStates are validated.
            @note on Success, managerStateChange is emitted.
        '''
        self._validateState(
            proposed, CONSTANTS.MANAGER_STATES, 
            'The proposed manager state {1} is not valid, must be one of {2}'
        )
        self._validateStateSet(
            nextStates, CONSTANTS.MANAGER_STATES,
            'The propposed possible transition {1} is not valid. Must be one of {2}'
        )
        # Now success is assured:
        
        self._managerState = proposed
        self._managerTransitions = nextStates
        self.managerStateChange.emit(proposed, nextStates)
    
    def legalTransitions(self) -> set[str]:
        '''@return set[str]  The allowed subsequent transitions'''
        return self._managerTransitions
    
    def readoutState(self) -> str:
        ''''@return str - the current collective readout state'''
        return self._readoutState
    def setReadoutState(self, proposed: str) -> None:
        '''
            @param propposed - the new proposed Readout state.
        '''
        self._validateState(
            proposed, CONSTANTS.READOUT_STATES, 
            '{1} is not a valid Readout state.  Must be one of {2}'
        )
        # Success is assured>.
        
        self._readoutState = proposed
        self.readoutStateChange.emit(proposed)
        
    
    #  Utility (private) methods:
    
    def _validateStateSet(self, proposed : set[str], allowed: set[str], msgFormat:str):
        for state in proposed:
            self._validateSTate(state, allowed, msgFormat)
    
    
    def _validateState(self, proposed: str, stateSet: set[str], msgFormat:str):
        # Raise a ValueError if the proposed state is not in stateSet.
        if proposed not in stateSet:
            msg = msgFormat.format(proposed, stateSet)
            raise ValueError(msg)
        

class RunState(QWidget):
    '''
        The view class for the run control/state part of the control panel.  Before I
        start, what's explicitly _not_ here is the elapsed run time and the
        recording toggle button. 
        What is here is:
          - Button(s) to guide the system to the next state transitions.
          - Labels that show that the current states of both the Reaodut and Manager are.
        
        An accompanied RunStateModel which, in turn, is manipulated by an external controller
        provides the logic of all of this.
        
        Attributes:
            model (readonly) - Get everything you need from it.
        Signals:
            transitionRequested(str) - Change to a new state was requested parameter is the next
                                        requested state.
    '''
    transitionRequested = pyqtSignal(str)
    
    
    def __init__(self, parent):
        super.__init__(parent)
        
        self._model = RunStateModel(parent=self)    # Gives us an initial state.
          
        # I need labels for the states and their values:
        
        self._mgrStateLabel = QLabel('Manager State: ', self)
        self._mgrState      = QLabel(self._model.managerState(), self)
        
        self._rdoStateLabel  = QLabel('Readout State', self)
        self._rdoState      = QLabel(self._model.readoutState(), self)
        
        # We need the following buttons:
        # Boot/Shutdown - If SHUTDOWN labeled with Boot otherwise with Shutdown
        # Begin/End     - If BOOT or HWINIT labeled Begin, if SHUTDOWN Ghosted, otherwise End.
        # Initialize    - If Shutdown or BEGIN ghosted.
        
        
        self._BootShutdown = QPushButton(self)
        self._BeginEnd     = QPushButton(self)
        self._HwInit       = QPushButton(self)
        self._adjustButtons()       # Set the buttons according to the state.
        
        
        # lay this crap all out:
        
        self._toplayout = QVBoxLayout(self)
        
        self._line1     =  QHBoxLayout(self)
        self._line1.addWidget(self._mgrStateLabel)
        self._line1.addWidget(self._mgrState)
        self._line1.addWidget(self._rdoStateLabel)
        self._line1.addWidget(self._rdoState)
        self._toplayout.addLayout(self._line1)
        
        self._line2 = QHBoxLayout(self)
        self._line2.addWidget(self._BootShutdown)
        self._line2.addWidget(self._BeginEnd)
        self._line2.addWidget(self._HwInit)
        self._toplayout.addLayout(self._line2)
        
        self.setLayout(self._topLayout)
        
        # Connect our buttons clicked to 
        # hanbdlers that figure out what the 
        # requested state transition is and emit a
        # transitionRequested based on the button
        # and allowed transitions.
        
        self._BootShutdown.clicked.connect(self._bootshutdown)
        self._BeginEnd.clicked.connect(self._beginend)
        self._HwInit.clicked.connect(self._hwinit)
        
    def model(self) -> RunStateModel:
        return self._model    
        
    # Slots (private).
    
    def _bootshutdown(self)->None:
        # The boot shutdown button was clicked:
        
        valids = self._model.legalTransitions()
        if 'SHUTDOWN' in valids:
            t = 'SHUTDOWN'
        else:
            t = 'BOOT'
    
        self.transitionRequested.emit(t)
        
    def _beginend(self) -> None:
        valids = self._model.legalTransitions()
        if 'BEGIN' in valids:
            t = 'BEGIN'
        else:
            t = 'END'
        self.transitionRequested.emit(t)
        
    def _hwinit(self) ->None:
        self.transitionRequested.emit('HWINIT')
        
        
    # Private utilties.    
    
    def _adjustButtons(self) -> None:
        # this is driven completely off the valid next states:
        
        validTransitions = self._model.legalTransitions()
        
        # The boot/shutdown button:
        
        if 'BOOT' in validTransitions:
            self._BootShutdown.setText('Boot')
            self._BootShutdown.setEnabled(True)
        elif 'SHUTDOWN' in validTransitions:
            self._BootShutdown.setText('Shutdown')
            self._BootShutdown.setEnabled(True)
        else:
            self._BootShutdown.setEnabled(False)
            
        # The BEGIN/END button:
        
        if 'BEGIN' in validTransitions:
            self._BeginEnd.setText('Begin')
            self._Begin.setEnabled(True)
        elif 'END' in validTransitions:
            self._BeginEnd.setText('End')
            self._Begin.setEnabled(True)
        else:
            self._Begin.setEnabled(False)
            
        # The HWINit button:
        
        if 'HWINIT' in validTransitions:
            self._HwInit.setEnabled(True)
        else:
            self._HwInit.setEnabled(False)