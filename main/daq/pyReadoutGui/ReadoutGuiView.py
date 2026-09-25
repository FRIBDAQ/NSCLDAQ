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
@file ReadoutGuiView.py
@brief Presents the entire python ReadoutGUi view.
@author Ron Fox
'''

from PyQt6.QtWidgets import QMainWindow, QWidget, QHBoxLayout, QVBoxLayout, QLabel, QSizePolicy, QMenuBar, QMenu, QCheckBox
from PyQt6.QtCore    import pyqtSignal
from PyQt6.QtGui      import QAction
from nscldaq.readoutgui import OutputManager, RunParamsView, RunTimerView, StateView


class ReadoutGuiCentralWidget(QWidget):
    '''
        This class is the central widget of the ReadoutGuiView.  It contains the
        RunParameters, State and Timed, and an OutputManager run controls.  These can be fetched
        from us via the public methods:
        
        RunParameters - Gets the RunParamsView widget. See docs for nscldaq.readoutgui.RunParamsView.RunParamsview
        StateControls - Gets the StateView.  See docs for nscldaq.readoutgui.StateView.StateButtons
        RunTimer      - Gets the timed run controls. See docs for nscldaq.readoutgui.RunTimer.RunTimer
        Outputs       - Gets the OutputManager. See docs for nscldaq.reaodutgui.OutputManager.OutputManager.
        Recording     - Recording checkbox.
    '''
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
    
        # THe entire layout is vertical ranges:
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        # We're going to want to control how things resize:
        
        fixedHeight = QSizePolicy(
            QSizePolicy.Policy.MinimumExpanding,     # Horizontal
            QSizePolicy.Policy.Fixed,                # Vertical
        )
        fullyStretched = QSizePolicy(
            QSizePolicy.Policy.MinimumExpanding,
            QSizePolicy.Policy.MinimumExpanding,
        )
        
        self._runparams = RunParamsView.RunParamsView(self)
        self._runparams.setSizePolicy(fixedHeight)
        self._layout.addWidget(self._runparams)
        
        # The StateView and RunTimerView are side by side:
        
        rslayout = QHBoxLayout()
        
        self._state = StateView.StateButtons(self)
        self._state.setSizePolicy(fixedHeight)
        rslayout.addWidget(self._state)
        
        self._timedruns = RunTimerView.RunTimerView(self)
        self._timedruns.setSizePolicy(fixedHeight)
        rslayout.addWidget(self._timedruns)    
        
        self._layout.addLayout(rslayout)
        
        # Recording button:
        
        self._recording = QCheckBox('Recording', self)
        self._recording.setSizePolicy(fixedHeight)
        self._layout.addWidget(self._recording)
        
        #  Below all of ths is the output manager.
        
        self._output = OutputManager.OutputManager(self)
        self._output.setSizePolicy(fullyStretched)
        self._layout.addWidget(self._output)
    
    # Public methods just allow clients to get components of the interface:
    
    def RunParameters(self) -> RunParamsView.RunParamsView:
        '''
        @return RunParamsView.RunParamsView - The run parameters view (title and run number)
        
        '''
        return self._runparams
    
    def StateControls(self) -> StateView.StateButtons:
        '''
        @return StateView.StateView - the widget that has the state control button(s)
        '''
        return self._state

    def RunTimer(self) -> RunTimerView.RunTimerView:
        '''
        @return RunTimerView.RunTimerView - the widget that has the timed run controls and elapsed time.
        '''
        return self._timedruns
    
    def Recording(self) -> QCheckBox:
        ''' 
        @return QCheckBox - the recording control.
        '''
        return self._recording
    
    def Outputs(self) -> OutputManager.OutputManager:
        '''
        @return OutputManager.OutputManager  - The output manager widget (tabs with outputs)
        '''
        return self._output
   
class ReadoutGuiMainWindow(QMainWindow):
    ''''
        This widget will be the central widget of the Readout GUI app.
        The Central Widget will be a ReadoutGuiCentralWidget.
        The status bar will have a permanent widget  in which
        messages can be set (see below).  This is intended to be used
        for the recorded data satatus.
        
        Methods:
            (All of the QCentralWidget methods of course)
            setStatusMessage - Sets the message in the permanent status bar widget.
        
        Signals:
        File Menu:
            fileMenuLoad       - Load a python script.
            fileMenuAddLibrary - add a directory to the module search path.
            fileLog            - Start logging
            fileDisableLog     - Disable logging.
            fileExit           - Exit program.
        Data Source Menu:
            dsAddSource        - Add a data source.
            dsDeleteSource     - Delete a data source
            dsListSources      - List data sources
        Settings menu:
            settingsEventLog    - Event log settings.
    '''
    # Signals:
    
    fileMenuLoad       = pyqtSignal()
    fileMenuAddLibrary = pyqtSignal()
    fileLog            = pyqtSignal()
    fileDisableLog     = pyqtSignal()
    fileExit           = pyqtSignal()
    
    dsAddSource        = pyqtSignal()
    dsDeleteSource     = pyqtSignal()
    dsListSources      = pyqtSignal()
    
    settingsEventLog   = pyqtSignal()
    
    
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        
        self.setCentralWidget(ReadoutGuiCentralWidget(self))
        sb  = self.statusBar()
        self._statusText = QLabel(sb)
        sb.addPermanentWidget(self._statusText, 50)    # 1/2 the sbar?
        
        self._createMenus()
        
    # Public methods:
    
    def setStatusMessage(self, msg : str) -> None:
        '''
        @param msg : str - The message string to put in the status text
        @note to clear, just use an empty string.
        '''
        self._statusText.setText(msg)
        
    # Utiltity methods:
    
    def _createMenus(self) -> None:
        # Create the menus and their actions.
        menubar = self.menuBar()
        fileMenu = menubar.addMenu('File')
        self._createFileMenu(fileMenu)
        
        dataSourceMenu = menubar.addMenu('Data Source')
        self._createDataSourceMenu(dataSourceMenu)
        
        settingsMenu = menubar.addMenu('Settings')
        self._createSettingsMenu(settingsMenu)
    
    
    def _createFileMenu(self, fileMenu : QMenu) -> None:
        # Create the entries for the file menu and map the action triggers
        # to our signals.
        self._fileLoadAction = QAction('Load...', fileMenu)
        self._fileLoadAction.triggered.connect(self.fileMenuLoad)
        fileMenu.addAction(self._fileLoadAction)
        
        self._fileAddLibAction = QAction('Add Library...', fileMenu)
        self._fileAddLibAction.triggered.connect(self.fileMenuAddLibrary)
        fileMenu.addAction(self._fileAddLibAction)
        
        fileMenu.addSeparator()
        
        self._fileLogAction = QAction('Log...', fileMenu)
        self._fileLogAction.triggered.connect(self.fileLog)
        fileMenu.addAction(self._fileLogAction)
        
        self._fileDisableLogAction = QAction('Disable Logging', fileMenu)
        self._fileDisableLogAction.triggered.connect(self.fileDisableLog)
        fileMenu.addAction(self._fileDisableLogAction)
        
        fileMenu.addSeparator()
        
        self._fileExitAction = QAction('Exit...', fileMenu)
        self._fileExitAction.triggered.connect(self.fileExit)
        fileMenu.addAction(self._fileExitAction)
    
    def _createDataSourceMenu(self, menu : QMenu) -> None:
        # Create the data source menu actions and map their
        # signals to our class's signals.
        
        self._dsAddAction = QAction('Add...', menu)
        self._dsAddAction.triggered.connect(self.dsAddSource)
        menu.addAction(self._dsAddAction)
        
        self._dsDelSourceAction = QAction('Delete...', menu)
        self._dsDelSourceAction.triggered.connect(self.dsDeleteSource)
        menu.addAction(self._dsDelSourceAction)
        
        menu.addSeparator()
        
        self._dsListAction = QAction('List', menu)
        self._dsListAction.triggered.connect(self.dsListSources)
        menu.addAction(self._dsListAction)
        
    def _createSettingsMenu(self, menu : QMenu) -> None:
        # Create the settings menu and connect its actions to our signals.
        
        self._settingsEvlogAction = QAction('Event Log...', menu)
        self._settingsEvlogAction.triggered.connect(self.settingsEventLog)
        menu.addAction(self._settingsEvlogAction)
        
        
#  Test code:

if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication
    from PyQt6.QtCore import Qt
    import sys

    app = QApplication(sys.argv)
    win = ReadoutGuiMainWindow()
    
    win.setStatusMessage('Recording to abc.evt 100Mb')
    win.centralWidget().Outputs().setRecording(True)
    win.centralWidget().Outputs().addToMain('Run started')
    win.centralWidget().StateControls().setState('Active')
    win.centralWidget().Recording().setCheckState(Qt.CheckState.Checked)
    
    win.show()
    sys.exit(app.exec())
        
        
        
    