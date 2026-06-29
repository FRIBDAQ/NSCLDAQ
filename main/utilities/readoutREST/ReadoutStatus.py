'''
    Provides a model/view for a readout status list.  The intent is to provide
    a relatively compact display of the readouts, where they live, their state and
    if they are running.  Note that as usual, the controller, which connnects to the
    DAQ system is external (RadoutStatusContoller.py for the managed environment).
    
    @file ReadouStatus.py
    @brief Model and view for readout status displays.
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


from PyQt6.QtCore import   Qt
from PyQt6.QtWidgets import QTableView
from PyQt6.QtGui  import QStandardItemModel, QStandardItem

class ReadoutStatusModel(QStandardItemModel):
    '''
        Model for the readout status.  This is a standard item model.
        As such we don't need any explicit signals to communicate with the
        view because the base class will do that for us via the Qt
        stadnard MV architecture.  What we will do is maintain
        parallel information to make updating the model simpler for
        our clients.
        
        Methods:
          load   - Initial load of the model.
          update - Update the readouts state and active data.
          
        Readout programs  are identified by their 'name' which is  unique
        across a user's DAQ application.
    '''
    def __init__(self, parent=None):
        '''
            Set up the initial model.  There are 4 colunms:
            *   Name - a unique name for the readout in that row.
            *   Host - Where the readout is running.
            *   State- State of that readout.  
            *   Running - Inidicates if the Readout is detectably running.
            
            @param parent - optional parent object of this model  if desired.
        '''
        
        super().__init__(parent)
        self._initializeModel()
        
        
        self._readouts = dict()
        
    def load(self, readouts : list[dict]) -> None:
        '''
            Load the model
            - The model is initially cleaared.
            - The number of columns and lables re-instated.
            - The model items are loaded from the readouts parameter.
            
            @param readouts  is a list of dicts.  Each dict describes a readout and
                 has key/values:
                 'name'  - name of the readout program.
                 'host'  - where the program runs.
                 'active' - True if the program is active.
                 'state'  - The Readout's state.
                 
            @note while we don't interpret the 'state' value, normal values come from:
            'idle', 'active' and 'unresponsive'.
        '''
        
        self._readouts = dict()
        self.clear()
        self._initializeModel()
        
        
        for readout in readouts:
            self._addReadout(readout)
                
    
    def update(self, readouts: list[dict])  -> None:
        '''
            Updates the active and state fields for the readouts in the readout list.
            
            @param readouts
            @note if a Readout is not in self._readouts it's added to it and the model
            @note since we are derived from QStandardItemModel appropriate signals
                  will be sent to the view.
        '''
        existing = self._readouts.keys()
        for readout in readouts:
            name = readout['name']
            if name not in existing:
                self._addReadout(readout)
            else:
                self._readouts[name][0].setText('X' if readout['active'] else ' ')
                self._readouts[name][1].setText(readout['state'])
            
        
        
    ## Utility private methods:
    
    def _addReadout(self, readout: dict) -> None:
        # Add a single readout to the model and internal state:
        
        name = QStandardItem(readout['name'])
        host = QStandardItem(readout['host'])
        active = QStandardItem('X' if readout['active'] else ' ')
        state = QStandardItem(readout['state'])
        
        # Noneof these are editable:
        
        flags = Qt.ItemFlag.ItemIsSelectable | Qt.ItemFlag.ItemIsEnabled
        for item in [name, host, active, state]:
            item.setFlags(flags)
            
        # Add the items to our internal state and the model:
        
        self.appendRow([name, host, active, state])
        self._readouts[readout['name']] = (active, state)    # These are the mutable items.
    
    def _initializeModel(self) -> None:
        self.setColumnCount(4)
        self.setHorizontalHeaderLabels(['Name', 'Host', 'State', 'Running'])
        
        
        
        
        
class ReadoutView(QTableView):
    '''
        This is a view of the Reaodut statuses.
        It's intended to be used with ReadoutStatusModel
        and, in fact creates its own model. Since the
        view provides all the functionality we need, this is
        pretty simple:
    '''
    
    def __init__(self, parent=None):
        ''''
            @parent the widgets parent object, if desired.
        '''
        super().__init__(parent)
        
        self.setModel(ReadoutStatusModel(self))
        
    

#  Test code:

if __name__ == "__main__":
    from PyQt6.QtWidgets import QApplication
    from PyQt6.QtCore    import QTimer
    import sys

    def twinkle():
        if len(data) == 2:
            data.append(
                {'name': 'added', 'host': 'spdaq50', 'active': 1, 'state': 'idle'}
            )
        
        # If an item is active , make it inactive and state unresponsive.
        # if it's inactive, make it actvie and idle:
        
        for readout in data:
            if readout['active']:
                readout['active'] = 0
                readout['state'] = 'unresponsive'
            else:
                readout['active'] = 1
                readout['state'] = 'idle'
    
        win.model().update(data)

    app = QApplication(sys.argv)
    win = ReadoutView()
    
    # Put some stuff in the model:
    
    data = [
        {'name': 'AReadout', 'host': 'localhost', 'active': 1, 'state': 'idle'},
        {'name': 'Another', 'host': 'spdaq11', 'active': 0, 'state': 'unresponsive'}
    ]
    win.model().load(data)
    
    # Set up to 'twinke' the data and add another item to the dict.
    
    timer = QTimer(win)
    timer.setInterval(5000)      # Slow twinkle.
    timer.setSingleShot(False)
    timer.timeout.connect(twinkle)
    timer.start()
    
    
    win.show()
    sys.exit(app.exec())