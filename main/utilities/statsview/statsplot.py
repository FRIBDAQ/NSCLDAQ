#!/usr/bin/python3

''''
This program provides a utility for plotting the statistics 
database.  The idea is that you can open a database file
and get a tabbed notebook with a tab for each ringbuffer.
The initial plots for each tab will be for the most recent
24 hours of data, however tabs will also have controls
that look like this:

+---------------------------------+
| Begins from     Ends with       |
| [ QDateEdit]   [QDateEdit]      |
| () since () between () before.  |
|        [ Refresh ]              |
+---------------------------------+
initalized with since checked, the leftmost QDateEdit enabled and
initialized with now-24 hour and th right QDateEdit disabled.

Checking "betwen" enables both QDateEdits and Checking "before" enables
the right most only.

The plo section of each tab contains six plot canvases,
Each canvas plots data vs time for 

+-----------------------------------------------------+
|    Data Rate      Events in run  Event Rate         |
|    Data Volume    Data in run    Events             |
+-----------------------------------------------------+


Future:
   Add the ability to follow new data.

'''
import sys
from PyQt5.QtWidgets import (QWidget, QApplication, QDateTimeEdit,
    QPushButton, QRadioButton, QLabel, QTabWidget,
    QMenuBar, 
    QVBoxLayout, QHBoxLayout, QGridLayout,
    QAction
)
from PyQt5.QtCore import (Qt, QDate, QTime, QDateTime, pyqtSignal)


import sqlite3
import matplotlib
matplotlib.use('Qt5Agg')


#  The control class for the bottom of each
#  Tabbed notebook:


class DateRange(QWidget):
    ''' 
      This widget provides a mechanism for
      choosing the range of dates for which plots are to be done.
      Properties:
        type  - One of 'since', 'between', or 'before'
                 indicating the type of date/time range
        start - QDateTime in the start date/time edit widget.
        end   - QDateTime in the end date/time widget.
      Signals:
         refresh - Refresh button clicked.
         
    '''
    def __init__(self, parent=None):
        refresh = pyqtSignal
        
        super(DateRange, self).__init__(parent)
        
        # Make the widgets:
        
        self._begin_label = QLabel("From", self)
        self._end_label   = QLabel("To", self)
        
        self._begin       = QDateTimeEdit(self)
        self._end         = QDateTimeEdit(self)
        
        self._since       = QRadioButton("Since", self)
        self._between     = QRadioButton("Between", self)
        self._before      = QRadioButton("Before", self)
        
        self._update      = QPushButton("Update", self)
        
        # Set the layout to a grid and layout the widgets:
        
        self._layout  = QGridLayout(self)
        self.setLayout(self._layout)
        
        self._layout.addWidget(self._begin_label, 0,  0)
        self._layout.addWidget(self._end_label, 0, 2)
        
        self._layout.addWidget(self._begin, 1, 0)
        self._layout.addWidget(self._end, 1, 2)
        
        self._layout.addWidget(self._since, 2, 0)
        self._layout.addWidget(self._between, 2, 1)
        self._layout.addWidget(self._before, 2, 2)
        
        self._layout.addWidget(self._update, 3, 1)
        
        
# Main entry:

if __name__ == "__main__":

    app = QApplication(sys.argv)
    window = DateRange()
    
    
    window.show()
    sys.exit(app.exec())
    
    
        
        
