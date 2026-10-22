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
@file DataSourceController.py
@brief Controller between the state machine and the data source manager.
@author Ron Fox

We need an object to connect state transition signals (enter sand precheck specifically) 
with slots in the Data Source manager singleton.  This connection is what actually does 
something useful on state transitions.
'''
import time

from nscldaq.readoutgui.DataSource import DataSource
from nscldaq.readoutgui.DataSourceManager import DataSourceManager
from nscldaq.readoutgui.Log import Log
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from nscldaq.readoutgui.StateMachine import ReadoutStateMachine


class DataSourceController:
    '''
        Instantiate one of these to make the connection.
        
    '''
    def __init__(self):
        self._dataSources  = DataSourceManager.instance()
        self._stateMachine = ReadoutStateMachine.instance()
        
        # Connect signals to internal slots.
        
        self._stateMachine.precheck.connect(self._precheckSources)  # If begin.
        self._stateMachine.leave.connect(self._updateSources)       # New state for the sources
        self._stateMachine.enter.connect(self._updateState)
        self._dataSources.failed.connect(self._transitionFailed)
        self._dataSources.sourceDied.connect(self._sourceDied)
        
    #  Internal slots... these do the real work.
    
    
    def _precheckSources(self, fromState : str, toState:str) -> None:
        #  If to state is "Active" the sources need to precheck.
        
        if toState == 'Active' and not self._dataSources.precheck():
            self._stateMachine.failedPrecheck() 
    
    def _updateSources(self, fromState : str, toState : str) -> None:
        # What we do depends on the toState:
        match toState:
            case 'Not Ready':
                # Stop all sources:
                self._dataSources.stop()
            case 'Starting':
                #  Start the data sources:
                self._dataSources.start()
                time.sleep(2)                   # Wait for the ReST servers to start.
                
            case 'Halted':
                # Run is ending.
                if fromState != 'Starting':
                    self._dataSources.end()
            case 'Active':
                # Depends on from state:
                if fromState == 'Halted':
                    # Begin run
                    info = mainWindow().centralWidget().RunParameters()
                    title = info.title()
                    run   = info.run()
                    self._dataSources.begin(run, title)
                elif fromState == 'Paused':
                    # Resume run:
                    self._dataSources.resume()
                else:
                    #  This happens Starting -> Active....handled by _halted below
                    pass
            case 'Paused':
                self._dataSources.pause()
                
    def _updateState(self, fromState : str, toState: str):
        print('Update', fromState, toState)
        # Set the state to Halted if in starting...presumably
        # If a source died, we'll go back to not ready...
        
        if fromState == 'Not Ready' and toState == 'Starting':
           self._stateMachine.transition('Halted')
    
    def _transitionFailed(self):
        # A data source failed the transition...let the state machine know.
        
        self._stateMachine.failTransition()
    
    def _sourceDied(self, name: str, source : DataSource):
        # IF a source died and the state is not 'Not Ready',
        # force a transition to 'Not Ready'
        Log.instance().error(f'Data source named {name} exited.')
        
        if self._stateMachine.state() != 'Not Ready':
            self._stateMachine.transition('Not Ready')