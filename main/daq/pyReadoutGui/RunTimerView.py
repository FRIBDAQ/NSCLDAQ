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
@file RunTimerView.py
@brief Provides elapsed run time and timed run support.
@author Ron FOx
'''


from PyQt6.QtWidgets import QCheckBox, QLabel, QSpinBox, QHBoxLayout, QVBoxLayout, QWidget
from PyQt6.QtCore    import pyqtSignal, QTimer, Qt


class RunTimerView(QWidget):
    '''
    Provides a widget for viewing the elapsed run time and
    controls for doing a timed run.
    
    Slots:
        start   - Clears the run timer and starts  it
        stop    - Stops timing the run.
        resume  - Starts the run timer without clearing it.
    
    Signals:
        end     - Timer was running with timed run selected and the time of the run passed.
    Attributes:
        timed   - Run is timed (bool). Read/Write
        plannedDuration - Planned run duration (int seconds) Read/Write
        duration- Current run duration (int seconds) Readonly
        
    '''
    end = pyqtSignal()
    
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        
        #The top level layout is vbox with hbox strips:
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        # Top strip has two labels side-by-side: :
        
        elapsedLayout = QHBoxLayout()
        elapsedLayout.addWidget(QLabel('Elapsed Run Time: ', self))
        self._elapsedTime = QLabel('0 00:00:00', self)
        elapsedLayout.addWidget(self._elapsedTime)
        
        self._layout.addLayout(elapsedLayout)
        
        # The bottom strip is the controls for timed runs.
        
        trControls = QHBoxLayout()
        self._timedRun = QCheckBox('Timed Run', self)
        trControls.addWidget(self._timedRun)
        
        self._days = self._spinbox(0,365)    # If a year isn't enough, sue me.
        trControls.addWidget(self._days)
        trControls.addWidget(QLabel('-', self))
        self._hours = self._spinbox(0, 24)
        trControls.addWidget(self._hours)
        trControls.addWidget(QLabel(':', self))
        self._minutes = self._spinbox(0, 59)
        trControls.addWidget(self._minutes)
        trControls.addWidget(QLabel(':', self))
        self._seconds = self._spinbox(0, 59)
        trControls.addWidget(self._seconds)
        
        self._layout.addLayout(trControls)
        
        # We need this to keep track of elapsed time:
        
        self._ticker = QTimer(self)
        self._ticker.setInterval(1000)
        self._ticker.setSingleShot(False)

        self._ticker.timeout.connect(self._tick)        
        
        
        # Attribute storage:
        
        self._plannedDuration = 0    # units of seconds.
        self._duration        = 0    # Units of seconds.
        
    # Private slotes:
    def _tick(self) -> None:
        #  Handle timer ticks.
        pass
    # Utilities:
    
    def _spinbox(self, low, high) -> QSpinBox:
        result = QSpinBox(self)
        result.setMinimum(low)
        result.setMaximum(high)
        
        return result
        
        
# Test code:

if __name__ == '__main__':
    import sys
    from PyQt6.QtWidgets import QApplication
    
    app = QApplication(sys.argv)
    win = RunTimerView()
    
    win.show()
    sys.exit(app.exec())
    
    