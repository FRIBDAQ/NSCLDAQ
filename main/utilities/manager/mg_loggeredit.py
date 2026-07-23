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
Provides an editor that defines eventloggers for the FRIB/NSCLDAQ managed environment.
Event loggers have the following properties:

- A container - that establishes the environment in which they run.
- A DAQROOT   - which determines that actual program image that's run.
- A host      - the system in which they execute.
- A source    - The URI of a ringbuffer from which they take data
- A destinatino - A directory into which they log.
- An enable flag - when this is true, the logger logs if global logging is enabled.
- A complete flag - This determines if the logger operates like a primary logger or a multilogger
                 (In the old ReadoutShell sense of the word).
- A critical flag - If this is true and the logger exits while in the BEGIN state, the experiment is 
                SHUTDOWN.
'''

from PyQt6.QtWidgets import (QTableView, QLabel, QLineEdit, QComboBox, QPushButton, QCheckBox,
    QVBoxLayout, QHBoxLayout, QWidget)
from PyQt6.QtGui import QStandardItemModel, QStandardItem
from PyQt6.QtCore import pyqtSignal, Qt, QObject, QModelIndex


class LoggerTable(QTableView):
    '''
      This is a combination table view and model for displaying
      event log definitions.
      
      Attributes:
        loggers - the logger definitions. See below.
        selected - (readonly) the selected logger.
      Signals:
        loggerSelected(dict) - A logger was selected via a double click.
        
        
     Loggers are defined by a dict that minimally has the following keys:
     * 'root' - The DAQRoot the logger operates in.
     * 'ring' - The ring URI from which the logger takes data.
     * 'host' - The host in which the logger runs.
     * 'partial' - True if the logger operates like a multilogger.
     * 'destination' - destination directory to which the logger puts data.
     * 'critical' - True if the logger is critical to experiment operation.
     * 'enabled' - True if the logger is enabled.
     * 'container' - Name of the container in which the logger runs.
     
     loggers are an iterable object of these dicts.
     
    '''
    loggerSelected = pyqtSignal(dict)
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._model = QStandardItemModel()
        self.setModel(self._model)
        
        self._clearModel()
        self.doubleClicked.connect(self._selectRelay)
    
    # Attributes:
    
    def setLoggers(self, loggers : list[dict]) -> None:
        '''
            Clear the table and set the loggers as requested.
            @param loggers - list of logger definitions. See the class docstring for what a 
                logger definition is.
        '''
        self._clearModel()                # Clear what was there and set the headers.
        
        for logger in loggers:
            container = QStandardItem(logger['container'])
            ring      = QStandardItem(logger['ring'])
            root      = QStandardItem(logger['root'])
            host      = QStandardItem(logger['host'])
            destination= QStandardItem(logger['destination'])
            enabled   = QStandardItem(self._indicator(logger['enabled']))
            critical  = QStandardItem(self._indicator(logger['critical'])) 
            partial   = QStandardItem(self._indicator(logger['partial']))
            
            self._model.appendRow([
                container, ring, root, host, destination, enabled, critical, partial
            ])
    
    def loggers(self) -> list[dict]:
        '''
            @return list[dict] - the dicts have the stucture of a logger definition. See class
               docstring above.
        '''
        result = list()
        for row in range(self._model.rowCount()):   
            result.append(self._rowToDict(row))    
        return result
    
    def selected(self) -> dict | None:
        '''' @return dict | None - the selected logger. None if nothing is selected. '''
        selection = self.selectedIndexes()
        if len(selection) > 0:
            row = selection[0].row()
            return self._rowToDict(row)
        
        return None
    # Private slots:
    
    def _selectRelay(self, index : QModelIndex):
        definition = self._rowToDict(index.row())
        self.loggerSelected.emit(definition)
    # Utilities:
    
    def _clearModel(self) -> None:
        # Clear the contents of the model and re-asssert the column headers:
        
        self._model.clear()
        self._model.setHorizontalHeaderLabels([
            'Container', 'Source', 'DAQROOT', 'Host',  'Destination', 'E', 'C', 'P' 
        ])
    
    def _rowToDict(self, row :int) -> dict:
        container = self._model.item(row, 0).text()
        ring      = self._model.item(row, 1).text()
        root      = self._model.item(row, 2).text()
        host      = self._model.item(row, 3).text()
        destination = self._model.item(row, 4).text()
        enabled  = self._indicatorBool(self._model.item(row, 5))
        critical  = self._indicatorBool(self._model.item(row, 6))
        partial   = self._indicatorBool(self._model.item(row, 7)) 
        return {
                'container' : container, 'ring': ring, 'root': root,
                'host': host, 'destination':destination, 
                'enabled': enabled, 'critical': critical, 'partial': partial
                
            }
    def _indicator(self, item : bool) -> str:
        return 'X' if item else ' '
    
    def _indicatorBool(self, item : QStandardItemModel) -> bool:
        return True if item.text() == 'X' else False
        
class  LoggerDescription(QWidget):
    ''''
    This compound widget contains the controls needed to specify
    a logger.  This can be used either to specify a new logger or
    edit an existing logger.  
    
    Attributes are:
    
    daqroot - Roo of the FRIB/NSCLDAQ directory tree to use.
    destination - A destination directory for the logged data.
    ring    - A ring buffer from which to take data.
    containers - List of containers to choose from.
    container - The selected container.
    host     - Host in which the logger runs.
    critical - bool - True if the logger is critical.
    partial  - bool - true if the logger is a partial one.
    enabled  - bool - true if the logger is enabled.
    
    Methods:
      These two are a pseudo attribute:
      
    setLogger - Given a logger definition dict (see LoggerTable class), fills the attributes of
             the form.
    logger - Retrieve the logger definition from the form.
    
    Signals:
        accept - the Add/Modify button is clicked.
        
    Note there are internal/autonomously handled signal/slots. I name those slots below.
    
    _browseRoot - browse for daqroot directory.
    _browseDest - Browse for a destination directory.
    _clear      - Clear the form.
    '''
    accept = pyqtSignal()
    
    def __init__(self, parent : QObject |None = None):
        super().__init__(parent)
        
        # The overal layout is a vertical stack of horiztonal strips of widgets:
        
        self._layout = QVBoxLayout(self)
        
        #  The first horizontal strip is run-time environment.
        
        self._layout.addLayout(self._makeRuntimeEnv())
        
        # The second horizontal strip is the sourcde/dest specifiers
        
        self._layout.addLayout(self._makeSourceDest())
        
        # Next to the bottom are the checkbuttons:
        
        self._layout.addLayout(self._makeCheckBoxes())
        
        # The bottom horizontal strip is the clear and Add/Modify buttons.
        # note that _makeButtons connects the buttons to appropriate
        # signal handlers.
        
        self._layout.addLayout(self._makeButtons())

        # Finalize the GU.
        
        self.setLayout(self._layout)
        
    # Implement attributes
    
    def daqroot(self) -> str:
        ''' @return str - the contents of the DAQROOT line edit'''
        return self._daqroot.text()
    def setDaqroot(self, root : str) -> None:
        ''' @param root : str - the new value of the DAQRoot. '''
        self._daqroot.setText(root)
        
    def destination(self) -> str:
        ''' @return str - the contents of the destination line edit. '''
        return self._destination.text()
    def setDestination(self, dest : str) -> None:
        ''' @param dest: str -the new value for the destination line edit contents '''
        self._destination.setText(dest)
        
    def ring(self) -> str:
        ''' @return str - The contents of the data source ringbuffer URI'''
        return self._ring.text()
    def setRing(self, ring : str) -> str:
        ''' @param ring: str - The URI of the ring to load into the destination text edit.'''
        
        self._ring.setText(ring)
        
    def containers(self) -> list[str]:
        ''' @return list[str] - The list of containers that are valid in the combobox.'''
        
        return [self._container.itemText(i) for i in range(self._container.count())]
    def setContainers(self, containers : list[str]) -> None:
        ''' @param containers: list[str] the set of containers to load into the combobox list.'''
        self._container.clear()
        for container in containers:
            self._container.addItem(container)
    
    def container(self) -> str:
        ''' @return str - the currently selected container. '''
        return self._container.currentText()
    def setContainer(self, container: str) -> None:
        ''' 
            @param container : str The containre to select. 
            @throw ValuError If cotainer that's not in the list
        '''
        if container not in self.containers():
            raise ValueError(f'{container} is not in the list of known containers')
        self._container.setCurrentText(container)
        
    def host(self) -> str: 
        ''' @return str - the current value of the host line edit'''
        return self._host.text()
    def setHost(self, host : str) -> None:
        ''' @param host: str - the host to put in the host line edit'''
        self._host.setText(host)
        
    def critical(self) -> bool:
        ''' @return bool state of the critical checkbox '''
        return self._checkboxValue(self._critical)
    def setCritical(self, state: bool) -> None:
        ''' @param state : bool - state to set the critical check box.'''
        self._setCheckbox(self._critical, state)

    def partial(self) -> bool:
        ''' @return bool -state of the partial checkbox. '''
        return self._checkboxValue(self._partial)
    def setPartial(self, value : bool) -> None:
        ''' @param value: bool - the state of the partial check box'''
        self._setCheckbox(self._partial, value)
    
    def enabled(self) -> bool:
        ''' @return bool - the state of the enabled checkbox '''
        return self._checkboxValue(self._enabled)
    def setEnabled(self, value: bool) -> None:
        ''' @param value: bool - state to set the enabled checkbox.'''
        self._setCheckbox(self._enabled, value)
    
    # Public methods:
    
    def setLogger(self, definition: dict) -> None:
        '''
        Set the logger defintition in one throw.
        @param definition is a dict whose definition is given in the docstrings for LoggerTable.
        @note  Some other agency had better have set the containers attribute first or else this will
            fail with a value error when calling setContainer to select the container.
        '''
        self.setDaqroot(definition['root'])
        self.setRing(definition['ring'])
        self.setHost(definition['host'])
        self.setPartial(definition['partial'])
        self.setDestination(definition['destination'])
        self.setCritical(definition['critical'])
        self.setEnabled(definition['enabled'])
        self.setContainer(definition['container'])
    
    def logger(self) -> dict:
        '''
            @return dict - A logger definition that contains the current state of the
                editor. 
            @note that there may be empty (not None) values in the dict.
            @note See the docstring for LoggerTable for a description of the dict returned.
        '''
        return {
            'root': self.daqroot(),
            'ring': self.ring(),
            'host': self.host(), 
            'partial': self.partial(),
            'destination': self.destination(),
            'critical' : self.critical(),
            'enabled'  : self.enabled(),
            'container': self.container()

        }
        
    # Local/private slots:

    def _browseRoot(self) -> None:
        #  Browse for a directory to put in the self._daqroot QLineEdit
        pass

    def _browseDest(self) -> None:
        # Browse for a destination directory
        pass

    def _clear(self) -> None:
        # Clear all the settable controls.
        pass

    # private GUI utility methods:

    def _makeRuntimeEnv(self) -> QHBoxLayout:
        #  Create the labels and controls for the loggers's runtime
        #  environment. These are put in a horiztonal strip (QHBoxLayout) and
        #  the layout returned to the caller to be put in the top level layout.
        
        layout = QHBoxLayout()
        layout.addWidget(QLabel('DAQROOT', self))
        
        self._daqroot = QLineEdit(self)
        layout.addWidget(self._daqroot)
        self._browseRootButton = QPushButton('Browse...', self)
        layout.addWidget(self._browseRootButton)
        self._browseRootButton.clicked.connect(self._browseRoot)
        
        layout.addWidget(QLabel('Container', self))
        self._container = QComboBox(self)
        layout.addWidget(self._container)
        
        layout.addWidget(QLabel('Host', self))
        self._host = QLineEdit(self)
        layout.addWidget(self._host)
        
        return layout

    def _makeSourceDest(self) -> QHBoxLayout:
        #  Return a horiztonal layout that contains the
        #  labels and control to set a logger's source
        #  and destination.
        
        layout = QHBoxLayout()
        
        layout.addWidget(QLabel('Ring URI', self))
        self._ring = QLineEdit(self)
        layout.addWidget(self._ring)
        
        layout.addWidget(QLabel('Destination', self))
        self._destination = QLineEdit(self)
        layout.addWidget(self._destination)
        self._browseDestinationButton = QPushButton('Browse...')
        layout.addWidget(self._browseDestinationButton)
        self._browseDestinationButton.clicked.connect(self._browseDest)

        return layout

    def _makeCheckBoxes(self) -> QHBoxLayout:
        #  The checkboxes for the bool values:
        layout = QHBoxLayout()
        
        self._critical = QCheckBox('Critical', self)
        layout.addWidget(self._critical)
        
        self._partial = QCheckBox('Partial', self)
        layout.addWidget(self._partial)
        
        self._enabled = QCheckBox('Enabled', self)
        layout.addWidget(self._enabled)
        
        return layout
    def _makeButtons(self) -> QHBoxLayout:
        # Return a layout that contains the clear and Add/modify buttons.
        # Note that signals are hooked up
        
        layout = QHBoxLayout()
        
        self._clearButton = QPushButton('Clear', self)    
        layout.addWidget(self._clearButton)
        self._clearButton.clicked.connect(self._clear)
        
        self._acceptButton = QPushButton('Add/Modify', self)
        layout.addWidget(self._acceptButton)
        self._acceptButton.clicked.connect(self.accept)
        
        return layout
    
    # Utlities:
    
    def _checkboxValue(self, box : QCheckBox) -> bool:
        # return checkbox value as a bool:
        
        return box.checkState() == Qt.CheckState.Checked
    
    def _setCheckbox(self, box : QCheckBox, value: bool):
        if type(value) is not bool:
            raise ValueError(f'Setting a checkbox: {box.text()}; {value} was not a bool.')
        box.setCheckState(
            Qt.CheckState.Checked if value else Qt.CheckState.Unchecked
        )
# Test code for now

if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication
    import sys

    def dbl(logger : dict) -> None:
        print(logger)
        
    def accept() -> None:
        print('accepted')
        print(edit.logger())
        
    app = QApplication(sys.argv)
    win = LoggerTable()
    
    loggers = [
        {'container': 'bucky', 'ring': 'tcp://localhost/ron', 
         'root':'/usr/opt/daq/12.2-009', 'host': 'localhost', 'destination': '/events/ron', 
         'enabled': False, 'critical': True, 'partial': True},
    ]
    win.setLoggers(loggers)
    print(loggers)
    print(win.loggers())
    print(loggers == win.loggers())
    
    win.loggerSelected.connect(dbl)
    win.show()
    
    # Edit widget:
    
    edit = LoggerDescription()
    
    edit.setContainers(['jessie', 'buster', 'bullseye', 'bucky', 'bookworm'])
    edit.setLogger(loggers[0])
    edit.show()
    edit.accept.connect(accept)
    
    sys.exit(app.exec())