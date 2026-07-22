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
from PyQt6.QtCore import pyqtSignal, QObject


class LoggerTable(QTableView):
    '''
      This is a combination table view and model for displaying
      event log definitions.
      
      Attributes:
        loggers - the logger definitions. See below.
    
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
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._model = QStandardItemModel()
        self.setModel(self._model)
        
        self._clearModel()
        
        
    # Utilities:
    
    def _clearModel(self) -> None:
        # Clear the contents of the model and re-asssert the column headers:
        
        self._model.clear()
        self._model.setHorizontalHeaderLabels([
            'Container', 'Ring', 'DAQROOT', 'Host', 'Source', 'Destination', 'E', 'C', 'P' 
        ])
        
        
        
# Test code for now

if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication
    import sys

    app = QApplication(sys.argv)
    win = LoggerTable()
    
    win.show()
    sys.exit(app.exec())