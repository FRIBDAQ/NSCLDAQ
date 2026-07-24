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
- A destination - A directory into which they log.
- An enable flag - when this is true, the logger logs if global logging is enabled.
- A complete flag - This determines if the logger operates like a primary logger or a multilogger
                 (In the old ReadoutShell sense of the word).
- A critical flag - If this is true and the logger exits while in the BEGIN state, the experiment is 
                SHUTDOWN.
'''

from PyQt6.QtWidgets import (QTableView, QLabel, QLineEdit, QComboBox, QPushButton, QCheckBox, QApplication,
    QVBoxLayout, QHBoxLayout, QWidget, QMessageBox, QFileDialog, QStyle, QDialog, QDialogButtonBox)
from PyQt6.QtGui import QStandardItemModel, QStandardItem
from PyQt6.QtCore import pyqtSignal, Qt, QObject, QModelIndex

import pathlib
import sqlite3
import sys
from mg_database import Container, EventLog

class LoggerTable(QTableView):
    '''
      This is a combination table view and model for displaying
      event log definitions.
      
      Attributes:
        loggers - the logger definitions. See below.
        selected - (readonly) the selected logger.
        model    - (readonly) the standard item model.
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
            
        self.resizeColumnsToContents()
        
    
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
        dir = QFileDialog.getExistingDirectory(self, 'Choose DAQ Root directory', '/usr/opt/daq')
        if dir.strip():
            self.setDaqroot(dir)

    def _browseDest(self) -> None:
        # Browse for a destination directory
        
        dir = QFileDialog.getExistingDirectory(
            self, 'Choose recording destination', str(pathlib.Path.home())
        )
        if dir.strip():
            self.setDestination(dir)

    def _clear(self) -> None:
        # Clear all the settable controls.  Note that the set of containers that can
        # be chosen is retained.  the first container in the list is
        # selected All checkboxes are unchecked.
        self.setDaqroot('')
        self.setDestination('')
        self.setRing('')
        self.setContainer(self.containers()[0])
        self.setHost('')
        self.setCritical(False)
        self.setPartial(False)
        self.setEnabled(False)
        

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


class EditLoggers(QWidget):
    ''''
        This consists of:
        LoggerTable  [Delete selected]
        LoggerDescription
        
        Autonomous activity:
           delete selected button removes the selected logger from the table.
           double clicking a logger loads it into the description.
           Add/Replace:
              - If a logger with that destination exists it is replaced with the definition in the description
              - Otherwise the description is added to the table.
              Note that if elements of the logger are missing, a messgae box will indicate that and Add/Replace
              will not happen.
        
        Attributes (readonly):
            table - returns the LoggerTable widget.
            description - Returns the LoggerDescription widget.
            
        Note: Normally, this will be encapsulated in an EditLoggersDialog, see that class below.
    
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # The primary layout is vertical stacking with the excursion to horiztonal 
        
        self._layout = QVBoxLayout(self)
        
        # Layout the table and delete selected buttons horiztonally though
        
        table_layout = QHBoxLayout()
        self._table = LoggerTable(self)
        table_layout.addWidget(self._table)
        
        self._delButton = QPushButton('Delete Selected', self)
        pixmap          = getattr(QStyle.StandardPixmap, 'SP_DialogCancelButton')
        icon            = self.style().standardIcon(pixmap)
        self._delButton.setIcon(icon)
        table_layout.addWidget(self._delButton)
        
        self._layout.addLayout(table_layout)
        
        # With the LoggerDescription below all of this:
        
        self._description  = LoggerDescription(self)
        self._layout.addWidget(self._description)
        
        # Set up local slots:
        
        self._table.loggerSelected.connect(self._loadDescription)
        self._delButton.clicked.connect(self._deleteSelected)
        self._description.accept.connect(self._addReplace)
        
    
    # Attributes:
        
    def table(self) -> LoggerTable:
        ''' @return LoggerTable - reference to the table widget.'''
        return self._table
    def description(self) -> LoggerDescription:
        ''' @return LoggerDescription  - reference to the description widget '''
        return self._description
    
    # Private slots:
    
    def _loadDescription(self, logger: dict) -> None:
        #  Load the description with the double clicked logger:
        
        self._description.setLogger(logger)
    def _deleteSelected(self) -> None:
        # Delete the selected table item.  We leverage that
        # the table really is a table view:
        
        selection = self._table.selectedIndexes()
        if len(selection) :
            row = selection[0].row()
            self._table.model().takeRow(row)
            
    def _addReplace(self) -> None:
        # The logger needs to have a root, destination, ring and host
        # Otherwise we just popup a messagebox and return.
        # If that's the case, the user an always complete the specification
        # Add/replace again.
        
        edit_logger = self._description.logger()
        if self._incomplete(edit_logger):
            QMessageBox.warning(self, 'Incomplete',
                'Loggers must have the root, destination, ring and host nonempty')
            return
        
        # If there's a logger with the same destination as the
        # described logger, replace it, otherwise, 
        # add it to the logger list:
        
        loggers = self._table.loggers()
        
        replaced = False
        for i, logger in enumerate(loggers):
            if edit_logger['destination'] == logger['destination']:
                loggers[i] = edit_logger
                replaced = True
                break
        if not replaced:
            loggers.append(edit_logger)
            
        # Rewrite the table
    
        self._table.setLoggers(loggers)
    
    # Utility methods        
    
    def _incomplete(self, logger: dict) -> bool:
        # Return false if we are missing elements of the
        # logger:
        
        return  not logger['root'].strip() or  \
                not logger['ring'].strip() or  \
                not logger['host'].strip() or  \
                not logger['destination'].strip()
# Test code for now

class EditLoggersDialog(QDialog):
    ''''
        A dialog that encapsulates the EditLoggers widget and Save/Cancel buttons
        workarea() retrieves the EditLoggers widget in the work area.
        
     Note that typicall, the controll just needs to connect with the accepted signal.
     The editor workarea is autonomous in all other respects.
     
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        self._workarea = EditLoggers(self)
        self._layout.addWidget(self._workarea)
        
        self._buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Save | QDialogButtonBox.StandardButton.Cancel
        )
        self._layout.addWidget(self._buttons)
        
        # Hook the button box into the dialog roles:
        
        self._buttons.accepted.connect(self.accept)
        self._buttons.rejected.connect(self.reject)
    
    def workarea(self) -> EditLoggers:
        ''' @return EditLoggers - the work area of the dialog'''
        
        return self._workarea

class EditLoggersController(QObject):
    '''
        Provide a controller for the EditLoggerDialog that
        can load it from the data base and handle the 
        accept button so that the database gets updated with the
        new definitions.
    '''
    def __init__(self, view : EditLoggersDialog, config: str, parent : QObject | None = None):
        '''
            @param view - the dialog widget.  
            @param config - The database configuration file.
            @param parewnt - any parent object desired by the application.
        '''
        super().__init__(parent)
        
        # * Save the parameters.
        # * Open and save a database connection.
        # * Stock the dialog (container list and logger definitions).
        # * Connect the dialog's accepted() signal to our code to update the
        #   database:
        
        self._view = view
        self._config = config
        self._db = sqlite3.connect(self._config)
        self._stockWorkarea()
        self._view.accepted.connect(self._updateDatabase)
        
    # Internal slots:
    
    def _updateDatabase(self) -> None:
        # To reconcile thingswe need both the table loggers and the
        # database loggers:
        
        logger_api = EventLog(self._db)
        db_list = logger_api.list()
        
        #Loggers are uniquified by their database Id which the table does not have,
        #and their destinations;
        
        db_dict = {x['destination'] : x for x in db_list}
        
        dlg_list = self._view.workarea().table().loggers()
        
        # An existing logger can be deleted:
        
        dlg_dests = [x['destination'] for x in dlg_list]
        for dest in db_dict:
            if dest not in dlg_dests:
                logger_api.delete(db_dict[dest]['id'])
        
        # An existing logger could be modified:  For now, if there's an
        # existing dest in the dialog, we delete and recreate it.
        # Otherwise it's a new logger:
        for logger in dlg_list:
            if logger['destination'] in db_dict.keys():
                logger_api.delete(db_dict[logger['destination']]['id'])   # Delete if modified. 
            logger_api.add(logger['root'], logger['ring'], logger['destination'], logger['container'], logger['host'],
                            {
                                'partial' : logger['partial'],
                                'critical' : logger['critical'],
                                'enabled'  : logger['enabled']
                            }
            )
    
    # Utilities
    def _stockWorkarea(self) -> None:
        # Get the components of the dialog:
        
        table = self._view.workarea().table()
        desc  = self._view.workarea().description()
        
        # Load the valid containers:
        
        container_api = Container(self._db)
        container_names = [x['name'] for x in container_api.list()]
        desc.setContainers(container_names)
        
        # Load the table with the existing event log definitions:
        
        eventlog_api = EventLog(self._db)
        table.setLoggers(eventlog_api.list())
    
    
def usage() -> None:
    ''' Print the program usage on stderr: '''    
    print('''
Usage:
    $DAQBIN/mg_loggeredit config_file_path
    
Edit the set of event loggers in the managed experment environment.
See, also, $DAQBIN/mg_logwizard for a wizard that creates loggers.

Where:
    config_file_path - is the path to the configuration file databsae.
        ''', file = sys.stderr)



def main() -> int:
    '''
      Program entry point.  Ensure the user gave us a database file and that it exists.
      
    '''
    if len(sys.argv) != 2:
        usage()
        return -1
    
    config_file = sys.argv[1]
    if not pathlib.Path(config_file).exists():
        print(f'{config_file} does not exist!', sys.stderr)
        usage()
        return -1
    
    # Set up the application user interface and hook in a controller:
    
    app = QApplication(sys.argv)
    dialog = EditLoggersDialog()
    _controller = EditLoggersController(dialog, config_file, dialog)
    
    dialog.show()
    return app.exec()
    
if __name__ == '__main__':
    sys.exit(main())