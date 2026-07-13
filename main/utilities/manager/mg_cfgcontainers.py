#!/usr/bin/env python3

'''
    This program is a python version of mg_cfgcontainers.tcl
    The UI is similar though there maybe some differences.
    
    The user interface is generated using PyQt6.  That means that
    the bookworm or later image is required...or any other environment
    that includes Qt6 and its python bindings.
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


from PyQt6.QtWidgets import (QWidget, QListWidget, QPushButton, QVBoxLayout, QHBoxLayout, 
        QFrame)
from PyQt6.QtCore import QObject, pyqtSignal
class ContainerSelectionWidget(QWidget):
    ''' 
        This class provides for:
        * Selecting and editing an existing container definition.
        * Creating a new container widget
        * Finishing.
        
        It is typically , the widget the application fires up.
        
        Properties:
            containers - names of the containers that populate the known containers list.
        
        Signals (from user interface selections)
            edit(str) - Edit the container named in the string.
            create()  - Create a new container.
            done()    - User wants to exit.
    '''
    
    edit   = pyqtSignal(str)
    create = pyqtSignal()
    done   = pyqtSignal()
    
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._layout = QVBoxLayout(self)   # predominantly stacked stuff:
        
        self._containers = QListWidget(self)   # container names go here.
        self._layout.addWidget(self._containers)
        
        # Now a frame for the new..edit.. buttons:
        # It'll have a horizontal box layout
        
        self._neweditframe = QFrame(self)
        self._neweditframe.setLineWidth(1)
        self._neweditframe.setFrameStyle(QFrame.Shape.Box)
        self._neweditLayout = QHBoxLayout(self._neweditframe)
        self._new = QPushButton('New...', self._neweditframe)
        self._edit = QPushButton('Edit...', self._neweditframe)
        self._neweditLayout.addWidget(self._new)
        self._neweditLayout.addWidget(self._edit)
        self._layout.addWidget(self._neweditframe)
        
        self._exit = QPushButton('Exit', self)
        
        self._layout.addWidget(self._exit)
        
        self.setLayout(self._layout)
    
        #  hook up the button signals:
        
        self._new.clicked.connect(self.create)
        self._exit.clicked.connect(self.done)
        self._edit.clicked.connect(self._emitEdit)
        
    #  Attribute  implementations:
    
    def containers(self)-> list[str]:
        ''' Return the container names in the list box. '''
        result = list()
        for row in range(self._containers.count()):
            result.append(self._containers.item(row).text())
        return result
        
    def setContainers(self, containers : list[str]) -> None:
        ''' 
            Clear and fill the list box.
            @param containers - new values to fill the list box with.
        '''
        # Clear first:
        while self._containers.count() > 0:
            self._containers.take(0)
        
        self._containers.addItems(containers)
    

    # Private slots:
    
    def _emitEdit(self) -> None:
        # Have to get the selecte item from the list box
        # If there isn't one we won't emit edit.
        
        selected = self._containers.currentItem()
        if selected is not None:
            self.edit.emit(selected.text())
            
            
# For now test code:

if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication
    import sys

    def new():
        print('make a new container')
    def edit(name):
        print('edit container named', name)

    def done() :
        app.exit(0)
        
    app = QApplication(sys.argv)
    
    win = ContainerSelectionWidget()
    
    # Pout some containers in the widget:
    
    win.setContainers([
        'container1', 'container2', 'lastone'
    ])
    print('loaded: ', win.containers())
    
    # Hook up signals:
    
    win.create.connect(new)
    win.edit.connect(edit)
    win.done.connect(done)
    
    win.show()
    
    sys.exit(app.exec())
        
        
        
        
        
        
        
        