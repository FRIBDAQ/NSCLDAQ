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
@file MVLCDataSource.ply
@brief Data source for the MVLC controller Readout.
@author Ron Fox
@note The program is normally installed with the mesytec-mvlc driver, if at all.

The registration must look for the program fribdaq-readout.  It looks for it in one of the
following places:

As normally installed in FRIBDAQ - /usr/opt/mesytec-mvlc/x.y.z   looking for it in all of the x.y.z directories and choosing
the one with the largest semantic version number e.g. on my dev system I have
0.0.1/ 0.1.0/ 0.1.3/ 0.1.4/ 0.1.5/  all of which were built to enable the installation of fribdaq-readout.
The one in 0.1.5 will be chosen.

If the environment variable MVLC_DIR is defined, that overrides this search and the program
is looked for in $MVLC_DIR/bin.

If the program is found this data source is registerd as 'MVLC'


'''

import getpass
import os
import pathlib

import nscldaq.readoutgui.FRIBDAQDataSource  # For fully qualified ConfigurationDisplay
from nscldaq.readoutgui.FRIBDAQDataSource import FRIBDAQSource
from packaging.version import Version
from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import (
    QApplication,
    QCheckBox,
    QComboBox,
    QFileDialog,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMessageBox,
    QPushButton,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)


def _findExecutable() -> str | None:
    # Locate the fribdaq-readout executable.
    
    if 'MVLC_DIR' in os.environ:
        binary =  pathlib.Path(os.environ['MVLC_DIR']) / 'bin/fribdaq-readout'
        if binary.exists():
            return str(binary)
        else:
            return None
    else:
        mvlc = pathlib.Path('/usr/opt/mesytec-mvlc')
        if not mvlc.exists():
            return None
        # Which subdirectories have our program in their 'bin' subdirs:
        
        exists_in = []
        for subdir in mvlc.iterdir():
            program = subdir / 'bin/fribdaq-readout'
            if program.exists():
                exists_in.append(subdir)
        
        if len(exists_in) == 0:
            return None                          # None of those subdirs has the readout.
        # exists_in are the subdires that have our readout program.. we need to get the
        # one with the largest semantic version.
        
        latest = exists_in[0] 
        for version in exists_in:
            latest_version = Version(latest.parts[-1])
            this_version   = Version(version.parts[-1])
            if this_version > latest_version :
                latest = version
            
        #  Now latest is the most recent version whose bin has fribdaq-readout so:
        
        return str(latest / 'bin/fribdaq-readout')
    

        
class MVLCDataSource(FRIBDAQSource):
    '''
    I need to support the following options in a manner that is also consistent with
    FRIBDAQ:
    
        --mvlc-eth <hostname>

    mvlc ethernet hostname (overrides CrateConfig)

    --mvlc-usb

    connect to the first mvlc usb device (overrides CrateConfig)

    --mvlc-usb-index <index>

    connect to the mvlc with the given usb device index (overrides CrateConfig)

    --mvlc-usb-serial <serial>

    connect to the mvlc with the given usb serial number (overrides CrateConfig)

    --init-only

    run the DAQ init sequence and exit

    --ignore-vme-init-errors

    ignore VME errors during the DAQ init sequence

    --ring <ring>

    ring buffer name

    --sourceid <sourceid>

    Event builder source id

    --timestamp-library <dll>

    Time stamp shared library file

    --init-script <initscript>

    Tcl initialization script

    --control-server <controlport>

    Slow controls server port

    --ctlconfig <ctlinitscript>

    Control server initliazation script

    --debug

    enable debug logging

    --trace

    enable trace logging

    --convert-tcl

    Crate crate yaml from FRIB/NSCLDAQ .tcl file

    --template <YamlTemplate>

    With --convert-tcl specify alternate template file

    <crateConfig>

    crate config yaml/tcl file

    
    We can do this by providing our own parameter set and just not bothering to have log/debug_level
    which are not supported by this.  We'll use our own prompter without deriving from the FRIBDAQ one.
    
    '''
    @classmethod
    def parameters(cls) -> dict[str, type]:
        '''
            Provide the data source options.
        '''
        
        executable = _findExecutable()
        if not executable:                        # Data source won't be created anyway.
            return {}
        
        
        params = super().parameters()      # Starting point:
        
        # Get rid of the unused items not supported by fribdaq-readout:
        
        del params['log']
        del params['debug_level']

        # Add the parameters we need
        
        params['connection_type'] = str      # eth, usb, usb-serial, usb-index
        params['connection_parameter'] = str # Depends on the connection type.
        # params['init_only'] = bool           # Only initialize then exit. we're not going to allow it
        params['ignore_vme_init_errors'] = bool # Ignore VME errors on init.
        params['timestamp_lib']   = str      # Path to timestamp extraction lib.
        params['ctlconfig']       = str      # Path to the control configuration script.
        params['debug']            = bool     # Enable debugging output.
        params['trace']            = bool     # Enable trace output.
        params['convert-tcl']      = bool     # Create the YAML configuration from the config tcl file.
        params['template']         = str      # Yaml template file..requires convert-tcl to be true.
        params['daqconfig']        = str      # DAQ configuration file.
        
        return params         


    def __init__(self, config : dict[str, object], **kwargs) : 
        '''
        The super class does all the work:
        '''
        exe = _findExecutable()
        if not exe:
            raise RunTimError('BUGCHECK - the frib-reaedout program cannot be located')
        
        config['program_path'] = exe
        
        super().__init__(config, **kwargs)
        
    def createCommandLine(self) -> str:
        '''
        Create the command line that will run on the end of the SSH Pipe.
        In addition to the options implied by the parameterization, we add an --init-script
        to start the httpd/ReST server in the program.  Note the parent class will
        take care of prepending any stuff needed to start a container in which the program runs.
        
        '''
        

        
        command = self.create_command_env()
        command += self.cget('program_path')    # __init__ patched this in place.

        # Now figure out the program options:
        
        command += ' --ring ' + self._configuration.get('ring', getpass.getuser())
        command += ' --sourceid ' + str(self._configuration.get('source_id', 0))
        
        if 'port' in self._configuration:
            command += f' --control-server {self.cget("port")}'
            command += f' --ctlconfig {self.cget("ctlconfig")}'
            
        # We need to add an initscript so that the ReST server starts.
        # Since daqsetup will not necessarily have been run in the target host:
        
        daqshare = os.environ['DAQSHARE']
        command += f' --init-script {daqshare}/scripts/rest_init_script.tcl'
        
        # Set up the connection to the MVLC - connection type is required:
        
        match self._configuration['connection_type']:
            case 'eth':
                command += f' --mvlc-eth {self.cget("connection_parameter")}'
            case 'usb':
                command += ' --mvlc-usb'
            case 'usb-serial':
                command += f' --mvlc-usb-serial {self.cget("connection_parameter")}'
            case 'usb-index':
                command += f' --mvlc-usb-index {self.cget("connection_parameter")}'
                
        if self._configuration.get('ignore_vme_init_errors', False):
            command += ' --ignore-vme-init-errors'
            
        if self._configuration.get('timestamp_lib') :
            command += f' --timestamp-library {self.cget("timestamp_lib")}'
        
        if self._configuration.get('debug'):
            command += ' --debug'
        
        if self._configuration.get('trace'):
            command += ' --trace'
        
        if self._configuration.get('convert-tcl'):
            command += ' --convert-tcl'
            if self._configuration.get('template'):   # Only allowed if --convert-tcl
                command += f'--template {self.cget("template")}'
        
        # Finally the daqconfig. not as an option but a command parameters:
        
        command += f" {self.get('daqconfig')}"
        
        return command
    
    # That's all we needed for FRIBDAQDataSource to take over the work.
    
    
class ConfigurationDisplay(nscldaq.readoutgui.FRIBDAQDataSource.ConfigurationDisplay):
    '''
        Display the configuration of our data source.
        The base class does some of the work.  We have to do the fribdaq-readout specific parameters.
    '''
    def __init__(self, source : MVLCDataSource, parent : QWidget | None = None):
        super().__init__(source, parent)
        config = source.getConfig()
        
        # For some connection types we have a connection parameter:
        
        connection_type = config['connection_type']
        if connection_type in ('eth', 'usb-serial', 'usb-index'):
            connection_type += ' ' + config['connection_parameter']
        self._addRow('Connection:', connection_type)
        
        self._addRow(
            'Ignore VME init errors', 'Yes' if config.get('ignore_vme_init_errors', False) else 'No'
        )
        self._addRow('Extract timestamps with: ', config.get('timestamp_lib', 'None'))
        self._addRow('Readout config', config['daqconfig'])
        self._addRow('Slow controls config: ', config.get('ctlconfig', 'None'))
        self._addRow('Debug output', 'enabled' if config.get('debug', False) else 'disabled')
        self._addRow('Trace output', 'enabled' if config.get('trace', False) else 'disabled')
        self._addRow('Convert Tcl Daqconfig -> Yaml', 'yes' if config.get('convert-tcl', False) else 'no')
        self._addRow('YAML template file: ', config.get('template') if config.get('template') else 'default')
        
        
class ConfigureSource(QWidget):
    '''
        This is a prompter for the configuration of an MVLC Readout program (fribdaq-readout).
        Note that instantiating this when the executable cannot be locate is a fatal error (after an
        explantory message box.)
    '''
    def __init__(self, source : MVLCDataSource | None = None, parent : QWidget | None = None):
        '''
            @param source : MVLCDataSource | None - if not None, the form is pre-loaeded with the
                          configuration of this source.
            @param parent : QWidget | None - If provided, the parent widget of us.
        '''
        # Must be able to find the fribdaq-reaedout executable:
        
        if not _findExecutable():
            QMessageBox.critical(
                self, 
                'No fribdaq-readout', 
                "Can't locate the fribdaq-readout program, define the MVLC_DIR to the installation \
directory of mesytec-mvlc and be sure it was built with FRIBDAQ support enabled."
            )
            QApplication.exit(-1)       # push an exit.
            return                      # Ensuring we don't do anythnig else
        
        super().__init__(parent)
        
        self.setLayout(QVBoxLayout())   # Primarily  a vertical layout.
        layout = self.layout()
        # Title the prompter including the location of fribdaq readout:
        
        titlelayout = QHBoxLayout()
        titlelayout.addWidget(QLabel('Configure', self))
        titlelayout.addWidget(QLabel(_findExecutable(), self))
        layout.addLayout(titlelayout)
        
        #  Connection type used:
        
        connectionlayout = QHBoxLayout()
        connectionlayout.addWidget(QLabel('Connect via', self))
        
        self._connection_type = QComboBox(self)
        self._connection_type.addItems(['usb', 'usb by serial', 'usb by index', 'ethernet' ])
        connectionlayout.addWidget(self._connection_type)
        self._connection_type.currentTextChanged.connect(self._connectionChanged)
        
        self._connection_string = QLineEdit(self)
        self._connection_string.setEnabled(False)
        connectionlayout.addWidget(self._connection_string)
    
        layout.addLayout(connectionlayout)
    
        # DAQConfig (browsable).
        
        daqcfglayout = QHBoxLayout()
        daqcfglayout.addWidget(QLabel('Readout Config: ', self))
        
        self._daqconfig = QLineEdit(self)
        daqcfglayout.addWidget(self._daqconfig)
        
        self._browsedaqconfig = QPushButton('Browse...', self)
        daqcfglayout.addWidget(self._browsedaqconfig)
        self._browsedaqconfig.clicked.connect(
            lambda : self._browseFile(self._daqconfig, yaml = True, tcl = True)
        )
        
        layout.addLayout(daqcfglayout)  
        
        
        
        # CTLCOnfig - browsable  only Tcl...
        
        ctlconfiglayout = QHBoxLayout()
        ctlconfiglayout.addWidget(QLabel('Slow controls config file', self))
        
        self._ctlconfig = QLineEdit(self)
        ctlconfiglayout.addWidget(self._ctlconfig)
        
        self._browsectlconfig = QPushButton('Browse...', self)
        ctlconfiglayout.addWidget(self._browsectlconfig)
        self._browsectlconfig.clicked.connect(
            lambda: self._browseFile(self._ctlconfig, yaml = False, tcl = True)
        )
        
        layout.addLayout(ctlconfiglayout)
        
        # Ringbuffer:
                
        ringlayout = QHBoxLayout()
        ringlayout.addWidget(QLabel('Ring Buffer:', self))
        
        self._ring = QLineEdit(self)
        ringlayout.addWidget(self._ring)
        
        layout.addLayout(ringlayout)
        
        # port is for slow controls:
        
        scportlayout = QHBoxLayout()
        scportlayout.addWidget(QLabel('Slow controls port', self))
        
        self._controlport = QSpinBox(self)
        scportlayout.addWidget(self._controlport)
        self._controlport.setMinimum(1024)
        self._controlport.setMaximum(29999)
        
        layout.addLayout(scportlayout)
        
        #  checkbutton for convert-tcl and anoptional
        #  template file that's disabled unless the checkbutton is enabled.
        #  The tamplate can also be browsed:
        
        self._converttcl = QCheckBox('Convert daqconfig from tcl', self)
        layout.addWidget(self._converttcl)
        self._converttcl.clicked.connect(self._setTemplateState)
        
        templatelayout = QHBoxLayout()
        templatelayout.addWidget(QLabel('Yaml Template File', self))
        self._template = QLineEdit(self)
        templatelayout.addWidget(self._template)
        self._template.setPlaceholderText('<Use default>')
        
        self._browseTemplate = QPushButton('Browse...', self)
        templatelayout.addWidget(self._browseTemplate)
        self._browseTemplate.clicked.connect(
            lambda: self._browseFile(self._template)
        )
        
        layout.addLayout(templatelayout)
        
        #  Output control are just a pair of checkbuttons:
        
        outputlayout = QHBoxLayout()
        
        self._debug = QCheckBox('Debug output', self)
        outputlayout.addWidget(self._debug)
        
        self._trace = QCheckBox('Trace output', self)
        outputlayout.addWidget(self._trace)
        
        layout.addLayout(outputlayout)
        
    # Slots (Private)
    
    def _connectionChanged(self, newtype) :
        # The connection type changed, enable/disable, connection_string accordingly.
    
        enabled = newtype in ['usb by serial', 'usb by index', 'ethernet']
        self._connection_string.setEnabled(enabled)
        
    def _browseFile(self, target : QLineEdit, yaml = True, tcl = False):
        # Browse for a file and put it in the target.  yaml - sets a filter to 
        # allow browsing type .yaml,yml and tcl adds a filter for .tcl
        
        filter = ''
        if tcl:
            filter += 'Tcl Files (*.tcl);;'
        if yaml:
            filter += 'YAML files (*.yaml *.yml);;'
        filter += 'All Files (*)'
        
        file, _  = QFileDialog.getOpenFileName(self, 'Choose file', '.', filter)
        
        if file.strip():
            target.setText(file)
    
    def _setTemplateState(self):
        # Set the state of the template widgets:
        
        state = self._converttcl.checkState() == Qt.CheckState.Checked
        for w in [self._template, self._browseTemplate]:
            w.setEnabled(state)
    
# Test code:

if __name__ == '__main__':
    import sys
    from nscldaq.readoutgui import ReadoutGuiView
    
    
    app = QApplication(sys.argv)
    dummyGui = ReadoutGuiView.ReadoutGuiMainWindow()
    config = {
        'host' : 'localhost',
        'ring' : 'fox',
        'source_id' : 123,
        'connection_type' : 'eth',
        'connection_parameter': 'mvlc0.frib.msu.edu',
        'trace': True,
        'convert-tcl' : True,
        'daqconfig' : '/home/ron/daqconfig.tcl'
    }
    source = MVLCDataSource(config)
    win = ConfigureSource(source)
    
    win.show()
    
    sys.exit(app.exec())                