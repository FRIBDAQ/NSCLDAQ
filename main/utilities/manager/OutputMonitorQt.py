'''
    This module provides a wrapping of 
    manager_client.OutputMonitor which is Qt friendly.
    By Qt friendly I mean that having constructed the
    OutputMonitor that is the base class of our class,
    We will poll for input via a QTimer.  
    Disconnects will also poll for reconnects on the same
    timer....for a specified number of seconds.
    
    See OutputMonitorQt for more information.
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

from PyQt6.QtCore import QObject, QTimer, pyqtSignal
from collections import namedtuple
from nscldaq import manager_client

Constants = namedtuple('Constants',
    ['POLL_MS', 'RECONNECT_TIMEOUT_SEC', 'DEFAULT_SERVICE'])

CONSTANTS = Constants(
    POLL_MS = 500,
    RECONNECT_TIMEOUT_SEC = 3600,
    DEFAULT_SERVICE=manager_client.CONSTANTS.DEFAULT_MANAGER_OUTPUT_SERVICE
)

class OutputMonitorQt(QObject):
    '''
        A Qt6 friendly output monitor.  We encapsulate an output monitor
        and, via some magic, provide the following to the user:
        
        Signals:
            input(str)   - Input was received and is passed to the slot.
            lost         - Connection to the servedr was lost and not re-estabilshed within the timeout.
        
        Note:
           when lost is signalled, the timer that drives us is destoyed.  The simplest thing to do, if you
           want to try to keep going is to just destroy and recreate the object.
    '''
    input = pyqtSignal(str)
    lost  = pyqtSignal()
    def __init__(
        self, 
        mgr_host : str, 
        mgr_user : str | None = None,
        mgr_service : str = CONSTANTS.DEFAULT_SERVICE,
        reconnect_seconds : int = CONSTANTS.RECONNECT_TIMEOUT_SEC, 
        parent : QObject | None = None):
        '''
            The constructor takes the following parameters:
            
            @param  mgr_host - host that is running the manager server (required)
            @param  mgr_user - User the manager server is running under (optional None uses the current)
            @param  mgr_service - The output monitor service advertised by the manager (optional
                          uses the default value if not given).
            @param  reconnect_seconds - If the connection to the server is lost, the number of seconds
                          for which reconnection is attempted.  (optional defaults to 3600 - an hour).
            @param  parent - the parent of the object (optional defaults to None - not parented).
            
            @note - if the server is not alive when we are constructed,
                    then manager_client.Disconnected is raised 
        '''
        
        super().__init__(parent)
        self._client = manager_client.OutputMonitor(mgr_host, mgr_user, mgr_service)
        self._reconnect_retries = reconnect_seconds * 1000/CONSTANTS.POLL_MS
        self._reconnect_counter = 0
        self._connected = True
        
        # Kick off the polling:
        
        self._timer = QTimer(self)
        self._timer.setInterval(CONSTANTS.POLL_MS)
        self._timer.setSingleShot(False)
        self._timer.timeout.connect(self._tick)
        self._timer.start()
        
        
    def _tick(self):
        #   Timer fired... if connected, try to read, if not,
        #   try to reconnect:
        
        if self._connected:
            try:
                data = self._client.read()
                if len(data) > 0:
                    self.input.emit(data)
            except manager_client.Disconnected:
                self._connected = False
                self._reconnect_counter = 0    # Enter try reconnect state
        else:
            try:
                self._client.reconnect()
                self._reconnect_counter = 0
                self._connected = True
            except manager_client.Disconnected:
                self._reconnect_counter += 1
                if self._reconnect_counter > self._reconnect_retries:
                    self.lost.emit()
                    self._timer.stop()    # we're done for.
    
if __name__ == "__main__":
    from PyQt6.QtWidgets import QApplication, QMainWindow
    import sys
    # Because I'm too lazy to figure out how to just
    # do a non gui application again
    
    def onInput(data):
        print(data)
        
    def onLost():
        print('Lost connection')
        app.exit(-1)
    
    app = QApplication(sys.argv)
    win = QMainWindow()
    
    input = OutputMonitorQt('localhost', reconnect_seconds=60)
    input.input.connect(onInput)
    input.lost.connect(onLost)
    
    win.show()
    sys.exit(app.exec())
    


