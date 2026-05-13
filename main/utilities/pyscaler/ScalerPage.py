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
  
  @file ScalerPage.py
  @brief Presentation and model for a single scaler display page.
  @author Ron Fox.
'''

from PyQt5.QtWidgets import QLabel, QTableView, QVBoxLayout, QHBoxLayout, QWidget
from PyQt5.QtGui     import QStandardItemModel, QStandardItem

class ScalerPageException(Exception):
    def __init__(self, reason):
        super().__init__(reason)
    

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
        self._lines = [];     # The line definitions.
    
    # Public methods:
    
    def addLine(self, line_info):
        '''
            Add a new line to the model.
            The line_info is a dict that is just like what comes out of the 
            line dicts from the processed config file keys are:
            
            type -  must be one of 'empty', 'single', 'pair' or 'ratio' see the class comments
            scalers - an array of one or two scaler names, depending on the type.
            
            For all of the appropriate counts/rate/ratio columns we create
            an item with the contents '0'.
            
            The line is appended to the set of existing lines.
            
            @param line_info - the dict described above
            @exception ScalerPageException:
                - If the type is invalid.
        '''
        items = []
        type = line_info['type']
        if type == 'empty':
            items.append(QStandardItem(' '))
            self.appendRow(items)
            self._lines.append(line_info)
            return
            
            
        if type not in ['single', 'pair', 'ratio'] : 
            raise ScalerPageException(f'Invalid scaler line type {type} ')
        
        #  Let's build the appropriate standard items:
        
    
        
        # All items have scaler 1:
        
        items.append(QStandardItem(line_info['scalers'][0]))   #name
        items.append(QStandardItem('0'))                       #total.
        items.append(QStandardItem('0.0'))                     # rate
        
        # If type type is a pair or ratio, it has the second scaler:
        
        if type in ['pair', 'ratio'] :
            items.append(QStandardItem(line_info['scalers'][1])) #Name
            items.append(QStandardItem('0'))                       #total.
            items.append(QStandardItem('0.0'))                     # rate
            
        # If type is 'ratio' it has the two ratios as well:
        
        if type == 'ratio':
            items.append(QStandardItem('0.0'))                   # ratio of totals.
            items.append(QStandardItem('0.0'))                   # Ratio of rates
            
        # If we got this far, we're going to succeed:
        
        self._lines.append(line_info)                            # Remember the line 
        self.appendRow(items)                                    # and add it to the model.
        
        # Make the items uneditable:
        
        for item in items:
            item.setEditable(False)
    
    def lines(self):
        ''' @return the array of line definitions in the model. '''
        return self._lines
    
    def update_line(self, row, totals, rates):
        '''
          This method updates a line of the model.
          @param row - is the row to update.
          @param totals - is an array of new totals.
          @param rates -  is an array of new rates 
          
          If necessary, we will compute the ratios.
          
          @exception ScalerPageException if the row is invalid.
          
          
        '''
        if row >= len(self._lines):
            raise ScalerPageException(
                f'{row} is not a valid line number Maximum is {len(self._lines)}'
            )
        type = self._lines[row]['type']
        # Can't update an empty row:
        
        if type == 'empty':
            raise ScalerPageException(
                f'You are attempting to update line {row} but that is an empty row.'
            )
        #every row has scaler 1:
        
        self.item(row, 1).setText(str(totals[0]))
        self.item(row, 2).setText(f'{rates[0]:.2f}')
        
        #  Pairs and ratios have a second scaler:
        
        if type in ['pair', 'ratio']:
            self.item(row, 4).setText(str(totals[1]))
            self.item(row, 5).setText(f'{rates[1]:.2f}')
            
        # Ratios have rates:
        
        if type == 'ratio':
            if totals[1] != 0:
                total_ratio = f'{totals[0]/totals[1]:.2f}'
            else:
                total_ratio = '****'
            if rates[1] != 0:
                rates_ratio = f'{rates[0]/rates[1]:.2f}'
            else:
                rates_ratio = '****'
            self.item(row, 6).setText(str(total_ratio))
            self.item(row, 7).setText(str(rates_ratio))
    
        
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
    from PyQt5.QtCore  import QTimer
    
    
    up = 0
    def update():
        global up
        # Simulate some running scalers.
         
        up += 1
         
        #  Update the singe scaler  rate is 100
        
        rates = [100.0,]
        totals = [up*100,]
        model.update_line(0, totals, rates)
        
        # Update the pair  scaler 1 rate is 50, scaler 2 rate is 75:
        
        rates = [50.0, 75.0]
        totals= [50*up, 75*up]
        model.update_line(2, totals, rates)
        
        # Update the ratios: rates are 25, 60:
        
        rates = [25.0, 60]
        totals = [25*up, 60*up]
        model.update_line(3, totals, rates)
        
        
         
    
    app = QApplication([])
    win = QMainWindow()
    widget = ScalerPageView(win)
    win.setCentralWidget(widget)
    widget.setTitle('This is the page title')
    
    model = ScalerPageModel()
    widget.setModel(model)
    
    # Some items in the model:
    # One of each type:
    model.addLine({
        'type' : 'single',
        'scalers' : ['raw2.name1']
    })
    model.addLine({
        'type' : 'empty'
    })
    model.addLine({
        'type': 'pair',
        'scalers': ['raw1.name1', 'raw1.name2']   
    })
    model.addLine({
        'type' : 'ratio',
        'scalers': ['raw2.name2', 'raw2.name3']
    })
    
    timer = QTimer()
    timer.setInterval(1000)    # Update the view every second
    timer.timeout.connect(update)
    timer.setSingleShot(False)
    timer.start()
    
    win.show()
    app.exec()
        