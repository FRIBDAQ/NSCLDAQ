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

This file provides a pythonized version of the Readout wizard.
It simplifies the creation of Readout programs and the helpers they
have to run.

@file mg_readout_wizard.py
@purpose simplify definition of readout programs and helpers
@author Ron Fox
'''


from PyQt6.QtWidgets import (QWizard, QWizardPage, QTextEdit, QVBoxLayout, QLineEdit, 
    QComboBox, QPushButton, QLabel, QHBoxLayout, QFileDialog
)
from PyQt6.QtCore    import QObject, pyqtSignal
import getpass

class IntroPage(QWizardPage):
    '''
        Just provides introductory text.
        This will be registered with page id 1 and always
        branches to page id 2.
        
    '''
    
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
    def initializePage(self) -> None:
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        self._intro  = QTextEdit(self)
        self._intro.setReadOnly(True)
        self._layout.addWidget(self._intro)
        
        # Fill the intro text:
        
        self._intro.setHtml('''
<p>
This wizard helps you configure readout programs and their
helpers.  The helper programs do things like ensure the readout
program knows the run number and title prior to running, as well
as informing the readout programs of state changes in the experiment.
</p>
<p>
This wizard applies to the FRIB/NSCLDAQ managed environment. 
Initially, you'll be prompted for values that are independent
of the type of readout and the readout type.  We support 
generating the configuration for the following Readout program types:
</p>
<ul>
    <li>XIA/DDAS Readout system</li>
    <li>VMUSB VME with Wiener/Jtec USB controller</li>
    <li>CCUSB CAMAC with Wiener/Jtec USB controller</li>
    <li>MVLC VME via the Mesytec MVLC  controller 
         (requires mesytec-mvlc installation with FRIB extensions)</li>
    <li>Custom readout (arbitrary Readout program).</li>
</ul>
<p>
Click the next button when you are ready to continue.
</p>
''')
        self.setTitle("Introduction:")
    def nextid(self) -> int:
        ''' stub for now:'''
        return 2
    
    def  pageId(self) -> int:
        '''' All our wizard pages will know their own page id and next id.'''   
        return 1
        
class CommonReadoutInfo(QWizardPage):
    ''' 
        There's a lot of common information that does not
        change from Readout type to readout type.  This is prompted
        for on this page, along with the Readout type. Our nextid will
        wind up depending on the value of the ReadoutType combobox.
        Fields:
            Name      - name of the readout program.
            Container - The Container to use.
            Host      - Host in which to run the programs.
            Directory - The CWD for the programs.
            RestService - The ReST service advertised by the Readout.
            RingBuffer - Name of the ring buffer Readout writes data to.
            SourceId   - Source id.
            User       - User that will run the manager/Readout.
            ReadoutType - The type of readout  that will be run (determines the next page id
                          See the ReadoutWizard docstring).
                          
        Attributes which the main wizard will need to expose in some way:
            containers - Valid container names.
                
            We'll try to set the combobox fields to be the current text rather than current index.
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        self._containerlist = []
        
    def initializePage(self) -> None:
        self.setTitle('Common parameters')
        self._layout = QVBoxLayout(self)
        self.setLayout(self._layout)   # Our usual vertical stack of strips.
        
        # Name of readout (Name Field):
        
        nameLayout = QHBoxLayout()
        nameLayout.addWidget(QLabel('Readout Name:', self))
        self._name = QLineEdit(self)
        self.registerField('Name', self._name)
        nameLayout.addWidget(self._name)
        
        self._layout.addLayout(nameLayout)
        
        # Container, host and cwd (browsable).
        
        environLayout = QHBoxLayout()
        environLayout.addWidget(QLabel('Container', self))
        self._container = QComboBox(self)
        self._container.addItems(self._containerlist)
        self.registerField('Container', self._container, 'currentText', QComboBox.currentTextChanged)
        environLayout.addWidget(self._container)
        
        environLayout.addWidget(QLabel('Host', self))
        self._host = QLineEdit(self)
        self._host.setText('            ')
        self.registerField('Host', self._host)
        environLayout.addWidget(self._host)
        
        environLayout.addWidget(QLabel('Working Dir', self))
        self._wd = QLineEdit(self)
        self._wd.setText('            ')
        self.registerField('Directory', self._wd)
        environLayout.addWidget(self._wd)
        self._browseWd = QPushButton('Browse...', self)
        environLayout.addWidget(self._browseWd)
        
        self._layout.addLayout(environLayout)
        
        #  How the Readout outputs data:
        
        outputLayout = QHBoxLayout()
        outputLayout.addWidget(QLabel('Output Ring', self))
        self._ring = QLineEdit(self)
        self.registerField('RingBufer', self._ring)
        self._ring.setText(getpass.getuser())
        outputLayout.addWidget(self._ring)
        
        outputLayout.addWidget(QLabel('Source ID', self))
        self._sourceid = QLineEdit(self)
        self.registerField('SourceId', self._sourceid)
        outputLayout.addWidget(self._sourceid)
        
        self._layout.addLayout(outputLayout)
        
        # ReST service information:
        
        restLayout  = QHBoxLayout()
        restLayout.addWidget(QLabel('ReST service', self))
        self._service = QLineEdit(self)
        self.registerField('RestService', self._service)
        self._service.setText('RedoutREST')
        restLayout.addWidget(self._service)
        
        restLayout.addWidget(QLabel('Manager username', self))
        self._user = QLineEdit(self)
        self.registerField('User', self._user)
        self._user.setText(getpass.getuser())
        restLayout.addWidget(self._user)
        
        self._layout.addLayout(restLayout)
        
        #  Finally the combobox with the Readout types:
        
        typeLayout = QHBoxLayout()
        typeLayout.addWidget(QLabel('Readout Type', self))
        self._readoutType = QComboBox(self)
        self.registerField('ReadoutType', self._readoutType, 'currentText', QComboBox.currentTextChanged)
        self._readoutType.addItems(['XIA/DDAS', 'VMUSB', 'MVLC', 'CCUSB', 'Custom'])
        typeLayout.addWidget(self._readoutType)
        
        self._layout.addLayout(typeLayout)
        
        
        # The browse button needs to browse directories:
        
        self._browseWd.clicked.connect(self._browseDir)
    # Attribute implementations:
    
    def containers(self) -> list[str]:
        ''' Return the containers in the combobox.'''    
       
        return self._containerlist
    
    def setContainers(self, containers : list[str]) -> None:
        self._containerlist = containers
    
    # Page navigation:
    
    def nextid(self) -> int:
        # The next page id depends on the value of the 'ReadoutType field.
        match self.wizard().field('ReadoutType'):
            case 'XIA/DDAS':
                return 100
            case _:
                return -1
            
    def pageId(self) -> int:
        return 2
    
    # Internal (private) slots:
    
    def _browseDir(self) -> None:
        # Browse for the home directory.
        
        dir = QFileDialog.getExistingDirectory(self, 'Choose Working Directory')
        if dir.strip():
            self._wd.setText(dir)
class ReadoutWizard(QWizard):
    '''
        Readout configuration wizard.  Note that this wizard
        is branching:
        Page ids starting at 100 are the XIA/DDAS configuration.
        Page ids starting at 200 are the VMUSB/CCUSB configuration
        Page ids starting at 300 are the MVLC configuration.
        Page ids starting at 400 are custom readouts.
        
        Page ids below that are common initial pages
        
        Attributes:
        containers - List of containers the user can choose from.
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # Register the pages and their ids:
        
        # Common pages:
        
        self._intro = IntroPage(self)
        self.setPage(self._intro.pageId(), self._intro)
        self.setStartId(self._intro.pageId())
        
        self._commonInfo = CommonReadoutInfo(self)
        self.setPage(self._commonInfo.pageId(), self._commonInfo)
        
    def containers(self) -> list[str]:
        ''' @return list[str] - list of containers that are available.'''
        return self._commonInfo.containers()
    def setContainers(self, containers : list[str]) -> None:
        self._commonInfo.setContainers(containers)
    

##  Test code for now:

if __name__ == "__main__":
    from PyQt6.QtWidgets import QApplication
    import sys

    def done():
        print("accepted:")
        print('Name', wiz.field('Name'))
        print('Container', wiz.field('Container'))
        print('Host', wiz.field('Host'))
        print('Directory', wiz.field('Directory'))
        print('Service', wiz.field('RestService'))
        print('Ring', wiz.field('User'))
        print('SrcId', wiz.field('SourceId'))
        print('User', wiz.field('User'))
        print('Type: ', wiz.field('ReadoutType'))
        
    app = QApplication(sys.argv)
    wiz = ReadoutWizard()
    wiz.accepted.connect(done)
    wiz.setContainers(['bookworm', 'bullseye', 'buster', 'jessie'])
    wiz.show()
    
    sys.exit(app.exec())
        
        