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
@file ReadoutCallouts.py
@brief Provide extension functionality for pyReadoutGui
@author Ron Fox

What we provide:
-   A base class that can be used to write extensions.
-   A mechanism to register extensions.
-   A mechanism for the pytReadoutGui.py to call into those extensions.
-   The mechanism for loading a default set of extension 
    (importing the module ReadoutCallouts if it exists.)
    Additional extension modules can be added in two ways:
       - importing them from ReadoutCallouts.
       - loading them via the GUI itself (File->Load... menu operation)
'''

import traceback
from operator import methodcaller
from typing import ClassVar, Self

from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from nscldaq.readoutgui.StateMachine import ReadoutStateMachine
from PyQt6.QtCore import Qt


class pyReadoutExtension:
    '''
        This class provides the ability to extend the ReadoutGUI with
        the following methods:
        
        onRegistered   - The Extension was registered and can do any initialization
                       that requires access.
        onAboutToStart - The system is about to start all data sources.
        onStarted      - The system has started all data sources.    
        onBeginning - The system is about to begin a run.
        onBegun        - The system has begun a run.
        onEnding       - The system is ending a run.
        onEnded        - The system ended a run.
        onPausing      - THe system is about to pause a run.
        onPaused       - the system has paused a run.
        onResuming     - the system is about to resume a run.
        onResumed      - the system has resumed a run.
        onShuttingDown - The system is about to shutdown 
        onShutDown     - THe system is shutdown.
        NOTE:               all methods are initially implemented as No-ops so you need
                            only implement the methods you need:    
                            
                            
        A concrete implementation of this class must:
        - import ReadoutCallouts
        - derive from pyReadoutExtension
        - The module it is in must intantiate it and invoke the CalloutExtensionManager singleton's 
          addExtension method 
        For example:
            from nscldaq.readoutgui.ReadoutCallouts import pyReadoutExtension, ExtensionManager
            
            class myExtension(pyReadoutExtension:
            ....  
            
            # At the module global level
            
            extensionInstance = myExtension()
            CalloutExtensionManager.instance().addExtension(extensionInstance)    
        
    '''
    def onRegistered(self) -> None:
        '''
        Called when the object is registered with the extension manager.
        At that point, you can assume the system is sufficiently initialized that
        you can interact with its components.
        '''
        pass
    
    def onAboutToStart(self) -> None:
        '''
        The 'Start' button on the gui has been clicked but the GUI has not yet
        started any of the data sources.
        '''
        pass
    
    def onStarted(self) -> None:
        ''' The gui has started all of the data sources successfuly. '''
        pass
    
    def onBeginning(self, run : int, title : str, isRecording: bool) -> None:
        '''
            The Gui is about to start a run, but has not yet done so.
            @param run - the run number of the run that will be started.
            @param title - The title of the run.
            @param isRecording True if this run will be recorded.
        '''
        pass
    
    def onBegun(self, run : int, title : str, isRecording: bool) -> None:
        '''
            The GUI has successfully started a run.
            @param run - the run number of the run that will be started.
            @param title - The title of the run.
            @param isRecording True if this run will be recorded.

        '''
        pass

    def onEnding(self) -> None:
        '''
        The GUI is about to end a run, but has not yet done so.
        '''
        pass
    
    def onEnded(self) -> None:
        '''
        The GUI has successfull ended the run.
        '''
        pass

    def onPausing(self) -> None:
        '''
        The GUI is about to pause an active run.
        '''
        pass
    
    def onPaused(self) -> None:
        '''
        The GUI has successfully paused the active run.
        '''
        pass
    
    def onResuming(self) -> None:
        '''
        The GUI is about to resume a paused run.
        '''
        pass
        
    def onResumed(self) -> None:
        '''
        The GUI has successfully resumed a paused run.
        '''
        pass

    def onShuttingDown(self) -> None:
        '''
        The GUI is about to shutdown all of the data sources.
        '''
        pass

    def onShutdown(self) -> None:
        '''
        The GUI has shutdown all data sources.
        '''
        
        
class CalloutExtensionManager:
    '''
    This is the singleton extension manager.
    It maintains a list of extensions derived from pyReadoutExtension.
    It calls methods on each of the extensions in the list at appropriate times.
    
    '''
    _instance : ClassVar[Self | None] = None
    
    def __init__(self):
        '''
        Prevents initialization from anything other than this file.  That 
        enforces the sigleton contract.
        
        @throws RuntimeError if an external file constructs us:
        '''
        caller = traceback.extract_stack()[-2]
        if caller.filename != __file__:
            raise RuntimeError(
                f'Attempted to violate the CalloutExtensinManager singleton nature by constructing in {caller.filename}'
            )
        
        self._extensions: list[pyReadoutExtension] = []
        
        sm = ReadoutStateMachine.instance()
        sm.enter.connect(self._enter)
        sm.leave.connect(self._leave)
    @classmethod
    def instance(cls) -> Self:
        '''
        @return CalloutExtensionManager - the singleton instance.
        '''
        if not cls._instance:
            cls._instance = cls()
        
        return cls._instance
        
    def addExtension(self, extension : pyReadoutExtension) -> None:
        self._extensions.append(extension)
        extension.onRegistered()
    
    # Private slots:

    def _leave(self, fromstate : str, tostate : str) -> None:
        # Leaving one state ... starting the transition to another.
        match tostate:
            case 'Starting':
                self._invokeExtensions('onAboutToStart')
            case 'Not Ready':
                self._invokeExtensions('onShuttingDown')
            case 'Halted':                 # Three ways to get here...from Starting, Active, or Paused.
                if fromstate != 'Starting':
                    self._invokeExtensions('onEnding')
                    
            case 'Active':                      # Could be begin or resume:
                if fromstate == 'Halted':
                    self._invokeExtensions('onBeginning')
                else:
                    self._invokeExtensions('onResuming')
            case 'Paused':
                self._invokeExtensions('OnPausing')
            case _:                                             # Other tostates do nothing.
                pass
    def _enter(self, fromstate : str, tostate: str) -> None:
        # Entering the new state successfully:
        match tostate:
            case 'Halted':    # 3 ways to get here:
                if fromstate == 'Starting:':                   # Boot finished:
                    self._invokeExtension('OnStarted')
                else:                            # End of an active/paused run:
                    self._invokeExtension('OnEnded')
            case 'NotReady':
                self._invokeExtension('OnShutdown')
            case 'Active':          # Begin or Resume:
                if fromstate == 'Halted':
                    self._invokeExtension('OnBegun')
                else:
                    self._invokeExtension('OnResumed')
            case 'Paused':
                self._invokeExtension('OnPaused')
            case _:                                            # Anything else - do nothing.
                pass
            

    # Utilties:
    
    def _invokeExtensions(self, methodname : str, *args) -> None:
        func = methodcaller(methodname)
        for extension in self._extensions:
            func(extension)
    
    def _invokeBegin(self, methodname : str) -> None:
            # This is special because we need the run number and the title.
            # for OnBeginning and OnBegun
            
            
            gui  = mainWindow()
            if gui:        # Might not be defined in testing.
                title = gui.centralWidget().RunParameters().title()
                run   = gui.centralWidget().RunParameters().run()
                recording = gui.centralWidget().Recording.checkState == Qt.CheckState.Checked
            else:
                # For testing:
                title = 'a test title'
                run   = 1234
                recording = False
            for extension in self._extensions:
                func = getattr(extension, methodname)
                func(run, title, recording)
        
# Tests:
#
        
if __name__ == '__main__':
    import unittest
    import sys
    

    from PyQt6.QtCore import QCoreApplication
    
    class TestCallouts(pyReadoutExtension):
        def __init__(self):
            super().__init__()
            self.lastCalled = None
        #  The callbacks will just set self.lastCalled to their methodnames...
        #   This is simpler than a custom class decorator...
        def  onRegistered(self) -> None:
            self.lastCalled = 'onRegistered'
        def onAboutToStart(self) -> None:
            self.lastCalled = 'onAboutToStart'
        def onStarted(self) -> None:
            self.lastCalled = 'onStarted'
        def onBeginning(self) -> None:
            self.lastCalled = 'onBeginning'
        def onBegun(self) -> None:
            self.lastCalled = 'onBegun'
        def onEnding(self) -> None:
            self.lastCalled = 'onEnding'
        def onEnded(self) -> None:
            self.lastCalled = 'onEnded'
        def onPausing(self) -> None:
            self.lastCalled = 'onPausing'
        def onPaused(self) -> None:
            self.lastCalled = 'onPaused'
        def onResuming(self) -> None:
            self.lastCalled = 'onResuming'
        def onResumed(self) -> None:
            self.lastCalled = 'onResumed'
        def onShuttingDown(self) -> None:
            self.lastCalled= 'onShuttingDown'
        def onShutdown(self) -> None:
            self.lastCalled = 'onShutdown'
        
    
    # Tests:
    
    class TestReadoutCallouts(unittest.TestCase):
        def setUp(self):
            #  Reset the callout manager and register our extension:
            
            CalloutExtensionManager._instance = None
            self._extension = TestCallouts()
            CalloutExtensionManager.instance().addExtension(self._extension)
            
            # The state manager is a bit dirtier.  We get the instancde
            # and force the state -> NotReaDy via its 'private' _state and _laststate
            # vars.  this just puts it in a known state.
            
            ReadoutStateMachine.instance()._state = 'Not Ready'
            ReadoutStateMachine.instance()._lastState = 'Not Ready'
            
        def test_registerCallback(self):
            self.assertIsNotNone(self._extension.lastCalled)
            self.assertEqual('onRegistered', self._extension.lastCalled)
            
        def test_Starting(self):
            ReadoutStateMachine.instance().transition('Starting')
            self.assertEqual('onAboutToStart', self._extension.lastCalled)
    
    app = QCoreApplication(sys.argv)   # I think I need this for signals to flow.
    unittest.main()                    # Run my tests.
    
    
        
        