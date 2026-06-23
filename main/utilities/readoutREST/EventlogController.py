'''
    This is a pair of event log controllers that mediates between the 
    manager server and the model/views for event logging.
    
    LoggerConfigurationController - mediates the configuration of event logging programs.
    LoggerEnableController        - mediates the overall eventlog enable.
    
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

#
#  We import both the logger and State:
#  State is polled in order to ghost the
#  view widgets when the state is Active.
#
from nscldaq.manager_client import Logger, State

from PyQt6.QtCore import QTimer, QObject, pyqtSignal, Qt     # For polling the  state.
from collections import namedtuple

# Define the ms between polls in milliseconds.

Constants = namedtuple('Constants', ['POLL_MS',])
CONSTANTS = Constants(POLL_MS = 1000)

class StatePoller(QObject):
    '''
        This class is an object that encapsulates a
        timer and a  state client.  When the periodic
        timer fires, the state is polled and, if the state has changed,
        
        stateChanged is signalled with the new state.
    '''
    stateChanged =pyqtSignal(str)
    
    def __init__(self, host: str, user: str | None = None, service: str = 'DAQManager', parent: QObject =None):
        '''
            @param host  - host in which the manager server is running.
            @param user  - user that ran the manager server defaults to the logged in user.
            @param servcie - the Service the manager is advertising for its ReST endpoint.
                   defaults to 'DAQManager', the default service.
            @param parent - Parent object, defaults to None.
            
        '''
        super().__init__(parent)
        
        self._client = State(host, user, service)
        
        self._timer = QTimer(self)    
        self._timer.setInterval(CONSTANTS.POLL_MS)
        self._timer.setSingleShot(False)
        self._timer.timeout.connect(self._poll)
        self._timer.start()
        
        self._lastState = None

        self._poll()       # Don't wait for expiration to emit the first state.
    
    def state(self) -> str | None:
        '''
            Returns the last known state (could be None though we did an initial)
            poll so not likely.
            
            @return str | None
        '''
        
        return self._lastState
    
    def _poll(self):
        #  Internal slot called when the timer fires..we just
        #  get the state and resignal stateChanged if it changed.
        
        state = self._client.status()
        if state != self._lastState:
            self._lastState = state
            self.stateChanged.emit(state)
        
    
class StatePollFactory:
    '''
       This class creates StatePoller objects re-using
       them if asked for pollers with the same parameterizations.
       The pollers created are unparented.  To use this factory:
       
       poller = StatePollFactory.getInstance(host, user, service)
       
       poller.stateChanged.connect(myhandler)
       
       If a poller for that host, user, service triplet has already 
       been  made, you'll get the same one otherwise, a new
       instance will be made and registered.
    '''
    
    pollers : list[dict] = list()   #  The pollers we've made so far.
    
    def getInstance(host : str, user: str | None = None, service : str = 'DAQManager') -> StatePoller:
        '''
          Creates an instance of a new poller or returns an existing one.
        '''
        pollerDict = {'host': host, 'user' : user, 'service': service}
        poller     = StatePollFactory._findPoller(pollerDict)
        if poller is None:
            poller = StatePollFactory._makePoller(pollerDict)
        
        return poller
    
    def _findPoller(spec : dict) -> StatePoller | None:
        # If there's a matching poller return it else None
        matches = [item for item  in StatePollFactory.pollers 
                   if
                    item['host'] == spec['host'] and 
                    item['user'] == spec['user'] and 
                    item['service'] == spec['service']
                ]
        if len(matches) > 0:
            return matches[0]['poller']
        
    def _makePoller(spec: dict) -> StatePoller:
        # Create and save a new poller:
        
        spec['poller'] = StatePoller(spec['host'], spec['user'], spec['service'])
        StatePollFactory.pollers.append(spec)
        return spec['poller']
    
    
# Now that we have the preliminaries out of the way so that
# we're not going to proliferate pollers, we can make our
# controllers.

class LoggerEnableController(QObject):
    '''
        A controller for the logger enable MVC.
        -  If the state is BEGIN or None, we disable the widget. This is updated with a StatePoller.
        -  If the enable is modified in the view we modify the state in the server.
        -  We periodically poll for isRecording so that we an maintain the state of the
           view.
    '''
    def __init__(
        self, view,
        host : str, user: str | None = None, service : str = 'DAQManager', 
        parent : QObject = None
    ):
        '''
            @param view  - View object.  Must duck type compatible with EventLog.Logger.
            @param host  - host in which the manager server is running.
            @param user  - user that ran the manager server defaults to the logged in user.
            @param servcie - the Service the manager is advertising for its ReST endpoint.
                   defaults to 'DAQManager', the default service.
            @param parent - Parent object, defaults to None.
            
        '''
        super().__init__(parent)
        self._view = view
        
        self._poller = StatePollFactory.getInstance(host, user, service)
        self._poller.stateChanged.connect(self._updateRunState)
        self._updateRunState(self._poller.state())
        
        # I'm going to need a logger client:
        
        self._client = Logger(host, user, service)
        
        #  Arrange a QTimer to keep the checkbox up-to-date.
        
        self._timer = QTimer(self)    
        self._timer.setInterval(CONSTANTS.POLL_MS)
        self._timer.setSingleShot(False)
        self._timer.timeout.connect(self._updateEnable)
        self._timer.start()
        
        # Catch events from the view:
        
        self._view.clicked.connect(self._setManagerState)
        
    # private slots:
    
    def _updateRunState(self, state : str | None) -> None:
        #  The run state has just changed...set the
        #  enable state of the view. We are ghosted if the state
        # is either unknown or BEGIN.
        
        if state in ('BEGIN', None):
            live = False
        else:
            live = True

        self._view.setEnabled(live)
    
    def _updateEnable(self) -> None:
        #  Called peridically to update the model's 
        # idea of the state of the enable:
        
        self._view.model().setEnabled(self._client.isRecording())
    
    def _setManagerState(self) -> None:
        # Called when the enable button is clicked.  Set the
        # manager to the appropriate state...all else will follow naturally.
        
        state = self._view.checkState()
        enable_state = True if state == Qt.CheckState.Checked else False
        self._client.record(enable_state)
        

# Test code:

if __name__ == '__main__':
    import Eventlog
    from PyQt6.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget
    import sys

    
    
    app = QApplication(sys.argv)
    win = QMainWindow()
    
    # The GUI:
    
    widget = QWidget(win)
    layout = QVBoxLayout()
    widget.setLayout(layout)
    
    # MVC for logger enables:
    enable = Eventlog.Logger(widget)
    layout.addWidget(enable)
    enable_controller = LoggerEnableController(enable, 'localhost', parent=widget)
    

    
    #  Start the application:
    
    win.setCentralWidget(widget)
    win.show()
    sys.exit(app.exec())