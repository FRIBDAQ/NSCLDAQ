#!/usr/bin/env python3

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
This program provides an editor for key value pairs in the
key value database of the FRIB/NSCLDAQ managed experiment environment.

'''

from PyQt6.QtWidgets import (QTableView, QWidget, QPushButton, QStyle, QLabel, QLineEdit, 
        QHBoxLayout, QVBoxLayout, QStyle, QMessageBox)
from PyQt6.QtGui import QStandardItemModel,  QStandardItem
from PyQt6.QtCore import pyqtSignal, QModelIndex, QObject, Qt

class KvTable(QTableView):
    '''
        KvView is a key value viewer.  It presents the KV
        data in two columns.  The left column, has editing turned
        off while the right column has it turned on.  The left item is not
        editable, and is the key. The right column is editable and is
        the value.  This necessiates a KvModel which we'll implement
        as a nested class derived from QStandardItemModel.  The purpose
        of that model is to provide a flags() method to enforce per
        column editability.
        
        Attributes:
          kv - Key value data.
        
        Methods:
            add  - Add a new key/value
            remove - remove a k/v from the table.
            selected - The key/value pair of the selected row.
    '''
    
    # The helper KVModel class is pretty simple. Just need to init the superclass
    # and overrid flags.
    
    class KvModel(QStandardItemModel):        
        def flags(self, index: QModelIndex) -> int:
            # Return the flags editability is onl on in col 2.
            
            result =  Qt.ItemFlag.ItemIsEnabled | Qt.ItemFlag.ItemIsSelectable # All get this.
            if index.column() == 1:
                result |= Qt.ItemFlag.ItemIsEditable
            
            return result
    # Implementation of the KvTable class starts here
    
    def __init__(self, parent: QObject | None = None)    :
        super().__init__(parent)
        
        self._model = KvTable.KvModel(self)
        self.setModel(self._model)
        self._clear()
        
    # Implement the attribute(s)
    
    def setKv(self, data : list[tuple[str,str]]) -> None:
        '''
          @param data - a list of tuples that are pairs of strings.
              the first element of each tuple is the key, the second the value.
        '''
        self._clear()
        for k, v in data:
            self.add(k,v)
           
    def kv(self) -> list[tuple[str,str]]:
        '''
            @return list[tuple[str,str]] The contents of the table as row/value pairs.
        '''
        result = list()
        for row in range(self._model.rowCount()):
            item = (self._model.item(row, 0).text(), self._model.item(row, 1).text())
            result.append(item)
        
        return result
    # Publie methods:
    
    def add(self, key : str, value : str) -> None:
        '''
            @param key : str - Key of new item to add.
            @param value : str - its value
        '''
        k = QStandardItem(key)
        v = QStandardItem(value)
        
        self._model.appendRow([k,v])
    
    def remove(self, key: str) -> None:
        '''
            @param key - the key to remove from the model.
            @throws ValueError if key is not in the model.
        '''
        # sadly findItems does not give a row, just the item which is worthelss
        
        for row in range(self._model.rowCount()):
            keyItem = self._model.item(row, 0).text()
            if keyItem == key:
                self._model.takeRow(row)
                return
        # No match:
        
        raise ValueError(f'There is no key {key}')

    def selected(self) -> tuple[str,str] | None:
        '''
        @return tuple[str,str] - the key/value of the selected row or None if there is no selection.
        '''
        
        selection = self.selectedIndexes()
        if len(selection) > 0:
            row = selection[0].row()
            return (self._model.item(row, 0).text(), self._model.item(row, 1).text())
        else:
            return None
        
    # Utilties:
    def _clear(self):
        self._model.clear()
        self._model.setHorizontalHeaderLabels(['Key', 'Value'])
        
class KvSpecifier(QWidget):
    '''
      This widget provides the controls needed to specify a key/value pair
      Specifically labeled line entries for the key and value as well as
      an Add button.
      Attributes:
        key - Contents of the key line entry.
        value - Contents of the value line entry.
      Signals:
        add - the add button was clicked.
    '''
    add = pyqtSignal()
    
    
    
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._layout = QHBoxLayout(self)
        self.setLayout(self._layout)
        
        self._layout.addWidget(QLabel('Key: ', self))
        self._key = QLineEdit(self)
        self._layout.addWidget(self._key)
        
        self._layout.addWidget(QLabel('Value: ', self))
        self._value = QLineEdit(self)
        self._layout.addWidget(self._value)
        
        
        self._addbutton = QPushButton('Add', self)
        self._layout.addWidget(self._addbutton)
        self._addbutton.clicked.connect(self.add)
    
    # Implement the attributes:
    
    def key(self) -> str:
        ''' @return str - the contents of the key entry.
        '''
        return self._key.text()
    def setKey(self, key: str) -> None: 
        ''' @param key : str - new contents of the key entry.'''
        self._key.setText(key)
    
    def value(self) -> str:
        ''' @return str - the contents of the value line endit'''
        return self._value.text()
    def setValue(self, value: str) -> None:
        ''' @param value: str - nea contents of  the value line edit.'''
        self._value.setText(value)


class KvEdit(QWidget):
    '''
    Compound of KvTable and KvSpecifier, and a delete button.
    KvEdit autonomously handles the delete and add buttons manipulating
    the table as needed.
    
    Attributes (readonly)
    
    table - gets the KvTableWidget.
    
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # Primarily stacked widgets:
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        # Though delete current and the table are side by side:
        
        table_layout = QHBoxLayout()
        self._table = KvTable(self)
        table_layout.addWidget(self._table)
        
        self._deletebutton = QPushButton('Delete Selected', self)
        pixmap = getattr(QStyle.StandardPixmap, 'SP_DialogCancelButton')
        icon   = self.style().standardIcon(pixmap)
        self._deletebutton.setIcon(icon)
        table_layout.addWidget(self._deletebutton)
        
        self._layout.addLayout(table_layout)
        
        #  Now the editor widget:
        
        self._editor = KvSpecifier(self)
        self._layout.addWidget(self._editor)
        
        # Hook into signals for autonmous actions:
        
        self._deletebutton.clicked.connect(self._deleteSelected)
        self._editor.add.connect(self._addEntry)
    
    # Implement table attribute:
    
    def table(self) -> KvTable:
        return self._table
    
    # Internal slots
    
    def _deleteSelected(self) -> None:
        # Delete the selected table entry.
        
        selection = self._table.selected()
        if selection:
            self._table.remove(selection[0])
    
    def _addEntry(self) -> None:
        # The add button in the specifier was clicked. 
        # If either key or value are empty, that's a popup
        # dialog info box.
        # If the key is already in the table, that's a warning dialog.
        # If neither is true, the table's add method is called.
        
        key = self._editor.key().strip()
        value = self._editor.value().strip()
        
        if not key or not value:
            QMessageBox.information(self, 'Missing input', 'Both the "Key" and "Value" fields must be non-empty.')
            return
        
        existing_keys = dict(self.table().kv()).keys()
        if key in existing_keys:
            QMessageBox.warning(
                self, 'Duplicate key', 
                f"There's already a '{key}' in the table. To edit the value of an existing key, just edit the value in the table"
            )
            return
        
        self._table.add(key, value)
        self._editor.setKey('')
        self._editor.setValue('')
        
        
# Debug/test code for now.

if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication
    import sys
    
    
    app = QApplication(sys.argv)
    
    win = KvEdit()
    win.table().setKv([
        ('run', '0'), ('title', 'this is a title')
    ])
    
    win.show()
    
    
    sys.exit(app.exec())
        
        