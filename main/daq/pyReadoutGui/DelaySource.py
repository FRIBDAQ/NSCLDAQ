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
@file DelaySource.py
@brief Provide a delay data source
@author Ron Fox
'''

from nscldaq.readoutgui.DataSource import DataSource
from nscldaq.readoutgui.DataSourceRegistry import DataSourceRegistry
from PyQt6.QtCore import QObject, QTimer
from PyQt6.QtWidgets import (
    QDialog,
    QHBoxLayout,
    QLabel,
    QProgressBar,
    QVBoxLayout,
    QWidget,
)


class _DelayProgress(QDialog):
    #  This local class is a widget that shows the progress of a delay...it's just
    # a progress bar in a dialog.
    
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        self._bar = QProgressBar(self)
        self._layout.addWidget(self._bar)
        self.setWindowTitle('Delay in progress')
    
    def bar(self) -> QProgressBar:
        return self._bar
        

class DelaySource(DataSource):
    '''
    This class provides a pseudo data source that just delays
    a specific amount of time.  If the delay is long enough,
    a progressbar pop-up modal window is displayed during the delay
    so the user knows the delay is in progress.
    '''
    
    def parameters() -> dict[str,type]:
        '''
            We have the following parameters:
            delay - ms to delay on begin run (int)
            enddelay - ms to delay on end run (int).
        '''
        return {
            'delay' : int,
            'enddelay' : int
        }
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
    
    # Implement the abstract methods of  the DataSource type:
    
    def start(self) -> None:
        pass                       # nothing to start.
    def check(self) -> bool:
        return True                # always running.,
    def stop(self) -> None:
        pass                       # Nothing to stop.
    def begin(self, _run :int, _title : str) -> None:
        '''
        If the configuration includes a 'delay' parameter, we
        do that delay:
        
        
        '''
    
        ms = self.getConfig().get('delay', 0)
        self._doDelay(ms)
        
    def end(self) -> None:
        '''
            If the configuration includes a 'enddelay' parameter, delay that long.
        '''
    
        ms = self.getConfig().get('enddelay', 0)
        self._doDelay(ms)
        
        
    def _doDelay(self, ms : int) -> None:
        if ms == 0:
            return              # No delay.
        #  Utility to do the delay with progress bar for ms milliseconds.
        
        #  This time times the whole delay:
        
        timer = QTimer(self)
        timer.setInterval(ms) 
        timer.setSingleShot(True)
        timer.timeout.connect(self._done)
        timer.start()
        # This timer ticks the progress bar forwards
        # If the delay is long enough to warrant it:
        
        if ms >= 1000:  
            self._delayTime = 0  
            self._totalDelay = ms
            self._ticker = QTimer(self)
            self._ticker.setSingleShot(True)
            self._ticker.setInterval(1000)   # Update every second.
            self._ticker.timeout.connect(self._tick)
            self._ticker.start()    
            
            
        # Setup the dialog
        # it shows the seconds in the delay/
        self._dialog = _DelayProgress(self.parent())
        bar = self._dialog.bar()
        bar.setMinimum(0)
        bar.setMaximum(int(ms/1000))    
        bar.setValue(0)

        self._dialog.exec()
        
    # Local slots:
    
    def _done(self):
        # Delay completed:
        
        self._dialog.accept()    # Finishes the dialog
        
    def _tick(self):
        # A second of delay has elapsed.
        
        self._delayTime += 1
        self._dialog.bar().setValue(self._delayTime)
        
        #  If theres's at least another second, schedule another tick.
        
        if self._delayTime +1 < self._totalDelay:
            self._ticker.start()

class ConfigurationDisplay(QWidget):
    '''
    Display the configuration of a Delay data source:

    '''
    def __init__(self, source : DelaySource, parent : QWidget |None = None):
        super().__init__(parent)
        
        self.setLayout(QVBoxLayout())
        
        beginlayout = QHBoxLayout()
        beginlayout.addWidget(QLabel('Begin run delay:', self))
        beginlayout.addWidget(QLabel(f'{source.getConfig().get("delay", 0)} ms', self))
        
        self.layout().addLayout(beginlayout)
        
        endlayout = QHBoxLayout()
        endlayout.addWidget(QLabel('End Run Delay: ', self))
        endlayout.addWidget(QLabel(f'{source.getConfig().get("enddelay", 0)} ms'))

        self.layout().addLayout(endlayout)

 # Test code:
 
if __name__ == '__main__':
     import sys
     from PyQt6.QtWidgets import QApplication, QMainWindow
     
     
     def endRun() -> None:
        # Infoke the end method of the delay.
        print("end Delay starts")
        delay.end()
        print('end delay ends')
     
     def beginRun() -> None:
        # Invoke the delay's begin run then schedule end a second after that returns.
        print('delay starts')
        delay.begin(0,'junk')
        print('delay done')
        
        timer = QTimer(delay)
        timer.setInterval(5000)
        timer.setSingleShot(True)
        timer.timeout.connect(endRun) 
        timer.start()
     
     app = QApplication(sys.argv)
     # I think we need a window....
     
     
     # The delay at begin will have a progress bar, the
     # one at end will be too short to tick.
     
     delay = DelaySource(parameters={'delay': 5000, 'enddelay': 100}, parent=None)
     config = ConfigurationDisplay(delay)
     config.show()
     
     # Schedule the start in 1 second...it will schedule the end
     
     start = QTimer(config)
     start.setInterval(1000)
     start.setSingleShot(True)
     start.timeout.connect(beginRun)
     start.start()
     
     sys.exit(app.exec())