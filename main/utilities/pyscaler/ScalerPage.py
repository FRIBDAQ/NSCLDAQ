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

##------------------------ The model class: ----------------------------

class ScalerPageModel(QStandardItemModel):
    '''
        This model should be attached to the SclaerPageView and 
        its contents appropriately updated with time.
        
        The model provides a bit more scaler like interface.
        lines in the model have the following columns:
        
        scaler1  counts1 rate1 scaler 2 counts1 rate1  counts-ratio rate-ratio
        
        The model supports three kinds of rows: 
        
        * single - only scaler1, counts1 rate1 are populated.
        * pair   - saler1, counts1, rate1 scaler2 counts2 rate2 are populated
        * ratio  - All items are populated.
        
        Local data maintains the type of each row, which can be queried
        along with the scaler names, and the count values and so on.
        
        A row can be updated by passing a dict containing the keys:
        
        type - the type of row - checked against the model row.
        scalers - array of scaler names
        counts  - array of counts
        rates   - array of rates.
        
        Note that the size of the counter arrays are not range checked.
        It's up tot the caller to ensure they are correct for the type.
        
        A vertical header is created with column labels.
    '''
    
    def __init__(self, *args):
        super().__init__(*args)
        
        # All we do here is set the horizontal headers:
        
        self.setHorizontalHeaderLabels([
            'Scaler', 'counts', 'rate',
            'Scaler', 'counts', 'rate',
            'ratio(counts)', 'ratio(rates)'
        ])
        
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
    
    model = ScalerPageModel()
    widget.setModel(model)
    
    win.show()
    app.exec()
        