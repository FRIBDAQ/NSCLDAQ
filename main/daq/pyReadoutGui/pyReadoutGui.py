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
import importlib
import pathlib
import sys
import traceback
import uuid

import nscldaq.readoutgui.DataSourceConfig  # Loads/registers the serializer for data sources.
import nscldaq.readoutgui.RunConfig  # Loads/registers the serializer for run config.
from nscldaq.mg_configutils import SaveDialog
from nscldaq.readoutgui import (
    DataSourceMenuControllers,
    DataSourceViews,
    DDASDataSource,
    FRIBDAQDataSource,  # This import also registers
    MVLCDataSource,  # the sources with the source  # noqa: F401
    VMUSBDataSource,  # registry.
)
from nscldaq.readoutgui.DataSourceController import DataSourceController

from nscldaq.readoutgui.DataSource import DataSource
from nscldaq.readoutgui.DataSourceManager import DataSourceManager
from nscldaq.readoutgui.DataSourceRegistry import DataSourceRegistry
from nscldaq.readoutgui.Log import Log
from nscldaq.readoutgui.ReadoutGuiView import ReadoutGuiMainWindow, ReadoutGuiCentralWidget, mainWindow
from nscldaq.readoutgui.StateMachine import ReadoutStateMachine
from nscldaq.readoutgui.StateSaveRestore import SerializationManager
from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import QApplication, QDialog, QFileDialog, QMessageBox

# Constants

AUTOSAVE_INTERVAL_MS = 1000   

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
        try:
            module_spec.loader.exec_module(loaded_module)   # Run the stuff in the module global space.
        except Exception as e:
            QMessageBox.warning(
                mainWindow(), 'Module failed', 
                f'Running the module global code for {file} failed: {e}\n{traceback.format_exc()}'
            )

def _addLibrary() -> None:
    #
    # File->Add Library menu command. IF we can just load a
    # file you might wonder why we might need this.  The answer is
    # that if we load a script with imports that are not in the module
    # paths, that load will fail and we'll need to extend the
    # module search path.
    
    dir = QFileDialog.getExistingDirectory(mainWindow(), 'Add to module search path', '.')
    if dir.strip():
        sys.path.append(dir)

def _setLogFile() -> None:
    # Prompt for a new log file for the logging subsystrem.
    
    file,_  = QFileDialog.getSaveFileName(
        mainWindow(), 'Choose log file',  '.', 'Log Files (*.log);;All Files'
    )
    if file.strip():
        Log.instance().setLogFile(file)

def _disableLogging() -> None:
    # Turn off logging to file - log messages will still
    # go to the appropriate output windows.
    
    Log.instance().disable()


def _confirmExit() -> None:
    # Handle File->Exit - confirm the user really wants to exit.
    # If so, save the configuration and ask the application to exit.
    
    if QMessageBox.question(
        mainWindow(), 'Confirm exit', 
        'Do you really want to exit?'
    ) == QMessageBox.StandardButton.Yes:
        _saveConfiguration()
        # Stop data sources:
        
        try:
            DataSourceManager.instance().stop()
        except Exception:  # noqa: BLE001, S110
            pass
        QApplication.exit(0)

def _addDataSource() -> None:
    # Prompt for and add a data source.

    dsTypes = DataSourceRegistry.instance().listSourceTypes()
    typedlg = DataSourceViews.TypeChooser(dsTypes, mainWindow())
    
    if typedlg.exec() == QDialog.DialogCode.Accepted:
        dataSourceType = typedlg.selected()
        print('Making ', dataSourceType)
        configdlg = SaveDialog(DataSourceRegistry.instance().sourceCreator(dataSourceType)())
        result = configdlg.exec()
        while  result == QDialog.DialogCode.Accepted:
            source : DataSource | str = configdlg.workarea().makeSource()
            if isinstance(source, DataSource):
                break
            else:
                QMessageBox.warning(configdlg, "Can't make source'", f'Data source cannot be made: {source}')
            result = configdlg.exec()
        # We have a data in source.  We need to name it and add it to the manager... in the future
        # Need to change up our widgets to get names.
        
        if result == QDialog.DialogCode.Accepted:
            name = dataSourceType + ':' +  uuid.uuid4().hex
            mgr  = DataSourceManager.instance()
            mgr.addSource(name, source)
            
            # If necessary, remove the pause state from the state manager.
            # note at present, we can't get this back.
            if not mgr.mergedCapabilities()['canPause']:
                ReadoutStateMachine.instance().removePause()
                
        

def _listDataSources() -> None:
    # Bring up a data source lister:
    
    lister = DataSourceMenuControllers.ListController(mainWindow())
    lister.exec()
    
def _deleteDataSource() -> None: 
    deleter = DataSourceMenuControllers.DeleteController(mainWindow())
    deleter.exec()
    
def _checkSources(stateManager : ReadoutStateMachine) -> None:
    # If the state is not Not Ready or Starting, ask
    # the data source manager to check source liveness.
    
    if stateManager.state() not in ['Not Ready', 'Starting']:
        DataSourceManager.instance().live()
        
#   --- Utility functions.

def  _saveConfiguration():
    # Stub function to save the configuration script.
    
    with open('readoutguiconfig.py', 'w', encoding='utf-8') as config:
        SerializationManager.instance().serialize(config)

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
        SerializationManager.instance().deserialize(readoutguiconfig)    # load the configuration.
    except ModuleNotFoundError:
        pass

def _setupFileMenu(mainWindow : ReadoutGuiMainWindow) -> None:
    #  Set up the connections between the file menu commands and
    #  the file menu slots.
    
    mainWindow.fileMenuLoad.connect(_loadModule)
    mainWindow.fileMenuAddLibrary.connect(_addLibrary)
    mainWindow.fileLog.connect(_setLogFile)
    mainWindow.fileDisableLog.connect(_disableLogging)
    mainWindow.fileExit.connect(_confirmExit)
    
 
def _setupDataSourceMenu(gui : ReadoutGuiMainWindow) -> None:
    gui.dsAddSource.connect(_addDataSource)
    gui.dsListSources.connect(_listDataSources)
    gui.dsDeleteSource.connect(_deleteDataSource)
    
def _setupStateInteractions(central : ReadoutGuiCentralWidget, manager : ReadoutStateMachine) -> None:
    #  Set up the signals so that the state machine will undertake the transitions requested
    #  by the gui buttons.
    
    buttons = central.StateControls()
    buttons.start.connect(lambda: manager.transition('Starting'))
    buttons.begin.connect(lambda: manager.transition('Active'))
    buttons.end.connect(lambda: manager.transition('Halted'))
    buttons.pause.connect(lambda: manager.transition('Paused'))
    buttons.resume.connect(lambda: manager.transition('Active'))
    manager.newstate.connect(buttons.setState)          # Update the buttons to match the new state.
    
#--------------------------------------------------- Program main --------------------------------------   
def main() -> int:
    app = QApplication(sys.argv)
    main_window = ReadoutGuiMainWindow()
    
    stateManager = ReadoutStateMachine.instance()   # We'll need this.
    
    _loadOptionalModules()
    _setupFileMenu(main_window)
    _setupDataSourceMenu(main_window)
    
    # Set up auto save.  Every second we'll save the configuration:
    
    autosave_timer = QTimer(main_window)
    autosave_timer.setSingleShot(False)
    autosave_timer.setInterval(AUTOSAVE_INTERVAL_MS)
    autosave_timer.timeout.connect(_saveConfiguration)
    autosave_timer.start()
    
    # Alsos use the timer to monitor data sources:
    
    autosave_timer.timeout.connect(lambda: _checkSources(stateManager, ))  # will signal on failuer.
    
    
    # Set the initial state of the buttons:
    
    
    main_window.centralWidget().StateControls().setState(stateManager.state())
    _setupStateInteractions(main_window.centralWidget(), stateManager)
    _dataSourceController = DataSourceController()      # Handles state machine/data source interactions.
    
    # Try to ensure the data sources shutdown:
    
    app.aboutToQuit.connect(DataSourceManager.instance().stop)
    
    #  Ensure the main window is visible.
    
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

