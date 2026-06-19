'''
  The RunInfo module provides a GUI and associated model for the information about a run.
  The following public classes are exported:
  
  *  RunInfo - A Qt6 widget that provides displays and edits for the title and run number.
  *  RunInfoModle - A model class which provides the data for this presentation widget.
  
  @file RunInfo.py
  @brief Provide display and controls for run information.
  @author Ron Fox.
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


from PyQt6.QtWidgets import (QHBoxLayout, QVBoxLayout, QWidget, 
        QLineEdit, QLabel, QSpinBox)
from PyQt6.QtCore import QObject, pyqtSignal
from collections import namedtuple    # So we can get enforced constants:

# Define module constants:

Constants = namedtuple('Constants', ['MAX_TITLE'],)
CONSTANTS = Constants(MAX_TITLE=80,)

class RunInfoModel(QObject):
    '''
        This class provides a model for the run information data.
        It is intended to interact with something that presents
        both the data and the controls for modifying the data.
        The following signals are provided, intended for
        use by the presnentation object(s):
        
        actualTitleChanged(str) - the actual title changed.
        actualRunChanged(str)   - The actual runnumber changed
        
        
        The following attributes are provided (these are the
        data maintained by this model):
        
        requestedTitle          - The requested title string.
        requestedRun            - The requested run number.
        actualTitle             - The actual run title.
        actualRun               - The actual run number.
        
        Normal use would have signal in the GUI presentation 
        for actual changes which the controller object would
        turn into requests of the manager and, on response,
        would update the actual values in the model which would
        signal the GUI to update as well.  
    '''
    
    # Define the signals:
    
    actualTitleChanged = pyqtSignal(str)
    actualRunChanged   = pyqtSignal(int)
    
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # For now initialize the data to some rather idiotic 
        # defaults.  The controller object logic would use the
        # attributes to  initialize them for real later:
        
        self._requestedTitle : str = ''
        self._requestedRun   : int = 0
        self._actualTitle    : str = ''
        self._actualRun      : int = 0
        
    
    # Implement the attributes:
    
    def requestedTitle(self) -> str:
        ''' @return str the current requested title'''   
        return self._requestedTitle
    def setRequestedTitle(self, title: str) -> None:
        ''' Set a new requested title 
            @param title :str - the new requested title
        '''
        self._requestedTitle = title
    
    def requestedRun(self) -> int:
        ''' @return int - the requested run number'''
        return self._requestedRun
    def setRequestedRun(self, run : int) -> None:
        ''' Set a new requested run number.
            @param run - the new run number.
        '''
        self._requestedRun = run

    def actualTitle(self) -> str:
        '''@return str - Our knowledge of the actual title.'''
        return self._actualTitle
    def setActualTitle(self, title) -> None:
        '''  Set a new actual title.
            @param title - the new actual title.
            @note the actualTitleChanged signal is emitted.
        '''
        self._actualTitle = title
        self.actualTitleChanged.emit(self._actualTitle)
        
    def actualRun(self) -> int:
        '''@return int - Our knowledge of the actual run numbers.'''
        return self._actualRun
    def setActualRun(self, run : int) -> None:
        ''' Set a new actual run number.
            @param run - the new run number to set.
            @note the actualRunChanged signal is emitted.
        '''
        self._actualRun = run
        self.actualRunChanged.emit(self._actualRun)
        
        
class RunInfo(QWidget):
    '''
        Provides the visual display and controls for the run information.
        
        Signals:
            requestedTitleChanged(str) - the requested title has changed.
            requestedRunChanged(int)   - the requested run number has changed.
            
        Attrributes:
            model (readonly)  retrieve the model.
            
        
    '''
    requestedTitleChanged = pyqtSignal(str)
    requestedRunChanged   = pyqtSignal(int)
    
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        
        # Define he widgets:
        
        self._titleLabel = QLabel('Current Title: ', self)
        self._actualTitle= QLabel(self)
        
        self._reqTitleLabel = QLabel('Next Title: ', self)
        self._reqTitle      = QLineEdit(self)
        self._reqTitle.setMaxLength(CONSTANTS.MAX_TITLE)       # Longest title string.
        self._setTitleEditWidth()
        
        self._runLabel      = QLabel('Current Run', self)
        self._actualRun     = QLabel('0', self)
        
        self._reqRunLabel   = QLabel('Next run: ', self)
        self._reqRun        = QSpinBox()
        self._reqRun.setMinimum(0)                    # non-negative values only.
        
        # Now lay them out:
        
        self._toplayout = QVBoxLayout(self)                 # the rows are stacked.
        self.setLayout(self._toplayout)
        
        # Title rows:
        
        self._line1 = QHBoxLayout(self)
        self._line1.addWidget(self._titleLabel)
        self._line1.addWidget(self._actualTitle)
        self._toplayout.addLayout(self._line1)
        
        self._line2 = QHBoxLayout(self)
        self._line2.addWidget(self._reqTitleLabel)
        self._line2.addWidget(self._reqTitle)
        self._toplayout.addLayout(self._line2)
        
        # Run number all one row:
        
        self._line3 = QHBoxLayout(self)
        self._line3.addWidget(self._runLabel)
        self._line3.addWidget(self._actualRun)
        self._line3.addWidget(self._reqRunLabel)
        self._line3.addWidget(self._reqRun)
        self._toplayout.addLayout(self._line3)
        
        # Add our model
        
        self._model = RunInfoModel(self)
        
        # Hook our signals and signal relays:
        
    ## Utiltity (internal) methods:
    
    def _setTitleEditWidth(self):
        # Set the width of the title line edit to 
        # hold the max chars (thank you google for this)
        # rather complex method.
        
        metrics = self._reqTitle.fontMetrics()
        text_width = CONSTANTS.MAX_TITLE*metrics.horizontalAdvance('x')  # text pixels.
        margins    = self._reqTitle.textMargins()
        content_margins = self._reqTitle.contentsMargins()
        
        # The total width from the text_width and margins is:
        
        total_width = (text_width 
            + margins.left() + margins.right() 
            + content_margins.left() + content_margins.right())
        
        self._reqTitle.setFixedWidth(total_width)

# Test code:

if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication, QMainWindow
    import sys
    
    app = QApplication(sys.argv)
    win = QMainWindow()
    widget = RunInfo(win)
    win.setCentralWidget(widget)
    
    win.show()
    
    sys.exit(app.exec())