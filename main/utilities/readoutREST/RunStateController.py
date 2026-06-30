'''
  A RunStateController works with the RunState and RunStateModel
  to connect with the manager and the Readout programs to maintain
  consistency between the model, the view and the manager and Readout programs.
  
  
  Our main work with the ReadoutPrograms is to monitor their states
  and responsiveness.  This is because the actual state transitions are
  accomplished by the manager's and the transient programs run during transitions.
  
  
    @file RunStateController.py
    @brief Controller to mediate between the run state view/model and the experiment.
    @author Ron Fox.
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

from PyQt6.QtWidgets import QWidget
from PyQt6.QtCore import QObject, QTimer
from nscldaq.readoutREST import readoutRestClient, rdo_utils
from nscldaq.manager_client import State
import getpass
from collections import namedtuple

Constants = namedtuple('Constants', ['DEFAULT_READOUT_REST_SERVICE', 'DEFAULT_MANAGER_REST_SERVICE', 'POLL_MS'])
CONSTANTS = Constants(
    DEFAULT_READOUT_REST_SERVICE = rdo_utils.CONSTANTS.DEFAULT_READOUT_REST_SERVICE,
    DEFAULT_MANAGER_REST_SERVICE = 'DAQManager',
    POLL_MS                      = 1000,
)
class RunStateController(QObject):
    def __init__(self, readouts : list[tuple[str,str | None]],  
                 view : QWidget,
                 mgr_host : str, 
                 mgr_user : str = getpass.getuser(), 
                 mgr_service : str = CONSTANTS.DEFAULT_MANAGER_REST_SERVICE, 
                 parent : QObject | None =None):
        '''
            @param readouts - a list of pairs of strings. Each pair contains:
                              - the program name of a Readout program.
                              - the ReST service advertised by that  program or None if it's the default
                                CONSTANTS.DEFAULT_READOUT_REST_SERVICE
            @param  view      - The view widget. Note that this must satisfy the interfaces of RunState.
            @param mgr_host - Required parameter - the host in which the  manager is running.
            @param mgr_user - Optional parameter, the username that started the manager, defaults
                                to the current user.
            @param mgr_service - Optinoal parameter, the manager ReST service defaults 
                                to CONSTANTS.DEFAULT_MANAGER_REST_SERVICE
            @param parent   - Optional parameter, Object's Qt parent defaults to None.
            
            @note for each polling pass, we relook up the ReadoutPrograms.  This allows them to move around
                  between manager server restarts between poll intervales.
                  
        '''
        
        super().__init__(parent)
        self._readouts = readouts
        self._view     = view
        self._host     = mgr_host
        self._user     = mgr_user
        self._service  = mgr_service
        self._stateClient = State(self._host, self._user, self._service)
        
        self._timer    = QTimer(self)
        
        # Initialize the model to the current state.
        
        self._updateManagerState()
        self._updateReadoutState()
        
        # Connect to the view's signals:
        
        self._view.transitionRequested.connect(self._transition)
        
        
        # set up the poll but don't start it until exec().
        
        self._timer.setInterval(CONSTANTS.POLL_MS)
        self._timer.setSingleShot(False)
        self._timer.timeout.connect(self._poll)
    def start(self) -> None:
        '''
            Start operating.
        '''
        self._timer.start()
        
    def exit(self) -> None:
        '''
            Shutdown the timer based polling.
        '''
        self._timer.stop()    # Stop polling.
    
    # private slots.
    
    def _transition(self, newstate) -> None:
        # We just request the state and let the poll update the actuals>
        # @todo In the future we may report json 'ERROR' status in some way.
        # One error case I can see is if there are more than one of us and someone's
        # change the state in between our polls in a way that makes newstate invalid.

        self._stateClient.transition(newstate)
        
    def _poll(self) -> None:
        #
        #  Called when the state polling timer time - out.
        #
        
        # Update the model's manager state (that's easy).
        
        self._updateManagerState()
        
        # update the readout state:
        
        self._updateReadoutState()
        
        # Update elapsed run time:
        
        self._view.model().setElapsed(self._stateClient.elapsed())

    def _updateManagerState(self):
        model = self._view.model()
        model.setManagerState(self._stateClient.status(), set(self._stateClient.allowed()))
       
        
    def _updateReadoutState(self):
        # Poll all the readouts for their states.
        # - if any are unreachable the state is 'unresponsive'
        # - if any differ from any others, the state is 'inconsistent'
        
        state_set = {}    #  Used to detect inconsitent.
        for readout in self._readouts:
            name   = readout[0]
            service= readout[1] if readout[1] is not None else CONSTANTS.DEFAULT_READOUT_REST_SERVICE
            host   = rdo_utils.getReadoutHost(self._host, self._user, self._service, name)
            model  = self._view.model()
            client = readoutRestClient.ReadoutClient(host, service, self._user)
            try:
                state = client.getState()
                if state['status'] == 'ERROR':
                    model.setReadoutState('unresponsive')
                    return
                state = state['state']
                state_set.add(state)
            except Exception:
                model.setReadoutState('unresponsive')
        
        if len(state_set) != 1:
            model.setReadoutState('inconsistent')
        else:
            # There's only one so:
            
            model.setReadoutState(state_set.pop())
            
                
            
if __name__ == "__main__":
    from PyQt6.QtWidgets import QApplication, QMainWindow
    from RunState import RunState, RunStateModel
    import sys

    # Hardcoded Readouts:
    #                 name             ReST service
    readouts  =[('Readout_readout', 'ReadoutREST'),]

    app = QApplication(sys.argv)
    win = QMainWindow()
    view = RunState(win)
    win.setCentralWidget(view)
    win.show()
    
    controller = RunStateController(readouts, view, 'localhost', parent = view)
    controller.start()
    app.exec()
    