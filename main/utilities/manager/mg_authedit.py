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
This program manipulates the authorization database.  WHile authorization is not yet used,
it is envisioned to be a key comoponent in the future.  The idea is that usernames can be
added and granted specific roles.  In the future, only authorized users will be allowed to 
connect to the manager server and what they can do will be constrained by the roles they've been granted.

So this program has to be able to do three things:
- Define the roles that exist.
- Define the users that exist.
- Define the roles granted to each user.


@file mg_authedit.py
@brief Manage the authorization database.
@author Ron Fox
'''


from PyQt6.QtWidgets import (QWidget, QListView, QHBoxLayout, QVBoxLayout, QComboBox, QLabel, QLineEdit,
        QMessageBox, QPushButton, QDialog, QStyle)
from PyQt6.QtGui     import (QStandardItemModel, QStandardItem)
from PyQt6.QtCore    import (QObject, QModelIndex, pyqtSignal)


class ItemDefiner(QWidget):
    '''
        This is a ListView and mechanisms for adding and removing items from the list.
        Items in the list are assumed to be unique. We're going to use this both
        for usernames and for role names.
        
        Attributes:
        items - the list contents.
        label - The label next to the add QLineEdit.
        
        Methods:
        addItem - add an item to the list.
        removeItem - Remove an item from the list.
        
        Signals:
        add(str)  - Add the string to the list.
        remove(str)  - Remove the string from the list.
    '''
    add     = pyqtSignal(str)
    remove  = pyqtSignal(str)
    
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        #  The listview and the remove button are side-by-side
        #  We'll also generate a model for the list.
        
        list_layout = QHBoxLayout()
        
        
        self._list  = QListView(self)
        self._model = QStandardItemModel(self._list)
        self._list.setModel(self._model)
        list_layout.addWidget(self._list)
        
        self._delButton = QPushButton('Delete Selected', self)
        pixmap          = getattr(QStyle.StandardPixmap, 'SP_DialogDiscardButton')
        icon            = self.style().standardIcon(pixmap)
        self._delButton.setIcon(icon)
        list_layout.addWidget(self._delButton)
        
        self._layout.addLayout(list_layout)
        
        # Suff at the bottom:
        
        add_layout = QHBoxLayout()
        self._label = QLabel(self)   # Client sets the text with setLabel.
        add_layout.addWidget(self._label)
        
        self._item = QLineEdit(self)
        add_layout.addWidget(self._item)
        
        self._addButton = QPushButton('Add', self)
        pixmap          = getattr(QStyle.StandardPixmap, 'SP_TitleBarShadeButton')
        icon            = self.style().standardIcon(pixmap)
        self._addButton.setIcon(icon)
        add_layout.addWidget(self._addButton)
        
        self._layout.addLayout(add_layout)
        
        #  Hook in the buttons:
        
        self._addButton.clicked.connect(self._addRelay)
        self._delButton.clicked.connect(self._delRelay)
        
    # Attributes:
    
    def label(self) -> str:
        '''
            @return str - the contents of the label of the line edit:
        '''
        return self._label.text()
    def setLabel(self, label: str) -> None:
        '''
            @param label : str - new label for the line edit.
        '''
        self._label.setText(label)
        
    def items(self) -> list[str]:
        '''
           @return list[str] - contents of the list.
        '''
        result = list()
        for row in range(self._model.rowCount):
            result.append(self._model.item(row).text())
        return result
    def setItems(self, items: list[str]) -> None:
        '''
         @param items : list [str] - new items to set in the list box.
        '''
        self._model.clear()
        for item in items:
            self.addItem(item)
    
    # Public method:
    
    def addItem(self, item : str) -> None:
        '''
            @param item : str - new item to append to the list.
        '''
        self._model.appendRow([QStandardItem(item),])
    
    def removeItem(self, item : str) -> None:
        '''
            @param item : str - the item to remove from the list.
            @throws KeyError - if item is not in the list.
        '''
        for row in range(self._model.rowCount()):
            if self._model.item(row).text() == item:
                self._model.takeRow(row)
                return
        
        # No such item:
        
        raise KeyError(f'The list does not contain {item}')
    
    #  Private slots:
    
    def _addRelay(self) -> None:
        # Pick up the list box value and emit the add signal.
        # If the item is empty, a message box letting them know we need something to add
        # Is popped instead.
        
        item_text = self._item.text().strip()
        if not item_text:
            QMessageBox.information(self, 'Need an item', "Can't add nothing.")
            return
        self.add.emit(item_text)
        
    def _delRelay(self) -> None:
        #  Get the selection and relay it to the delete. 
        # 
        indices = self._list.selectedIndexes()
        if len(indices) > 0:
            index = indices[0]
            item  = self._model.itemFromIndex(index).text()
            self.remove.emit(item)
            
        
    
if __name__ == '__main__':
    import sys
    from PyQt6.QtWidgets import QApplication
    
    app = QApplication(sys.argv)

    win = ItemDefiner()
    win.setItems(['a', 'b', 'c', 'd'])
    win.add.connect(win.addItem)
    win.remove.connect(win.removeItem)
    win.setLabel('a label')
    win.show()
    
    sys.exit(app.exec())