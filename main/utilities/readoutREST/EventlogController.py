'''
    This is a pair of event log controllers that mediates between the 
    manager server and the model/views for event logging.
    
    LoggerConfigurationController - mediates the configuration of event logging programs.
    LoggerEnableController        - mediates the overall eventlog enable.

@file EventlogController.py
@brief Provide a controller for the Event log MVs for the managed environment.
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
        
    

class LoggerConfigController(QObject):
    '''
        A controller in the MVC pattern sense mediating between the
        Event log configuration view/model and the managed experiment
        environment.  We need to:
        - Poll the state and ghost the view if we're in the BEGIN state.
        - Poll the configuration and update it in the model if it changes.
          This is a bit more complex... For each poll, we need to:
          * See if an existing logger changed configuration  in which case we
            delete and re-create it.
          * See if an existing logger has been removed, in which case delete it.
          * See if a new logger has popped into existence, in which case, add it.
          * See if an existing logger has changed enable state in which case update that state.
        - connect to the view's enableChanged signal, and make the appropriate server request.
        
        
    '''
    def __init__(
        self, view,
        host: str, user : str | None = None, service :str = 'DAQManager', 
        parent : QObject | None = None
    ):
        '''
            @param view - the view object, must be duck type compatible with EventLog.LoggerConfig
            @param host - host the manager server is running in.
            @param user - (Optional defaults to None) User that ran the manager server.
                          If None, the current logged in user is used.
            @param service - (Optional defaults to 'DAQManager') Provides the name of the ReST service
                         advertised  by the manager server.  If not supplied, the default service name
                         is used.
            @para parent - (Optional defaults to None) - Our parent object in the Qt6 sense of the word.
        '''
        super().__init__(parent)
        
        self._view = view
        
        # Get an instancde of our state poller and our logger client:
        
        self._statePoller = StatePollFactory.getInstance(host, user, service)
        self._logClient   = Logger(host, user, service)
        
        self._initModel()                 # Set the initial set of loggers.
        
        # Create a timer to for polling the logger states:
        
        self._timer = QTimer(self)    
        self._timer.setInterval(CONSTANTS.POLL_MS)
        self._timer.setSingleShot(False)
        
        # Connect to the various signals
        
        self._statePoller.stateChanged.connect(self._updateRunState)
        self._view.enableChanged.connect(self._changeEnable)
        self._timer.timeout.connect(self._updateLoggers)
        
        # Start the timer for the logger state polls.
        
        self._timer.start()
    
    # Private methods.
    
    def _initModel(self) -> None:
        # Load the model from the server:
        
        loggers = self._logClient.list()
        for logger in loggers:
            self._view.model().addLogger(logger)    
           
    def _updateRunState(self, new_state: str) -> None:    
        #  We can update enables only as long as the system is not in BEGIN.
        
        if new_state == 'BEGIN':
            live = False
        else:
            live = True
        self._view.setEnabled(live)
        
    def _changeEnable(self, which : str, enable : bool) -> None:
        # Tell the server to change the state of a logger.  
        # which - the destination of the logger that changed.
        # enable - True to turn on , False to turn of.
        # Presumably the next _updateLoggers call will make the model/view
        # update.  Letting that take care of it handles multlple controllers
        # properly.
        #
        if enable:
            self._logClient.enable(which)
        else:
            self._logClient.disable(which)
            
    def _updateLoggers(self):
        # This is probably the most complicated chunk of code. 
        # See the class comments for a list of what we have to do:
        
        # To begin with throw the loggers the model knows about
        # and the ones the manager server knows about into maps that are
        # indexed by the destination as that's unique:
        # That can be done trough the magic of a dict comprehension:
        
        model = self._view.model()
        model_logger_list = model.loggers()
        model_loggers    = {x['destination'] : x for x in model_logger_list}
        
        server_logger_list = self._logClient.list()
        server_loggers    = {x['destination'] : x for x in server_logger_list}
        
        # Remove any loggers in model_loggers not in server_loggers:
        
        for ml in model_loggers:
            if ml not in server_loggers.keys():
                model.deleteLogger(model_loggers[ml]['destination'])
        
        # Add any loggers in server loggers that are _not_ in model_loggers.
        
        for sl in server_loggers:
            if sl not in model_loggers.keys():
                model.addLogger(server_loggers[sl])
        

        # If loggers in server loggers differ in definition from the same logger in
        # the model, delete and re-enter it...remove that entry from the
        # model_loggers to exempt it from checking to update the enable state since we just
        # implicitly set it properly.
        
        
        for dest, sl in server_loggers.items():
            if dest in model_loggers.keys() and      \
                not self.compare_loggers(sl, model_loggers[dest]):
                model.deleteLogger(dest)
                model.addLogger(sl)
                del model_loggers[dest]
        
        
        # For the loggers in the server log list, if they existin
        # in the model but their enable state differs, 
        # Update that as well.  Note any sever loggers _not_ in the
        # model logger list have been set in the model from the server logger list and
        # therefore have the correct enable state.
        
        
        for dest, sl in server_loggers.items():
            if dest in model_loggers.keys() and \
                sl['enabled'] != model_loggers[dest]['enabled']:
                if sl['enabled']:
                    model.enableLogger(dest)
                else:
                    model.disableLogger(dest)
    
    # Utility method to see if logger definition changed.
        
    def compare_loggers(self, l1: dict, l2: dict) -> bool:
        # Determine if two loggers are the same (not considering
        # their ids, enable state and destination) as the
        # destination is assumed the same.
        
        return (
            l1['ring'] == l2['ring']        and
            l1['daqroot'] == l2['daqroot']  and
            l1['host'] == l2['host']        and
            l1['partial'] == l2['partial']  and
            l1['critical'] == l2['critical'] and
            l1['container'] == l2['container']
        )
        
    
# Test code:

if __name__ == '__main__':
    import nscldaq.readoutREST.Eventlog as Eventlog
    from PyQt6.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget
    import sys

    
    
    app = QApplication(sys.argv)
    win = QMainWindow()
    
    # The GUI:
    
    widget = QWidget(win)
    layout = QVBoxLayout()
    widget.setLayout(layout)
    
    # MVC for g;pbal; logger enable:
    enable = Eventlog.Logger(widget)
    layout.addWidget(enable)
    enable_controller = LoggerEnableController(enable, 'localhost', parent=widget)
    
    # MVC for individual logger enables:
    
    config = Eventlog.LoggerConfig(widget)
    layout.addWidget(config)
    config_controller = LoggerConfigController(config, 'localhost', parent=widget)
    
    #  Start the application:
    
    win.setCentralWidget(widget)
    win.show()
    sys.exit(app.exec())