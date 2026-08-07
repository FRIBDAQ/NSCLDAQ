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


import getpass
import sqlite3
import sys
from typing import Protocol

from nscldaq import mg_database
from nscldaq.mg_configutils import EditableTable
from PyQt6.QtCore import QObject, Qt, pyqtSignal
from PyQt6.QtGui import QIntValidator
from PyQt6.QtWidgets import (
    QApplication,
    QCheckBox,
    QComboBox,
    QFileDialog,
    QGridLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QRadioButton,
    QSpinBox,
    QTextEdit,
    QVBoxLayout,
    QWizard,
    QWizardPage,
)


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
    def nextId(self) -> int:
        ''' stub for now:'''
        return 2
    
    def  pageId(self) -> int:
        '''' All our wizard pages will know their own page id and next id.'''   
        return 1
        
class CommonReadoutInfo(QWizardPage):
    ''' 
        There's a lot of common information that does not
        change from Readout type to readout type.  This is prompted
        for on this page, along with the Readout type. Our nextId will
        wind up depending on the value of the ReadoutType combobox.
        Fields:
            Name      - name of the readout program.
            Container - The Container to use.
            Host      - Host in which to run the programs.
            ManagerHost - Where the manager will be running.
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
        self.registerField('Name*', self._name)
        nameLayout.addWidget(self._name)
        
        self._layout.addLayout(nameLayout)
        
        # Container, host and cwd (browsable).
        
        environLayout = QHBoxLayout()
        environLayout.addWidget(QLabel('Container', self))
        self._container = QComboBox(self)
        self._container.addItems(self._containerlist)
        self.registerField('Container', self._container, 'currentText', QComboBox.currentTextChanged)
        environLayout.addWidget(self._container)
        
        environLayout.addWidget(QLabel('Readout Host', self))
        self._host = QLineEdit(self)
        self.registerField('ReadoutHost*', self._host)
        environLayout.addWidget(self._host)
        self._layout.addLayout(environLayout)
        
        workdirLayout= QHBoxLayout()
        workdirLayout.addWidget(QLabel('Working Dir', self))
        self._wd = QLineEdit(self)
        self.registerField('Directory*', self._wd)
        workdirLayout.addWidget(self._wd)
        self._browseWd = QPushButton('Browse...', self)
        workdirLayout.addWidget(self._browseWd)
        
        self._layout.addLayout(workdirLayout)
        
        
        
        #  How the Readout outputs data:
        
        outputLayout = QHBoxLayout()
        outputLayout.addWidget(QLabel('Output Ring', self))
        self._ring = QLineEdit(self)
        self.registerField('RingBuffer*', self._ring)
        self._ring.setText(getpass.getuser())
        outputLayout.addWidget(self._ring)
        
        outputLayout.addWidget(QLabel('Source ID', self))
        self._sourceid = QLineEdit(self)
        self._sidValidator = QIntValidator()
        self._sidValidator.setBottom(0)
        self._sourceid.setValidator(self._sidValidator)
        self.registerField('SourceId', self._sourceid)
        self._sourceid.setText('0')
        outputLayout.addWidget(self._sourceid)
    
        self._layout.addLayout(outputLayout)
        
        # ReST service information:
        
        restLayout  = QHBoxLayout()
        restLayout.addWidget(QLabel('ReST service', self))
        self._service = QLineEdit(self)
        self.registerField('RestService', self._service)
        self._service.setText('ReadoutREST')
        restLayout.addWidget(self._service)
        
        self._layout.addLayout(restLayout)
        
        managerLayout = QHBoxLayout()
        
        managerLayout.addWidget(QLabel('Manager username', self))
        self._user = QLineEdit(self)
        self.registerField('User', self._user)
        self._user.setText(getpass.getuser())
        managerLayout.addWidget(self._user)
        
        
        
        managerLayout.addWidget(QLabel('Manager host', self))
        self._mgrHost = QLineEdit(self)
        self.registerField('ManagerHost*', self._mgrHost)
        managerLayout.addWidget(self._mgrHost)
        self._layout.addLayout(managerLayout)
        
        
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
    
    def nextId(self) -> int:
        # The next page id depends on the value of the 'ReadoutType field.
        match self.wizard().field('ReadoutType'):
            case 'XIA/DDAS':
                return 100
            case 'CCUSB' | 'VMUSB':   # These have the same parameter sets.
                return 200
            case 'MVLC':
                            return 300
            case 'Custom':
                return 400
            
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

class XIAParameters(QWizardPage):
    '''
        The parameters associated with the XIA/DDAS readout. 
        This defines the fields:
        
        XIA_SortHost - Host running the sorter.
        XIA_SortWindow - The sort window in seconds.
        XIA_SortRingBuffer - Where the sorter puts sorted hits.
        XIA_InfinityClock  - True if infinity is checked.
        XIA_ClockMultiplier - Clock multiplier if infinity.
        XIA_ReadoutBufferSize - Size of the readout buffer.
        XIA_ScalerPeriod      - Scaler Readout period in seconds.
        XIA_ReadoutFIFOTHreshold - FIFO THreshold Readout uses.
        XIA_CrateDirectory   - Where the XIA Crate files are.
          
    '''
    def __init__(self, parent : QObject | None = None) :
        super().__init__(parent)
    
    def initializePage(self) -> None:
        # The usual vertical stack of strips:
        
        self.setTitle('XIA/DDAS Readout parameters')
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        # Sort parameters:
        
        sortLayout = QHBoxLayout()
        sortLayout.addWidget(QLabel('Sort Host', self))
        self._sorthost = QLineEdit(self)
        self.registerField('XIA_SortHost*', self._sorthost)
        sortLayout.addWidget(self._sorthost)
        
        sortLayout.addWidget(QLabel('Sort Output Ring', self))
        self._sortring = QLineEdit(self)
        self.registerField('XIA_SortRingBuffer', self)
        self._sortring.setText(getpass.getuser() + "_sorted")
        sortLayout.addWidget(self._sortring)
        self._layout.addLayout(sortLayout)
        
        sortwindowLayout = QHBoxLayout()
        sortwindowLayout.addWidget(QLabel('SortWindow', self))
        self._sortwindow = QSpinBox(self)
        self._sortwindow.setRange(1, 20)
        self.registerField('XIA_SortWindow', self._sortwindow)
        self._sortwindow.setValue(2)
        sortwindowLayout.addWidget(self._sortwindow)
        self._layout.addLayout(sortwindowLayout)
        
        
        
        # Clock parameters:
        
        clockLayout = QHBoxLayout()
        self._infinity = QCheckBox('Infinity Clock')
        self.registerField('XIA_InfinityClock', self._infinity)
        clockLayout.addWidget(self._infinity)
        
        clockLayout.addWidget(QLabel('Clock Mutipler', self))
        self._multiplier = QLineEdit(self)
        self._multValidator = QIntValidator()
        self._multValidator.setBottom(1)
        self._multiplier.setValidator(self._multValidator)
        self.registerField('XIA_ClockMultiplier', self._multiplier)
        self._multiplier.setText('1')
        clockLayout.addWidget(self._multiplier)
        
        self._layout.addLayout(clockLayout)
        
        
        # Additional readout parameters:
        
        cratedirLayout = QHBoxLayout()
        cratedirLayout.addWidget(QLabel('Crate file directory: ', self))
        self._cratedir = QLineEdit(self)
        self.registerField('XIA_CrateDirectory*', self._cratedir)
        cratedirLayout.addWidget(self._cratedir)
        self._browseCratedir = QPushButton("Browse...", self)
        self._browseCratedir.clicked.connect(self._BrowseCrateDir)
        cratedirLayout.addWidget(self._browseCratedir)
        self._layout.addLayout(cratedirLayout)
        
        
        readoutLayout = QHBoxLayout()
        readoutLayout.addWidget(QLabel('Rdo Buffer Size'))
        self._readoutBuffer = QLineEdit(self)
        self._bufferValidator = QIntValidator()
        self._bufferValidator.setBottom(8192)
        self._bufferValidator.setTop(1024*1024*2) 
        self._readoutBuffer.setValidator(self._bufferValidator)
        self.registerField('XIA_ReadoutBufferSize', self._readoutBuffer)
        self._readoutBuffer.setText('16384')
        readoutLayout.addWidget(self._readoutBuffer)
        
        readoutLayout.addWidget(QLabel('FIFO Threshold (bytes)', self))
        self._fifothreshold = QLineEdit(self)
        self._fifoValidator = QIntValidator()
        self._fifoValidator.setBottom(128)
        self._fifoValidator.setTop(1024*1024)
        self.registerField('XIA_ReadoutFIFOThreshold', self._fifothreshold)
        self._fifothreshold.setText('81920')
        readoutLayout.addWidget(self._fifothreshold)
        self._layout.addLayout(readoutLayout)
        
        scalerLayout = QHBoxLayout()
        scalerLayout.addWidget(QLabel('Scaler Period (sec)', self))
        self._scalersecs = QSpinBox(self)
        self.registerField('XIA_ScalerPeriod', self._scalersecs)
        self._scalersecs.setRange(1,100)
        self._scalersecs.setValue(2)
        scalerLayout.addWidget(self._scalersecs)
        
        
        self._layout.addLayout(scalerLayout)
        
            
    def nextId(self) -> int:
        return -1       # No more pages.
    
    def pageId(self) -> int:
        return 100     # first and only XIA page.
    # Internal slots
    
    def _BrowseCrateDir(self) -> None:
        # Browse for the crate directory:
        
        dir = QFileDialog.getExistingDirectory(self, 'Crate file directory', '.')
        if dir.strip():
            self._cratedir.setText(dir)

class XXUSBParameters(QWizardPage):
    '''
        Collects the parameters for the CCUSB or VMUSB readout.
        The only difference between them is the program run
        which is implied by the type.  We define the fields:
        
        * XXUSB_BySerial - True if the device is to be looked up by serial number.
        * XXUSB_Serial   - The serial number string of the module to look up.
        * XXUSB_DAQConfig- Path to the DAQConfig file.
        * XXUSB_UseTsExtractor - True if a timestamp extractor is supplied.
        * XXUSB_TSExtractor - Path to time stamp extractor shared library.
        * XXUSB_UseControlServer - True if the control server is  enabled.
        * XXUSB_CTLConfig  - Path to the control server configuration file.
        * XXUSB_CTLServerPort - Control server port number
        * XXUSB_EnableLogging - True if logging is turned on.
        * XXUSB_LogFile     - Path to log file.
        
        Note that as the form is filled in, some controls will be disabled/enabled
        depending on the state of the flags.
        
        
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        self._initialized = False
        
    def initializePage(self) -> None:
        self.setTitle('XXUSB Readout parameters')
        if not self._initialized:
            self._initialized = True
            
            
            self._layout = QVBoxLayout()
            self.setLayout(self._layout)
            
            # How to connect to the XXUSB:
            
            connectLayout = QHBoxLayout()
            self._byserial = QCheckBox('Connect By serial', self)
            self.registerField('XXUSB_BySerial', self._byserial)
            self._byserial.clicked.connect(self._toggleSerialString)
            connectLayout.addWidget(self._byserial)
            
            connectLayout.addWidget(QLabel('Serial String', self))
            self._serialstring = QLineEdit(self)
            self._serialstring.setDisabled(True)
            self.registerField('XXUSB_Serial', self._serialstring)
            connectLayout.addWidget(self._serialstring)
            
            
            self._layout.addLayout(connectLayout)
    
            
            # DAQ Configuration stuff:
            
            daqconfigLayout = QHBoxLayout()
            daqconfigLayout.addWidget(QLabel('DAQ Config file:', self))
            self._daqconfigfile = QLineEdit(self)
            self.registerField('XXUSB_DAQConfig', self._daqconfigfile)
            daqconfigLayout.addWidget(self._daqconfigfile)

            self._browsedaqconfig = QPushButton('Browse...', self)
            daqconfigLayout.addWidget(self._browsedaqconfig)
            self._browsedaqconfig.clicked.connect(self._browseDaqConfig)        
            
            self._layout.addLayout(daqconfigLayout)
            
            
            
            # Timestamp stuff:
            
            tsLayout = QHBoxLayout()
            self._extractts = QCheckBox('Extract Timestamps')
            self.registerField('XXUSB_UseTsExtractor', self._extractts)
            tsLayout.addWidget(self._extractts)
            self._extractts.clicked.connect(self._toggleTsFields)
            
            tsLayout.addWidget(QLabel('Extractor library', self))
            self._tsextractor = QLineEdit(self)
            self.registerField('XXUSB_TSExtractor', self._tsextractor)
            self._tsextractor.setDisabled(True)
            tsLayout.addWidget(self._tsextractor)
            
            self._browsextractor = QPushButton('Browse...')
            self._browsextractor.clicked.connect(self._browseExtractor)
            self._browsextractor.setDisabled(True)
            tsLayout.addWidget(self._browsextractor)
            
            self._layout.addLayout(tsLayout)
            
            # Control server:
            
            self._usectlserver = QCheckBox('Run Control server', self)
            self.registerField('XXUSB_UseControlServer', self._usectlserver)
            self._layout.addWidget(self._usectlserver)
            self._usectlserver.clicked.connect(self._toggleControlServer)
            
            ctlLayout = QHBoxLayout()
            ctlLayout.addWidget(QLabel('CTL Config file', self))
            self._ctlconfig = QLineEdit(self)
            self.registerField('XXUSB_CTLConfig', self._ctlconfig)
            self._ctlconfig.setDisabled(True)
            ctlLayout.addWidget(self._ctlconfig)
            
            self._browsectlconfig = QPushButton('Browse...', self)
            self._browsectlconfig.clicked.connect(self._browseCtlConfig)
            ctlLayout.addWidget(self._browsectlconfig)
            
            ctlLayout.addWidget(QLabel('Port', self))
            self._ctlport = QSpinBox(self)
            ctlLayout.addWidget(self._ctlport)
            self.registerField('XXUSB_CTLServerPort', self._ctlport)
            self._ctlport.setMinimum(1024)     # Non privileged port.
            self._ctlport.setMaximum(29999)    # Below the managed ports.
            self._ctlport.setDisabled(True)
            
            self._layout.addLayout(ctlLayout)
        
        
            # Log configuration.    
        
            logLayout = QHBoxLayout()
            
            self._logging = QCheckBox('Enable Logging', self)
            self.registerField('XXUSB_EnableLogging', self._logging)
            logLayout.addWidget(self._logging)
            self._logging.clicked.connect(self._toggleLogging)
            
            self._logfile = QLineEdit(self)
            self.registerField('XXUSB_LogFile', self._logfile)
            self._logfile.setDisabled(True)
            logLayout.addWidget(self._logfile)
            
            self._browselog = QPushButton('Browse...')
            self._browselog.setDisabled(True)
            self._browselog.clicked.connect(self._browseLogFile)
            logLayout.addWidget(self._browselog)
            
            self._layout.addLayout(logLayout)
        
    def nextId(self) -> int:
        return -1
    def pageId(self) -> int:
        return 200
    
    # Private slots:
    
    def _toggleSerialString(self) -> None:
        # Called to enable/disable the self._serialstring field
        # depending on the state of the _byserial checkbutton.
        
        disabled = False if self._byserial.checkState() == Qt.CheckState.Checked else True
        self._serialstring.setDisabled(disabled)
        
    def _browseDaqConfig(self) -> None:
        # Browse button clicked for daq confi8g:
        
        file_path, _  = QFileDialog.getOpenFileName(self, 'DAQConfig script', '.', 'Tcl (*.tcl);;All Files (*)', '*.tcl')
        if file_path.strip():
            self._daqconfigfile.setText(file_path)
    def _toggleTsFields(self) -> None:
        ## The tmestamp extract enable checkbutton was tolggled. This affects
        # the line edit and button states:
        
        disabled = False if self._extractts.checkState() == Qt.CheckState.Checked else True        
        self._tsextractor.setDisabled(disabled)
        self._browsextractor.setDisabled(disabled)
        
    def _browseExtractor(self) -> None:
        # Browse for a file to put in the ts extractor line edit.
        
        file, _ = QFileDialog.getOpenFileName(
            self, 'Timestamp extraction library', '.', 'Shared Libs (*.so);;All Files (*)', '*.so'
        )
        if file.strip():
            self._tsextractor.setText(file)
           
    def _toggleControlServer(self) -> None:
        # Toggle the enables on the control server 
        
        disabled = False if self._usectlserver.checkState() == Qt.CheckState.Checked else True
        
        self._ctlconfig.setDisabled(disabled)
        self._browsectlconfig.setDisabled(disabled)
        self._ctlport.setDisabled(disabled)
    
    def _browseCtlConfig(self) -> None:
        # Browse for a control configuration script:
        
        file_path, _  = QFileDialog.getOpenFileName(self, 'CTLConfig script', '.', 'Tcl (*.tcl);;All Files (*)', '*.tcl')
        if file_path.strip():
            self._ctlconfig.setText(file_path)
    
    def _toggleLogging(self) -> None:
        # Toggle the enables on the widgets for logging:
        
        disabled = False if self._logging.checkState() == Qt.CheckState.Checked else True
        self._logfile.setDisabled(disabled)
        self._browselog.setDisabled(disabled)
        
    def _browseLogFile(self) -> None:
        # Browse for a log file.  This can be a new file:
        
        file, _ = QFileDialog.getSaveFileName(self, 'Log file', '.', 'Log (*.log);;All (*)', 'Log(*.log)')
        if file.strip():
            self._logfile.setText(file)
      
class CustomParameters1(QWizardPage):
    ''' 
        First page of prompts for custom readout parameters.
        This page gets the Readout executable. Subsequent pages
        collect program command line parameters, and options
        as well as environment variables.  We have the single field
        
        CUSTOM_Executable - the program to run for the _Readout program.
    '''  
    def __init__(self, parent : QObject | None = None) :
        super().__init__(parent)
        
    def initializePage(self) -> None:
        self.setTitle('Custom Parameters 1:')
        
        self._layout = QHBoxLayout()
        self.setLayout(self._layout)
        
        self._layout.addWidget(QLabel('Readout Program:', self))
        self._executable = QLineEdit(self)
        self.registerField('CUSTOM_Executable', self._executable)
        self._layout.addWidget(self._executable)
        
        self._browse = QPushButton('Browse...', self)
        self._browse.clicked.connect(self._browseExecutable)
        self._layout.addWidget(self._browse)
        
    
    def nextId(self) -> int:
        
        return 401         # Prompt for parameters and options.
    def pageId(self) -> int:
        return 400
    
    # Internal (private) slots:
    
    def _browseExecutable(self) -> None:
        #  Browse for an executable file to load into self._executable:
        
        file, _ = QFileDialog.getOpenFileName(self, 'Readout Program', '.', 'All Files (*)')
        if file.strip():
            self._executable.setText(file)

class CustomParameters2(QWizardPage):
    '''
    Define the program options for a custom program.  This is page 401.
    Because I'm too dumb to understand how to make custom fields.
    We provide:  getOptions which must by somehow exported by the wizard itself.
    That returns a list of pairs that are the program options entered by the user.
    
    '''
    def __init__(self, parent  : QObject | None = None):
        super().__init__(parent)
    
    def initializePage(self):
        self.setTitle('Set Readout program options')
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        self._options = EditableTable()
        self._options.table().setColumnCount(2)
        self._options.table().setHorizontalHeaderLabels(['Option', 'Value'])
        
        self._layout.addWidget(self._options)
        
    def getOptions(self) -> list[tuple[str,str]]:
        return self._options.getPairs()

    def nextId(self) -> int:
        return 402
    def pageId(self) -> int:
        return 401
class CustomParameters3(QWizardPage):
    '''
    Define the program parameters for a custom program.
    These are exported via getParameters which must be exposed
     in some way in the wizard.
    '''
    def __init__(self, parent: QObject | None = None):
        super().__init__(parent)
        
    def initializePage(self):
        self.setTitle('Set Readout Program Parameters')
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        self._parameters = EditableTable()
        self._parameters.table().setColumnCount(1)
        self._parameters.table().setHorizontalHeaderLabels(['Parameter',])
        
        self._layout.addWidget(self._parameters)
        
    def getParameters(self) -> list[str]:
        return self._parameters.col0List()
    
    def nextId(self) -> int:
        return 403
    def pageId(self) -> int:
        return 402

class CustomParameters4(QWizardPage):
    '''
        Provides a wizard page to set the custom program parameters for the
        Readout.  These can be fetched via getEnvironmentwhich must be
        exposed by the wizard somehow.
        
        Note that as these are executed ina shell, the parameter
        values can specify other environment variables e.g. 
        TCLLIBPATH=$DAQTCLLIBS.
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
    def initializePage(self):
        self.setTitle('Set Readout environment variables')
        
        self._layout= QVBoxLayout()
        self.setLayout(self._layout)
        
        self._environ = EditableTable()
        self._environ.table().setColumnCount(2)
        self._environ.table().setHorizontalHeaderLabels(['Variable', 'Value'])
        
        self._layout.addWidget(self._environ)
        
    def getEnvironment(self) -> list[tuple[str,str]]:
        return self._environ.getPairs()
    
    def nextId(self) -> int:
        return -1                          #last page.
    def pageId(self) -> int:
        return 403

class MVLCProgram(QWizardPage):
    '''
        In order to use the MVLCReadout the user must find the
        fribdaq-readout program in the file system.
        This is usually somewhere in /usr/opt/mesytec-mvlc/x.y.z/bin
        
        This page prompts for that.  It sets the field:
        
        MVLC_Readout - to the program path.
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
    def initializePage(self) -> None:
            self.setTitle('Select MVLC FRIB readout program.')
            
            # Some explanatory text followed by a label, entry and browse button.
            
            self._layout = QVBoxLayout()
            self.setLayout(self._layout)
            
            # explanatory text:
            
            self._intro = QTextEdit(self)
            self._intro.setReadOnly(True)
            self._intro.setHtml('''
    <p>
        The Readout program for the MVLC is not atually part of FRIB/NSCLDAQ
        it is, instead, a program named <tt>fribdaq-readout</tt>. That
        is part of the mesytec MVLC driver software, when built with support
        for FRIB/NSCLDAQ.
    </p>
    <p>
        At the FRIB, this is installed in <br/>
        <tt>/usr/opt/mesytec-mvlc/&lt;version&gt;/bin/fribdaq-readout</tt>.
        where &lt;version&gt; is a version of the mesytec-mvlc driver software.
        In this page, you are prompted to specify the specific fribdaq-readout program.
    </p>
            ''')
            self._layout.addWidget(self._intro)
            
            promptLayout = QHBoxLayout()
            promptLayout.addWidget(QLabel('fribdaq-readout program:', self))
            
            self._program = QLineEdit(self)
            promptLayout.addWidget(self._program)
            self.registerField('MVLC_Readout', self._program)
            
            self._browse = QPushButton('Browse...', self)
            promptLayout.addWidget(self._browse)
            self._browse.clicked.connect(self._browseProgram)
            
            self._layout.addLayout(promptLayout)
    
    def nextId(self) -> int:
        return 301         # Connection parameters.
    def pageId(self) -> int:
        return 300
    def _browseProgram(self) -> None:
        # Browse in /usr/opt/mesytec-mvlc for fribdaq-readout.
        
        path, _ = QFileDialog.getOpenFileName(
            self, 'fribdaq-readout path', '/usr/opt/mesytec-mvlc',
            'FRIB readout (fribdaq-readout);;All Files (*)',
            'fribdaq-readout'
        )
        if path.strip():
            self._program.setText(path)
class MVLCConnection(QWizardPage):
    '''
        The fribdaq-readout has several connection option, these set
        various fields though:
        
        MVLC_ConnectionType  - one of: ethernet, firstusb, usbbyindex, usbbyserial
        MVLC_Host            - Ethernet host to connect to.
        MVLC_UsbIndex        - Index of USB to connect to.
        MVLC_UsbSerial       - USB Serial string to connect to.
        
        Note that we use/maintain a hidden LineEdit field that is set
        by clicking the radio buttons ...and initialized to firstusb
    '''
    def __init__(self, parent : QObject | None = None) :
        super().__init__(parent)
        
        
    def initializePage(self) -> None:
        self.setTitle('Set MVLC Connection method')
        
        # There will be several lines with radio buttons on ech line:
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        # Hidden connection type entry:
        
        self._connectionType = QLineEdit()
        
        self.registerField('MVLC_ConnectionType', self._connectionType)
        self._connectionType.hide()
        self.setField('MVLC_ConnectionType', 'firstusb')
        
        # Ethernet Not initially selected, has a host label/line edit/
        
        ethernetLayout = QHBoxLayout()
        self._selectethernet = QRadioButton('Ethernet', self)
        self._selectethernet.setChecked(False)
        ethernetLayout.addWidget(self._selectethernet)
        
        ethernetLayout.addWidget(QLabel('Host:', self))
        self._host = QLineEdit(self)
        self.registerField('MVLC_Host', self._host)
        ethernetLayout.addWidget(self._host)
        self._host.setDisabled(True)
        
        self._selectethernet.toggled.connect(self._selectEthernet)
        
        self._layout.addLayout(ethernetLayout)
        
        # First usb is a standalone radio button:
        # It's initially selected.
        
        self._firstusb = QRadioButton('First Usb', self)
        self._firstusb.setChecked(True)
        self._layout.addWidget(self._firstusb)
        self._firstusb.toggled.connect(self._selectFirstUsb)
        
        
        # USB Index is a radio button with a spinbox for the index.
        # Spinbox goes 0-256 which ought to be enough:
        
        usbindexLayout = QHBoxLayout()
        self._usbindexselect = QRadioButton('USB By index', self)
        self._usbindexselect.setChecked(False)
        usbindexLayout.addWidget(self._usbindexselect)
        
        usbindexLayout.addWidget(QLabel('Index:', self))
        
        self._usbindex = QSpinBox(self)
        self._usbindex.setMinimum(0)
        self._usbindex.setMaximum(255)
        self._usbindex.setDisabled(True)
        self.registerField('MVLC_UsbIndex', self._usbindex)
        
        self._usbindexselect.toggled.connect(self._selectUsbIndex)
        usbindexLayout.addWidget(self._usbindex)
        
        
        self._layout.addLayout(usbindexLayout)
        
        # USB By serial string:
        
        usbserialLayout = QHBoxLayout()
        self._usbbyserial = QRadioButton('USB By Serial', self)
        self._usbbyserial.setChecked(False)
        self._usbbyserial.toggled.connect(self._selectUsbSerial)
        usbserialLayout.addWidget(self._usbbyserial)
        
        usbserialLayout.addWidget(QLabel('Serial String', self))
        
        self._usbserial = QLineEdit(self)
        self.registerField('MVLC_UsbSerial', self._usbserial)
        self._usbserial.setDisabled(True)
        usbserialLayout.addWidget(self._usbserial)
        
        self._layout.addLayout(usbserialLayout)
        
        
    def nextId(self) -> int:
        return 302   # For now.
    def pageId(self) -> int:
        return 301
    
    # Internal slots:
    #   These respond to toggles of the radio buttons
    #   setting the connection type if they are selected
    #   enabling/dsabling any associated wigets as appropriate.
    
    def _selectEthernet(self, state : bool) -> None:
        # Ethenet selected update MVCL_ConnectionType and
        # The enables:
        
        if state:
            self.setField('MVLC_ConnectionType', 'ethernet')
            disabled = False
        else:
            disabled = True
        self._host.setDisabled(disabled)     # Enable the host field.
    
    
    def _selectFirstUsb(self, state : bool) -> None:
        # First usb connection selected:
        
        if state:
            self.setField('MVLC_ConnectionType', 'firstusb')
        
        
    def _selectUsbIndex(self, state: bool) -> None:
        if state:
            self.setField('MVLC_ConnectionType', 'usbbyindex')
            disabled = False
        else:
            disabled = True
        
        self._usbindex.setDisabled(disabled)
    
    def _selectUsbSerial(self, state: bool) -> None:
        if state:
            self.setField('MVLC_ConnectionType', 'usbbyserial')
            disabled = False
        else:
            disabled = True
        
        self._usbserial.setDisabled(disabled)
        
class MVLCDAQParameters(QWizardPage):
    '''
        Provide the FRIB/NSCLDAQ parameters for the fribdaq-readout program.
        The following fields are defined:
        
        MVLC_Ring - Ringbuffer to which data will be put.
        MVLC_SourceId - The sourced the data will be tagged with.
        MVLC_HaveTsLibrary - True if a timestamp extraction library should be supplied.
        MVLC_TimestampSo  - Shared object that will be loaded to extract timestamps.
        
    '''
    def __init__(self, parent : QObject | None = None) :
        super().__init__(parent)
        
    def initializePage(self) -> None:
        self.setTitle('FRIB/NSCLDAQ Data Acquisition system parameters')
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        # The ring buffer
        
        ringLayout  = QHBoxLayout()
        ringLayout.addWidget(QLabel('Output ringbuffer [not URI]', self))
        
        self._ring = QLineEdit(self)
        self.registerField('MVLC_Ring', self._ring)
        ringLayout.addWidget(self._ring)
        
        self._layout.addLayout(ringLayout)
        
        #  The source id.
        
        sidLayout = QHBoxLayout()
        sidLayout.addWidget(QLabel('Source id:', self))
        
        self._sid = QSpinBox(self)
        self._sid.setMinimum(0)
        self._sid.setMaximum(0x7fffffff)
        self.registerField('MVLC_SourceId', self._sid)
        sidLayout.addWidget(self._sid)
        
        self._layout.addLayout(sidLayout)
        
        # Optional timestamp library:
        
        tslibLayout = QHBoxLayout()
        self._havetslib = QCheckBox('Extract Timestamps', self)
        self.registerField('MVLC_HaveTsLibrary', self._havetslib)
        self._havetslib.clicked.connect(self._toggleHaveTsLib)
        tslibLayout.addWidget(self._havetslib)
        
        self._tslib = QLineEdit(self)
        self.registerField('MVLC_TimestampSo', self._tslib)
        self._tslib.setDisabled(True)
        tslibLayout.addWidget(self._tslib)
        
        self._browsetslib = QPushButton('Browse...', self)
        self._browsetslib.setDisabled(True)
        self._browsetslib.clicked.connect(self._browseTslib)
        tslibLayout.addWidget(self._browsetslib)
        
        self._layout.addLayout(tslibLayout)
        
    
    def nextId(self) -> int:
        return 303
    def pageId(self) -> int:
        return 302
    
    # Slots (private);
    def _toggleHaveTsLib(self) -> None:
        # Toggle the state of the enable for the timestamplib and
        # it's browser.
        
        if self.field('MVLC_HaveTsLibrary'):
            disabled = False
        else:
            disabled = True
            
        self._tslib.setDisabled(disabled)
        self._browsetslib.setDisabled(disabled)
        
    def _browseTslib(self) -> None:
        file, _ = QFileDialog.getOpenFileName(
            self, '.', 'Timestamp shared object',
            'Shared Objects (*.so);; All Files (*)'
        )
        if file.strip():
            self._tslib.setText(file)
class MVLCReadoutConfig(QWizardPage):
    '''
    Prompt for the readout configuration.
    MVLC_Config        - Configuration file.
    MVLC_ConvertFromTcl - True to convert the config file from tcl -> Yaml.
    
    MVLC_OverrideTemplate  - True to override the yaml template fie.
    MVLC_YamlTemplate  - YAML template file to sactually use.
    
    MVLC_UseInitScript - True if an init script is specified.
    MVLC_InitScript    - Path to the initscript.
    
    MVLC_RunCtlServer  - True if the control server shoul be used.
    MVLC_CtlServerPort - Port on which the control server listens.
    MVLC_CtlScript     - Control server script.
    
    
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
    
    def initializePage(self) -> None:
        self.setTitle('Readout configuration:')
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        # The readout configuration file and how to interpret it:
        # We support browsing for it:
        
        rdocfgLayout = QHBoxLayout()
        self._cvttcltoyaml = QCheckBox('Convert .tcl -> .yaml', self)
        self.registerField('MVLC_ConvertFromTcl', self._cvttcltoyaml)
        self._cvttcltoyaml.setCheckState(Qt.CheckState.Checked)    # That's the most common use.
        rdocfgLayout.addWidget(self._cvttcltoyaml)
        
        rdocfgLayout.addWidget(QLabel('Readout Config File', self))
        self._rdoconfig = QLineEdit(self)
        self.registerField('MVLC_Config', self._rdoconfig)
        rdocfgLayout.addWidget(self._rdoconfig)
        
        self._browserdoconfig = QPushButton('Browse...', self)
        self._browserdoconfig.clicked.connect(self._BrowseReadoutFile)
        rdocfgLayout.addWidget(self._browserdoconfig)
        
        self._layout.addLayout(rdocfgLayout)
        
        # Allow the user to override the default .yaml template file:
        
        templateLayout = QHBoxLayout()
        self._overrideTemplate = QCheckBox('Use Custom Yaml Template', self)
        self.registerField('MVLC_OverrideTemplate', self._overrideTemplate)
        templateLayout.addWidget(self._overrideTemplate)
        self._overrideTemplate.clicked.connect(self._toggleTemplateOverride)
        
        templateLayout.addWidget(QLabel('YAML Template file: '))
        self._yamltemplate = QLineEdit(self)
        self._yamltemplate.setDisabled(True)
        self.registerField('MVLC_YamlTemplate', self._yamltemplate)
        templateLayout.addWidget(self._yamltemplate)
        
        self._browsetemplate = QPushButton('Browse...', self)
        self._browsetemplate.setDisabled(True)
        self._browsetemplate.clicked.connect(self._browseTemplate)
        templateLayout.addWidget(self._browsetemplate)
        
        
        self._layout.addLayout(templateLayout)
        
        # Control server configuration.
        
        ctlserverLayout = QHBoxLayout()
        
        self._enableserver = QCheckBox('Use slow control server')
        self.registerField('MVLC_RunCtlServer', self._enableserver)
        ctlserverLayout.addWidget(self._enableserver)
        
        ctlserverLayout.addWidget(QLabel('Configuration script:', self))
        self._ctlserverscript =QLineEdit(self)
        self.registerField('MVLC_CtlScript', self._ctlserverscript)
        ctlserverLayout.addWidget(self._ctlserverscript)
        
        self._browseCtlConfig = QPushButton('Browse...', self)
        self._browseCtlConfig.setDisabled(True)
        self._browseCtlConfig.clicked.connect(self._BrowseControlConfig)
        ctlserverLayout.addWidget(self._browseCtlConfig)
        
        ctlserverLayout.addWidget(QLabel('Port', self))
        self._ctlserverPort= QSpinBox(self)
        self._ctlserverPort.setMinimum(1024)
        self._ctlserverPort.setMaximum(29999)
        self.registerField('MVLC_CtlServerPort', self._ctlserverPort)
        self._ctlserverPort.setValue(1024)
        ctlserverLayout.addWidget(self._ctlserverPort)
        
        self._ctlserverscript.setDisabled(True)
        self._ctlserverPort.setDisabled(True)
        
        self._enableserver.clicked.connect(self._toggleControlServer)
        
        self._layout.addLayout(ctlserverLayout)
        
        # Initialization script:
        
        initscriptLayout = QHBoxLayout()
        
        self._enableinitscript = QCheckBox('Use Init script', self)
        self.registerField('MVLC_UseInitScript', self._enableinitscript)
        initscriptLayout.addWidget(self._enableinitscript)
        
        self._initscript = QLineEdit(self)
        self.registerField('MVLC_InitScript', self._initscript)
        self._initscript.setDisabled(True)
        initscriptLayout.addWidget(self._initscript)
        
        self._browseinitscript = QPushButton('Browse...', self)
        self._browseinitscript.setDisabled(True)
        self._browseinitscript.clicked.connect(self._BrowseInitScript)
        initscriptLayout.addWidget(self._browseinitscript)
        
        self._enableinitscript.clicked.connect(self._toggleInitScript)
                                           
        
        self._layout.addLayout(initscriptLayout)
    
    def nextId(self) -> int:
        return 304   # For now.
    def pageId(self) -> int:
        return 303    
    
    # Internal, private slots:
    def _BrowseReadoutFile(self) -> None:
        # The Browse... button was clicked on for the readout config.
        # One complication is that the filters are determined by
        # the MVLC_ConvertFromTcl Field:
        
        if self.field('MVLC_ConvertFromTcl'):
            # Prefer tcl.
            filters = 'Tcl Files (*.tcl);;Yaml Files (*.yaml);;All Files (*)'
            selected_filter = '*.tcl'
        else:   
            # Prefer Yaml.
            filters = 'Yaml Files (*.yaml);;Tcl Files (*.tcl);;All Files (*)'
            selected_filter = '*.tcl'
        
        file, _ = QFileDialog.getOpenFileName(
            self, 'Readout config', '.', filters, selected_filter
        )
        if file.strip():
            self.setField('MVLC_Config', file)
     
    def _toggleTemplateOverride(self) -> None:
        if self.field('MVLC_OverrideTemplate'):
            disabled = False
        else:
            disabled = True
        
        self._yamltemplate.setDisabled(disabled)
        self._browsetemplate.setDisabled(disabled) 
    
    def _browseTemplate(self) -> None:
        file, _  = QFileDialog.getOpenFileName(
            self, 'YAML Template', '.', 
            'Yaml Files (*.yaml);;All Files (*)', '*.yaml'
        )  
        if file.strip():
            self._yamltemplate.setText(file)       
            
    def _toggleControlServer(self) -> None:    
        # The control server enable has changed.
        
        if self.field('MVLC_RunCtlServer'):
            disabled = False
        else:
            disabled = True

        self._ctlserverscript.setDisabled(disabled)
        self._ctlserverPort.setDisabled(disabled)
        self._browseCtlConfig.setDisabled(disabled)
    
    def _BrowseControlConfig(self) -> None:
        # Browse for a Tcl config file for the control server:
        
        file, _ = QFileDialog.getOpenFileName(
            self, 'Control config file', '.', 'Tcl Files (*.tcl);;All Files (*)', '*.tcl'
        )
        if file.strip():
            self._ctlserverscript.setText(file)
        
    
    def _toggleInitScript(self) -> None:
        # The init script enable changed:
        
        if self.field('MVLC_UseInitScript'):
            disabled = False
        else:
            disabled = True
            
        self._initscript.setDisabled(disabled)
        self._browseinitscript.setDisabled(disabled)
    
    def _BrowseInitScript(self) -> None:
        # Browse for hte init script:
        
        file, _  = QFileDialog.getOpenFileName(
            self, 'Init script', '.', 'Tcl Files (*.tcl);;All Files (*)', '*.tcl'
        )
        if file.strip():
            self._initscript.setText(file)

class MVLCDebugOptions(QWizardPage):
    '''
    Provides access to the minidaq debug options (fribdaq_reaodut is based on
    minidaq):
    MVLC_InitOnly - Just run intialization and exit (off by default)
    MVLC_IgoreInitErrors - Ignore VME errors on initialization (on by default)
    MVLC_Debug   - Debug logging on (off by default)
    MVLC_Trace   - Trace debugging.
    '''
    
    def __init__(self, parent : QObject | None):
        super().__init__(parent)    
    
    def initializePage(self) -> None:
        self.setTitle('Debuging options') 
        
        self._layout = QGridLayout()
        self.setLayout(self._layout)
        
        self._ignoreInitErrors = QCheckBox('Ignore init Errors', self)
        self.registerField('MVLC_IgnoreInitErrors', self._ignoreInitErrors)
        self.setField('MVLC_IgnoreInitErrors', True)
        self._layout.addWidget(self._ignoreInitErrors, 0,0)
        
        self._initOnly = QCheckBox('Only Initialize', self)
        self.registerField('MVLC_InitOnly', self._initOnly)
        self._layout.addWidget(self._initOnly, 0,1)
        
        self._debug = QCheckBox('Debug logging', self)
        self.registerField('MVLC_Debug', self._debug)
        self._layout.addWidget(self._debug, 1,0)
        
        self._trace = QCheckBox('Trace logging', self)
        self.registerField('MVLC_Trace', self._trace)
        self._layout.addWidget(self._trace, 1,1)
        
    def nextId(self) -> int:
        return -1                        #end of the MVLC line.
    def pageId(self) -> int:
        return 304
        
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
        
        # XIA Parameter page(s)
        
        self._xiainfo = XIAParameters(self)
        self.setPage(self._xiainfo.pageId(), self._xiainfo)
        
        # XXUSB parameter page(s)
        
        self._xxusbinfo = XXUSBParameters(self)
        self.setPage(self._xxusbinfo.pageId(), self._xxusbinfo)
        
        # MVLC parameter pages
        
        self._mvlcprogram = MVLCProgram(self)
        self.setPage(self._mvlcprogram.pageId(), self._mvlcprogram)
        
        self._mvlcconnection = MVLCConnection(self)
        self.setPage(self._mvlcconnection.pageId(), self._mvlcconnection)
        
        self._mvlcdaqparams = MVLCDAQParameters(self)
        self.setPage(self._mvlcdaqparams.pageId(), self._mvlcdaqparams)
        
        self._mvlcreadoutconfig = MVLCReadoutConfig(self)
        self.setPage(self._mvlcreadoutconfig.pageId(), self._mvlcreadoutconfig)
        
        self._mvlcdebug = MVLCDebugOptions(self)
        self.setPage(self._mvlcdebug.pageId(), self._mvlcdebug)
        
        # Custom parameter pages:
        
        self._custom1 = CustomParameters1(self)
        self.setPage(self._custom1.pageId(), self._custom1)
        
        self._custom2 = CustomParameters2(self)
        self.setPage(self._custom2.pageId(), self._custom2)
        
        self._custom3 = CustomParameters3(self)
        self.setPage(self._custom3.pageId(), self._custom3)
        
        self._custom4 = CustomParameters4(self)
        self.setPage(self._custom4.pageId(), self._custom4)
        
        
    def containers(self) -> list[str]:
        ''' @return list[str] - list of containers that are available.'''
        return self._commonInfo.containers()
    def setContainers(self, containers : list[str]) -> None:
        self._commonInfo.setContainers(containers)
    
    def getCustomProgramOptions(self) -> list[tuple[str,str]]:
        return self._custom2.getOptions()

    def getCustomProgramParameters(self) -> list[str]:
        return self._custom3.getParameters()

    def getCustomProgramEnvironment(self) -> list[tuple[str, str]]:
        return self._custom4.getEnvironment()

# All Readout generators must support the generate method described below:

class ReadoutGenerator(Protocol):
    def generate(self, wiz : ReadoutWizard) -> None:
        ...

class XIAGenerator:
    '''
        Class to generate the Readout program and the BOOT bootreadouts sequence
        step to start the XIA readout and sorter.  This is bundled as  a single program
        because what we start is the script that starts both, appropriately parameterized.
    '''
    def __init__(self, db : sqlite3.Connection):
        '''
        @param db - the datbase connection to the configuration sqlite3 database.
        '''
        
        self._db = db
    

    def generate(self, wiz : ReadoutWizard) -> None:
        '''
            Generate the program and boot step:
            
            @param wiz - wizard, contains the parameters we care about as XIA_xxxx.
            @note This is based loosely on the Tcl code
        '''
        options = self._makeOptions(wiz)
        environment = self._makeEnvironment(wiz)
        
        self._makeReadoutProgram(options, environment, wiz)
        self._addBootStep(wiz)
        
        
    def _makeOptions(self, wiz : ReadoutWizard) -> list[tuple[str,str]]:
        # Given the wizard parameters, return the 
        # command line options as a list of (name, value) pares e.g. ("-ring", "ron")
        
        # Build a list of field name, option name tuples which will drive this:
        
        optionlookup = [
            ('ReadoutHost', "-readouthost"), ('SourceId', '-sourceid'), ('RingBuffer', '-readoutring'),
            ('XIA_SortHost', '-sorthost'), ('XIA_SortRingBuffer', "-sortring"), ('XIA_SortWindow', '-window'),
            ('XIA_ReadoutFIFOThreshold', '-fifothreshold'), ('XIA_ReadoutBufferSize', '-buffersize'),
            ('XIA_ClockMultiplier', '-clockmultiplier'), ('XIA_ScalerPeriod', '-scalerseconds'),
            ('XIA_CrateDirectory', '-cratedir')
        ]
        options = []
        
        for field,optname in optionlookup:
            options.append((optname, wiz.field(field)))

        # Infinity clock:
        
        if wiz.field('XIA_InfinityClock'):
            options.append(('-infinity', 'on'))    

        # We also need to add an initscript for the ReSt server:
        
        options.append(('-initscript','$DAQSHARE/scripts/rest_init_script.tcl'))
        
        return options
    
    def _makeEnvironment(self, wiz: ReadoutWizard) -> list[tuple[str,str]]:
        #  Make the environment list of name/value pairs:
        
        # The RDOREST_KEEPSTDIN  one is needed to make the stdin stay open so the script that runs REadout
        # Does not think it's exited.
        
        return [
            ('RDOREST_KEEPSTDIN', '1'),
            ('SERVICE_NAME', wiz.field('RestService'))
        ]
        
    def _makeReadoutProgram(
        self, options : list[tuple[str,str]], env : list[tuple[str,str]], 
        wiz : ReadoutWizard
    ) -> None:
        # Make the program in the database.  
        
        api = mg_database.Program(self._db)
        
        full_name = wiz.field('Name') + "_readout"
        path      = '$DAQBIN/ddasReadout'
        host      = wiz.field('ReadoutHost')    # Where the controlling script runs.
        container = wiz.field('Container')
        wd        = wiz.field('Directory')
        
        api.add(full_name, path, host, container, wd, {
            'options': options,
            'environment' : env
        })
    
    def _addBootStep(self, wiz: ReadoutWizard) -> None:
        # Add a boot step to run the readout program.
        
        api = mg_database.Sequence(self._db)
        full_name = wiz.field('Name') + "_readout"
        
        api.addStep('bootreadouts', full_name, 0,0)
        

def makeGenerator(db : sqlite3.Connection, rdoType : str) -> ReadoutGenerator:
    '''
        Factory that  creates and returns the appropriate readout generator.
        
        @param db - the sqlite database connection to the configuration database.
        @param rdoType - the readout type to make a generator for.
        @return An object that fulfils the ReadoutGenerator protoco
    '''
    match rdoType:
        case 'XIA/DDAS':
            return XIAGenerator(db)
        case _:
            raise NotImplementedError(f'{rdoType} is not a supported readout type')
    
    
    

class Controller(QObject):
    '''
        Controller for the wizard.  Note that all we actually do is 
        stock the wizard with containers, and, on the accepted signal
        do some common stuff and farm the bulk of the work off to a
         type specific creator.
    '''
    def __init__(self, wizard : ReadoutWizard, config : str, parent : QObject | None = None):
        super().__init__(parent)
        
        self._view = wizard
        self._config = config
        self._db     = sqlite3.connect(self._config)
        
        self._view.accepted.connect(self._generate)
        
        # Load the view with containers:
        
        api = mg_database.Container(self._db)
        container_names = [x['name'] for x in api.list()]
        self._view.setContainers(container_names)
        
    def _generate(self) -> None:
        self._ensureSequences() 
        self._makeControlPrograms()
        self._makeInitialSequenceSteps()
        
        # Select the appropriate type specific generator and run it.
        
        generator = makeGenerator(self._db, self._view.field('ReadoutType'))
        generator.generate(self._view)

    def _ensureSequences(self) -> None:
        #
        #    We insert programs we generate into a specific set of sequences.
        #    this method creates any missing sequences:
        #    We assume if the right sequence name exists it also has the right trigger:
        
        # Sequences, if they need to be generated, have no steps. Those are added later.
        
        api = mg_database.Sequence(self._db)
        # We need:
        
        sequences = [
            ('bootreadouts', 'BOOT'),
            ('initreadouts', 'HWINIT'),
            ('beginreadouts', 'BEGIN'),
            ('endreadouts',   'END'),
            ('shutdownreadouts', 'SHUTDOWN')
        ]
        
        for name, trigger in sequences:
            if not api.exists(name):
                api.add(name, trigger, [])            # We'll add steps later.
    
    
    def _makeKvProgram(self, image :str, suffix :str, params : dict) -> None:
        # Make a program that sets something from the keyvalue store.
        # image the name of the program in $DAQBIN, suffix. the suffix to append
        #    to the program name.
        # params the parameter dictionary from _makeControlPrograms below.
        
        api = mg_database.Program(self._db)
        
        full_image = f'$DAQBIN/{image}'
        full_name  = f'{params["name"]}_{suffix}'
        readout_name = f'{params["name"]}_readout'
    
        program_parameters = [
            params['mgr_host'],
            params['mgr_user'],
            readout_name,
        ]
        environment = [('SERVICE_NAME', params['service']),]
        
        api.add(full_name, full_image, params['rdo_host'], params['container'], params['dir'],
                {'parameters': program_parameters, 'environment': environment})
    
    def _makeControlProgram(self, image :str, suffix: str, command : str, params : dict) -> None:
        # Make a program that sends a control param to a Readout program.
        # image - the image of the program in $DAQBIN
        # suffix - the suffix the program gets in constructing its name.
        # command - the command to give the readout.
        # params - the parameter dict constructedin _makeControlPrograms.
        api = mg_database.Program(self._db)
        
        full_image = f'$DAQBIN/{image}'
        full_name  = f'{params["name"]}_{suffix}'
        
        program_params = [
            params['rdo_host'], params['mgr_user'], command 
        ]
        environment = [('SERVICE_NAME', params['service']),]
        
        api.add(full_name, full_image, params['rdo_host'], params['container'], params['dir'],
        {'parameters': program_params, 'environment': environment}
        )
    
    def _makeControlPrograms(self) -> None:
        # Make the control programs that we'll eventually insert
        # Into the sequences.  The Readout program will be created by the
        # type specific generators.
        # We'll run these in the same host as the manager itself.
        
        params = {
            'name': self._view.field('Name'),
            'container' : self._view.field('Container'),
            'mgr_host'  : self._view.field('ManagerHost'),
            'rdo_host'  : self._view.field('ReadoutHost'),
            'dir'       : self._view.field('Directory'),
            'mgr_user'  : self._view.field('User'),
            'service'   : self._view.field('RestService')
            
        }
        
        self._makeKvProgram('rdo_titleFromKv', 'settitle', params)
        self._makeKvProgram('rdo_runFromKv', 'setrun', params)
        
        self._makeControlProgram('rdo_control', 'beginrun', 'begin', params)
        self._makeControlProgram('rdo_control', 'init', 'init', params)
        self._makeControlProgram('rdo_control', 'endrun', 'end', params)
        self._makeControlProgram('rdo_control', 'shutdown', 'shutdown', params)
    
    def _makeInitialSequenceSteps(self) -> None:
        # The control programs provide common sequence steps for the state transitions:
        
        name = self._view.field('Name')  # Name prefix.
        api  = mg_database.Sequence(self._db)
        
        # BOOT gets nothing - it will be filled in by the appropriate type
        # handler with the stuff needed to run the readout.
        
        # initreadouts gets {name}_init:
        
        api.addStep('initreadouts', f'{name}_init', 0, 0)
        
        # beginreadouts gets settitle, setrun and beginrun
        
        api.addStep('beginreadouts', f'{name}_settitle', 0,0)
        api.addStep('beginreadouts', f'{name}_setrun', 0,0)
        api.addStep('beginreadouts', f'{name}_beginrun', 0,0)
        
        # endreadouts getwss end:
        
        api.addStep('endreadouts', f'{name}_endrun', 0,0)
        
        # shutdownreadouts gets shutdown:
        
        api.addStep('shutdownreadouts', f'{name}_shutdown', 0,0)
        
        
        
def usage() -> None:
    '''
    Print the program usage to stderr.
    '''
    print('''
Wizard to help you define readout programs.

Usage:
    $DAQBIN/mg_readout_wizard config_path
Where:
    config_path - is the path to the configuration database file.

        ''', file = sys.stderr)

def main() -> int:
    ''' Main entry point '''
    
    if len(sys.argv) != 2:
        usage()
        return -1

    config_file = sys.argv[1]
    
    # setup the GUI and the controller to handle it:
    
    app = QApplication(sys.argv)
    dialog = ReadoutWizard()
    
    _controller = Controller(dialog, config_file, dialog)
    
    dialog.show()
    return(app.exec())

if __name__ == "__main__":
    sys.exit(main())
