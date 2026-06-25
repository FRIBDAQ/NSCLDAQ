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



class RunInfoController:
    '''
        This class is an intermediary between the RunInfo model/view
        and some underlying system that impelements the KVStore
        run and title convenience functions.
        
    '''
    def __init__(self, view, client):
        '''
            @param view - a RunInfo compatible object.
            @param client - A KVStore compiatible object. 
        '''
        
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
        
        
        #  Set up signal handlesr to keep our client updated:
        
        self._view.requestedTitleChanged.connect(self._updateTitle)
        self._view.requestedRunChanged.connect(self._updateRun)
        
    # Signal handlers:
    
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
    
    
    