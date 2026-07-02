'''
    Provides the model and view for Readout statistics.  The readout provide the
    per run and cumulative trigger statistics for a readout program.
    
    @file ReadoutStatistics.py
    @brief Provides a model and view for Readout trigger statistics.
    @author Ron Fox
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


from PyQt6.QtCore import QObject, Qt
from PyQt6.QtGui  import QStandardItemModel, QStandardItem
from PyQt6.QtWidgets import QTableView


class RunStatisticsModel(QStandardItemModel):
    '''
        Provides a model containing the statistics. Since we inter-operate
        with an e.g. QTableView, we don't need any signals as the model
        will generate the right ones to up date the view.
        
        We're going to maintain the QStandardItem models in an
        internal hash that has 'perRun' and 'cumulative' keys
        which have as members the 'triggers', 'acceptedTriggers' and 'bytes' keys.
    
        We're also going to provide both vertical headers ("Per Run" and "Cumulative")
        and horizontal heaers ('Triggers', 'Accepted Triggers' and 'Bytes of Data').
        
    Attributes:
        statistics  - read/write where the representation is a dict with the keys 
            'perRun' for per run statistics and 'cumulative' for cumulative statistics.
            The values of these keys are dicts with the keys:
            * 'trigger' - number of triggers seen.
            * 'acceptedTriggers' - number of triggers that resulted in an event.
            * 'bytes' - number of bytes of event data emitted.
    '''
    def __init__(self, parent : QObject = None):
        super().__init__(parent)
        
        # Horizontal headers:
         
        self.setHorizontalHeaderLabels([' Triggers', ' Accepted Triggers', ' Bytes of Data'])
        
        # Now make the standard items and save them.. we initialize them to 0's.
        
        self._items = dict()                  # Where we'll store them for updating:
        self._items['perRun'] = dict()
        self._items['perRun']['triggers']         = self._makeItem()
        self._items['perRun']['acceptedTriggers'] = self._makeItem()
        self._items['perRun']['bytes']            = self._makeItem()
        
        self._items['cumulative'] = dict()
        self._items['cumulative']['triggers']         = self._makeItem()
        self._items['cumulative']['acceptedTriggers'] = self._makeItem()
        self._items['cumulative']['bytes']            = self._makeItem()
        
        
        # Put the items in the model:
        
        self.appendRow([
            self._items['perRun']['triggers'],
            self._items['perRun']['acceptedTriggers'], 
            self._items['perRun']['bytes']]
        )
        self.appendRow([
            self._items['cumulative']['triggers'],
            self._items['cumulative']['acceptedTriggers'], 
            self._items['cumulative']['bytes']]
        )
        # Vertical headers.
        
        self.setVerticalHeaderLabels(['Per Run', 'Cumulative'])
        
        
        
        
    # implement properties:
    
    def statistics(self) -> dict[dict]:
        '''
          @return dict - a dictionary of the current statistics. See class docstring for
             the keys this dict will have.
        '''
        result = dict()
        for which,stats in self._items.items():
            result[which] = dict()
            for name,item in stats.items():
                result[which][name] = int(item.text())
    
        return result
    
    def setStatistics(self, statistics : dict[dict]) -> None:
        '''
        @param statistics - the new statistics.  This is a dict of dicts as desribed
            in the class docstring.
        '''
            
        for which, statistic in statistics.items():
            for name, value in statistic.items():
                self._items[which][name].setText(str(value))

    #  Utilities:
    
    def _makeItem(self) -> QStandardItem:
        # Utility to create a standard item with the flags we want.
        item = QStandardItem('0')
        item.setFlags(Qt.ItemFlag.ItemIsEnabled)   # Mostly we want to disable editing.
        return item
        
        
 # View
    
class RunStatistics(QTableView):
    '''
        We just need to hook ourselves into a model.
    '''
    def __init__(self, parent : QObject | None=None):
        super().__init__(parent)
        
        self._model = RunStatisticsModel(self)
        self.setModel(self._model)
        
        self.horizontalHeader().setStretchLastSection(True)
        
    
## Test code:

if __name__  == "__main__":
    from PyQt6.QtCore    import QTimer
    from PyQt6.QtWidgets import QApplication, QMainWindow
    import sys

    def update() -> None:
        # Fake statistics update.
        
        model = widget.model()
        add = 100
        statistics = model.statistics()
        
        for section, statdict in statistics.items():
            for item, counter in statdict.items():
                statistics[section][item] = counter + add
                add = add * 2
                
        model.setStatistics(statistics)
        

    app = QApplication(sys.argv)
    win = QMainWindow()
    widget = RunStatistics(win)
    
    timer = QTimer()
    timer.setInterval(1000)
    timer.setSingleShot(False)
    timer.timeout.connect(update)
    timer.start()
    
    
    win.setCentralWidget(widget)
    win.show()
    sys.exit(app.exec())