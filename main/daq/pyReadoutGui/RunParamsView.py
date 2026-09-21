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
@file RunParamsView.py
@brief Provides an ediable view of the title and run number.
@author ROn FOx
'''

from PyQt6.QtWidgets import QLineEdit, QLabel, QHBoxLayout, QWidget, QStyle, QStyleOptionFrame
from PyQt6.QtGui     import QIntValidator
from PyQt6.QtCore    import QSize


class RunParamsView(QWidget):
    MAX_TITLE : int = 80              # Max chars in title.
    '''
        Editable view of the run title and run number.
        
        Attributes:
        title  - The title stringh.
        run    - The run number.
        
        @note - The title length is limited to MAX_TITLE
        @note - The run is validated to be an unsigned integer, 0 is legal.
    '''
    
    def __init__(self, parent : QWidget | None = None) :
        super().__init__(parent)
        
        #  The widgets are all laid out horizontally.
        
        self._layout = QHBoxLayout()
        self.setLayout(self._layout)
        
        self._layout.addWidget(QLabel('Title: ', self))
        self._title = QLineEdit(self)
        self._title.setMaxLength(self.MAX_TITLE)
        self._title.setText('Set A New Title')
        self._layout.addWidget(self._title)
        
        self._layout.addWidget(QLabel('Run: ', self))
        self._run   = QLineEdit(parent)
        self._run.setText('0')
        self._runValidator= QIntValidator(self)
        self._runValidator.setBottom(0)   # 32 bit default top limit is fine.
        self._run.setValidator(self._runValidator)
        self._layout.addWidget(self._run)
        
        self._setTitleWidgetWidth()
        
        
    # Utilities:
    
    def _setTitleWidgetWidth(self):
        # Set the self._title widget to 80 characters wide
        # (Thank you google gemini).
        
        fm = self._title.fontMetrics()
        width = fm.horizontalAdvance('x') * self.MAX_TITLE
        width += self._title.textMargins().left() + self._title.textMargins().right()
        width += self._title.contentsMargins().left() + self._title.contentsMargins().right()
        
        option = QStyleOptionFrame()
        self._title.initStyleOption(option)
        frame_width = self._title.style().sizeFromContents(
            QStyle.ContentsType.CT_LineEdit, option, QSize(width, 0)
        ).width()
        
        self._title.setFixedWidth(frame_width)
        
        
# test code
if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication
    import sys
    
    
    app = QApplication(sys.argv)
    win = RunParamsView()
    
    win.show()
    sys.exit(app.exec())
        
    
