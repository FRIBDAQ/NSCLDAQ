#!/usr/bin/env python3

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
@file pyReadoutGui.py
@brief THe main program for the python Readout GUI.
@author Ron Fox
'''
import sys
import importlib
import pathlib
import uuid

from nscldaq.readoutgui import (
    DDASDataSource,
    FRIBDAQDataSource,  # This import also registers
    MVLCDataSource,  # the sources with the source
    VMUSBDataSource,  # registry.
)
from nscldaq.readoutgui.ReadoutGuiView import ReadoutGuiMainWindow, mainWindow
from nscldaq.readoutgui.StateMachine import ReadoutStateMachine
from PyQt6.QtWidgets import QApplication, QFileDialog, QMessageBox

#  --- Slots.

def _loadModule() -> None:
    # File->Load operation.
    # Prompt for a file to load and load it using the
    # python dynamic load mechanism.  We'll generate
    # a module name using a UUID.
    
    # Prompt for the file:
    
    file,_ = QFileDialog.getOpenFileName(
        mainWindow(), 'Choose python file', '.', 'Python Files (*.py);;All Files (*)'
    )
    if file.strip():
        module = 'user_' + uuid.uuid4().hex
        path = pathlib.Path(file)
        module_spec = importlib.util.spec_from_file_location(module, path)
        if not module_spec:
            QMessageBox.warning(
                mainWindow(), 'Module load failed', f'Could not create a module load specification for {file}'
            )
            return 
        loaded_module = importlib.util.module_from_spec(module_spec)
        sys.modules[module] = loaded_module
        module_spec.loader.exec_module(loaded_module)   # Run the stuff in the module global space.

#   --- Utility functions.
def _loadOptionalModules() -> None:
    #  Perform some initializations.
    #  In this case, that means:
    #  1. trying to load a user initialiation module
    #     (readoutguiinit). That's an opportunity for the user
    #      to register new data sources and readout callouts.
    #  2. Load readoutguiconfig which might setup the data sources
    #     we have and whatever other stuff we need (e.g. event recording)
    #  
    #  It's not an error to fail the load of either of these.
    #  E.g. for the initial run of a plain vanilla system.
    #
    try:
        import readoutguiinit
    except ModuleNotFoundError:
        pass
    try:
        import readoutguiconfig
    except ModuleNotFoundError:
        pass

def _setupFileMenu(mainWindow : ReadoutGuiMainWindow) -> None:
    #  Set up the connections between the file menu commands and
    #  the file menu slots.
    
    mainWindow.fileMenuLoad.connect(_loadModule)
    #mainWindow.filemenuAddLibrary.connect(_addLibrary)
    #mainWindow.fileLog.connect(_setLogFile)
    #mainWindow.fileDisableLog.connect(_disableLogging)
    #mainWindow.fileExit.connect(_confirmExit)
    
    
def main() -> int:
    app = QApplication(sys.argv)
    main_window = ReadoutGuiMainWindow()
    
    _loadOptionalModules()
    _setupFileMenu(main_window)
    
    # Set the initial state of the buttons:
    
    main_window.centralWidget().StateControls().setState(ReadoutStateMachine.instance().state())
    
    main_window.show()
    
    
    return app.exec()

if __name__ == '__main__':
    sys.exit(main())
else:
    print('This is a main program, not intended to be loaded as a module!!!')
    sys.exit(-1)#    This software is Copyright by the Board of Trustees of Michigan
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

