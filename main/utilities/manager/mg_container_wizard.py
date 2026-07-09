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
    QListWidget, QLabel)
from collections import namedtuple
import sys, os, pathlib
import configparser

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
    
        
        
        self.setWindowTitle("Container definition wizard")
        
    

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

def done(r: int, win: QWizard) -> None:
    if r == 1:
        print('Container: ', win.field('containername'))

def main() -> int:
    config = read_config_file()
    print(config.sections())

    app = QApplication(sys.argv)
    win = ContainerWizard(config)
    win.show()
    win.finished.connect(lambda r : done(r, win))
    
    return app.exec()
    


if __name__ == '__main__':
    sys.exit(main())