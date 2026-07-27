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
    
if __name__ == '__main__':
    import sys
    from PyQt6.QtWidgets import QApplication
    
    app = QApplication(sys.argv)

    win = ItemDefiner()
    win.show()
    
    sys.exit(app.exec())