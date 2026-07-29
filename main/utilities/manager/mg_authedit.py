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
        QMessageBox, QPushButton, QDialog, QDialogButtonBox, QStyle, QApplication)
from PyQt6.QtGui     import (QStandardItemModel, QStandardItem)
from PyQt6.QtCore    import (QObject, QModelIndex, pyqtSignal)
from nscldaq.editablelist6 import ListToListEditor

from nscldaq.mg_database import Auth
import sqlite3
import sys

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
        selected(str) - an ite was double clicked.
        
    '''
    add     = pyqtSignal(str)
    remove  = pyqtSignal(str)
    selected= pyqtSignal(str)
    
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
        self._list.doubleClicked.connect(self._selectRelay)
        self._item.returnPressed.connect(self._addRelay)
        
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
        for row in range(self._model.rowCount()):
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
    
    def clearEntry(self) -> None:
        '''  Clear the text entry widget:'''
        self._item.setText('')
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
            
    def _selectRelay(self, index : QModelIndex) -> None:
        # Relay the double clikced signal -> selected.
        # Probably could just have easily have been a lambda
        self.selected.emit(self._model.itemFromIndex(index).text())

class RoleDefiner(ListToListEditor):
    '''
        This extends the list to list editor by 
        providing  a mechanism to more easily stock
        the list boxes:
        
    Attributes:
        roles - the set of roles in the left box (the ones the user doesn't have).
        granted - the set of roles in the right box (the ones that the user does have).
    '''
    def __init__(self, parent: QObject | None = None):
        super().__init__(parent)
    
    #  Public methods:
    
    def roles(self) -> list[str]:
        ''' @return list[str] - the roles the user could get.'''
        sb = self.sourcebox()
        result = list()
        for row in sb.count():
            result.append(sb.item(row).text())
        
        return result
    def setRoles(self, roles : list[str]) -> None:
        ''' @param roles : list[str]  - roles the user can be granted.'''
        sb = self.sourcebox()
        sb.clear()
        for role in roles:
            sb.addItem(role)
        
    def granted(self) -> list[str]:
        ''' @return list[str] - roles the user was granted'''
        return self.list()
    def setGranted(self, roles : list[str]) -> None:
        ''' @param roles - the roles that have been granted to the user.'''
        
        rb = self.selectedbox()
        rb.setList(roles)
    
# These dialogs are for adding users and roles to the system.

class DefineUsersDialog(SaveDialog):
    '''  A Save dialog with an ItemDefiner workarea that is
         labeled 'User'
    '''
    def __init__(self, parent: QObject | None = None):
        super().__init__(ItemDefiner(), parent)
        self.workarea().setLabel('Users')
        self.workarea().add.connect(self._addItem)
        self.workarea().remove.connect(self.workarea().removeItem)

    def _addItem(self, name :str) -> None:
        # Don't allow duplicates:
        if name in self.workarea().items():
            QMessageBox.warning(
                self, 'Duplicate name', 
                f'{name} would be a duplicate.'
            )
        else:
            self.workarea().addItem(name)
            self.workarea().clearEntry()

class DefineRolesDialog(DefineUsersDialog):
    ''' Relabel the label Role'''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        self.workarea().setLabel('Role')

class GrantRevokeDialog(SaveDialog):
    '''
        Save dialog with a RoleDefiner as the 
        work area.
    '''
    def __init__(self, parent : QObject|None = None):
        super().__init__(RoleDefiner(), parent)
    
class AuthorizeUsers(QWidget):
    '''
        Top level widget for the UI.
        This is mostly just a button box with buttons
        for defining new users, roles and assigning roles
        to users.
        
        Attributes:
        users   - Get/Set the users in the combobox next to the 'grant/revoke roles button'
        selectedUser - readonly the currently selected user
        Signals:
        
        defineUsers()
        defineRoles()
        grantRoles(str) - the str is the user to work with.
        done()   
    '''
    defineUsers = pyqtSignal()
    defineRoles = pyqtSignal()
    grantRoles  = pyqtSignal(str)
    done        = pyqtSignal()
    
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        self._users = QPushButton('Add/Remove Users...', self)
        self._layout.addWidget(self._users)
        
        self._roles = QPushButton('Add/Remove Roles...', self)
        self._layout.addWidget(self._roles)
        
        grantlayout = QHBoxLayout()
        
        self._assignroles = QPushButton('Assign Roles for...', self)
        grantlayout.addWidget(self._assignroles)
        
        self._userlist = QComboBox(self)
        self._userlist.setEditable(False)   # Just in case.
        grantlayout.addWidget(self._userlist)
        
        self._layout.addLayout(grantlayout) 
        
        self._done = QPushButton('Exit', self)
        self._layout.addWidget(self._done)
        
        # Connect the button singnals.. Except for grantRoles,
        # these are effectively passthroughs:
        
        self._users.clicked.connect(self.defineUsers)
        self._roles.clicked.connect(self.defineRoles)
        self._assignroles.clicked.connect(self._assignRolesRelay)
        self._done.clicked.connect(self.done)
        
    # Implement roles:
    def users(self) -> list[str] :
        ''' @return list[str] users in the user selection combobox.'''    
        result = []
        for row in range(self._userlist.count()):
            result.append(self._userlist.itemText(row))
        return result
    
    def setUsers(self, users: list[str]) -> None:
        ''' @param users : list[str] = new set of users to load in to the combobox.'''
        self._userlist.clear()
        self._userlist.addItems(users)
        
    def selectedUser(self) -> str:
         ''' @return str - the currently selected user.'''           
         return self._userlist.currentText()
    
    # internal (Private) signals
    
    def _assignRolesRelay(self) -> None:
        self.grantRoles.emit(self.selectedUser())


class AuthorizationController(QObject):
    '''
        Interacts with the AuthorizeUsers view and 
        pops up dialogs as needed, stocking them and
        updating the database from the results of their  operation.
        
        Note to simplify somewhat, nested controller classes are defined
        for each of the dialog types we can pop up.
        
    '''
    def __init__(
        self, view : AuthorizeUsers, config_file : str, parent : QObject | None = None
    ):
        super().__init__(parent)
        self._view = view
        self._config = config_file
        self._db     = sqlite3.connect(self._config)
        
        self._loadUsers()                 # Load the user pull down in the view.
        
        # Connect to the buttons:
        
        self._view.defineUsers.connect(self._users)
        self._view.defineRoles.connect(self._roles)
        self._view.grantRoles.connect(self._grant)
        self._view.done.connect(self._exit)
        
    # internal/private slots:
    
    def _users(self) -> None:
        # Add/remove users.
        dialog = DefineUsersDialog(self._view)
        workarea = dialog.workarea()
        auth_api = Auth(self._db)
        workarea.setItems(auth_api.listUsers())
        
        if dialog.exec() == QDialog.DialogCode.Accepted:
            self._updateUsers(auth_api, workarea)
            self._loadUsers()                      # update the combobox too.
        
    def _roles(self) -> None:
        # Add/remove roles.
        dialog = DefineRolesDialog(self._view)
        workarea = dialog.workarea()
        auth_api = Auth(self._db)
        workarea.setItems(auth_api.listRoles())
        
        if dialog.exec() == QDialog.DialogCode.Accepted:
            self._updateRoles(auth_api, workarea)
            
    def _grant(self, user : str) -> None:
        # grant/revoke roles for a user.
        
        dialog = GrantRevokeDialog(self._view)
        dialog.setWindowTitle(f'Grant/revoke roles for {user}')
        workarea = dialog.workarea()
        api      = Auth(self._db)
        
        # The left list is all the roles the user does not have
        # the right list all the ones they do have:
        
        all_roles = api.listRoles()
        granted_roles = api.grantedRoles(user)
        ungranted_roles = [x for x in all_roles if x not in granted_roles]
        workarea.setRoles(ungranted_roles)
        workarea.setGranted(granted_roles)
        
        if dialog.exec() == QDialog.DialogCode.Accepted:
            self._updateGrants(api, user, workarea)
        

    def _exit(self) -> None:
        # User wants to exit.
        QApplication.instance().exit(0)
  
    # Utility methods:
    
    def _loadUsers(self):
        # Load the defined users into the View' combobox:
        
        auth_api = Auth(self._db)
        self._view.setUsers(auth_api.listUsers())
    
    def _updateUsers(self, api: Auth, userlist : ItemDefiner) -> None:
        # UPdate the database, deleting deleted users  and adding new ones:
        
        dlg_users = userlist.items()
        db_users   = api.listUsers()
        
        delete_userlist = [x for x in db_users if x not in dlg_users]
        add_userlist    = [x for x in dlg_users if x not in db_users]
        
        for user in delete_userlist:
            api.removeUser(user)
        for user in add_userlist:
            api.addUser(user)
        
    def _updateRoles(self, api: Auth, rolelist : ItemDefiner) -> None:
        # Update the database deleteing deleted roles and adding added roles:
        
        dlg_roles = rolelist.items()
        db_roles  = api.listRoles()
        
        deleted_rolelist = [x for x in db_roles if x not in dlg_roles]
        add_rolelist     = [x for x in dlg_roles if x not in db_roles]
        
        for role in deleted_rolelist:
            api.removeRole(role)
        for role in add_rolelist:
            api.addRole(role)
    
    def _updateGrants(self, api: Auth, user : str, listing : RoleDefiner) -> None:
        # Update the database to reflect the roles that have been granted to the user:
        
        dialog_roles = listing.granted()
        db_roles     = api.grantedRoles(user)
        
        revoke_list = [x for x in db_roles if x not in dialog_roles]
        grant_list  = [x for x in dialog_roles if x not in db_roles]
        
        for role in revoke_list:
            api.revoke(role, user)
        for role in grant_list:
            api.grant(role, user)

        
def usage():
    ''' Output program usage to stderr: '''  
    print('''
Configure users and roles and grant/revoke them from users in the FRIB/NSCLDAQ
managed experiment environment.

Usage:
    $DAQBIN/mg_authedit config_file_path
Where:
    config_file_path - Path to the configuration database file for the experiment.
        ''', file = sys.stderr)
    
# Program entry:
def main()  -> int:    
    if len(sys.argv) != 2:
        usage()
        return -1
    
    config =sys.argv[1]
    app = QApplication(sys.argv)
    
    ui = AuthorizeUsers()
    
    _controller = AuthorizationController(ui, config, ui)
    
    ui.show()
    return app.exec()
    
if __name__ == '__main__':
    
    sys.exit(main())       
    