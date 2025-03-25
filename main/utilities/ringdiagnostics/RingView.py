'''
This module provides the view of the rings.
The view is a tree.  The top items in the tree are hostnames.  
Below the host are rings and proxies.  When you get to proxies and rings, there
are several columns of data:
  * The ring name,
  * Command line of the ring producer (or <None> if there isn't one).
  * Size of the ring,
  * Free space in the ring


Below each ring is the set of consumers which have the columns:
   * command line of the consumer.
   * Backlog of the consumer.

   
So the tree looks like:

Hostname
     +-- Name | Producer | Size | Free space|
       +-------------------------------------|consumer| pid | backlog|
          ...
       + Remotes
       +-- Host
          +----------------------------------|Consumer| pid | Backlog|
        ...
     +-- Name
     ... 
 Hostname
   ...          

Thus expanding a host gives the rings.  Expanding a ring gives
its consumers and Remotes.  Expanding Remotes gives the
set of hosts to which the ring is being hoisted.
Expanding the host gives the set of consumers in the remote host.

Since we are just the view, we don't really do much but smile and be happy.
The model really is where all this hieararchy is built, though we have to build the 
headers.

'''

from PyQt5.QtWidgets import (
    QTreeView, QHeaderView, QAbstractItemView
)
from PyQt5.QtGui import QStandardItemModel, QStandardItem
from PyQt5.Qt import *



class RingView(QTreeView):
    ''' 
     This is just a treeview with the appropriate headers.  The client
     populates this by first getting our model and then
     appending the appropriate hierarchy to it (an unfortunate design
     decision in Qt5 is that the QTreeWidget model supplies the headers too
     so never deletee row 0 of the model as that's the headers).
     
     To update the view, simply get the model again and update its contents.
     The treeview (should) update automatically.
     
     Note that we forbid selecting anything in the treeview.
    '''
    def __init__(self, *args):
        super().__init__(*args)
        self._model = QStandardItemModel()
        self.setModel(self._model)
        self._buildHeader()
        self.setSelectionMode(QAbstractItemView.NoSelection)
        
    
    def model(self):
      '''Return the model associated with the view.'''
      
      return self._model
    
    #  Buidl the header view and set it.
    def _buildHeader(self):
        print("building header")
        self._header = QHeaderView(Qt.Horizontal)
        self._headerItems = []
        self._headerItems.append(QStandardItem())      # Where the tree is (I think).
        self._headerItems.append(QStandardItem('Name'))
        self._headerItems.append(QStandardItem('Producer'))
        self._headerItems.append(QStandardItem('Producer PID'))
        self._headerItems.append(QStandardItem('Size (kb)'))
        self._headerItems.append(QStandardItem('Free (kb)'))
        self._headerItems.append(QStandardItem('Consumer'))
        self._headerItems.append(QStandardItem('Consumer PID'))
        self._headerItems.append(QStandardItem('Backlog (kb)'))
        self.model().appendRow(self._headerItems)
        self.model().setHorizontalHeaderLabels([
          '', 'Name', 'Producer', 'Producer PID', 'Size(kb)', 'Free(kb)',
          'Consumer', 'Consumer PID', 'Backlog (kb)'
        ])
        self.setHeader(self._header)
        self.setHeaderHidden(False)
        
        

#   Test code:

if __name__ == "__main__":
    from PyQt5.QtWidgets import QApplication, QMainWindow
    
    app = QApplication(['ringview test'])
    mw = QMainWindow()
    tree = RingView()
    mw.setCentralWidget(tree)
    mw.show()
    app.exec()
        