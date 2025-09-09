'''
  Contains all of the machinery needed to display a table of
  Readout Statuses. Specifically a ReadoutStatusModel, which is
  can be updated from a list of status items gotten from the ReST API
  and ReadoutStatusView which is not much more than a TableView
  with a ReadouStatusModel set into it..
'''

from PyQt5.QtWidgets import (QTableView)
from PyQt5.QtGui import ( QStandardItemModel, QStandardItem)

from PyQt5.QtCore import (pyqtSignal, Qt, QTimer)

class ReadoutStatusModel(QStandardItemModel) :
    '''
        This model is intended to be used with a table for the
        states/statistics of the readout programs.  It maintains the following
        columns:
        * name - the name of the readout program.
        * state - The state of the readout program which can be one of:
            - Discon       - the program is not reachable via ReST.
            - Idle         - the program is connected but the run is not active.
            - Active       - The program is connected and a run is Active.
        * cum trg - cummulative triggers.
        * cum atrg - Cummulative accetped triggers
        * cum bytes - Cummulative data event payload bytes.
        * run trg   - Triggers since last begin run.
        * run atrg  - Triggers accepted since last begin run.
        * run bytes - bytes of event data payload since last begin run.
    '''
    def __init__(self):
        super().__init__(self)
        
        # Define the headings.  We assume the 
        # table has this header turned on.
        
        self.setHorizontalHeaderLabels((
            "Name", "State", 
            "Cum trg", "cum atrg", "cum bytes",
            "run trg", "run atrg", "run bytes"
        ))
        
    #   Return a dict indexed by program name whose values are the line of the table.
    def _enumeratePrograms(self):
        rows = self.rowCount()
        result = dict()
        
        for row in range(0, rows):
            name = self.item(row, 0).text()
            result[name] = row
        
        return result
        
        
    #  Given a list of dicts with a 'name' key, returns the list of name values
    #  in that list
    
    @staticmethod
    def _present(info):
        result = []
        for item in info:
            result.append(item['name'])
        return result
    
    def update(self, info):
        '''
            We update the table from an iterable of dicts where each dict has been returned
            from getStatistics and augmented with a 'name' and 'state' key that has the program name.
            
            If a name exists in the model, we just update the rest of the line.
            If a name does not yet exist in the model, it is added.  If the model has a name
            but it's not in the dict, the name is marekd with the 'Discon' state.
        '''
        
        #  Enumerate names in the model in a dict indexed by name with value the row the name is in:
        
        existingPrograms  = self._enumeratePrograms()
        names = existingPrograms.keys()
        for item in info :
            if item['name'] not in names:
                existingPrograms[item['name']] = self.rowCount()
                self.appendRow(QStandardItem(item['name']))   # Make a new row.
                
            # Update the row rest of the row in place:
            row = existingPrograms[item['name']]
            self.setItem(row, 1, QStandardItem(item['state']))
            
            self.setItem(row, 2, QStandardItem(item['cumulative']['triggers']))
            self.setItem(row, 3, QStandardItem(item['cumulative']['acceptedTriggers']))
            self.setItem(row, 4, QStandardItem(item['cumulative']['bytes']))
            
            self.setItem(row, 5, QStandardItem(item['perRun']['triggers']))    
            self.setItem(row, 6, QStandardItem(item['perRun']['acceptedTriggers']))
            self.setItem(row, 7, QStandardItem(item['perRun']['bytes']))
        
        # Now Set the state of rows that are not in info list
        
        connectedNames = ReadoutStatusModel._present(info)
        for name in names:
            if name not in connectedNames:
                # Not connected.
                self.setItem(existingPrograms[name], 1, QStandardItem('Discon'))
        

class ReadoutStatusView(QTableView):
    onUpdate = pyqtSignal(QTableView)
    
    '''
        Provides a table with the ReadoutStatusModel as its model.
        A QTimer is bundled into this view which allows the 
        application to be informed periodically to update the model.
        Therefore:
        
        Methods:
           setInterval  - Set the interval on the timer.  Note that
             an interval of 0 disables the timer unlike the normal QTimer
             behavior.
        Slots:
           onUpdate    - Passed the tableview as a parameter, this is called on
                QTimer expiration.   The model can then be fetched via 
                getModel.
        
    '''
    def __init__(self, *args):
        super().__init__(*args)
        self.timer = QTimer(self)
        self.model = ReadoutStatusModel(self)
        self.setModel(self.model)
        self.horizontalHeader().show()
        
        #  Connect the timer's timeout to our relay:
        
        self.timer.timeout.connect(self._timerRelay)
        
    #  private methods 
    
    #  Fir our onUpdateSignal
    
    def __timerRelay(self):
            self.onUpdate.emit(self)
    
    # Set the update interval in seconds.
    # Note that 0 seconds disabls the timer and
    # non-zero enables it.
    
    def setInterval(self, seconds):
        if seconds == 0:
            self.timer.stop()
        else :
            ms = seconds*1000
            self.timer.setSingleShot(False)   # Repeating
            self.timer.start(ms)              # Start with new interval
            
