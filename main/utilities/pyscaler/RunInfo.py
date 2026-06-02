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


''''
  This file contains a RunInfo widget.  It displays above the
  tabbed notebook and shows several bits of information:
  
  Run State:  Inactive or Active. Title:       The run title      
  Run Number: n.                  Time in run: d hh:mm:sec
  
  @file RunInfo.py
  @brief Presentation for run information.
  @author Ron Fox
'''

from PyQt5.QtWidgets import QLabel, QGridLayout, QWidget


class RunInfo(QWidget):
    '''
      Provides a nice display of the information about the current run.
      
      Properties:
      runNumber - the current run number.
      runTitle  - the current run title.
      runState  - Active or Inactive.
      time   - Seconds into the run.
      
      See the file header for the shape of this widget.
    '''
    
    def __init__(self, *args):
        super().__init__(*args)
        
        self._layout = QGridLayout(self)
        
        self._stateLabel = QLabel('Run State: ', self)
        self._state      = QLabel('Inactive', self)    
        
        self._runnumLabel = QLabel('Run Number: ',self)
        self._runnum      = QLabel('0', self)
        
        self._titleLabel = QLabel('Title:', self)
        self._title      = QLabel('                                          ', self) 
        
        self._timeLabel  = QLabel('Run Time:', self)
        self._time       = QLabel('  0 00:00:00', self)
        self._seconds    = 0                             # So I don't have to back convert.
        
        # Layout the widgets:
        
        self._layout.addWidget(self._stateLabel, 0,0)
        self._layout.addWidget(self._state, 0, 1)
        self._layout.addWidget(self._titleLabel, 0,2)
        self._layout.addWidget(self._title, 0, 4)
        
        self._layout.addWidget(self._runnumLabel, 1, 0)
        self._layout.addWidget(self._runnum, 1,2)
        self._layout.addWidget(self._timeLabel, 1,3)
        self._layout.addWidget(self._time, 1, 4)
        
        self.setLayout(self._layout)
        
    
    #  Property implementations:
    
    def runNumber(self):
        return int(self._runnum.text())
    def setRunNumber(self, newValue):
        self._runnum.setText(str(newValue))
        
    def runTitle(self):
        return self._title.text()
    def setRunTitle(self, title):
        self._title.setText(title)
        
    def runState(self):
        return self._state.text()
    def setRunState(self, state):
        '''On your honor to use a valid "state" variable'''
        
        self._state.setText(state)
        
    def time(self):
        ''' Return as seconds.  '''
        return self._seconds
    def setTime(self, seconds):
        
        self._seconds = seconds
        # Now compute the text field.
        
        secs = seconds % 60
        minutes = int(seconds/60)
        mins = minutes % 60
        hours = int(minutes/60)
        hrs   = hours %24
        days  = int(hours/24)
        
        time_string = f'{days} {hrs:02d}:{mins:02d}:{secs:02.2f}'
        self._time.setText(time_string)
        
    
# Test code:

if __name__ == '__main__':
    
    def tick() :
        widget.setTime(widget.time() + 1)
    
    from PyQt5.QtWidgets import QMainWindow, QApplication
    from PyQt5.QtCore   import QTimer
    
    app = QApplication([])
    main = QMainWindow()
    widget = RunInfo(main)
    widget.setRunState('Active')
    print('State: ', widget.runState())
    widget.setRunNumber(1234)
    widget.setTime(24*3600)   # test days.
    print('NUmber: ', widget.runNumber())
    widget.setRunTitle('this is an arbitrary title')
    print('title: ', widget.runTitle())
    main.setCentralWidget(widget)
    
    timer = QTimer(widget)
    timer.setInterval(1000)
    timer.setSingleShot(False)
    timer.timeout.connect(tick)
    timer.start()
    
    #  Lets's set some stuff and
    #  Increment the seconds every few secs.
    
    main.show()
    app.exec()
