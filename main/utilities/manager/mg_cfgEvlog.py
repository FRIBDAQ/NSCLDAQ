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

from PyQt6.QtWidgets import QTableView, QPushButton, QWidget, QHBoxLayout, QStyle
from PyQt6.QtGui     import QStandardItemModel, QStandardItem
from PyQt6.QtCore    import pyqtSignal, QObject, QModelIndex
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
        
    
# Test code for now:

if __name__ == "__main__":
    from PyQt6.QtWidgets import QApplication
    import sys

    app = QApplication(sys.argv)
    wid = DeleteableEventlogTable()
    win = wid.table()
    def sel(logger: dict) -> None:
        print(logger)
    
    someloggers = [
        {
            'root' : '/usr/opt/daq/12.2-009', 'ring' : 'tcp://localhost/ron', 
            'host' : 'localhost', 'partial' : True, 
            'destination' : '/home/ron/stagearea/partial', 'critical' : False, 'enabled' : True,
            'container' : 'bookworm'
        },
        {
            'root' : '/usr/opt/daq/12.2-009', 'ring' : 'tcp://localhost/built', 
            'host' : 'localhost', 'partial' : False, 
            'destination' : '/home/ron/stagearea/built', 'critical' : True, 'enabled' : True,
            'container' : 'bookworm'
        }
    ]
    win.setLoggers(someloggers)
    win.selected.connect(sel)
    wid.show()
    sys.exit(app.exec())