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


from PyQt6.QtCore import Qt, QTimer, pyqtSignal
from PyQt6.QtWidgets import (
    QCheckBox,
    QHBoxLayout,
    QLabel,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)


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
        
        self._duration        = 0    # Units of seconds.
    
    # Implement attributes:
    
    def timed(self) -> bool:
        ''' @return bool - True if the timed run checkbox is set. '''
        
        return self._timedRun.checkState() == Qt.CheckState.Checked
    
    def setTimed(self, timed : bool ) -> None:
        '''
            @param timed : bool - True turns on the timed run checkbox, false, turns it off:
        '''
        checkstate = Qt.CheckState.Checked  if timed else Qt.CheckState.Unchecked
        self._timedRun.setCheckState(checkstate)
    
    def plannedDuration(self) -> int:
        ''' @return int - seconds in the planned run duration.
            @note you still need to query the timed attribute to know 
                  if this is a timed run.
        '''
        days    = self._days.value()
        hours   = self._hours.value()
        minutes = self._minutes.value()
        secs    = self._seconds.value()
        
        return secs + minutes*60 + hours * 3600 + days * 3600*24
    
    def setPlannedDuration(self, seconds : int) -> None:
        '''@param seconds - seconds to set the planned run time widgets.
           @note you must still use setTimed to enable timed runs.
        '''
        
        (days, hours, min, secs) = self._secsToTimes(seconds)
        self._days.setValue(days)
        self._hours.setValue(hours)
        self._minutes.setValue(min)
        self._seconds.setValue(secs)
        
        
    def duration(self) -> int:
        ''' @return int - run duration in seconds. '''
        return self._duration
    
    # Public slots:
    
    def start(self) -> None:
        '''
          Zero the duration, update th view of the elapsed time
          and start the ticker which will update the duration and
          elapsed time label.
        '''
        self._duration = 0
        self._updateElapsedLabel()
        self._ticker.start()
        
    def stop(self) -> None:    
        '''
             Top updating the duration.. note the duration value
             is unchanged
        '''
        
        self._ticker.stop()
    
    def result(self) -> None:
        '''
            Resume updating the duration without zeroing it.
        '''
        self._ticker.start()
        
    
    # Private slotes:
    def _tick(self) -> None:
        #  Handle timer ticks.
        
        # Update the elapsed time display 
        
        self._duration += 1
        self._updateElapsedLabel()
        
        # If the timed run has run past the duration, emit the end signal.
        
        if self._duration >= self.plannedDuration() and self.timed():
            self.end.emit()
        
    # Utilities:
    
    def _spinbox(self, low, high) -> QSpinBox:
        result = QSpinBox(self)
        result.setMinimum(low)
        result.setMaximum(high)
        
        return result

    def _updateElapsedLabel(self) -> None:
        # Update self._elapsedTime from self._duration
        
        (days, hours, min, secs) = self._secsToTimes(self._duration)
        labelStr = f'{days} - {hours:02d}:{min:02d}:{secs:02d}'
        self._elapsedTime.setText(labelStr)
            
    
    def  _secsToTimes(self, seconds : int) -> tuple[int, int, int, int]:
        #  Turns seconds into (days, hours, minutes, seconds)
           
        secs      = seconds % 60
        remainder = int(seconds/60)
        min       = remainder  % 60
        remainder = int(remainder/60)
        hours     = remainder % 60
        days      = int(remainder/60) % 24
        
        return (days, hours, min, secs)
        
# Test code:

if __name__ == '__main__':
    import sys

    from PyQt6.QtWidgets import QApplication
    
    def done() -> None:
        print("timed run would end here.")
        win.stop()
    
    app = QApplication(sys.argv)
    win = RunTimerView()
    win.setTimed(True)       # time run.
    win.setPlannedDuration(60)
    win.end.connect(done)
    win.start()
    
    
    win.show()
    sys.exit(app.exec())
    
    