'''
  Provide common utilities for readout ReST client programs.
  
  @file rdo_utils.py
  @brief  Useful utility functions factored out of Readout ReST clients.
  @author Ron Fox
'''
from nscldaq.manager_client import Programs, State
from PyQt6.QtCore import QTimer, QObject, pyqtSignal, Qt     # For polling the  state.
from collections import namedtuple


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

def getReadoutHost(mgr_host: str, user: str, service: str, name: str) -> str:
    '''
        Get the host in which the named program is running:
        @param mgr_host - where the manager is running.
        @param user     - user running the DAQ
        @param service  - mgr service.
        @param name     - name of program to look up.
    '''
    client = Programs(mgr_host, user, service)
    info = client.status()
    if info['status'] != 'OK':
        raise RuntimeError('Failed to fetch program status from server', info['message'])    

    for program in info['programs']:
        if program['name'] == name:
            return program['host']
    
    # Not found:
    
    raise IndexError(f'There is no program named {name}')

## Aggregate state polling:

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