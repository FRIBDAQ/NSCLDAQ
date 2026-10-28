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
import io
import types
from typing import Self, ClassVar, Any
import traceback

from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from nscldaq.readoutgui.StateSaveRestore import Serializer, SerializationManager
from PyQt6.QtCore import Qt




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
        super().__init__(f'Attempted to instantiate a ingleton from {filename} not allowed')
     

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
        if self._instance:
            raise RuntimeError('Attempt to double construct the EventLogRegistry singleton.')
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
    def find(self, logger : object) -> str :
        '''
        Given a logger object, locate the name of its type.
        @return str - nmae that type of logger is registered as in the manager.
        @throws EventLogRegistryException if the logger object type isn't in the registry.
        '''
        
        for name, (loggerClass, _a, _b, _c) in self._registry.items():
            # Need an exact type match in case there are sublasses e.g.
            if type(logger).__name__ ==  loggerClass.__name__:
                return name
        
        # No match:
        
        raise EventLogRegistryException(f'The logger class {logger.type().__name__} has not been registered.')
        
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
        except KeyError:
            raise NoSuchName(name)
        
    
class EventLogManager:
    '''
    This class is where _instances_ of event loggers should be registered.
    Our leave method shouild be called on the state manager leave. method.
    If event recording is active, all enabled loggers will be 
    started.  end will be called for enabled loggers in transitions to
    Halted.  Transitions to Not Ready will invoke _all_ stop 
    methods, for enabled or disabled loggers (in case loggers have some
    persistent process that needs to be shutdown).
    
    This, too is a singleton so you should use the instance() method rather
    than construction to get an instance.
    
    The manager makes  a correspondence between a logger name and
    a tuple contaning a bool (enable/disable state), and the logger object instance
    itself.
    '''
    _instance : ClassVar[Self | None] = None
    def __init__(self) :
        '''Use the instance() class method to get the singleton instance, not construction'''
        
        
        # Enforce singleton-ness
    
        caller = traceback.extract_stack()[-2]    # Our caller.
        if caller.filename != __file__:
            raise SingletonViolation(caller.filename)    
        if self._instance:
            raise RuntimeError('Attempt to double construct the EventLogRegistry singleton.')

        # Make the registry:
        
        self._registry : dict[str, (bool, object)] = {}
    
    @classmethod
    def instance(cls) -> Self:
        if not cls._instance:
            cls._instance = cls()
        return cls._instance
        
    def add(self, name : str, logger: object, enabled : bool = True) -> None:
        '''
        Add a logger to the manager:
        @param  name  : str    - A unique name for the event logger.
        @param logger : object - the event logger object. Must satisfy the duck typing defined
             by the nscldaq.readoutgui.Eventlog.EventLog base class
        @param enabled : bool - optional parameter, the enable state of the logger, 
             Defaults to True if not provided, enabling the logger by default.
        @throws DuplicateName if 'name' is already defined.
        '''
        if name not in self._registry:
            self._registry[name] = (enabled, logger)
    
    def remove(self, name : str) -> None:
        '''
        Remove a logger from the manager.
        @param naame :str - Name of the logger.
        @throws NoSuchName if there is no mathching logger:
        '''
        try:
            del self._registry[name]
        except KeyError:
            raise NoSuchName(name)
           
    def find(self, name : str) -> tuple[bool, object]:
        '''
            @param name - name of a logger.
            @return tuple[bool, object] - enable state and logger object.
            @throws NoSuchName if there is no matching logger.
            
        '''
        try:
            return self._registry[name]
        except KeyError:
            raise NoSuchName(name)
    
    def list(self) -> list[str, bool, object]:
        '''
        @return list [str, bool, object] List of the names of loggers and 
                    their enabled state and loggers themselves.
        '''
        result = []
        for name, info in self._registry.items():
            result.append((name, info[0], info[1]))

        return result

    def enable(self, name : str) -> None:        
        '''
            Enable a logger. Note if the logger is already enabled, it is not an error.
            @param name : str - name of the logger.
            @throws NoSuchName - if name is not a logger.
        '''
        try:
            self._registry[name] = (True, self._registry[name][1])  # Tuples are immutable.
        except KeyError:
            raise NoSuchName(name)
        
    def disable(self, name :str) -> None:
        '''
        Disable a logger.  Note that if the logger is already disabled, its' not an error.
            @param name : str - name of the logger.
            @throws NoSuchName - if name is not a logger.        
        '''
        try:
            self._registry[name] = (False, self._registry[name][1]) # Tuples are immutable.
        except KeyError:
            raise NoSuchName(name)
    
        
    def leave(self, leaving: str, entering: str) -> None:
        '''
        Iterate over the various event loggers doing what's needed.
        @param leaving : str - state being left.
        @param entering: str - state being entered.
        '''
        # The method we need to call, if the object has to be enabled
        # depends on the state being entered.
        # If nothing is to be done, the method name will be None
        method_name = None
        require_enabled = True           # Most common case:
        match entering:
            case  'Not Ready':      # Shutting down:
                method_name = 'stop'
                require_enabled = False  # Everyone shuts down
            case 'Active':          # Run Starting if from  Halted
                if self._logging() and entering == 'Halted':
                        method_name = 'start'
            case 'Halted':
                if self._logging() and entering in ['Active', 'Paused']:
                        method_name = 'end'
        #  Ok ready to iterate if there's a method name:
        
        if method_name:
            for logger in self._registry.values():
                if not require_enabled or (require_enabled and logger[0]):
                    getattr(logger[1], method_name)
        
        
    # Utility methods:
    
    def _logging(self) -> bool:
        # return True if global logging is enabled in the GUI.
        
        return mainWindow().centralWidget.Recording().checkState() == Qt.CheckState.Checked
        

class EventLogConfig(Serializer):
    '''
    Responsible for saving/restoring event log configuration.  Will save
    as a list of dicts in the variable eventLoggers = []
    
    
    This should be enough information to allow loggers to be restored.
    The assumption is that all loggers are in the manager.  
    '''
    def serialize(self, destination: io.TextIOBase) -> None:
        ''' We will make alist of dicts. Each list item describes a logger:
        Each list item is a dict containing:
            * name - name of the event logger.
            * type - Type of the event logger.
            * enabled - Enable state of the logger.
            * configuration - the loggers' configuraiton dict.
        
        '''
        manager = EventLogManager.instance()
        registry = EventLogRegistry.instance()
        configuration = []
        for name, enabled, logger in manager.list():
            typeName = registry.find(logger)
            configuration.append({
                'name' : name,
                'type' : typeName,
                'enabled' : enabled,
                'configuration' : logger.getConfig()
            })
        destination.write(f'eventLoggers = {configuration}\n')
        
        
    def deserialize(self, mod : types.ModuleType) -> None:
        registry = EventLogRegistry.instance()
        manager  = EventLogManager.instance()
        knownTypes = registry.list()        
        try:
            loggers = mod.eventLoggers
        except AttributeError as a:
            print(f'Unable to get the event logger configuraiton {a}')
            return    # Can't restore configuration.
        
        for loggerConfig in loggers:
            if loggerConfig['type'] in knownTypes:
                logger = registry.logger(loggerConfig['type'])(configuration=loggerConfig['configuration'])
                manager.add(loggerConfig['name'], logger, loggerConfig['enabled'])
            else:
                print(f'Logger type {loggerConfig["type"]} has not been registered, logger {loggerConfig["name"]} cannot be restored')
            
        
SerializationManager.instance().addSerializer(EventLogConfig())