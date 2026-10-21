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
@file EventLog.py
@brief Class that encapsulate eventlog

Note there are two types of event loggers that can be run.
Partial and Full.   

Partial event loggers just make a soup
of event files in the target directory.
Full event loggers maintain the directory structure users have
come to know and love/hate in the Tcl ReadoutGUI primary
event logger. See the individual class descriptions
for more.

Note as well, that unlike the ReadoutGUI event loggers can
run in any host that can be a client to the DAQ system.
They are therefore run on the end of a pySSHProcess.
This implies the container we might run in is reconstructed
in the target system.  

This code also encapsulates an EventLogManager that
contains a list of event loggers constructed externally
by the user interacting with, e.g. the ReadoutGui.

We include a ReadoutConfiguration saver to allow the
set of event loggers and their configurations to be saved
and restored on restart.
'''
from abc import ABC, abstractmethod
import datetime
from typing import ClassVar, Self

from nscldaq.readoutgui.pySSHProcess import SSHProcess

class EventLog(ABC):
    '''
    Abstract base class that defines the methods
    every event logger must implement.
    We also provide for configuration.  At this
    level, we don't assume much of anything
    about the event logger that's being run to allow
    foreignn event loggers to work.
    '''
    def __init__(self, configuration : dict[str, object]):
        '''
        @param configuration : dict[str, object] - the initial 
            configuration of the event.  It should
            conform to the types returned by the
            parameters abstractmethod.
        '''
        self._configuration = configuration
    
    @classmethod
    @abstractmethod
    def parameters(cls) -> dict[str, type]:
        '''
        @return dict[str,type] - specifies the configuration parameter names and types.
            e.g. {'ringbuffer' : str'}  there is a confiiguration parameter named 'ringbuffer' 
            that has str type.
        '''
        ...
        
    def configure(self, name : str, value : object) -> None:
        '''
        Modifies a configuration parameter (or sets it if not yet set).
        @param name : str - name of the parameter.
        @param value : object - The new value to give the parameter,
            must be consistent with the type of that parameter from the
            parameters method.
        @throws IndexError if there's no matching parameter name.
        @throws TypeError the value is not of the required type.
        '''
        
        required_type = self.parameters()[name]
        if  isinstance(value, required_type):
           self._configuration[name] = value
        else:
            raise TypeError(
                f'''Type {value} for parameter {name} must be of type {required_type.__name__} but was of {type(value).__name}'''
            ) 
    
    @abstractmethod
    def start(self) -> None:
        '''
        For concrete classes, this should start the event logger.
        '''
        ...
    @abstractmethod
    def end(self) -> None:
        '''
        For concrete classes, this should do what's done at the end of
        a run for the event logger.  This includes ensuring the
        eventlogger actually exits at some point.
        '''
        ...
    @abstractmethod
    def stop(self) -> None:
        '''
        Called to stop an event logger (e.g when the run ends). This is a best effort attempt.
        '''
        ...
        
    # Proctecte methods
    def _validateConfig(self) -> bool:
        '''
        Validate the configuration against parameter types.
        @return bool - True if all elements validate, False if not.
        '''
        return  len(self._listInvalidParameters()) == 0
            
        
    def _listInvalidParameters(self) -> list[str]:
        '''
        @return list[str] - list of parameter names that are of invalid type.
        '''
        parameters = self.parameters()
        result = []
        for name, value in self._configuration.items():
            if not isinstance(value, parameters[name]):
                result.append(name)
            
        
        return result
        
    
                
        
        
        
    



