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
@file StateMachine.py
@brief State machine for the pyReadoutGUI.
@author Ron Fox
'''

'''
This module provides a state machine for the pyReadoutShell program
It defines states and their successors and allows the program
as a whole to hang actions off state transitions.  This is
closely modeled after the RunStateMachine.tcl module, however
it's assumed that we're living in they PyQt6 ecosystem. As such,
rather than bundles, the state transition events and pre-transition
events are signalled and those signals can be connected to slots
to make things happen.

States are:
*  Not ready - no data sources are active.
*  Starting  - The data sources are now being started.
*  Halted    - All data sources are active but the run is not.
*  Active    - Data taking is active.
*  Paused    - Data taking is now paused.
'''

from typing     import ClassVar, Self
import traceback
from PyQt6.QtCore import QObject, pyqtSignal

class StateTransitionException(Exception):
    '''
        Base class of all exception types we implement.
    '''
    def __init__(self, message: str):
        ''' @param message : str - the state transition error message.'''
        super().__init__(message)
    
class SingletonViolation(StateTransitionException):
    '''
        Violation of singleton:
    '''
    def __init__(self):
        super().__init__('Use the instance() class method to get the singleton instance of the state machine. You cannot directly instantiate one')
    
class IllegalStateTransition(StateTransitionException):
    ''''
        Raised if an invalid transition is requested.
    '''
    def __init__(self, fromState : str, toState: str):
        '''
            @param fromState : str - The from state.
            @param toState   : str - The invalid to state.
        '''
        super().__init__(f'Transitions from {fromState} to {toState} are not allowed')
    

class ReadoutStateMachine(QObject):
    '''  
        This captures the statemachine for Readout.
        It has the states described in the module
        docstring comments.
        
    Signals:
    precheck(str, str)  - Allows performance of pre transition checks passed from/to state names
    leave(str, str)     - Called when leaving a state (from, to passed).
    enter(str, str)     - Called when entering a new state (from, to passed).
    failed(str, str)    - The transition was failed by one of the signal handlers.
    newstate(str)       - Successful state change to str.
    
    Attributes:
        state  [Readonly] - the current state
    Methods:
        instance          - Return the singleton instance.
        listStates        - Lists all of the states
        listTransitions   - List the legal next states
        transition        - Attempt a transition.
    Slots:
        failPrecheck     - Called by a precheck to abort a transitino.
        failTransition   - Call to fail the transition in progress.  This is normally
                           called by a signal handler to indicate a state transition failed.
                           it causes the state machine to return to the prior state without any
                           signals.
            
        
    '''
    precheck = pyqtSignal(str, str)
    leave    = pyqtSignal(str, str)
    enter    = pyqtSignal(str, str)
    failed   = pyqtSignal(str, str)
    newstate = pyqtSignal(str)
    
    # The transition diagram is a dict that has a list of legal subsequent states keyed
    # by current state:
    
    _LegalTransitions : ClassVar[dict[str, list[str]]] = {
        'Not Ready' : ['Starting',],
        'Starting'  : ['Halted', 'Not Ready'],
        'Halted'    : ['Active', 'Not Ready'],
        'Active'    : ['Halted', 'Paused', 'Not Ready'],
        'Paused'    : ['Halted', 'Active', 'Not Ready']
    }
    def __init__(self, parent : None | QObject = None) :
        
        
        # Check for violation of singleton instantiation:
        
        caller = traceback.extract_stack()[-2]
        if caller.filename != __file__:
            raise SingletonViolation()
        
        super().__init__(parent)
        self._state = 'Not Ready'
        self._lastState = 'Not Ready'
        
        # Various transition flags.
        
        self._precheckFailed   = False    # failedPrecheck slot called.
        self._transitionFailed = False    # failTransition slot called.
        
    # Class level methods:
    
    def instance() -> Self:
        '''
        @return ReadoutStateMachine - return the singleton instance of the state machine.
        '''
        return _instance  
    
    # Attributes:
    
    def state(self) -> str:
        ''' 
        @return str - the current state.
        '''
        return self._state

    # Public methods:
    
    def listStates(self) -> list[str]:
        '''
        @return list[str] - list of valid state names.
        '''
        return list(ReadoutStateMachine._LegalTransitions.keys())

    def listTransitions(self) -> list[str]:
        return ReadoutStateMachine._LegalTransitions[self._state]
    
    
    def transition(self, to : str) -> None:
        '''
            Try to do the transition requested.
            
            @param to : str - the requested state to transition to.
        '''
        if to not in self.listTransitions():
            raise IllegalStateTransition(self._state, to)
        
        self._precheckFailed   = False
        self._transitionFailed = False
        
        # Precheck:
        
        self.precheck.emit(self._state, to)
        
        if not self._precheckFailed:
            self.leave.emit(self._state, to)
            
            if self._transitionFailed:     
                # Don't do the transition.
                self.failed.emit(self._state, to)
                return

            prior = self._lastState     # For the rollback case.
            self._lastState = self._state
            self._state     = to

            self.enter.emit(self._lastState, self._state)
            
            if self._transitionFailed:
                # Rollback the transition:
                
                self._state = self._lastState
                self._lastState = prior
                self.failed.emit(self._state, to)
                return
                
        else:
            self.failed.emit(self._state, to)     # Pre-check failed.
            return
            
        # Transition succeeded, signal that too:
                
        self.newstate.emit(self._state)
            
    # Slots (public)
    
    def failedPrecheck(self) -> None:
        '''
            Call only  when a state transition pre-check is in progress.
            In that case, the state transition is not attempted.
        '''
        self._precheckFailed = True

    def failTransition(self) -> None:
        '''
            Call only when a state transition operation failed.
            in that case the state transition is rolledback.
        '''
        self._transitionFailed = True
        
    
        
    
# this is a singleton:

_instance = ReadoutStateMachine()



# Tests run them if the module is run.:

if __name__ == '__main__':
    from PyQt6.QtCore import QCoreApplication
    import unittest
    import sys
    
    app = QCoreApplication(sys.argv)  # Needed to emit signals.... I think.
    
    #  Test class:
    
    class StateTests(unittest.TestCase):
        def setUp(self) ->  None:
            global _instance
            #  Give us a fresh instance so tests are idempotent.
            
            _instance = ReadoutStateMachine()
    
        def test_instance(self) -> None:
            # Test instance gives the right thing:
            
            self.assertEqual(_instance, ReadoutStateMachine.instance())
        
        def test_initialization(self) -> None:
            # Ensure the instance is properly initialized:
            self.assertEqual(_instance._state, 'Not Ready')
            self.assertEqual(_instance._lastState, 'Not Ready')
            self.assertFalse(_instance._transitionFailed )
            self.assertFalse(_instance._precheckFailed)
    
        def test_state(self) -> None:
            self.assertEqual('Not Ready', ReadoutStateMachine.instance().state())
    
        def test_statelist(self) -> None:
            # Test we get the correct list of sstates.
            
            states = ReadoutStateMachine.instance().listStates()
            self.assertEqual(len(states), len(ReadoutStateMachine._LegalTransitions.keys()))
            self.assertEqual(states, list(ReadoutStateMachine._LegalTransitions.keys()))
        
        def test_transitionList(self) -> None:
            # The legal transitions given are correct.
            
            states = ReadoutStateMachine.instance().listStates()  # Walk through these:
            for state in states:
                ReadoutStateMachine.instance()._state = state     # Hoke the state.
                self.assertEqual(
                    ReadoutStateMachine._LegalTransitions[state],
                    ReadoutStateMachine.instance().listTransitions()
                )    
        
        # Testing transitions and the associated signals.
        
        def test_precheckOk(self) -> None:
            prechecked = False
            f          = ''
            t          = ''
            def precheck(fr : str, to : str) -> None:
                nonlocal prechecked
                nonlocal f
                nonlocal t
                prechecked = True
                f = fr
                t = to
            
            # Connect to the precheck signal
            i = ReadoutStateMachine.instance()
            i.precheck.connect(precheck)
            nextState = i.listTransitions()[0]    # Just pick one.
            now      = i.state()  
            i.transition(nextState)
            
            # CHeck that my precheck was called with the right args.
            
            self.assertTrue(prechecked)
            self.assertEqual(now, f)
            self.assertEqual(nextState, t)   
            
            # The transition completed:
            
            self.assertEqual(nextState, i.state())
        
        def test_precheckFailed(self) -> None:
            # Failed precheck aborts the transition:
            
            def precheck(fr, to) -> None:
                ReadoutStateMachine.instance().failedPrecheck()
                
            i = ReadoutStateMachine.instance()
            i.precheck.connect(precheck)
            nextState = i.listTransitions()[0]    # Just pick one.
            now      = i.state()  
            i.transition(nextState)
            
            # Transition should not have happened:
            
            self.assertEqual(now, i.state())
         
        def test_badTransitino(self) -> None:
            # Invalid transition raises    IllegalStateTransition:
            
            with self.assertRaises(IllegalStateTransition):
                ReadoutStateMachine.instance().transition('Active')
        
        def test_leaveOk(self) -> None:
            # Test that the leave signal works:
            
            l = False
            f = None
            t = None
            
            def leave(fr : str, to: str) -> None:
                nonlocal l, f, t

                l = True
                f = fr
                t = to
            
            i = ReadoutStateMachine.instance()
            i.leave.connect(leave)
            
            was = i.state()
            willbe = i.listTransitions()[0]
            i.transition(willbe)
            
            self.assertTrue(l)
            self.assertEqual(f, was)
            self.assertEqual(t, willbe)
            self.assertEqual(willbe, i.state())
        
        def test_leaveFails(self) -> None:
            # Fail the transition aborts it:
            
            def leave(_f : str, _t : str) -> None:
                
                ReadoutStateMachine.instance().failTransition()
                
            i = ReadoutStateMachine.instance()
            i.leave.connect(leave)
            
            to = i.listTransitions()[0]
            s     = i.state()    
            i.transition(to)
            self.assertEqual(s, i.state())   # Won't transition.
        
        
        def test_enterOk(self) -> None:
            # Enter works for a good state change
            
            e = False
            f = None
            t = None

            def enter(fr : str, to: str) -> None:
                nonlocal e,f,t
                e = True
                f = fr
                t = to
            
            i = ReadoutStateMachine.instance()
            i.enter.connect(enter)
            initial = i.state()
            final   = i.listTransitions()[0]
            i.transition(final)
            
            self.assertTrue(e)
            self.assertEqual(final, i.state())    # good transition.
            self.assertEqual(initial, f)
            self.assertEqual(final, t)
        
        def test_enterFailed(self) -> None:
            def enter(_f : str, _t: str) -> None:
                ReadoutStateMachine.instance().failTransition()
            
            i = ReadoutStateMachine.instance()
            i.enter.connect(enter)
            
            initial = i.state()
            final   = i.listTransitions()[0]
            i.transition(final)
            
            # We must still be in the initial state:
            
            self.assertEqual(initial, i.state())
            
        def test_newstateSignalled(self) -> None:
            # if a transition suceeded, the newstate signal is emitted.
            
            new = False
            state = None
            
            def newstate(s : str) -> None:
                nonlocal new, state
                new = True
                state = s

            i = ReadoutStateMachine.instance()
            i.newstate.connect(newstate)
            
            next = i.listTransitions()[0]
            i.transition(next)
            
            self.assertTrue(new)
            self.assertTrue(next, state)
            
        def test_newstateNotSignalled_1(self):
            # NO newstate signal if precheck fails.
            
            new = False
            def newstate(s : str) -> None:
                nonlocal new
                new = True
            
            def precheck(f : str, t : str) -> None:
                ReadoutStateMachine.instance().failedPrecheck()
                
            i  = ReadoutStateMachine.instance()
            i.newstate.connect(newstate)
            i.precheck.connect(precheck)
            
            i.transition(i.listTransitions()[0])
            
            self.assertFalse(new)
        def test_newstateNotSignalled_2(self):
            # No newstate signal if leave failed the transition.
            
            new = False
            def newstate(_ : str) -> None:
                nonlocal new
                new = True

            def leave(_f : str, _t : str) -> None:
                ReadoutStateMachine.instance().failTransition()
            
            i  = ReadoutStateMachine.instance()
            i.newstate.connect(newstate)
            i.leave.connect(leave)
            
            i.transition(i.listTransitions()[0])
            
            self.assertFalse(new)
    
        def test_newstateNotSignalled(self) -> None:
            # New newstate signal if enter failed the transition:
            
            new = False
            def newstate(_ : str) -> None:
                new = True
            
            def enter(_f: str, _t: str) -> None:
                ReadoutStateMachine.instance().failTransition()

            i  = ReadoutStateMachine.instance()
            i.newstate.connect(newstate)
            i.enter.connect(enter)
            
            i.transition(i.listTransitions()[0])
            
            self.assertFalse(new)
                
         
        def test_failedSignaled_1(self) -> None:
            #  failed is signalled if precheck fails.
            
            fail = False
            fr   = None
            to   = None
            def failed(f : str, t : str) -> None:
                nonlocal fail, fr, to
                fail = True
                fr = f
                to = t
            def precheck(_f : str, _t: str) -> None:
                ReadoutStateMachine.instance().failedPrecheck()  
            
            i = ReadoutStateMachine.instance()
            i.failed.connect(failed)
            i.precheck.connect(precheck)
            
            initial = i.state()
            next    = i.listTransitions()[0]
            
            i.transition(next)
            self.assertTrue(fail)
            self.assertEqual(initial, fr)
            self.assertEqual(next, to)
                    
    unittest.main()
    