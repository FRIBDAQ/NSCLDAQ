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
Contains code to configure programs in the managed
experiment envirohnment.   The user interface
constists of a list of the programs that are defined
in a table and a New... button. Double clicking
a program loads it into an editor dialog. 
Clicking New... brings the program generation wizard
the user to define all of the characteristics of a program.


'''


from PyQt6.QtWidgets import (
    QWidget,  QVBoxLayout, QHBoxLayout, QPushButton, QListWidget, QListWidgetItem,
    QTableView, QFrame, QLabel, QLineEdit, QComboBox, QGroupBox, QRadioButton)
from PyQt6.QtGui import (QStandardItemModel, QStandardItem)
from PyQt6.QtCore import pyqtSignal, QModelIndex, QObject


class ProgramSelector(QWidget):
    '''
        This widget consists of a table of
        program definitions followed by a New.. button.
        
        For simplicity, this widget includes the table model
        and simple methods that allow its maniplation specifically:
        
    
        Properties:
        programs - The programs in the table.  These are a list of dicts with the
              keys:
              * 'name' - Name of the program.
              * 'path' - Path to the file that's run for the program.
              * 'host' - Host in which the program is run.
              * 'container' name of the container in which the program is run.
    
    Signals:
        edit(str) - An item in the table was double clicked, the string is the
                    name of the program.
        new      - new pushbutton was clicked.
    '''
    edit = pyqtSignal(str)
    new = pyqtSignal()
    
    def __init__(self, parent : QObject = None):
        super().__init__(parent)
        
        # Table of programs:
        
        self._model = QStandardItemModel(self)
        self.setPrograms(list())
        
        self._list = QTableView(self)
        self._list.setModel(self._model)
        
        self._newButton = QPushButton('New...', self)
        
        # Layout in a vertical box:
        
        self._layout = QVBoxLayout(self)
        self._layout.addWidget(self._list)
        self._layout.addWidget(self._newButton)
        
        self.setLayout(self._layout)
        
        # Hook signals:
        
        self._newButton.clicked.connect(self.new)   # Just relay new.
        self._list.doubleClicked.connect(self._editRequested)
        
    # Private slots.
    
    def _editRequested(self, index : QModelIndex) -> None:
        # Figure ot the name of the program that was double clicked
        # and emit edit:
        row = index.row()
        name = self._model.item(row, 0).text()
        
        self.edit.emit(name)
        
        
    # Attributes:
    #
    
    def programs(self) -> list[dict]:
        '''
            Return the programs that have been loaded into the
            table's model.  See the class docstring for what the
            contents of the dicts will look like:
            
        @return list[dict]  see above.
        '''
        result= list()
        for row in range(self._model.rowCount()): 
            name = self._model.item(row, 0).text()
            image = self._model.item(row, 1).text()
            host  = self._model.item(row, 2).text()
            container = self._model.item(row, 3).text()
            
            result.append({
                'name' : name, 'path' : image, 'host' : host,
                'container': container
            })
        return result
    
    def setPrograms(self, programs : list[dict]) -> None:
        '''
            Replaces the contents of the model with the programs defined in
            the programs parameter.
            
            @param programs - iterable of dicts that define the programs
               to load. See the class docstring for a description of
               these dicts.
        '''
        self._model.clear()
        self._model.setHorizontalHeaderLabels([
            'Name', 'Program File', 'Host', 'Container'
        ])
        for program in programs:
            name = QStandardItem(program['name']) 
            image = QStandardItem(program['path'])
            host  = QStandardItem(program['host'])
            container = QStandardItem(program['container'])
            
            self._model.appendRow([
                name, image, host, container
            ])
class ValueBox(QWidget):
    '''
     This widget is a list box that has names that can be edited.
     It provides a listbox for the items and an entry for
     adding new items.  A pushbutton adds the text in the
     line edit to the list. double clicking an item
     loads it into the line edit and removes it from the list.
     All this is done autonomously providing only the 
     attributes:
     
     title - A title label displayed above the listbox.
     items - The items in the list box.
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        self._layout = QVBoxLayout(self)
        self.setLayout(self._layout)
        
        self._title = QLabel(self)
        self._layout.addWidget(self._title)
        
        self._list = QListWidget(self)
        self._layout.addWidget(self._list)
        
        valuelayout = QHBoxLayout()
        valuelayout.addWidget(QLabel('Value:', self))
        self._value = QLineEdit(self)
        valuelayout.addWidget(self._value)
        self._layout.addLayout(valuelayout)
        
        self._new = QPushButton('Add', self)
        self._layout.addWidget(self._new)
        
    # Attributes:
    
    def title(self) -> str:
        ''''
            @return str - title value.
        '''
        return self._title.text()
    def setTitle(self, title : str) -> None:
        '''
            @param title - new title string.
        '''
        self._title.setText(title)
    
class NameValueBox(QWidget):
    '''
        This widget is a listbox that has name=value stuff
        in it. It includes line edits for names and values
        and a push button that autonomousle adds a 
        value to the list.  Double clicking a value in the
        list, loads it into the editor and removes it from
        the list.  This also allows you to remove items
        from the list completely.
        
        Attributes:
            title - A title label displayed above the list.
            items - list of name value pairs in the listbox.
    
        Intended for use as the program options and 
        environment editors in the program editor below
        but could be used for other stuff too.
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._layout = QVBoxLayout(self)
        self.setLayout(self._layout)
        
        self._title = QLabel(self)
        self._layout.addWidget(self._title)
        
        self._list   = QListWidget(self)
        self._layout.addWidget(self._list)
        
        namelayout = QHBoxLayout()
        namelayout.addWidget(QLabel('Name:', self))
        self._name  = QLineEdit(self)
        namelayout.addWidget(self._name)
        self._layout.addLayout(namelayout)
        
        valuelayout = QHBoxLayout()
        valuelayout.addWidget(QLabel('Value', self))
        self._value = QLineEdit(self)
        valuelayout.addWidget(self._value)
        self._layout.addLayout(valuelayout)
        
        self._new = QPushButton('Add', self)
        self._layout.addWidget(self._new)
        
        self._new.clicked.connect(self._addItem)
        self._list.itemDoubleClicked.connect(self._editItem)
        

    # Attribute implementations.
    
    def title(self) -> str:
        '''
        @return str the title attribute
        '''
        return self._title.text()
    def setTitle(self, title : str) -> None:
        ''''
        @param title - the new value of the title attribute/widget text.
        '''
        self._title.setText(title)
    def items(self) -> list[tuple[str, str]]:
        '''
            Return the name/value pairs currently  in the
            list box.
            
            @return list of name value pairs (tuples).
        '''
        result = list()
        for row in range(self._list.rowCount()):
            item = self._list.item(row)
            (name,value) = item.split('=')
            result.append((name, value))
        
        return result
    
    def setItems(self, nameValuePairs : list[tuple[str,str]]) -> None:
        '''
            @param nameValuePairs - an iterable of name value pairs. to load
                 into the listbox.
        '''
        self._list.clear()
        for item in nameValuePairs:
            self._list.addItem(f'{item[0]}={item[1]}')
    
    #internal slots.
    
    def _addItem(self) -> None:
        # To operate both the name and values must be nonempty.
        
        name = self._name.text().strip()
        value = self._value.text().strip()
        
        if name and value:
            self._list.addItem(f'{name}={value}')
            # Clear the text entries to make adding another simpler
            
            self._name.setText('')
            self._value.setText('')
            
    def _editItem(self, item : QListWidgetItem) -> None:
        # Load the double clicked item into the editor
        # and remove it from the list.
        
        (name, value) = item.text().split('=')
        row = self._list.row(item)
        self._list.takeItem(row)
        
        self._name.setText(name)
        self._value.setText(value)
        
        
        
class ProgramEditor(QWidget):
    '''
        This is a form that can be used to edit a program
        definition.  Normally it is the widget that is displayed
        in the ProgramEditorDialog but is separate here to
        * Make that dialog easier to build 
        * Allow it to be used outside of the dialog if later desired.
        
    Attributes:
        name - the program name.
        path - what's run when the program is run.,
        host - where the program is run.
        container - Container in which the program is run.
        containers - THe containers the user can select from.
        wd   - Working directory in which the program is run.
        type - program type, one of 'Transitory', 'Critical' or 'Persistent'
        options - list of option/value pairs e.g. [['--ring', 'ringname'], ['--id', '2'],...]
        parameters - List of program parameters that don't have values
        environment - List of name/value pairs in the environment e.g. 
            [['TCLLIBPATH', '/usr/opt/daq/12.2-009'], ...]
        
    Slots
        addOption(name: str, value: str) - add a new option to the options list.
        addParameter(param: str)         - add a new program parameter to the parameter list.
        addToEnvironment(name: str, value : str) - add a new environment variable to the listbox.
    '''
    def __init__(self, parent : QObject | None = None) :
        super().__init__(parent)
        
        # The layout is a vertical stack of frames:
        
        self._layout = QVBoxLayout(self)
        self.setLayout(self._layout)
        
        self._layout.addWidget(self._createIdFrame())
        self._layout.addWidget(self._createHowFrame())
        self._layout.addWidget(self._createWdFrame())
        self._layout.addWidget(self._createProgramTypeFrame())
        self._layout.addWidget(self._createEnvironmentFrame())
    
    # Gui segment creating utilitty methods:
    
    def _createIdFrame(self) -> QFrame:
        #  Create's a frame with the labels and controls to
        # identify the program.
        
        self._idframe = QFrame(self)
        idlayout = QHBoxLayout(self._idframe)
        self._idframe.setLayout(idlayout)
        
        idlayout.addWidget(QLabel('Name:', self._idframe))
        self._name = QLineEdit(self._idframe)
        idlayout.addWidget(self._name)
        
        idlayout.addWidget(QLabel('Program File:', self._idframe))
        self._path = QLineEdit(self._idframe)
        idlayout.addWidget(self._path)
        
        return self._idframe

    def _createHowFrame(self) -> QFrame:
        #  Create a frame that contains the controls
        # for how the program is run (host and container name.)
        
        frame = QFrame(self)
        layout = QHBoxLayout(frame)
        frame.setLayout(layout)
        
        layout.addWidget(QLabel('Host: ', frame))
        self._host = QLineEdit(frame)
        layout.addWidget(self._host)
        
        layout.addWidget(QLabel('Container', frame))
        self._container = QComboBox(frame)
        layout.addWidget(self._container)
        
        self._howframe = frame
        return frame
    def _createWdFrame(self) -> QFrame:
        # Create the controls to seleect the working directory:
        
        frame = QFrame(self)
        layout = QHBoxLayout(frame)
        frame.setLayout(layout)
        
        layout.addWidget(QLabel('Working Directory:', frame))
        self._wd = QLineEdit(frame)
        layout.addWidget(self._wd)
        self._wdbrowse = QPushButton('Browse...', frame)
        layout.addWidget(self._wdbrowse)
                
        self._wdframe  = frame
        return frame

    def _createProgramTypeFrame(self) -> QGroupBox:
        # create controls to select the program type:
        
        frame = QGroupBox(self)
        frame.setTitle('Program Type')
        frame.setFlat(False)    # Let's see what that looks like
        layout = QHBoxLayout(frame)
        frame.setLayout(layout)
        
        self._critical = QRadioButton('Critical', frame)
        self._persistent= QRadioButton('Persistent', frame)
        self._transitory= QRadioButton('Transitory', frame)
        for widget in (self._critical, self._persistent, self._transitory):
            layout.addWidget(widget)
        
        self._pgmtypeframe = frame
        return frame
    
    def _createEnvironmentFrame(self) -> QFrame:
        frame  = QFrame(self)
        layout = QHBoxLayout(frame)
        
        self._options = NameValueBox(frame)
        self._options.setTitle('Program Options')
        layout.addWidget(self._options)
        
        
        self._parameters = ValueBox(frame)
        self._parameters.setTitle('Program Parameters')
        layout.addWidget(self._parameters)
        
        self._environment = NameValueBox(frame)
        self._environment.setTitle('Program Environment')
        layout.addWidget(self._environment)
        
        self._envframe = frame
        return frame
        
# test code (for now)

if __name__ == '__main__':
    
    def create() -> None:
        print("Make a new program.")
        
    editor = None
    def edit(name : str) -> None:
        global editor
        editor = ProgramEditor(None)
        editor.show();
    
    from PyQt6.QtWidgets import QApplication
    import sys

    app = QApplication(sys.argv)
    win = ProgramSelector()
    
    # Fake program defs (enough to make win.setPrograms work).
    
    fake_programs = [
        {'name': 'george', 
         'path' : '/usr/opt/daq/12.2-009/bin/ddasReadout', 
         'host' : 'localhost',
         'container': 'bookworm-12.2-009'},
        {'name' : 'shemp', 
         'path' : '/home/ron/bin/Readout',
         'host' : 'localhost',
         'container' : 'bullseye-11.3-021'}
    ]
    win.setPrograms(fake_programs)
    win.new.connect(create)
    win.edit.connect(edit)
    
    
    win.show()
    sys.exit(app.exec())
