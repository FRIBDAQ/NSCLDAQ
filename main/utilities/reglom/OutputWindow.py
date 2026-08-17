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
@file Outputwindow.py
@brief Provide an output window (readonly text widget).
@author Ron FOx
'''


from PyQt6.QtWidgets import QTextEdit
from PyQt6.QtCore import pyqtSignal, QObject

class OutputWindow(QTextEdit):
    '''
        This is just a readonlyh text edit. With limited
        lines of text.  The intent is to provide a widget in which
        the output/errors of programs can be captured.
        The number of blocks (paragraphs) can be limited.
        
        Methods:
            append  - This is overriden fromt the QTextEdit to support the 
                      newText signal.
            setLimit   - Set paragraph limits on the display.
            
        Signals:
            newText(str) - New text was added (passed to the slot).
        
    '''
    newText = pyqtSignal(str)
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self.setReadOnly(True)       # Can't edit.
    
    def append(self, text: str) -> None:
        ''' Appends the text and fires the newText signal
            @Param text : str - string to append to the output.
        '''
        super().append(text)             # Actually add the text to the widget.
        self.newText.emit(text)
    
    def setLimit(self, paras : int) -> None:
        '''
            sets the limit on the numb er of paragraphs that can be added to the widget.
            @param paras : int - New limit. 
        '''
        self.document().setMaximumBlockCount(paras)
        
        
