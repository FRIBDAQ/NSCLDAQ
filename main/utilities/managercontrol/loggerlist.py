"""
This module provides a class to describe and control event loggers.
The main class is LoggerList which lists the loggers and provides
controls.

"""

from PyQt5.QtWidgets import (QTableWidget, QTableWidgetItem, QCheckBox, QPushButton, QWidget, QVBoxLayout)
from PyQt5.QtCore import pyqtSignal, Qt


class LoggerTable(QTableWidget):
    """This is a table of loggers.  The table has the following columns
    (A row descsribes a logger):
    
    Source - URI of the data source (ringbuffer).
    Destination - Path to the destination diretory
    C   - X'd if the logger is critical
    P   - X'd if the logger is partal.
    Enable - A check button that  checked if the logger is enabled.
    
    Important method:
    
    update - Takes the output from nscldaq.manager_client.Logger.list()
            and refreshes the table.  Note that, unlike programs, 
            we don't clear items.  We do an update in place. 

    Base Class:
        QTableWidget
    Signals:
        toggle(list) - Whena  checkbox is toggled.  The list contains a boolean and string:
             - bool - the new state of the checkbox - True  means checked. False not.
             - str  - Destination of the logger, e.g. if the user is enabling a logger that
             logs to /user/fox/stagearea/full, toggle will pass [True, "/user/fox/stagearea/full"]
    """
    toggle = pyqtSignal(list)
    def __init__(self, *args):
        """Construction.
                The base class is constructed and we set up the column labels.
                Content will be loaded/modified via calls to update().
           Arguments:
           args - is passed to the base class constructor.
           
           
        """
        super().__init__(*args)
        self.setColumnCount(5)
        self.setHorizontalHeaderLabels([
            "Source", "Destination", "C", "P", "Enable"
        ]
            
        )
    def updateData(self, listing):
    
        """Update the table in place.  The algorithm is pretty simple:
           - For each row in the table
             *  Find the listing by destination - if not found kill that row.
             *  Update the row except for the destination value.
             *  Remove that item from the listing.
             
           - For each list item, add a new table row.

        Args:
            listing (_type_): _description_
        """
        
        delete_rows = []
        delete_defs = []
        for row in range(self.rowCount()):
            
            dest = self.item(row, 1).text()
            (index, definition) = self._findDest(dest, listing) 
            if index is None:
                delete_rows.append(row)
            else:
                self._updateRow(row, definition)
                delete_defs.append(index)
        
        # Delete the rows that don't have definitions
        # and the definitions that we updated.
        
        delete_rows.sort(reverse=True)
        delete_defs.sort(reverse=True)
        for row in delete_rows:
            self.removeRow(row)
        for index in delete_defs:
            listing.pop(index)

        # Add rows for any remaining definitions:
        
        for definition in listing:
            self._addRow(definition)
        return     
    # Private utility methods    
    
    def _findDest(self, dest, listing):
        # Find a destination in a listing of loggers.
        # Returns (index, definition) if found else
        # (None, None) if not found.
        
        for index, definition in enumerate(listing):
            if definition['destination'] == dest:
                return (index, definition)
        return (None, None)      # stub - not found.)
    
    def _updateRow(self, row, definition):
        #  Update the contents of a row:
        #  The source might change, the criticality of the logger may change
        #  as well as the partialness, the state of the enable widget too
        #  May change.
        #  Since the row was found by destination, that's assumed constant.
        #  With the exception of the enable widget, we only modify a column if it's content
        #  Value should change.
        
        # Data source:
        sourceItem = self.item(row, 0)
        if sourceItem.text() != definition['ring']:
            sourceItem.setText(definition['ring'])
        
        # Critical flag:
        
        critical_text = 'X' if definition['critical'] == 1 else '' 
        criticalItem = self.item(row, 2)
        if critical_text != criticalItem.text():
            criticalItem.setText(critical_text)
           
        # Partial flag:
        

        partial_text = 'X' if definition['partial'] == 1 else '' 
        partialItem = self.item(row, 3)
        if partial_text != partialItem.text():
            partialItem.setText(partial_text)
            
        # Now the enable widget:
        
        enabled = definition['enabled'] == 1     # convert to a bool.
        self.cellWidget(row, 4).setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        
        
    def _addRow(self, definition):
        new_row = self.rowCount()
        self.insertRow(new_row)
        
        # Now load up the row....and hook the checkbox to our
        # signal maker.
        
        self.setItem(new_row, 0, QTableWidgetItem(definition['ring']))
        self.setItem(new_row, 1, QTableWidgetItem(definition['destination']))
        self.setItem(new_row, 2, QTableWidgetItem('X' if definition['critical'] == 1 else ''))
        self.setItem(new_row, 3, QTableWidgetItem('X' if definition['partial'] == 1 else ''))
        check = QCheckBox(self)
        self.setCellWidget(new_row, 4, check)
        enabled = definition['enabled'] == 1 
        check.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        
        check.stateChanged.connect(self._checkToggled)
        
    def _checkToggled(self, state):
        # a checkbox was toggled... but which one??
        # We need to find it in the table and send the associated 
        # destinatino on to the toggle we'll emit.
        
        widget = self.sender()
        for row in range(self.rowCount()) :
            if widget == self.cellWidget(row, 4):
                destination = self.item(row, 1).text()
                self.toggle.emit([state == Qt.Checked, destination])
                return
        
        # No matching widget...for now lets mumble that.
        
 
class LoggerManager(QWidget):
    """
        Just a logger table with a button at the bottom to request
        an update.
        
        Methods:
            update - to update the table from a logger listing.
        Signals:
            toggle(list) - relays the toggle signal from the table.
            requpdate       - Update buttonclicked.
            
        This is really just for testing.
    """
    toggle = pyqtSignal(list)
    requpdate = pyqtSignal()
    
    def __init__(self, *args):
        """Constructor.
        Args:
            args - passed to the base class initializer.
            
            
        """
        super().__init__(*args)
        layout = QVBoxLayout()
        
        self._table = LoggerTable(self)
        layout.addWidget(self._table)
        
        self._update = QPushButton('Update', self)
        layout.addWidget(self._update)
        
        self.setLayout(layout)
    
        # Hook the signals:
        
        self._table.toggle.connect(self.toggle)
        self._update.clicked.connect(self.requpdate)
    
    def updateData(self, definitions):
        self._table.updateData(definitions)

# Test code:


if __name__ == "__main__":
    from PyQt5.QtWidgets import (QApplication, QMainWindow)
    from nscldaq.manager_client import Logger 
    import sys
    
    def changeEnable(info):
        newstate = info[0]
        destination = info[1]
        client = Logger('localhost')
        if newstate:
            client.enable(destination)
        else:
            client.disable(destination)
        
        # Dont' update the table as this will be called for new toggles resulting
        # in recursion in the update.   Hopefully the toggle state is correct anyway.    
        #updater()

    def updater():
        client = Logger('localhost')
        widget.updateData(client.list())
    
    app = QApplication(sys.argv)
    main = QMainWindow()
    
    widget = LoggerManager()
    main.setCentralWidget(widget)
    widget.toggle.connect(changeEnable)
    widget.requpdate.connect(updater)
    widget.updateData([ 
        {'ring': 'tcp://localhost/ron', 'destination': '/home/ron/events/complete', 'critical': 1, 'partial': 0, 'enabled': 0},
        {'ring': 'tcp://localhost/fox', 'destination': '/home/ron/events/partial', 'critical': 0, 'partial': 1, 'enabled': 1}
    ])
    
    main.show()
    app.exec()
        