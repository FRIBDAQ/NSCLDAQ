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
    QVBoxLayout, QHBoxLayout)
from PyQt6.QtGui import QStandardItemModel, QStandardItem
from PyQt6.QtCore import pyqtSignal, QObject, QModelIndex


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
        
        
# Test code for now

if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication
    import sys

    def dbl(logger : dict) -> None:
        print(logger)
        
        
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
    sys.exit(app.exec())