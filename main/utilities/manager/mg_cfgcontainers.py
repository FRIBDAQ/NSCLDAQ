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


from PyQt6.QtWidgets import (QWidget, QListWidget, QListWidgetItem,
        QPushButton, QVBoxLayout, QHBoxLayout, 
        QFrame, QLabel, QLineEdit, QFileDialog, QDialog, QDialogButtonBox)
from PyQt6.QtCore import QObject, pyqtSignal


# Utility method:

def _clearListWidget(widget : QListWidget) -> None:
    widget.clear()

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
        self._containers.itemDoubleClicked.connect(self._editItem)
        
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
        
        _clearListWidget(self._containers)
        self._containers.addItems(containers)
    

    # Private slots:
    
    def _emitEdit(self) -> None:
        # Have to get the selecte item from the list box
        # If there isn't one we won't emit edit.
        
        selected = self._containers.currentItem()
        if selected is not None:
            self.edit.emit(selected.text())
            
    def _editItem(self, _ : QListWidgetItem) -> None:
        self._emitEdit()
class ContainerEdit(QWidget):
    '''
        This widget is an editor for a container.
        It can be used standalone or as a modal
        dialog via the ContainerEditDialog class below.
        
        Attributes:
            name  - name to be given to the container.
            image - Image of the container (apptainer image file).
            bindings - List of one or two element items that 
                        have the bindings for the container.  THe
                        first element of a binding is the source.  If the
                        second element is not present, the target is the same as the source.
                        If the second element is present, that's the target for the binding.
                        
                        Note that bindings are listed as one of:
                        source                     (one element binding)
                        sourcee -> target          (two element binding)
            initscript - This is a file whose contents will be sucked in as the initialization
                         script for the container. Usually this will do things like set up the DAQ env.

        The widget itself does not emit any signals, it has slots for signals from internal buttons.
       
       @todo Might be the regions of the widget would be even better as objects in their own right
            rather than just frames....they could derive from frames to provide the capability
            of borders 
    '''
    def __init__(self, parent : QObject | None  = None):
        super().__init__(parent)
        
        self._layout = QVBoxLayout(self)    # Primarly a stacked set of widgets.
        
        # There are fewer complaints if I put the sublayouts in frames.. otherwise
        # Qt6 seems to warn about widgets already haveing layouts when I addLayout
        #
        
        # Top frame is a strip containng the name of the container, an image
        # and a browse button for the container image ... pluse labesl.
        
        self._layout.addWidget(self._makeContainerCharacteristicsGui())
        
        # The next frame down is the bindings definition frame.
        # It has a from/to  browse (for the from) and add button.
        # The original had a remove, but this one will just let use remove
        # the selected binding from the list instead.
        
        self._layout.addWidget(QLabel('Define a binding:', self)) 
        self._layout.addWidget(self._makeBindingDefinitionFrame())
        
        # The bindings list is a frame with a listbox and a Remove button laid out
        # horizontally
        
        self._layout.addWidget(self._makeBindingsList())
        
        # Select the initialization script file
        
        
        self._layout.addWidget(self._makeInitScriptSelector())
        
        
        #  Finalize the layout.
        
        self.setLayout(self._layout)
        
        # We have slots for he pushbuttons in our widget.
        
        self._browseImage.clicked.connect(self._browseImageFile)
        self._browsefrom.clicked.connect(self._browseFrom)
        self._addBinding.clicked.connect(self._addBindingToList)
        self._removebinding.clicked.connect(self._removeCurrentBinding)
        self._browseScript.clicked.connect(self._browseInitScript)
        
    # Implement the attributes:
    
    def name(self) -> str:
        ''' Return the image name from the line edit it's stored in '''
        return self._containerName.text()
    def setName(self, name : str) -> None:
        ''' Set a new container name value:
            @param name - the new container name.
        '''
        self._containerName.setText(name)
    
    def image(self) -> str:
        ''' @return str - image file for the container'''
        return self._containerImage.text()

    def setImage(self, filepath : str) -> None:
        '''
            Set the contents of the container image edit line:
            
            @param filepath - new path to container image.
        '''
        self._containerImage.setText(filepath)
        
    def bindings(self) -> list[list[str, str] | list[str]]:
        '''
            @return a list of the binding definitions from the listbox.
               This is a list of one or two element sublists:
               -  A one element sublist is a binding of the form src:src
               - A two element sublist is a binding of the form src:tgt
                where src is the first sublist elmeent and tgt the second.
        '''
        # First get the raw bindings:
        raw_bindings = list()
        for row in range(self._bindinglist.count()):
            raw_bindings.append(self._bindinglist.item(row).text())
        
        return self._rawbindingsToBindingList(raw_bindings)
    
    def setBindings(self, bindings : list[list[str,str] | list[str]]) -> None:
        '''
            Set the contents of the bindings list.
            @param bindings - the bindings in the form described in the 'bindings' method.
        '''
        bindingsList = self._bindingsToTextList(bindings)
        _clearListWidget(self._bindinglist)
        self._bindinglist.addItems(bindingsList)
        
    def initscript(self) -> str:
        '''@return str - the path to the bindings list file from the lineedit.'''
        
        return self._initscriptfilename.text()
    
    def setInitsscript(self, path : str) -> None:
        ''' Set a new init script path
        
            @param path - new path to initscdript.
            @note initialization scripts get sucked into the 
                  container definition.
        '''
        self._initscript.setText(path)
        
    #   Private slots:    
        
    
    def _browseImageFile(self) -> None:
        # Browse for the image filename.
        
        image_path,_ = QFileDialog.getOpenFileName(
            self._browseImage, 'Select Image file', filter='Apptainer Images (*.img)'
        )
        if image_path.strip():
            self._containerImage.setText(image_path)
            
    def _browseFrom(self) -> None:
        # Browse for a from directory binding:
        # Note that this
        # Loads the from line edit and clears the two
        # so the default of binding to the same position in the filesystem is
        # maintained.
        
        from_dir = QFileDialog.getExistingDirectory(
            self._browsefrom, 'Select from directory'
        )
        if from_dir.strip():
            self._from.setText(from_dir)
            self._to.setText('')
            
    def _addBindingToList(self) -> None:
        #  Add the current binding to the list. 
        #  If the from string is empty, this is a no-op.
        
        from_str = self._from.text().strip()
        if from_str:
            binding = from_str
            to_str = self._to.text().strip()
            if to_str:
                binding +=  ' -> ' + to_str
                
            self._bindinglist.addItem(binding)
            
            # Clear the text edits for the next one:
            
            self._from.setText('')
            self._to.setText('')
    def _removeCurrentBinding(self) -> None:
        #  Remove the seleted binding from the list.
        
        self._bindinglist.takeItem(self._bindinglist.currentRow())
    
    def _browseInitScript(self) -> None:
        # Browse for a container initialization script.
        
        init_script,_ = QFileDialog.getOpenFileName(
            self._browseScript, 'Select init script',
            filter='Script file (*.bash *.sh)'
        )
        if init_script.strip() :
            self._initscriptfilename.setText(init_script)
    
    # Utilities:
    
    def _rawbindingsToBindingList(self, raw : list[str]) -> list[list[str] | list[str,str]]:
        # Convert the list of bindings strings into the 
        # list of one/two element lists
        result = list()
        
        for bstring in raw:
            item = list()
            elements = bstring.split('->')
            item.append(elements[0].strip())
            
            # Only add a second element if there's a 2 element list
            
            if len(elements) == 2:
                item.append(elements[1].strip())
            result.append(item)

        return result

    def _bindingsToTextList(self, bindings : list[list[str] | list[str,str]]) -> list[str]:
        # Convert the from/to bindings list into a simple textual list.
        # 
        
        result = list()
        for binding in bindings:
            item = binding[0]
            if len(binding) == 2:
                item += ' -> ' + binding[1]
            result.append(item)
        return result
        
    #  Utilities that make building the UI a bit simpler to decode:
    
    def _makeContainerCharacteristicsGui(self) -> QFrame:
        # Make and populoate the frame for the container name and image:
        
        
        self._containerchars = QFrame(self)
        self._containerlayout = QHBoxLayout(self._containerchars)
        self._containerchars.setLayout(self._containerlayout)
         # container name.
        
        self._containerlayout.addWidget(QLabel('Name: ', self._containerchars))
        self._containerName = QLineEdit(self._containerchars)
        self._containerlayout.addWidget(self._containerName)
         
         #container image:
         
        self._containerlayout.addWidget(QLabel('Image: ', self._containerchars))
        self._containerImage  = QLineEdit(self._containerchars)
        self._containerlayout.addWidget(self._containerImage)
        self._browseImage = QPushButton('Browse...', self._containerchars)
        self._containerlayout.addWidget(self._browseImage)
        
        return self._containerchars
        
    def _makeBindingDefinitionFrame(self) -> QFrame:
        # Make and populate the frame for defining a binding.
        
        self._bindingdef = QFrame(self)
        self._bindingdeflayout = QHBoxLayout(self._bindingdef)
        self._bindingdef.setLayout(self._bindingdeflayout)
        
        self._bindingdeflayout.addWidget(QLabel('From: ', self._bindingdef))
        self._from  = QLineEdit(self._bindingdef)
        self._bindingdeflayout.addWidget(self._from)
        self._browsefrom = QPushButton('Browse...', self._bindingdef)
        self._bindingdeflayout.addWidget(self._browsefrom)
        self._bindingdeflayout.addWidget(QLabel('To: ', self._bindingdef))
        self._to  = QLineEdit(self._bindingdef)
        self._bindingdeflayout.addWidget(self._to)
        self._addBinding = QPushButton('Add', self._bindingdef)
        self._bindingdeflayout.addWidget(self._addBinding)
        
        return self._bindingdef
    
    def _makeBindingsList(self) -> QFrame:
        self._bindings = QFrame(self)
        self._bindinglayout = QHBoxLayout(self._bindings)
        self._bindings.setLayout(self._bindinglayout)
        
        self._bindinglist = QListWidget(self._bindings)
        self._bindinglayout.addWidget(self._bindinglist)
        self._removebinding = QPushButton('Remove', self._bindinglist)
        self._bindinglayout.addWidget(self._removebinding)       
        
        return self._bindings
        
    def _makeInitScriptSelector(self) -> QFrame:
        self._initscript = QFrame(self)
        self._initscriptlayout = QHBoxLayout(self._initscript)
        self._initscript.setLayout(self._initscriptlayout)
        
        self._initscriptlayout.addWidget(QLabel('Init Script:', self._initscript))
        self._initscriptfilename = QLineEdit(self._initscript)
        self._initscriptlayout.addWidget(self._initscriptfilename)
        self._browseScript = QPushButton('Browse...', self._initscript)
        self._initscriptlayout.addWidget(self._browseScript) 
        
        return self._initscript
 
class ContainerEditDialog(QDialog):
    '''
        This will be a modal dialog with a ContainerEdit in the
        work area.  Rather than delegating all of the attributes of that
        dialog, we just supply getEditor to get a reference to it.
        
        Properties:
            workArea - gets the editor object.
    
        Normal usage is to construct one, populate its work area
        exec() it and then query the work area if  exec() returned
        QDialog.Accepted.  
    '''     
    def  __init__(self, parent=None):
        super().__init__(parent)
        
        self._layout = QVBoxLayout(self)
        self._workarea = ContainerEdit(self)
        self._layout.addWidget(self._workarea)
        
        self._buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Save | QDialogButtonBox.StandardButton.Cancel, self)
        self._layout.addWidget(self._buttons)
        
        self._buttons.accepted.connect(self.accept)
        self._buttons.rejected.connect(self.reject)
        
        self.setLayout(self._layout)
    
    def workArea(self) -> ContainerEdit:
        return self._workarea
    
        
# For now test code:

if __name__ == '__main__':
    from PyQt6.QtWidgets import QApplication
    import sys
    cheat = None
    def new():
        print('make a new container')
    def edit(name):
        dialog = ContainerEditDialog(win)
        dialog.workArea().setName(name)
        if dialog.exec() == QDialog.DialogCode.Accepted:
            print('Accepted')
            editor = dialog.workArea()
            print('name' , editor.name())
            print('image', editor.image())
            print('bindings', editor.bindings())
            print('initscript path', editor.initscript())
        else:
            print('cancelled')

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
        
        
        
        
        
        
        
        