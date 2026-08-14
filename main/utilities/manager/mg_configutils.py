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

''' Module with configuration utilities.

'''
from PyQt6.QtCore import QObject
from PyQt6.QtWidgets import (
    QDialog,
    QDialogButtonBox,
    QHBoxLayout,
    QPushButton,
    QStyle,
    QTableWidget,
    QTableWidgetItem,
    QVBoxLayout,
    QWidget,
)


class SaveDialog(QDialog):
    '''
        This is a dialog with a button box that has save and cancel buttons.
        with a workarea widget that's passed in at construction time.

    '''
    def __init__(self, work_area: QWidget, parent: QObject | None = None):
        super().__init__(parent)
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        self._workarea = work_area
        self._layout.addWidget(self._workarea)
        
        self._buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Save | QDialogButtonBox.StandardButton.Cancel,
            self
        )
        self._layout.addWidget(self._buttons)
    
        self._buttons.accepted.connect(self.accept)
        self._buttons.rejected.connect(self.reject)
        
        
    def workarea(self) -> QWidget:
        return self._workarea
    
    # Turn off key handling:
    
    def keyPressEvent(self, event) -> None:
        pass
    
class OkDialog(QDialog):
    '''
        This is a dialog that has Ok and Cancel buttons.
        It's modeled after the SaveDialog
    '''
    def __init__(self, work_area: QWidget, parent: QObject | None = None):
        super().__init__(parent)
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        self._workarea = work_area
        self._layout.addWidget(self._workarea)
        
        self._buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel,
            self
        )
        self._layout.addWidget(self._buttons)
    
        self._buttons.accepted.connect(self.accept)
        self._buttons.rejected.connect(self.reject)
        
    def workarea(self) -> QWidget:
            return self._workarea
        
        # Turn off key handling:
        
    def keyPressEvent(self, event) -> None:
        pass

class EditableTable(QWidget):
    '''  Megawidget that has a table that can be edited.
      Editing consists of supported deletion of the current row,
      Adding rows to the end, and editing the entries.
     
      Layout is:
       
        +--------------------------+
        |    The table             | [+]
        |                          | [x]
        +--------------------------+
    
       The table() method returns the table widget.
       this allows for things like setting the headers, the # of colomns
       loading it etc.
    '''
    def __init__(self, *args):
        super().__init__(*args)
        layout = QHBoxLayout()
        
        self._table = QTableWidget(self)
        defaultItem = QTableWidgetItem('')
        self._table.setItemPrototype(defaultItem)
        layout.addWidget(self._table)
        
        #  The buttons:
        
        buttonlayout = QVBoxLayout()
        self._addrow = QPushButton('+', self)
        buttonlayout.addWidget(self._addrow)
        self._deleterow = QPushButton(self)
        self._deleterow.setIcon(self.style().standardIcon(getattr(QStyle.StandardPixmap, 'SP_DialogDiscardButton')))
        buttonlayout.addWidget(self._deleterow)
        
        
        layout.addLayout(buttonlayout)
        
        self.setLayout(layout)    
        self._addrow.clicked.connect(self._addRow)
        self._deleterow.clicked.connect(self._deleteRow)
        
    def _addRow(self):
        self._table.setRowCount(self._table.rowCount() + 1)
        new_row = self._table.rowCount() - 1
        cols = self._table.columnCount()
        for c in range(cols):
            self._table.setItem(new_row, c, self._table.itemPrototype().clone())
    def _deleteRow(self):
        row = self._table.currentRow()
        self._table.removeRow(row)
        
    def table(self):
        ''' Returns the table itself. '''
        return self._table
    def col0List(self):
        ''' Returns a list of the values in column 0 with empties suppressed: '''
        
        table = self._table
        rows  = table.rowCount()
        return [table.item(r, 0).text()
                for r in range(rows)
                if len(table.item(r,0).text()) > 0 
                and not table.item(r, 0).text().isspace()]

    def getPairs(self):
        '''
           Return a list of col0, col1 pairs with
           Elements that have no col0 value omitted.
           The result is a list of two element lists for cases
           when both columns are filled in and one element lists if the
           col1 is not filled in.
        '''
        
        table = self._table
        rows = table.rowCount()
        result = []
        
        rawoptions = [[table.item(r, 0).text(), table.item(r, 1).text()] 
                      for r in range(rows)
                      if len(table.item(r, 0).text()) > 0
                      and not table.item(r, 0).text().isspace()
                      ]
        for (option, value) in rawoptions:
            if len(value) == 0 or value.isspace():
                result.append((option,))
            else:
                result.append((option, value))
        return result
        
