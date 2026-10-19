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

from nscldaq.mg_configutils import SaveDialog
from nscldaq.readoutgui import (
    DataSourceViews,
    DDASDataSource,
    FRIBDAQDataSource,  # This import also registers
    MVLCDataSource,  # the sources with the source
    VMUSBDataSource,  # registry.
    DataSourceMenuControllers,
)
from nscldaq.readoutgui.DataSourceRegistry import DataSourceRegistry
from nscldaq.readoutgui.DataSource import DataSource
from nscldaq.readoutgui.DataSourceManager import DataSourceManager
from nscldaq.readoutgui.Log import Log
from nscldaq.readoutgui.ReadoutGuiView import ReadoutGuiMainWindow, mainWindow
from nscldaq.readoutgui.StateMachine import ReadoutStateMachine
from PyQt6.QtWidgets import QApplication, QDialog, QFileDialog, QMessageBox

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
            print('Generated data source name', name)
            DataSourceManager.instance().addSource(name, source)
        

def _listDataSources() -> None:
    # Bring up a data source lister:
    
    lister = DataSourceMenuControllers.ListController(mainWindow())
    lister.exec()
    
def _deleteDataSource() -> None: 
    deleter = DataSourceMenuControllers.DeleteController(mainWindow())
    deleter.exec()
    
#   --- Utility functions.

def  _saveConfiguration():
    # Stub function to save the configuration script.
    print("Trying to save configuration.")
    pass

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
    mainWindow.fileMenuAddLibrary.connect(_addLibrary)
    mainWindow.fileLog.connect(_setLogFile)
    mainWindow.fileDisableLog.connect(_disableLogging)
    mainWindow.fileExit.connect(_confirmExit)
    
    #  If the GUI is being destroyed, nonetheless, save the config if possible.
    
    mainWindow.destroyed.connect(_saveConfiguration)
 
def _setupDataSourceMenu(gui : ReadoutGuiMainWindow) -> None:
    gui.dsAddSource.connect(_addDataSource)
    gui.dsListSources.connect(_listDataSources)
    gui.dsDeleteSource.connect(_deleteDataSource)
#--------------------------------------------------- Program main --------------------------------------   
def main() -> int:
    app = QApplication(sys.argv)
    main_window = ReadoutGuiMainWindow()
    
    _loadOptionalModules()
    _setupFileMenu(main_window)
    _setupDataSourceMenu(main_window)
    
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

