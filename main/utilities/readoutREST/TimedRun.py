'''
    One important capability is to perform a timed run.  A timed run
    is a run that has a specified duration.  The view provides
    both the duraction and a checkbutton that allows timed run
    to be enabled/disabled.   The model maintains the
    enable state and the elapsed run time.  It also emits a 
    signal when the elapsed run time extends beyond the
    desired elapsed run time..if a timed run is enabled.
    
    An external controller is presumed to hook into the
    DAQ to know when state changes to active have
    occured and to maintained the elapsed run time.
    
    The external controller, presumable hooks onto the 
    model's time to end run signal and attempts to end
    the run when that signal is received.
    
@file TimedRun.py
@brief Model and view to support timed runs.
@author Ron Fox
'''


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


from PyQt6.QtCore import QObject, pyqtSignal, Qt
from PyQt6.QtWidgets import (QWidget, QSpinBox, QCheckBox, QLabel, 
                            QHBoxLayout, QVBoxLayout, QGridLayout)
from parse import parse

# Utitlity used by both classes.
def _makeTime(d:int, h: int, m: int, sec: int) -> dict :
        # Make a time dict from broken down time..
        return {
            'days' : d,
            'hours' : h,
            'minutes' : m,
            'seconds' : sec
        }
class TimedRun(QObject):
    '''
        This is a timed run model. It maintains
        the desired run time and provides
        the ability to set the current elapsed run time.
        If the current elapsed run time is set to be
        at least the desired run time, runExpired is
        emitted.
        
        
        Times are internally storead in dicts with keys like
        'days', 'hours', 'minutes', 'seconds'
        
        Fractional seconds are not used.
        
    Attributes:
        desiredLength - desired run length.
        elapsedTime   - elapsed run time.
        limitRun      - True if the run shoulid be limited.
    Signals:
        runExpired   - the run is limited and the elapsed time is at least the desired length.
        newDesired   - The desired run length changed.
        elapsedChanged- The elapsed time changed.
        limitChanged - The limit flag changed.
    '''
    runExpired   = pyqtSignal()
    newDesired   = pyqtSignal(dict)
    elapsedChanged= pyqtSignal(dict)
    limitChanged = pyqtSignal(bool)
    
    def __init__(self, parent=None):
        super().__init__(parent)
        
        self._desired = _makeTime(0,0,0,0)
        self._elapsed = _makeTime(0,0,0,0)
        self._limitRun= False

    # Implement attributes:

    def desiredLength(self) -> dict :
        ''' @return dict - containing the time as described in the class docstring. '''
        return self._desired

    def setDesiredLength(self, timestring: str) -> None:
        '''
            @param timestring -  A string of the form 'd hh:mm:ss'.
            @throw ValueError - if timestring is not valid.
            
            on success, the newDesired signal is emitted.
        '''
        self._desired = self._strToTime(timestring)
        self.newDesired.emit(self._desired)
            
    def elapsedTime(self) -> dict:
        ''' @return dict - the elapsed time dict.'''
        
        return self._elapsed
    
    def setElapsedTime(self, timestring: str) -> None:
        '''
            @param timestring - a string of the form 'd hh:mm:ss'
            @throw ValueError if the timestring is invalid.
            
            If the elapsed time is >= the desired tie and the limit flag is set,
            the runExpired signal is emitted.
        '''
        
        self._elapsed = self._strToTime(timestring)
        self.elapsedChanged.emit(self._elapsed)
        if self._limitRun and self._expired():
            self.runExpired.emit()

    def limitRun(self) -> bool:
        ''' @return bool - state of the limit run length flag.'''
        return self._limitRun
    
    def setLimitRun(self, limit : bool) -> None:
        ''' @param limit - new state of limit run flag
            
            Note that the limitChanged signal is emitted.
        '''
        self._limitRun = limit
        self.limitChanged.emit(self._limitRun)

    # Utility methods:
    
    
    
    def _strToTime(self, timestring : str) -> dict:
        # Make atime dict from a time string.
        result = parse('{days:d} {hours:d}:{minutes:d}:{seconds:d}', timestring)
        if result is not None:
            return result.named
        else:
            raise ValueError(f'{timestring} is not a valid time string')
        
    def _expired(self) -> bool:
        # True if the desired run time is <= the elapsed time.
        
        if self._elapsed['days'] > self._desired['days']:
            return True
        elif self._elapsed['days'] == self._desired['days']:
            if self._elapsed['hours'] > self._desired['hours']:
                return True
            elif self._elapsed['hours'] == self._desired['hours']:
                if self._elapsed['minutes'] > self._desired['minutes']:
                    return True
                elif self._elapsed['minutes'] == self._desired['minutes']:
                    return self._elapsed['seconds'] >= self._desired['seconds']
        return False
                    

class TimedRunView(QWidget):
    '''
      The view that provides visual controls and  displays of
      needed for limited runs.  We provide:
      - The elapsed run time
      - A set of controls for setting  desired run length.
      - Checkbutton for turning on and off run limits.
      
      It is assumed an external Controller will manage us and interact
      with the DAQ system to  update elapsed run time and stop
      the run when the model signals we should.
    
    Signals:
        desiredChanged(dict) When the user changes the desired time.  The dict in the signal has the 
                 format described for time dicts in the TimedRun (the model).  class.
        timedChanged(bool)  When the checkbox with the timed run selection on/off changes
    '''
    desiredChanged =  pyqtSignal(dict)
    timedChanged   =  pyqtSignal(bool)
    
    
    def __init__(self, parent=None):
        super().__init__(parent)
        
        # the overall layout is a set of strips.
        
        self._layout = QVBoxLayout(self)
        self.setLayout(self._layout)
        
        # Set up the elapsed time labels:
        
        self._elapsedLabel = QLabel('Elapsed Time:', self)
        self._elapsedTime  = QLabel('0 00:00:00', self)
        self._elapsedLayout = QHBoxLayout(self)
        self._elapsedLayout.addWidget(self._elapsedLabel)
        self._elapsedLayout.addWidget(self._elapsedTime)
        self._layout.addLayout(self._elapsedLayout)
        
        #  enable timed run.
        
        self._isTimed = QCheckBox('Timed Run', self)
        self._layout.addWidget(self._isTimed)
        
        # Now the labels and spin boxes for the  run length.
        
        self._lengthLayout = QGridLayout(self)
        self._daylabel     = QLabel('DD', self)
        self._hrslabel     = QLabel('HH', self)
        self._minlabel     = QLabel('MM', self)
        self._seclabel     = QLabel('SS', self)
        
        self._lengthLayout.addWidget(self._daylabel, 0,0)
        self._lengthLayout.addWidget(self._hrslabel, 0,1)
        self._lengthLayout.addWidget(self._minlabel, 0,3)    # Space for ':'.
        self._lengthLayout.addWidget(self._seclabel, 0,5)
        
        self._days = self._makeSpinBox(0, 365)                           # A year run is pretty long :-P
        self._lengthLayout.addWidget(self._days, 1, 0)
        
        self._hours = self._makeSpinBox(0, 59)
        self._lengthLayout.addWidget(self._hours, 1,1)
        self._HrColon = QLabel(':', self)
        self._lengthLayout.addWidget(self._HrColon, 1,2)
        
        self._mins = self._makeSpinBox(0,59)
        self._lengthLayout.addWidget(self._mins, 1,3)
        self._MinColon = QLabel(':', parent)
        self._lengthLayout.addWidget(self._MinColon, 1,4)
        
        self._secs = self._makeSpinBox(0,59)
        self._lengthLayout.addWidget(self._secs)
        
        self._layout.addLayout(self._lengthLayout)
        
        #  Add our model and set the GUI in accordance with its
        #  initial state:
        
        self._model = TimedRun(self)
        self._loadModel()
        
        # Connect to the signals we need to update the view from the
        # model:
        
        self._model.newDesired.connect(self._desiredChanged)
        self._model.limitChanged.connect(self._newLimited)
        self._model.elapsedChanged.connect(self._updateElapsed)
        
        #  Now handle the UI signals that we're going to cook and forward
        #  to controllers:
        
        self._isTimed.clicked.connect(self._forwardEnable)

        self._days.valueChanged.connect(self._forwardNewLimit)
        self._hours.valueChanged.connect(self._forwardNewLimit)
        self._mins.valueChanged.connect(self._forwardNewLimit)
        self._secs.valueChanged.connect(self._forwardNewLimit)
        
    def model(self) -> QObject:
        return self._model  
    
    # Note: Override setEnabled so that the elapsed stuff is always
    # Highly visible:
    
    def setEnabled(self, state: bool) -> None:
       
        self._elapsedLabel.setEnabled(True)
        self._elapsedTime.setEnabled(True)
        
        for widget in [self._isTimed, self._days, self._hours, self._mins, self._secs]:
            widget.setEnabled(state)
    
        # Set the widgets that do get modified:
    # Private slots:
    
    def _forwardEnable(self) -> None:
        # Marshall the isTimed checkbox value to a bool and emit
        # timedChanged
        
        state = True if self._isTimed.checkState() == Qt.CheckState.Checked else False
        self.timedChanged.emit(state)
    
    def _forwardNewLimit(self, newValue : int) -> None:
        # The value isn't that useful...as we need to construct
        # the full timedict from all the spinboxes:
        
        
        self.desiredChanged.emit({
            'days'    : self._days.value(),
            'hours'   : self._hours.value(),
            'minutes' : self._mins.value(),
            'seconds'    : self._secs.value()
        })
        
    
    def _desiredChanged(self, limit: dict) -> None:
        self._days.setValue(limit['days'])
        self._hours.setValue(limit['hours'])
        self._mins.setValue(limit['minutes'])
        self._secs.setValue(limit['seconds'])
        
    def _newLimited(self, limit: bool) -> None:
                
        checked = Qt.CheckState.Checked if limit else Qt.CheckState.Unchecked
        self._isTimed.setCheckState(checked)
    
    def _updateElapsed(self, elapsed : dict) -> None:   
        timestr = f'{elapsed["days"]:02d} {elapsed["hours"]:02d}:{elapsed["minutes"]:02d}:{elapsed["seconds"]:02d}'
        self._elapsedTime.setText(timestr)
        
    # Private utilities.
    
    def _makeSpinBox(self, low : int, high: int) -> QSpinBox:
        # Convenience method to make a spinbox we parent with specified limits.
        widget = QSpinBox(self)
        widget.setMinimum(low)
        widget.setMaximum(high)
        return widget
    
    def _loadModel(self) -> None:
        # Load the UI from the model:
        
        # Elapsed time:
        
        model = self.model()
        elapsed = model.elapsedTime()
        self._updateElapsed(elapsed)
        
        
        # Requested time limit:
        
        limit = model.desiredLength()
        self._desiredChanged(limit)
        
        # The enable checkbox:
        
        checked = model.limitRun()
        self._newLimited(checked)

# Test code:

if __name__ == '__main__':
    
    from PyQt6.QtWidgets import QApplication, QMainWindow
    from PyQt6.QtCore import QTimer
    import sys
    
    def endRun() -> None:
        global elapsed_secs
        elapsed_secs = 0
        # Simulate the end of run by just
        # Setting the elapsed back to zero..
        
        widget.model().setElapsedTime('0 00:00:00')
        
    def setTimed(isTimed : bool) -> None:
        # Echo back to the controller:
        #
        widget.model().setLimitRun(isTimed)
        
    def setNewDesired(desired : dict) -> None:
        # Just turn that into a timestring and  and 
        # Set it into the model:
        
        timestr =  \
            f'{desired["days"]:02d} {desired["hours"]:02d}:{desired["minutes"]:02d}:{desired["seconds"]:02d}'
        widget.model().setDesiredLength(timestr)
    
    def tick():
        global elapsed_secs
        elapsed_secs += 1
        secs = elapsed_secs % 60
        resid = int(elapsed_secs/60)
        mins = resid % 60
        resid = int(resid/60)
        hrs  = resid % 24
        days = int(resid/24)
        
        timestr = \
            f'{days:02d} {hrs:02d}:{mins:02d}:{secs:02d}'
        widget.model().setElapsedTime(timestr)
            
    
    app = QApplication(sys.argv)
    win = QMainWindow()
    
    
    widget = TimedRunView(win)
    win.setCentralWidget(widget)
    
    # Now a fake controller:
    
    widget.model().runExpired.connect(endRun)
    widget.timedChanged.connect(setTimed)
    widget.desiredChanged.connect(setNewDesired)
    
    # A timer to update the elapsed time (run is always active here.).
    
    elapsed_secs = 0
    timer = QTimer()
    timer.setSingleShot(False)
    timer.setInterval(1000)     # 1 second in ms.
    timer.timeout.connect(tick)
    timer.start()
    
    win.show()
    sys.exit(app.exec())