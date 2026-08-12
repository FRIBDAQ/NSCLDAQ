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
@file offlinereglom.py
@brief Drive offline regloming of event built event files.
@author Ron Fox
@note  Replacement for offlinereglom.tcl - Issue #510
'''


from PyQt6.QtWidgets import QApplication, QLabel, QSpinBox, QRadioButton, QPushButton, QWidget, QHBoxLayout, QVBoxLayout
from PyQt6.QtCore    import pyqtSignal, QObject, Qt
import sys
from enum import Enum

class TimestampPolicy(Enum):
    earliest = 1
    latest   = 2
    average  = 3

class TsPolicySelector(QWidget):
    ''' 
    Select a timestamp policy:
        Attributes:
            policy - one of values of the TimestampPolicy enum.
        Signals:
            changed - the policy changed.
    '''
    changed = pyqtSignal()
    
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._layout = QHBoxLayout()
        self.setLayout(self._layout)
        
        self._layout.addWidget(QLabel('Timestamp Policy:', self))
        
        self._earliest = QRadioButton('Earliest', self)
        self._layout.addWidget(self._earliest)

        self._latest   = QRadioButton('Latest', self)
        self._latest.setChecked(True)   # Default to latest.
        self._layout.addWidget(self._latest)
        
        self._average = QRadioButton ('Average', self)
        self._layout.addWidget(self._average)
        
        
        #  Route the button clicked signals to the common changed signal:
        
        self._earliest.clicked.connect(self.changed)
        self._latest.clicked.connect(self.changed)
        self._average.clicked.connect(self.changed)
        
    #  Attribute implementation:

    def policy(self) -> TimestampPolicy:
        if self._earliest.isChecked():
            return TimestampPolicy.earliest
        elif self._latest.isChecked():
            return TimestampPolicy.latest
        elif self._average.isChecked():
            return TimestampPolicy.average
        else:
            raise RuntimeError('No valid timestamp policy is checked!!')
    def setPolicy(self, policy: TimestampPolicy) -> None:
        TimestampPolicy(policy)     # Throws an exception for bad value.
        match policy:
            case TimestampPolicy.earliest:
                widget = self._earliest
            case TimestampPolicy.latest:
                widget = self._latest
            case TimestampPolicy.average:
                widget = self._average
        
        widget.setChecked(True)
        
 # tests for now:
 
if __name__ == "__main__":
    
    def changed(w :TsPolicySelector) -> None:
        print('Selected', w.policy().name)    
    
    app = QApplication(sys.argv)
    w   = TsPolicySelector()
    w.setPolicy(TimestampPolicy.earliest)
    w.changed.connect(lambda : changed(w))
    w.show()
    sys.exit(app.exec())