'''
    This module supplies a single class:
    
    RunInfoController  - this class connects  RunInfo and RunInfoModel
    objects to the manager server
    
    @file RunInfoController.py
    @brief Provide controller (MVC sense) for Run Info and the manager server.
    @author Ron Fox
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

from PyQt6.QtCore import QTimer, QObject
from nscldaq.readoutREST import rdo_utils

class RunInfoController(QObject):
    '''
        This class is an intermediary between the RunInfo model/view
        and some underlying system that impelements the KVStore
        run and title convenience functions.
        
    '''
    def __init__(self, view, client, parent=None):
        '''
            @param view - a RunInfo compatible object.
            @param client - A KVStore compiatible object. 
            @param parent - our parent object if there is one.
        '''
        super().__init__(parent)
        self._view = view
        self._model = view.model()
        self._client = client

        # Set up the model/view in accordance with the current values:
        
        title = self._client.title()
        run   = self._client.run()
        
        self._model.setActualTitle(title)
        self._model.setActualRun(run)
        
        # Make the requested values the same:
        
        self._view.setTitle(title)
        self._view.setRun(run)
        
        
        #  Set up signal handlers to keep our client updated:
        
        self._view.requestedTitleChanged.connect(self._updateTitle)
        self._view.requestedRunChanged.connect(self._updateRun)
        
        # There might be other controllers so we need to
        # monitor the state in the manager.
        
        self._timer = QTimer(self)
        self._timer.setInterval(rdo_utils.CONSTANTS.POLL_MS)
        self._timer.setSingleShot(False)
        self._timer.timeout.connect(self._update)
        self._timer.start()
        
    # Signal handlers:
    
    def _update(self):
        # Update the state from the client:
        
        
        self._model.setActualTitle(self._client.title())
        self._model.setActualRun(self._client.run())
        
    
    def _updateTitle(self, title:str) -> None:
        # Update the title in the server and model:
        
        self._client.setTitle(title)
        self._model.setActualTitle(title)   # Signals the UI.

        
    
    def _updateRun(self, run:int) -> None:
        # Update the run number in the server and model.
        
        self._client.setRun(run)
        self._model.setActualRun(run)
    
    
# Test code:

if __name__ == "__main__":
    import sys
    from PyQt6.QtWidgets import QApplication, QMainWindow
    from nscldaq.readoutREST.RunInfo import RunInfo
    from nscldaq.manager_client import KVStore
    app = QApplication(sys.argv)
    win = QMainWindow()
    widget = RunInfo(win)
    win.setCentralWidget(widget)
    win.show()
    
    # Now the controller...assume the server is running as us
    # on localhost.
    
    client = KVStore('localhost')
    controller = RunInfoController(widget, client)
    
    sys.exit(app.exec())
    
    
    