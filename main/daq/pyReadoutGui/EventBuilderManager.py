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
@file EventBuilderManager.py
@brief Manages the event builders the user wants in their experiment
@author Ron Fox

The EventBuilderManager is yet another name -> object mapping.  In this 
case, the objects managed are EventBuilder.EventBuilder objects.

Hooking the manager into state transitions -> Active allows
it to start the event builders it's had registered.
Note that the implemntation of EventBuilder's is such that 
they and their sources are restarted on every such transition.

This also allows event builder definitions (I'm mostly thinking of --dt settings)
to be modified between runs.
'''
import io
import traceback
import types
from typing import ClassVar, Self

from nscldaq.readoutgui.EventBuilder import EventBuilder
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from nscldaq.readoutgui.StateSaveRestore import SerializationManager, Serializer
from PyQt6.QtCore import QObject, QProcess, pyqtSignal


class EVBManagerException(Exception):
    def __init__(self, msg: str):
        super().__init__(msg)

class SingletonViolation(EVBManagerException):
    def __init__(self, filename : str):
        super.__init__(
            f'Attempted to directly construct an EventBuilderManager from {filename} use the instance method instead'
        )
class DuplicateName(EVBManagerException):
    def __init__(self, name : str):
        super.__init__(f'{name} is already a registered event builder')
        
class NoSuchName(EVBManagerException):
    def __init__(self, name : str):
        super.__init__(f'{name} is not a registered event builder')
        
class EventBuilderManager(QObject):
    '''
    This is the singleton event builder manager, it provides methods to add,
    remove and replace event builders as well as methods to start and
    stop them, and get their configurations.  Event builders
    are identified with a unique name.  This name is not
    part of the builder's configuration but, when listing them,
    the name tags a long so that it can be loaded into a wizard
    for editing the builder's configuration.
    Signals:
    evbExited(str) - Event builder exited unexpectedly.
    
    '''
    _instance : ClassVar[Self | None] = None   # Singleton instance variable.
    evbExited = pyqtSignal(str) 
    def __init__(self):
        self._init__(None)
        '''
        Do not invoke this directly, that will result in a SingletonViolation
        being raised.  Use EventBuilderManager.instance() to get the singleton
        instance of this manager.
        '''
        caller = traceback.extract_stack()[-2]   # Our caller.
        if caller.filename != __file__:
            raise SingletonViolation(caller.filename)
        
        if self._instance:
            raise RuntimeError('Attempt to double construct an EventBuilderManaer.')
        
        self._eventBuilders : dict[str, EventBuilder] = {}     # Will store the event builders.
        self._stopped: bool = True                             # Don't relay evbExited signal.
    @classmethod
    def instance(cls) -> Self:
        '''
        @return EventBuilderManager - the singleton instance - constructing if needed
        '''
        if not cls.instance:
            cls._instance = cls()
        
        return cls._instance
    
    #  Methods that manipulate the registry:
    
    def add(self, name : str, builder : EventBuilder) -> None:
        '''
        Add a new event builder.
        @param name : str - name of the event builder, must be unique.
        @param buider : EventBuilder - the event builder to add/associate with name.
        @throws DuplicateName if the name is already in use.
        '''
        if name in self._eventBuilders:
            raise DuplicateName(name)
        
        self._eventBuilders[name] = builder
        
        # Connect the event builder signals.
        
        builder.eventBuilderExited.connect(lambda _exitst, _procstate: self.evbExited.emit(name))
        builder.eventBuilderExited.connect(lambda _exitst, _procstate: self._reportBuilderExit(name))
        builder.eventBuilder.sourceExited(lambda srcname, exitstat, procstate: self._reportSrcExit(name, srcname, exitstat, procstate))
        
        builder.eventBuilderOutput.connect(lambda output : self._outputRelay(name, None, output))
        builder.sourceOutput.connect(lambda srcname, output : self._outputRelay(name, srcname, output))
    
    def remove(self, name: str) -> EventBuilder:
        '''
        Remove an event builder from the registry.
        @param name - name of the event builder to remove.
        @return EventBuilder -the event builder that was removed.
        @throws NoSuchName - if name is not a registered event builder.
        '''
        result = self._eventBuilders.pop(name, None)
        if not result:
            raise NoSuchName(name)
        
        return result
        
    def replace(self, name : str, builder : EventBuilder) -> EventBuilder:
        '''
        Replace an existing event builder with a new one.
        @param name - name of the event builder to replace.
        @param builder  - New builder.
        @return EventBuilder - the prior event buildeer.
        @throws NoSuchName - if name is not an event builder.
        '''
        result = self._eventBuilders.pop(name, None)
        if not result:
            raise NoSuchName(name)
        
        self._eventBuilders[name] = builder

        return result
        
    # Methods that interrogate the event bulder list:
    
    def names(self) -> list[str]:
        '''
        @return list of the defined event builder names.
        '''
        return [x for x in self._eventBuilders] # to get list[str] instead of a view.
        
    def configs(self) -> list[tuple[str, dict[str, object], list[dict[str, object]]]]:
        '''
        @return list[tuple[str, dict[str, object], list[dict[str, object]]]]
           List of event builders and their configuration.   Each list element is
           a 3 element tuple containing:
           [0] - event builder name.
           [1] - That event bulder's pipline configuratino
           [2] - The list of the configurations of that event bulder's source configs.
        '''
        result = []
        for name, builder in self._eventBuilders.items():
            config = builder.getConfig()
            result.append((name, config[0], config[1]))
            
        return result
    
    def builderConfig(self, name : str) -> tuple[dict[str, object], list[dict[str, object]]]:
        '''
        @param name -name of the event builder for which we want the configuration.
        @return tuple[dict[str, object], list[dict[str, object]]] the configuration:
           [0] the configuration of the pipeline.
           [1] the list of fragment sourcde configurations.
        @throws NoSuchName if name is not a registered event builder.
        '''
        builder = self._eventBuilders.get(name)
        if not builder:
            raise NoSuchName(name)
        return builder.getConfig()
    
    def find(self, name : str) -> EventBuilder | None:
        '''
        @param name of an event builder to get.
        @return EventBuilder | None - event builder requested.
        @retval None - no such named event builder.
        '''
        return self._eventBuilders.get(name)
    
    # Methods called by connections run transitions.
    
    def start(self)  -> None:
        '''
        Start all the event builders.  Call this at begin run.
        '''
        self._stopped = False
        for evb in self._eventBuilders:
            evb.start()
            
    def stop(self) -> None:
        '''
        Normally this is called just before calling start to clena up
        any event builders and sources that are hanging around.
        crash (Active |  Paused -> NotReady)
        '''
        self._stopped = True
        for evb in self._eventBuilders:
            evb.stop()
        
    # Utilities:
    
    def _ouptut(self, txt : str) -> None:
        # Output text to our window:
        
        output = mainWindow().centralWidget.Output().addOutput('EVB')
        output.append(txt + '\n')
        
    # Internal (private) slots  
        
        
    def _reportBuilderExit(self, name : str) -> None:
        # an event builder exited.  If self._stopped is not true, report that.
        
        if not self._stopped:
            self._output(f'Event builder {name} exited unexpectedly')
            
    def _reportSrcExit(self, name : str, srcname : str, _exitstat : int, procstate : QProcess.ExitStatus) -> None:
        # The event source srcname for a specific ring exited.
        
        if not self._stopped and (procstate != QProcess.ExitStatus.NormalExit):
            self._output(f'Event builder {name} source for ring {srcname} exited abnormally.')
    
    def _outputRelay(self, name : str, srcname : str | None, output : str) -> None:
        # either an event source or an event builder output something:
        # Sources will set the source name, event builders will set it to None.
        
        if srcname:
            txt = f'Event buider {name} source for ring {srcname} output: {output}'
        else:
            txt = f'Event builder {name} output: {output}'
            
        self._output(txt)
        

#  Save/restore the event builder definitions the manager knows about.

class EventBuilderSaveRestore(Serializer):
    '''
    Save and restore the state of the event builders. We're going to write
    EventBuilders = ...and then what we get from the manager's configs method.
    '''
    def serialize(self, destination : io.TextIOBase) -> None:
        '''
        Write the configs to file
        param destination : io.TextIOBase - the text io cofiguration file.
        '''
        destination.write(f'EventBuilders = {EventBuilderManager.instance().configs()}')
        
    def deserialize(self, mod : types.ModuleType) -> None:
        '''
        Restore the configuration (if it exists).
        @param mod - the module the configuration file was read into.
        '''
    
        try:
            config = mod.EventBuilders
        except AttributeError:
            return
        
        manager = EventBuilderManager.instance()
        for builder in config:
            evb = EventBuilder(builder[2], builder[1])
            manager.add(builder[0], evb)

# Register an instance of our serializer:

SerializationManager.instance().addSerializer(EventBuilderSaveRestore())