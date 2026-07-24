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

from PyQt6.QtWidgets import QTableView
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
        
# Debug/test code for now.

if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication
    import sys
    
    app = QApplication(sys.argv)
    
    win = KvTable()
    win.setKv([
        ('run', '0'), ('title', 'this is a title')
    ])
    
    print(win.kv())
    win.show()
    sys.exit(app.exec())
        
        