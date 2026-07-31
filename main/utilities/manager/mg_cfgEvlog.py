#/usr/bin/env python3

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
Configuration program for event loggers.  See the event log
wizard for a simpler way to get started.

@file mg_cfgEvlog.py
@brief Configure event loggers.
@author Ron Fox.
@note  The loggers are represented for the most part by a dict which has the following keys:
* root - DAQ Root describing, along with the container which event log to run.
* ring - Source of data to be logged.
* host - Host the logger runs in.
* partial - If true, the logger is like a mutlilogger just making a soup of event files, otherwise 
         maintains the full directory structure.
* destination - Where the data are logged, should be an existing directory by the time the
        logger starts.
* critical - True if the experiment should be shutdown if the logger unexpectedly exits.
* enabled - True if the logger should log when event logging is turned on.
* container - Container in which the logger runs.
'''

from PyQt6.QtWidgets import (
    QTableView, QPushButton, QWidget, QLabel, QLineEdit,  QComboBox, QCheckBox, QFileDialog,
    QHBoxLayout, QVBoxLayout, QApplication, QStyle
)
from PyQt6.QtGui     import QStandardItemModel, QStandardItem
from PyQt6.QtCore    import pyqtSignal, QObject, QModelIndex, Qt

from nscldaq.mg_configutils import SaveDialog
from nscldaq.mg_database import Container, EventLog


import sqlite3
import sys
class EventLogTable(QTableView):
    '''
        This is a table view with integrated model for listing
        event loggers and their properties. 
        
        Properties:
            loggers - see the module docstrings for informationa bout
                     the format these are in.
        Methods:
            updateRow - Update a row to the new dict.
            deleteRow - Delete a row in the model.
            rowToDIct - Get logger dict  from row.
        Signals:
            selected(dict) - A logger was double clicked somewhere.  The 
                    dict describing the logger is passed to the slot.
                
    '''
    selected = pyqtSignal(dict)
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        
        self.setModel(QStandardItemModel())
        self._clear()           # Sets up the column headers.
        
        # Handle the doubleClicked signal and turn it into selected:
        
        self.doubleClicked.connect(self._doubleClickRelay)
    
    # Implement attributes:
    
    def loggers(self) -> list[dict]:
        '''
        @return list[dict] - list of logger definitions in the model.
        '''
        result = list()
        for row in range(self.model().rowCount()):
            result.append(self._rowToDict(row))
        
        return result
    
    def setLoggers(self, loggers : list[dict]) -> None:
        '''
        @param loggers : list[dict] - list of logger definition dicts to load
            into the table.  All previously defined loggers are removed.
        '''
        self._clear()
        for logger in loggers:
            self.model().appendRow(self._dictToRow(logger))
    
        self.resizeColumnsToContents()
        self.verticalHeader().hide()
    # Public methods:
    
    def updateRow(self, olddest: str, logger: dict) -> None:
        '''
            Given a row with the specified destination, update it
            to match the new logger definition (this can change the
            destination too).
            
            @param olddest :str - the destination in the row to change.
            @param logger  : dict - new logger definition to laod in that row.
        '''
        m = self.model()
        row = self._findDestination(olddest)
        rowValues = self._dictToRow(logger)
        
        for col, item in enumerate(rowValues):
            m.setItem(row, col, item)
    
    def deleteRow(self, destination: str) -> None:
        '''
            @param destination : str - the logger destination row to delete.
        '''
        row = self._findDestination(destination)
        self.model().takeRow(row)
    
    def getRow(self, row: int) -> dict:
        return self._rowToDict(row)   
    # Internal/private slots:
    
    def _doubleClickRelay(self, index :QModelIndex) -> None:
        row = index.row()
        logger = self._rowToDict(row)
        self.selected.emit(logger)
    # Utility methods:
    #
    def _dictToRow(self, logger : dict) -> list[QStandardItem]:
        # Utility to turn a logger dict into the list of standard items that
        # fit in a row:
        
        root        = QStandardItem(logger['root'])
        source      = QStandardItem(logger['ring'])
        host        = QStandardItem(logger['host'])
        partial     = self._boolToItem(logger['partial'])
        destination = QStandardItem(logger['destination'])
        critical    = self._boolToItem(logger['critical'])
        enabled     = self._boolToItem(logger['enabled'])
        container   = QStandardItem(logger['container'])
        
        return [
            root, source, host, destination, container, partial, critical, enabled
        ]
    
    def _rowToDict(self, row : int) -> dict:
        # Return the dict that corresponds to the row specified in the model:
        
        m = self.model()      # For brevity.
        root = m.item(row, 0).text()
        source = m.item(row, 1).text()
        host  = m.item(row,2).text()
        destination = m.item(row, 3).text()
        container   = m.item(row, 4).text()
        partial     = self._itemToBool(m.item(row,5))
        critical    = self._itemToBool(m.item(row, 6))
        enabled     = self._itemToBool(m.item(row, 7))
        return {
            'root': root, 'ring': source, 
            'host': host, 'partial': partial, 'destination' : destination,
            'critical' : critical, 'enabled': enabled, 'container' : container
        }
        
    def _clear(self) -> None:
        self.model().clear()
        self.model().setHorizontalHeaderLabels(["DAQ Root", "Source", "Host", "Destination", "Container", "P", "C", "E"])
        
    def _boolToItem(self, value : bool) -> QStandardItem:
        return QStandardItem('X' if value else ' ')
    
    def _itemToBool(self, item : QStandardItem) -> bool:
        return True if item.text() == 'X' else False
    
    def _findDestination(self, dest) -> int:
        #  Return the row that has the specified destination or IndexError if no such row.
        
        m = self.model()
        matches = m.findItems(dest, column=3)  # Destination column.
        if len(matches) == 0:
            raise IndexError(f'There is no row with the destination {dest}')
        
        row = matches[0].row()
        return row

class DeleteableEventlogTable(QWidget) :
    '''
        This is an event log table that allows the selected table item to be
        deleted by clicking a delete button to the right of the table.
        The deletion is fully autonomous. 
        
        Attributes:
        table -  accesses to the EventLogTable
        
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._layout = QHBoxLayout(self)
        self.setLayout(self._layout)
        
        self._table = EventLogTable(self)
        self._layout.addWidget(self._table)
        
        self._delete = QPushButton(self)
        self._delete.setIcon(
            self.style().standardIcon(getattr(QStyle.StandardPixmap, 'SP_DialogDiscardButton'))
        )
        self._layout.addWidget(self._delete)
        
        self._delete.clicked.connect(self._deleteRow)
    
    def table(self) -> EventLogTable:
        return self._table

    # Private internal slots:
    
    def _deleteRow(self) :
        # Delete button was clicked figure out the selected row's dest and delete it:
        t = self.table()
        selection = t.selectedIndexes()
        if len(selection) > 0:
            row = selection[0].row()
            item = t.getRow(row)
            dest =  item['destination']
            t.deleteRow(dest)
        

class EventLogDefiner(QWidget):
    ''' This is a widget that provides for editing the definition of an
        event logger.
        
        Attributes:
        root - get/set daqroot
        source - get set ringbuffer URI
        destination - get/set destination directory.
        host  - Get set host the logger runs in.
        containrs - Get/set the valid containers (combobox).
        container - Get/set the container to use
        partial - Get/set the partial flag.
        critical - get/set the criticality flag.
        enabled - get/set the enabled flag
        
        definition - Get/set the entire form from/to a dict.
        
        The form supplies autonomous directory browsing for the 'root' and 'destiniation'
        line edits.
        
        Signals:
           done - user is done editing.
        
    '''
    done = pyqtSignal()
      
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # The layout is a bunch of stacked strips
        self._layout = QVBoxLayout(self)
        self.setLayout(self._layout)
        
        # The top strip has the source and destination:
        
        srcdestlayout = QHBoxLayout()
        srcdestlayout.addWidget(QLabel('Source URI:', self))
        self._source  = QLineEdit(self)
        srcdestlayout.addWidget(self._source)
        
        srcdestlayout.addWidget(QLabel('Destination:', self))
        self._dest   = QLineEdit(self)
        srcdestlayout.addWidget(self._dest)
        self._browsedest = QPushButton('Browse...', self)
        srcdestlayout.addWidget(self._browsedest)
        
        self._layout.addLayout(srcdestlayout)
        
        #  The second strip is environment in which the logger runs:
        
        envlayout = QHBoxLayout()
        envlayout.addWidget(QLabel('Host: ', self))
        self._host = QLineEdit(self)
        envlayout.addWidget(self._host)
        
        envlayout.addWidget(QLabel('Container:', self))
        self._container = QComboBox(self)
        envlayout.addWidget(self._container)
        
        envlayout.addWidget(QLabel('DAQ Root:', self))
        self._root = QLineEdit(self)
        envlayout.addWidget(self._root)
        self._browseroot = QPushButton('Browse...', self)
        envlayout.addWidget(self._browseroot)
        
        self._layout.addLayout(envlayout)
        
        #  THe bottom strip are the checkboxes that the flags
        
        flaglayout = QHBoxLayout()
        self._partial = QCheckBox('Partial', self)
        flaglayout.addWidget(self._partial)
        
        self._critical = QCheckBox('Critical', self)
        flaglayout.addWidget(self._critical)
        
        self._enabled = QCheckBox('Enable', self)
        flaglayout.addWidget(self._enabled)
        
        self._layout.addLayout(flaglayout)
        
        #  Well there's also a button to Add/Modify:
        
        self._commit = QPushButton('Add/Modify', self)
        self._commit.setIcon(
             self.style().standardIcon(getattr(QStyle.StandardPixmap, 'SP_DialogApplyButton'))
        )
        self._layout.addWidget(self._commit)
    
        # Connect to signals
        
        self._commit.clicked.connect(self.done)
        self._browseroot.clicked.connect(self._getroot)
        self._browsedest.clicked.connect(self._getdest)
    
    # Implement the attributes:
    
    def root(self) -> str:
        ''' @return str - The contents of the root line entry'''
        return self._root.text()
    def setRoot(self, root: str) -> None:
        ''' @param root : str - new value of the root line entry'''
        self._root.setText(root)
        
    def source(self) -> str:
        ''' @return str - the contents of the sourcde line edit'''   
        return self._source.text()
    def setSource(self, uri : str) -> None:
        ''' @param uri - the URI of the ringbuffer data source: '''
        self._source.setText(uri)
    
    def destination(self) -> str:
        ''' @return str - the contents of the destination line edit'''
        return self._dest.text()
    def setDestiniation(self, dir : str) -> None:
        ''' @param dir : str - directory in which to record data'''
        self._dest.setText(dir)
        
    def host(self) -> str:
        ''' @return str - the host field value'''
        return self._host.text()
    def setHost(self, host : str) -> None:
        ''' @param host - the ne value for the host field'''
        self._host.setText(host)
    
    def containers(self) -> list[str]:
        ''' @return list[str] - list of containers in the containeres pulldown'''
        
        result = list()
        for i in range(self._container.count()):
            result.append(self._container.itemText(i))
        return result  
    def setContainers(self, containers: list[str]) -> None:
        ''' @param containers - the  list of containers to load in the combobox.'''
        self._container.clear()
        for c in containers:
            self._container.addItem(c)
    
    def container(self) -> str:
        ''' @return str - the currently selected container'''
        return self._container.currentText()
    def setContainer(self, container: str) -> None:
        ''' @param container  - container to select.
            @throws ValueError if container is not in the containers list.
        '''
        if container not in self.containers():
            raise ValueError(f'{container} is not in the list of valid containers')
        self._container.setCurrentText(container)
        
    def partial(self) -> bool:
        ''' @return bool  - state of the partial checkbutton.'''
        return self._checkstate(self._partial)
    def setPartial(self, isPartial: bool) -> None:
        ''' @param isPartial : bool - True if is a partial logger.'''
        self._setCheckstate(self._partial, isPartial)
    
    def critical(self) -> bool:
        '''' @return bool - state of the critical check button'''
        return self._checkstate(self._critical)
    def setCritical(self, isCritical : bool) -> None:
        ''' @param isCritical - criticality flag.'''
        self._setCheckstate(self._critical, isCritical)
        
    def enabled(self) -> bool:
        ''' @return bool - state of the enabled checkbox'''
        return self._checkstate(self._enabled)
    def setEnabled(self, isEnabled : bool) -> None:
        ''' @param isEnabled : bool - desired state of enabled checkbox'''
        self._setCheckstate(self._enabled, isEnabled)
        
    # Mega attribute:
    
    def definition(self) -> dict:
        ''' @return dict - the dictionary describing the logger 
            @note it is perfectly possible some fields will be empty.
        '''
        return {
            'root': self.root(), 'ring': self.source(), 'host' : self.host(),
            'partial' : self.partial(), 'destination' : self.destination(),
            'critical' : self.critical(), 'enabled': self.enabled(),
            'container' : self.container()
        }
    def setDefinition(self, logger: dict) -> None:
        ''' @param logger - logger definition dict.
            @note the ['container'] dict entry must be inthe
              valid container list else a ValueError will be raised.
              
        '''
        self.setRoot(logger['root'])
        self.setSource(logger['ring'])
        self.setHost(logger['host'])
        self.setPartial(logger['partial'])
        self.setDestiniation(logger['destination'])
        self.setCritical(logger['critical'])
        self.setEnabled(logger['enabled'])
        self.setContainer(logger['container'])
        
    # internal/private slots:
    
    def _getroot(self) -> None:
        #  Browser for and set a root directory:
        
        dir = QFileDialog.getExistingDirectory(self, 'DAQROOT directory')
        if dir.strip():
            self.setRoot(dir)
    
    def _getdest(self) -> None:
        dir = QFileDialog.getExistingDirectory(self, 'Log in:')
        if dir.strip():
            self.setDestiniation(dir)
    
    # Utilities:
    
    def _checkstate(self, widget : QCheckBox) -> bool:
        return True if widget.checkState() == Qt.CheckState.Checked else False
    def _setCheckstate(self, widget : QCheckBox, value : bool) -> None:
        widget.setCheckState(Qt.CheckState.Checked if value else Qt.CheckState.Unchecked)

class EventLogEditor(QWidget):
    '''
        This megawidget combines a DeletableEventlogTable with an
        EventlogDefiner to make a complete editor.  EventLogTable
        and the EventLogDefiner are gettable, so they can be loaded and
        read out.  Doubleclicking the table autonomously loads the 
        selected table line into the definer.
    '''   
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # Stack the two widgets:
        
        self._layout = QVBoxLayout(self)
        self.setLayout(self._layout)
        
        self._list = DeleteableEventlogTable(self)
        self._layout.addWidget(self._list)
        
        self._editor = EventLogDefiner(self)
        self._layout.addWidget(self._editor)
      
        # Handle the double clicks in the table:
        
        self.table().selected.connect(self._editor.setDefinition)  
    # Fetch the bits and pieces:
    
    def table(self) -> EventLogTable:
        return self._list.table()
    def editor(self) -> EventLogDefiner:
        return self._editor


class EventLogEditorDialog(SaveDialog):
    def __init__(self, parent : QObject | None = None):
        super().__init__(EventLogEditor(), parent)     
    
    
class EventLogEditController(QObject):
    '''
        Controller that links the eventlogeditor dialog
        to the database so that it is a fully functional editor
        for the FRIB/NSCLDAQ managed environment.
    '''
    def __init__(self, view : EventLogEditorDialog, config: str, parent : QObject | None = None):
        '''
        @param view -the dialog widget that is our user interface.
        @param config - The  configuration database file system path.
        @parma parent - a parent object, if any.
        '''
        super().__init__(parent)
        
        self._view = view
        self._config = config
        self._db     = sqlite3.connect(self._config)
        
        self._loadview()
    
    
    # Internal(private) utiltity methods:
    
    def _loadview(self) -> None:
        # Load the view from the database:
        
        workarea = self._view.workarea()
        # The editor container list:
        
        container_api = Container(self._db)
        workarea.editor().setContainers([x['name'] for x in container_api.list()])
        
        # The logger definitions:
        
        logger_api = EventLog(self._db)
        workarea.table().setLoggers(logger_api.list())
    
# Test code for now:

if __name__ == "__main__":
    
    
    app = QApplication(sys.argv)
    win = EventLogEditorDialog() 
    _controller = EventLogEditController(win, 'testing.db')
    
    
    win.show()
   
    sys.exit(app.exec())