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
@file EventLogManager.py
@brief Manage event log instances and types
@author Ron Fox

The two classes of interest in this file are:

EventLogManager - which will contain all event logger instancesw and start them 
   as needed when runs start.  It will also kill them on demand (usually as
   the program is exiting).
   
EventLogRegistry - a registry of event log types.  Each entry is a named tuple
   of the class that can, when created with a configuration produces the logger,
   a configuration display class for PyQt6, and a GUI for creating
   that logger.  Any function that needs to be run when the prompter is
   created.  This deals with the fact that the full and multilog prompters
   are the same...just the state of a checkbutton is differernt.
   
ConfigurationSerializer - Registered with the configuration save/restore infrastructure
    to save the configuration of event loggers and pull it back into the EventLogManager.
    
   
'''
from collections.abc import Callable
from typing import Self, ClassVar, Any
import traceback


# Exceptions:

#  Exception classes:

class EventLogRegistryException(Exception):
    '''
    Base class for our exception hierarchy:
    '''
    def __init__(self, msg : str):
        super().__init__(msg)

class DuplicateName(EventLogRegistryException):
    '''
    Attempt to register two loggers/types with the same name
    '''
    def __init__(self, name : str):
        super().__init__(f'{name} has already been registered')
        
class NoSuchName(EventLogRegistryException):
    '''
        Attempted to fetch a logger type/name that has not been registered.
        
    '''
    def __init__(self, name : str):
        super().__init__(f'No logger/logger type  named {name} has been registered.')
        
class SingletonViolation(EventLogRegistryException):
    ''''
    Someone from outside the module has attempted to instantiate the registry.
    
    '''    
    def __init__(self, filename : str):
        super().__init__(f'Attempted to instantiate the data source registry singleton from {filename} not allowed')
     

class EventLogRegistry:
    '''  
    Registry of event log types, how to create them and how to 
    display their configurations and how to prompt for them.
    
    The registry itself is a dict of triplet containing
    - The class object that can create the event logger, the constructor is 
      expected to accept keyword parameters:  configuration - the configurationdict
      and optionally parent - a parent Qt object.
    - A class object that, given a logger of this type, can display its configuration.
    - A class object that, prompts for a new logger.
    - A function object that is invoked when a prompter is instantiated to initialize it.
      the function is passed the object reference of the prompter.  This can be None if no
      initialization is required.
    '''
    _instance : ClassVar[Self | None] = None
    def __init__(self):
        '''
        Do not invoke this direcly, use the class method instance() to get an instance
        of the singleton object.
        '''
        
        # Ensure we are being invoked from this module.
        caller = traceback.extract_stack()[-2]    # Our caller.
        if caller.filename != __file__:
            raise SingletonViolation(caller.filename)    
        self._registry : dict[str, tuple[type, type, type, Callable | None]]= {}     # Empty registry.
    
    
    @classmethod
    def instance(cls) -> Self:
        '''
        @return EventLogRegistry - the singleton instance of the registry.
        '''
        if not cls._instance:
            cls._instance = cls()
        return cls._instance
    
    # Public methods:
    
    def add(self, 
            logType : str, 
            loggerClass : type, displayerClass : type, prompterClass : type, prompterInit : Callable | None = None
    ) -> None:
        '''
        Add a new logger type to the registry:
        @param logType : str - a name to associate with that logger type.
        @param loggerClass : type - the class whose instances are that logger.  This is required to accept a
            configuration dict as its constructor parameter.
        @param displayerClass : type - class whose instances display the configuration of this logger type.  
               accepts an instance of the logger as the parameter for its constructor, along with an optional
               QWidget parent.
        @param prompterClass : type - same as for displayerClass, but prompts for the  configuration of the
            logger, and any logger passed in is used to initiliaze the form.
        @param prompterInit : Callable - function that, if not None, is called to initialize a propmter.
            this supports shared prompters.  The calable will get the instance of the prompter as a parameter.
            If no initialization is needed, this can be None.
        @throws DuplicateName - logType has already been registered.
        @note it's the client's responsibility to invok the prompterInit method.
        '''
        if logType not in self._registry:
            self._registry[logType] = (loggerClass, displayerClass, prompterClass, prompterInit)
        else:
            raise DuplicateName(logType)
    
    # Query methods:
    
    def list(self) -> list[str]:
        '''
        @return list[str] - list of registered names:
        '''
        return list(self._registry.keys())    # Coerce it out of a view.
        
    def logger(self, name : str) -> type:
        '''
        @param name : str - the name of the logger class to look up.
        @return type      - The class of that logger.
        @throw  NoSuchName - if there is no logger type registered as name
        '''
        return self._get(name)[0]
    
    def displayer(self, name : str) -> type:
        '''
        @param name : str -name of the logger tpe being looked up.
        @return type      - The class of the logger displayer.
        @throw NoSuchName - If name is not registered.
        '''
        return self._get(name)[1]
    
    def prompter(self, name : str) -> type:
        '''
        @param name : str -name of the logger tpe being looked up.
        @return type      - The class of the logger prompter
        @throw NoSuchName - If name is not registered.
        '''
        return self._get(name)[2]
    
    def prompterInit(self, name : str) -> Callable | None:
        '''
        @param name : str -name of the logger tpe being looked up.
        @return Callable | None - if not None, a function to call with the prompter object
              as a parameter to initialize the prompter.
        @throw NoSuchName - If name is not registered.
        '''
        return self._get(name)[3]
    
    # Utilities:
    
    def _get(self, name : str) -> (type, type, type, Callable | None):
        # Common code to fetch the registry entry for 'name' or toss a NoSuchName
        # Exeption.
        
        try:
            return self._registry[name]
        except IndexError:
            raise NoSuchName(name)
        
    