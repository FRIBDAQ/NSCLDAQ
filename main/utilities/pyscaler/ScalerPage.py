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
  This file contains stuff needed to provide a page of scaler information.
  This includes
  
  ScalerPageView - A megawidget containing a title and table view.
  ScalerPageModel - A QStandardItemModel that containst he data the table view displays.
  
  
'''

from PyQt5.QtWidgets import QLabel, QTableView, QVBoxLayout, QHBoxLayout, QWidget
from PyQt5.QtGui     import QStandardItemModel, QStandardItem


##------------------------ The view class:   ----------------------------


class ScalerPageView(QWidget):
    '''
        Consists of a title label and string and a table view.  The
        widget has attributes:
        
        * title - the title string to display next to the Title: label.
        * model - The QStandardItemModel that drives the data in the table view.
        
        
    '''
    def __init__(self, *args):
        super().__init__(*args)
        
        # The title label and string are in a horizontal box layout:
        
        self._titleLayout = QHBoxLayout()
        self._titleLabel   = QLabel('Title: ', self)
        self._title        = QLabel(self)
        
        self._titleLayout.addWidget(self._titleLabel)
        self._titleLayout.addWidget(self._title)
        
        # The full widget uses a vbox layout:
        
        self._layout = QVBoxLayout()
        self._layout.addLayout(self._titleLayout)
        self.setLayout(self._layout)
        
        # At the bottom of the widgt is our table:
        
        self._table = QTableView(self)
        self._table.setShowGrid(True)
        self._layout.addWidget(self._table)
        
    #  Implement the attributes:
    
    def title(self):
        return self._title.text()
    def setTitle(self,title):
        self._title.setText(title)
    
    def model(self):
        return self._table.model()
    def setModel(self, model):
        self._table.setModel(model)
        
        
if __name__ == '__main__':
    from PyQt5.QtWidgets import QMainWindow, QApplication
    
    app = QApplication([])
    win = QMainWindow()
    widget = ScalerPageView(win)
    win.setCentralWidget(widget)
    widget.setTitle('Some random page title')
    win.show()
    app.exec()
        