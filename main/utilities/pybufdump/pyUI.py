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
This module provides graphical user interface elements for the
pybufdump utility a graphical buffer dumper.
'''
from PyQt6.QtWidgets import QMainWindow, QMenuBar, QStatusBar, QToolBar, QTextEdit, QWidget
from PyQt6.QtGui     import QFont

class DumpWidget(QTextEdit):
    '''
    This is a text widget that:
    -  Has a fixed font
    -  Is readonly.
    -  Provides a convenience function to set its contents that
       is a bit kinder to the eye than setPlainText.
    '''
    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)
        self.setReadOnly(True)
        
        fixed_font = QFont('Monospace')
        fixed_font.setStyleHint(QFont.StyleHint.Monospace)
        
        self.setFont(fixed_font)
        
    def setText(self, text : str) -> None:
        '''
            Erase the current text and set it to 
            the text parameters
            
            @param text : str - new contents of the
                               widget.
        '''
        self.setPlainText(text)
        
        
# Test code

if __name__ == '__main__':
    import sys
    from PyQt6.QtWidgets import QApplication
    
    app = QApplication(sys.argv)
    win = DumpWidget()
    win.show()
    win.setText('line1\nline2\nanother line')
    
    sys.exit(app.exec())
    
        