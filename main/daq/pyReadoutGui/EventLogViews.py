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


from typing import ClassVar

from PyQt6.QtWidgets import QTableView, QCheckBox, QWidget
from PyQt6.QtGui import QStandardItem, QStandardItemModel
from PyQt6.QtCore import pyqtSignal, Qt, QModelIndex

class EventLogList(QStandardItemModel):
    ''''
    This is a standard item model that contains a list of the
    Event loggers.   It displays the following columns:
    * Name - the logger name
    * Type - The logger type.
    * Logger host ('N/A' if not applicable)
    * Logger Source ('N/A' if not applicable)
    * Logger Destination ('N/A' if not applicable)
    * Logger enable state check showing the enable state of a logger.
    
    Column headers are: Name Type, Host, Source, Destination Enabled
    
    Key methods:
      
      load               - Load the model.  See its docstring for what it expects.
    
    '''
    
    
    def __init__(self, enables: bool = False, parent : QWidget | None = None):
        '''
        @param enables : bool = False - if True, the enables, when loaded are live checkbuttons.
        @param parent : QWidget | None = None - Parent widget if desired.
        '''
        
        super().__init__(parent)
        
        self._enableCheckboxes = enables
        
        # Relay the double click signal:
        
        

    def load(self, loggerInfo : list[(str, str,  bool, object)]) -> None:
        '''
        - Clear the model,
        - Set the horizontal headers.
        - Load the model:
        
        @param loggerInfo : list[str, str, bool, object] The info to load into the model.
             Each element of the list is a tuple containing:
             - str - the name of the logger.
             - str - The type of the logger (as registered with the registry)
             - bool - The enabled state of the logger.
             - object - The Event logger itself.
        '''
        self.clear()                      # Clears the column headers so:
        self.setHorizontalHeaderLabels(['Name', 'Type',  'Host', 'Source', 'Destination', 'Enabled'])
        
        for (name, typename, enabled, logger) in loggerInfo:
            self._addRow(name, typename, enabled, logger)
            
    
    
    # Utility methods
    
    def _addRow(self, name : str, typename : str, enabled : bool, logger : object) -> None:
        config = logger.getConfig()
        print('Adding row:', config)
        nameItem     = QStandardItem(name)
        typeNameItem = QStandardItem(typename)
        host         = QStandardItem(config.get('host', 'N/A'))
        source       = QStandardItem(config.get('source', 'N/A'))
        print('Source: ', source.text())
        destination  = QStandardItem(config.get('destination', 'N/A'))
        enabledItem      = QStandardItem('')      # Placeholder.
        
        self.appendRow([nameItem, typeNameItem, host, source, destination, enabledItem])
        
        
        
    
class EventLogListView(QTableView):
    '''
    The view for the event log list.  We just pre-set the model
    to an EventLogList parented by us.  The client can get and load
    the model as well as attach to its signals.  We provide
    a signal
    
    selected(str) - If a row is double cliked, the signal paramter
          is the name of the logger in the selected row.
     enableToggled(str, bool) - An enable was toggled, name of thel logger is passed
                                   and the new state.
    '''
    selected = pyqtSignal(str)   
    enableToggled = pyqtSignal(str, bool)
    
    # This style sheet is applied to all of the
    # QCheckBoxWigets - it makes them readable even if
    # disabled, as they are for just the list.
    
    checkBoxStyle : ClassVar[str] = '''
    QCheckBox:disabled {
        color: #333333; /* Forces text to remain dark and readable */
    }

    QCheckBox::indicator:disabled {
        border: 1px solid #999999;
        background-color: #ffffff; /* Keeps the checkbox background white */
    }

    QCheckBox::indicator:disabled:checked {
        /* If you are using custom images, ensure the checkmark stays visible */
        image: url(:/images/checkbox_checked_disabled.png); 
    } 
    '''
    def __init__(self, enables: bool = False, parent :QWidget | None = None):
        '''
        @param enables bool (False) - have the model enable the enables checkboxes for modification.
        @param parent : QWidget | None (None) - parent of this widget.
        
        '''
        
        super().__init__(parent)
        
        self.setModel(EventLogList(enables, self))
        self._enables = enables
        self.doubleClicked.connect(self._selectedRelay)
    
    def load(self, loggerInfo : list[(str, str,  bool, object)]) -> None:
        '''
        (Re)Load the model and set up the enable check boxes as per self._enables
        
        @param loggerInfo : list[(str, str,  bool, object)] The info to load into the model.
             Each element of the list is a tuple containing:
             - str - the name of the logger.
             - str - The type of the logger (as registered with the registry)
             - bool - The enabled state of the logger.
             - object - The Event logger itself.
        '''
        self.model().load(loggerInfo)
        
        # Now we need to add the enables.  These are in column 5.
        
        for row, info in enumerate(loggerInfo):
            index = self.model().index(row, 5)
            check = QCheckBox(self)
            check.setCheckState(Qt.CheckState.Checked if info[2] else Qt.CheckState.Unchecked)
            check.setStyleSheet(self.checkBoxStyle)
            check.setDisabled(not self._enables)
            check.clicked.connect(lambda: self._toggled(check, info[0]))
            self.setIndexWidget(index, check)
            
            
        
    # Internal (private) slots:
    
    def _toggled(self, loggerName : str, checkBox : QCheckBox):
        # PRocess checkbox clicks into enableToggled signals
        
        self.enableToggled.emit(loggerName, checkBox.checkState() == Qt.CheckState.Checked)
        
    
    def _selectedRelay(self, index :QModelIndex) -> None:
        # Figure out the name of the logger in the clicked row and
        # emit the selected signal.
        
        nameIndex = index.siblingAtColumn(0)
        nameItem  = self.model().itemFromIndex(nameIndex)
        self.selected.emit(nameItem.text())
    
