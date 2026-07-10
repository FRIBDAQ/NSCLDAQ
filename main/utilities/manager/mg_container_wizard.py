#!/usr/bin/env/python3

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
  This file provides a wizard that lets you define containers in
  the managed experiment configuration.  A container is a container
  image and a set of bindings to that image that are assigned a name.
  Containers also have an initialization script which is run when the
  container is started.   The container script is loaded into the database.
  
'''


from PyQt6.QtWidgets import (QWizard, QWizardPage, QTextEdit, QVBoxLayout, QApplication,
    QListWidget, QLabel, QPushButton, QLineEdit, QFileDialog, QHBoxLayout, QTextEdit,
    QMessageBox)
from PyQt6.QtCore import pyqtProperty

from nscldaq.mg_database import Container

from collections import namedtuple
import sys, os, pathlib
import configparser
import sqlite3
import tempfile

Constants = namedtuple('Constants',
    ['DESCRIPTION_FILENAME',
    'DESCRIPTION_SEARCH_PATHS']
)


if 'CONTAINER_CNFIG' in os.environ:
    spath = os.environ['CONTAINER_VERSION']
else:
    spath = list()
spath += [os.getcwd(), str(pathlib.Path.home()), '/usr/opt', '/user/opt/non-container']

    
CONSTANTS = Constants(
    DESCRIPTION_FILENAME = 'containers.ini',
    DESCRIPTION_SEARCH_PATHS = spath
)

#-------------------------------------- The wizard and its pages 
class IntroPage(QWizardPage):
    '''
        This page of the wizard just provides
        introductory text that orients the user to the
        container wizard.
    '''
    def __init__(self, parent=None):
        super().__init__(parent)
    
    def initializePage(self) -> None:
        self.setTitle('Introduction')
        self._layout = QVBoxLayout(self)
        
        self._introText = QTextEdit(self)
        self._introText.setReadOnly(True)
        
        intro_text = '''
This wizard creates container definitions for
the managed experiment environment.  Using it you 
will:
<ul>
<li>Choose from a set of known container images to
set the linux environment your container will run.
</li>
<li>Choose the version of FRIB/NSCLDAQ your  experiment
will use when running in that container. This choice
will be made from the set of DAQ versions that are supported
in that container image.
</li>
<li>Select which directories in the host system will be bound 
into the container's filesystem and where they will appear.
Note that the /usr/opt file-system binding will have already
been done for you.
</li>
<li>If appropriate, edit the initialization script that will
be run when activating programs in that image.  The
starting point of this script will set up the environment
</li>
</ul>
        '''
        self._introText.setHtml(intro_text)    
        self._layout.addWidget(self._introText)
        
        # Prompt, as well for the name of a container:
        
        self._nameLabel = QLabel('Container name in experiment:', self)
        self._name      = QLineEdit(self)
        
        # Require the user name the container for the experiment.
        
        self._namelayout = QHBoxLayout(self)
        self._namelayout.addWidget(self._nameLabel)
        self._namelayout.addWidget(self._name)
        self._layout.addLayout(self._namelayout)
        
        self.registerField('econtainer*', self._name)
        
        self.setLayout(self._layout)
        
class ContainerSelectionPage(QWizardPage):
    
    def __init__(self,configuration, parent = None):
        super().__init__(parent)
        self._configuration = configuration
    
    def initializePage(self) -> None:
        # Enumerate the containers.  These are sections 
        # not named 'CONFIG'
        
        self._containers = dict()
        for section in self._configuration.sections():
            if section != 'CONFIG':
                container_name = section
                container_image = self._configuration[container_name]['path']
                container_usropt= self._configuration[container_name]['usropt']
                self._containers[container_name] = (container_image, container_usropt)
                
        # build a list box of containers.
        self.setTitle('Choose container')
        self._layout = QVBoxLayout(self)
        self.setLayout(self._layout)
        self._containerList = QListWidget(self)
        self._containerList.addItems(self._containers.keys())
        self._layout.addWidget(self._containerList)
        
        # Add a label at the bottom that will reflect the selected row.
        # That will get registered as a field for retrieval ('containername')
        
        self._selected = QLabel(self)
        self._layout.addWidget(self._selected)
        
        
        self.registerField('containername*', self._selected, "text")
        self._containerList.currentTextChanged.connect(self._updateContainer)
        
    def _updateContainer(self, t : str) -> None:
        self.wizard().setField('containername', t)
        self.completeChanged.emit()

class DAQSelectionPage(QWizardPage):
    # Select DAQ version. 
    def __init__(self, config, parent=None):
        # config is the read in configuration dict.
        
        super().__init__(parent)
        self._configuration = config
    
    def initializePage(self) -> None:
        self.setTitle('Select DAQ version')
        #
        #   The 'containername' field already
        #   has the container we've chosen.  Using
        #   the config, we know it's /usr/opt/ directory tree
        #   and can enumerate the daq directory to get the list of 
        #   versions:
        
        container_name = self.wizard().field('containername')
        usropt = self._configuration[container_name]['usropt']
        daqdir = pathlib.Path(usropt) / 'daq'
        
        # Make a listbox and populate it with all of the subdirs 
        # daqdir:
        
        self._layout = QVBoxLayout(self)
        self.setLayout(self._layout)
        self._list   = QListWidget(self)
        for name in daqdir.iterdir():
            if name.is_dir():
                self._list.addItem(str(name))
        
        self._layout.addWidget(self._list)
    
        # A label for the selected item..register the field and 
        # connect to the text changed signal to update the field
        # and let the wizard know that.
        
        self._selected = QLabel(self)
        self._layout.addWidget(self._selected)
        self.registerField('daqversion', self._selected, 'text')
        
        self._list.currentTextChanged.connect(self._update)
    
    def _update(self, t: str) -> None:
        # Update the field from the new selection and 
        # Let the wizard know:
        
        self.wizard().setField('daqversion', t)
        self.completeChanged.emit()

class BindingsSelectionPage(QWizardPage):  
    # Setup the list of bindings.
    # Note that we pre-populate the bindings for /usr/opt.
    
    def __init__(self, config, parent=None):
        super().__init__(parent)
        self._configuration = config
    
    def initializePage(self) -> None:
        self.setTitle('Create bindings list')
        
        # We can't do fields the way we want to.
        # We'll supply a method, the wizard will
        # relay to to provide the list of strings
        # in the bindings QListWidget.
        
        self._layout = QVBoxLayout(self)
        
        # List widget, pre-populated with the /usr/opt binding:
        
        self._bindingsList = QListWidget(self)
        self._populateInitialBinding()
        self._layout.addWidget(self._bindingsList)
        
        # labels, entries and browse buttons on line w:
        
        self._bindinglayout = QHBoxLayout(self)
        self._srclabel = QLabel('Source: ', self)
        self._bindinglayout.addWidget(self._srclabel)
        self._source  = QLineEdit(self)
        self._bindinglayout.addWidget(self._source)
        self._browse  = QPushButton('Browse...', self)
        self._bindinglayout.addWidget(self._browse)
        self._tgtlabel = QLabel('Bound at:', self)
        self._bindinglayout.addWidget(self._tgtlabel)
        self._target = QLineEdit(self)
        self._bindinglayout.addWidget(self._target)
        
        self._layout.addLayout(self._bindinglayout)
        
        # Binally, a pair of push buttons to add and delete bindings 
        # from the list:
        
        self._actionlayout = QHBoxLayout(self)
        self._add   = QPushButton('Add Binding', self)
        self._actionlayout.addWidget(self._add)
        self._delete = QPushButton('Remove selected', self)
        self._actionlayout.addWidget(self._delete)
        
        self._layout.addLayout(self._actionlayout)
        
        #  Add a note about blank destinations.
        
        self._notelabel = QLabel(
            'Note that if the "Bound At" entry is empty, it is the same as Source', 
            self
        )
        self._layout.addWidget(self._notelabel)
        self.setLayout(self._layout)        
        
        # Set up the button handlers:
        
        self._connectButtons()
        
        
        
    def bindings(self) -> list[str]:
        result = list()
        for index in range(self._bindingsList.count()):
            result.append(self._bindingsList.item(index).text())
        
        return result
    def setBindings(self, items) -> None:
        # Clear the items:
        
        for _ in range(self._bindingsList.count()):
            self._bindingsList.take(0)
            
        self._bindingsList.addItems(items)
        
    def _populateInitialBinding(self) -> None:
        #   Populate the bindings list with the usr/opt binding:
        
        container_name = self.wizard().field('containername')
        usropt         = self._configuration[container_name]['usropt']
        usropt += ':/usr/opt'
        self._bindingsList.addItem(usropt)

    def _connectButtons(self) -> None:
        # Connect button signal handlers:
        self._browse.clicked.connect(self._browseDirs)
        self._add.clicked.connect(self._addBinding)
        self._delete.clicked.connect(self._deleteSelected)
        
    # Private slots

    def _addBinding(self) -> None:
        # The sourcde must be non blank.  If the destination is also non-blank it's
        # It's appended along with a colon:
        # Both src and dest have extra blanks stripped:
        
        source = self._source.text().strip()
        if source:
            binding = source
            dest  = self._target.text().strip()
            if dest:
                binding += ':' + dest
            
            # Add to the list of bindings, and clear the src/dest:
            
            self._bindingsList.addItem(binding)
            self._source.setText('')
            self._target.setText('')
        
    def _deleteSelected(self) -> None:
        # Support deleting the selected item:
        
        self._bindingsList.takeItem(self._bindingsList.currentRow())
        
    def _browseDirs(self) -> None:
        #  Browse for a directory to put in the source of the bindings list.
        
        dir = QFileDialog.getExistingDirectory(self, 'Choose a source directory')
        if dir:
            self._source.setText(dir)
            
class InitializationScritPage(QWizardPage):
    #  Editor for the initialization script.
    #  This will get pre-stocked with the shebang
    #  for /usr/bin/bash
    #  and a source of the correct daqsetup.bash script.
    
    def __init__(self, configuration, parent=None):
        super().__init__(parent)
        
        self._configuration = configuration
    
    def initializePage(self) -> None:
        self.setTitle('Edit initialization script')
        self._layout = QVBoxLayout(self)
        self._help   = QLabel('This script is run to before executing programs in the container')
        self._layout.addWidget(self._help)
        
        
        self._editor = QTextEdit(self)
        self._layout.addWidget(self._editor)
        
        # Load the editor with the initial stuff:
        daqdir =  self.wizard().field('daqversion')
        self._editor.setPlainText(
f'''#!/usr/bin/bash
source {daqdir}/daqsetup.bash
'''
        )
        self.setLayout(self._layout)
        
        # Make the text editor a field.
        
        self.registerField('startscript', self._editor, 'plainText')
        self._editor.textChanged.connect(self.completeChanged.emit)
               
                
class ContainerWizard(QWizard):
    ''' 
        Encapsulates the entire wizard.
    '''
    def __init__(self, configuration, parent = None):
        ''' 
          @param configuration - the configuration read in from the ini file.
        '''
        
        super().__init__(parent)
        self._configuration = configuration
        # Setup the pages:
        
        self._intro = IntroPage(self)
        self.addPage(self._intro)
        
        self._containerChooser = ContainerSelectionPage(configuration, self)
        self.addPage(self._containerChooser)
    
        self._daqChooser = DAQSelectionPage(configuration, self)
        self.addPage(self._daqChooser)
        
        self._bindingsPage = BindingsSelectionPage(configuration, self)
        self.addPage(self._bindingsPage)
        
        self._initScriptPage = InitializationScritPage(configuration, self)
        self.addPage(self._initScriptPage)
        
        self.setWindowTitle("Container definition wizard")

    # Can't make fields lists so:
    
    def bindings(self) -> list[str]:
        return self._bindingsPage.bindings()   
    
# Utility functions:

def usage() -> None:
    #  Print out the program usage and exit with an error:
    
    print('''
Usage:
    $DAQBIN/mg_container_wizard  config-path
Where:
    config-path is the path to a managed experiment configuration Sqlite database file.
          ''', file = sys.stderr)
    sys.exit(-1)

def read_config_file() -> dict:
    # Search for the container configuration file
    # in all of the paths in CONSTANTS.DESCRIPTION_SEARCH_PATHS
    # if not found an error is raised. If found,
    # the file is read and returned as a dict.

    for directory in CONSTANTS.DESCRIPTION_SEARCH_PATHS:
        filename = pathlib.Path(directory) / CONSTANTS.DESCRIPTION_FILENAME
        if filename.is_file():
            parser = configparser.ConfigParser()
            parser.read(str(filename))            
            return parser
    raise FileNotFoundError('The container configuration file could not be found')

def convert_bindings(raw_bindings : list[str]) -> list:
    # We have to convert the bindings from the wizard into the
    # form expected by the database add method.  That's
    # a list of either singles or pairs.
    
    return [x.split(':') for x in raw_bindings]

def done(r: int, config : dict, win: QWizard) -> None:
    # Save the configured container if the wizard finished with Finish:
    
    if r == 1:
        
        econtainer = win.field('econtainer')
        cfgcontainer = win.field('containername')
        bindings     = convert_bindings(win.bindings())
        startscript = win.field('startscript')
        
        handle = sqlite3.connect(sys.argv[1])    # Connect to the database file.
        # The api expects the init script in a file... we'll stuff it in a tempfile.
        
        script = tempfile.NamedTemporaryFile(mode='w', encoding='utf-8')
        scriptname = script.name
        with  script.file as f:
            print(startscript, file=f)
            f.flush()
            scriptname = script.name
            database = Container(handle)
            
            # Prompt for deletion if it exists:
            
            if database.exists(econtainer):
                if QMessageBox.question(
                    win, 'Container exists', 
                    f'There is already a container named {econtainer}. Replace it?'
                ) == QMessageBox.StandardButton.Yes:
                    database.remove(econtainer)
                else:
                    QApplication.quit()
                    return
            
            # Shoulid be able to add the container:
            
            database.add(econtainer, config[cfgcontainer]['path'], scriptname, bindings)
    QApplication.quit()

# Main entry point.

def main() -> int:
    
    # Ensure we have a database filename and
    # It exists:
    
    if len (sys.argv) != 2:
        usage()
    
    
    
    config = read_config_file()

    app = QApplication(sys.argv)
    app.setQuitOnLastWindowClosed(True)
    win = ContainerWizard(config)
    win.show()
    win.finished.connect(lambda r : done(r, config, win))
    
    return app.exec()
    


if __name__ == '__main__':
    sys.exit(main())