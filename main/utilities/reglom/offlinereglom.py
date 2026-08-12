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


import sys
from enum import Enum

from PyQt6.QtCore import QObject, Qt, pyqtSignal
from PyQt6.QtWidgets import (
    QApplication,
    QHBoxLayout,
    QFileDialog,
    QLabel,
    QLineEdit,
    QPushButton,
    QRadioButton,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)


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
    
class ReGlomControls(QWidget):
    '''
    The actual full reglom controls.
    
    Attributes:
      dt   - Glom assembly window.
      sourceid - Output Source ID.
      tspolicy - Timestamp policy.
      infile   - Input file template (segment e.g)
      outfile  - Output file.
      
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # The widget is a bunch of horizontal strips:
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        #  The Glom timee window and sid:
        
        glomlayout = QHBoxLayout()
        
        glomlayout.addWidget(QLabel('Glom interval (dt)', self))
        self._dt = QSpinBox(self)
        self._dt.setMinimum(1)
        self._dt.setMaximum(0x7fffffff)    # Maxint for i32
        glomlayout.addWidget(self._dt)
        
        glomlayout.addWidget(QLabel('Ouput SID', self))
        self._sid = QSpinBox(self)
        self._sid.setMinimum(0)
        self._sid.setMaximum(0x7fffffff)
        glomlayout.addWidget(self._sid)
        
        self._layout.addLayout(glomlayout)
        
        # Timestamp policy:
        
        self._tsPolicy = TsPolicySelector(self)
        self._layout.addWidget(self._tsPolicy)
        
        # Input file:
        
        infile = QHBoxLayout()
        infile.addWidget(QLabel('Input File:', self))
        self._infile = QLineEdit(self)
        infile.addWidget(self._infile)
        self._browseinfile = QPushButton('Browse...')
        self._browseinfile.clicked.connect(self._browseInputFile)
        infile.addWidget(self._browseinfile)
        
        self._layout.addLayout(infile)
        self._layout.addWidget(QLabel('For multisegment event file choose any segment', self))
    
        # Outfile:
        
        outfile = QHBoxLayout()
        outfile.addWidget(QLabel('Output File', self))
        self._outfile = QLineEdit(self)
        outfile.addWidget(self._outfile)
        self._browseoutfile = QPushButton('Browse...', self)
        self._browseoutfile.clicked.connect(self._browseOutputFile)
        outfile.addWidget(self._browseoutfile)
        
        self._layout.addLayout(outfile)
        self._layout.addWidget(QLabel('Output files are not segmented', self))
        
    #  Internal slots: 
    
    def _browseInputFile(self) -> None:
        file, _ = QFileDialog.getOpenFileName(
            self, 'Input File (segment)', '.', 'Event Files (*.evt);; All Files (*)', '*.evt'
        )
        if file.strip():
            self._infile.setText(file)
    
    def _browseOutputFile(self) -> None:
        file, _ = QFileDialog.getSaveFileName(
                    self, 'Reglommed File', '.', 'Event Files (*.evt);; All Files (*)', '*.evt'
                )
        if file.strip():
            self._outfile.setText(file)
    
 # tests for now:
 
if __name__ == "__main__":
    
    def changed(w :TsPolicySelector) -> None:
        print('Selected', w.policy().name)    
    
    app = QApplication(sys.argv)
    w   = ReGlomControls()
   
    w.show()
    sys.exit(app.exec())