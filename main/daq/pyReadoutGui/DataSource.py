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
@file DataSource.py
@brief Abstract base class that defines the interface for data sources.
@author Ron Fox
'''
from abc import ABCMeta, abstractmethod

from PyQt6.QtCore import QObject

# This class is needed to resolve the meta class conflict between ABC and QOjbect
# Thank you google gemini:

class QObjectABCMeta(type(QObject), ABCMeta):
    pass

class DataSource(QObject, metaclass=QObjectABCMeta):
    '''
    Abstract base class that defines the interface of a DataSource for pyReadoutGui.
    Note that this is a PyQt6 QObject so that it can emit and consume signals.
    '''
    
    
    @classmethod
    @abstractmethod
    def parameters() -> dict[str, type]:
        '''
            This class method returns a dictionary who's keys
            are parameters that instances of this class
            can be insantiated with and who's values are the types of those
            parametes e.g. {'sourceid' : int, 'host': str}  is a parameter named 'sourceid' whose
            value is an integer and a parameter named 'host' whose parameter is 
            a string.
            
        '''
        ...
    def __init__(self, parameters : dict[str, object], **kwargs):
        '''
        @param parameters - initial configuration  parameters.
        @param **kwargs   - Keyword args, parent should be done here.
        '''
        
        super().__init__(**kwargs)
        self._configuration = parameters
    
    def configure(self, name : str, value: object) -> None:
        '''
        Configures the value of a parameter.
        @param name - the name of the parameter, must be one of the keys returned by the paramters method.
        @param value - new value for that parameter.  Must be of the same type as the parameter.
        @throws IndexError if there's no matching parameter name.
        @throws TypeError the value is not of the required type.
        '''
        
        required_type = self.parameters()[name]
        if type(value) != required_type:
            raise TypeError(
                f'''The value {value} for parameter {name} should have been type {required_type.__name__} 
not of {type(value).__name__}'''
            )
        self._configuration[name] = value

    def cget(self, name: str) -> object:
        '''
        @param name : str - The name of a configurable parameter.
        @return object: The value of that parameter
        @throws IndexError if that parameter does not have a value.   
        @note Not having a value is not the4 same as not being defined.  It's possible, on intialization,
        for not all parameters to be configured.
        '''
        return self._configuration[name]
    
    def sourceType(self) -> str:
        '''
        @return the actual class name of the data source:
        '''
        return type(self).__name__
        
    
    # Not pure virtual but often overridden:
    
    def capabilities(self) -> dict[str, bool]:
        '''
            Returns the data source capabilities.  The capabilities are
            canPause - The data source supports pause/resume transitions.
            runsHaveTitles - The runs have titles.
            runsHaveNumbers  the runs have numbers.
            
            @return dict[str, bool] - by default, all capabilities are present, override if
               that's not the case for a specific data source.
        '''
        
        return {
            'canPause' : True, 'runsHaveTitles' : True, 'runsHaveNumbers' : True
        }
    def canBegin(self) -> bool:
            '''
            If possible, interact with the source to see if starting a run
            is likely to succeed (precheck).
            @return bool - True if starting a run is likely to succeed
            '''    
            return True
    
    def pause(self) -> None:
        '''
        Called to pause a run - only gets called if canPause is one of the capabilities.
        Default action does nothing.
        '''
        pass
    
    def resume(self) -> None:
        ''''
        Called when a run is about to be resumed.   This only gets called if canPause is a capbility.
        Default action is to do nothing.
        '''
        pass
    
    def init(self) -> None:
        '''
        Called to initialize the data source after its started.
        Default action is to do nothing
        '''
        pass
            
    #
    #  Pure virtual methods:
    # 
    @abstractmethod   
    def start(self) -> None:
        '''
        Start the data source this object manages.
        '''
        ...
    @abstractmethod
    def check(self) -> bool:
        '''
        @return bool - True if the data source is still responsive and alive.
        '''
        ...
    @abstractmethod        
    def stop(self) -> None:
        ''' Stop the data source this object manages'''
        ...
    
    @abstractmethod        
    def begin(self, run : int, title : str) -> None:
        '''
          Called at the start of a run for the data source.
          @param run : int - the run number.
          @parm title : str - The title of the run.
        '''
        ...
    @abstractmethod
    def end(self) -> None:
        '''
        Called when a run is going to end.
        '''
        ...
    