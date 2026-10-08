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
@file VMUSBDataSource.py
@brief Data Source for VMUSBReadout
@author Ron Fox with an assist from copilot
'''


import os
from typing import ClassVar

import nscldaq.readoutgui.FRIBDAQDataSource
from PyQt6.QtWidgets import (
    QCheckBox,
    QFileDialog,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QRadioButton,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)
from PyQt6.QtCore import Qt


#  Factor out computation of program path.

def _programPath() -> str:
    return os.path.join(
        os.environ['DAQBIN'], 'VMUSBReadout'
                )

class VMUSBDataSource(nscldaq.readoutgui.FRIBDAQDataSource.FRIBDAQSource):
    """FRIBDAQ source configured to run VMUSBReadout by default."""

    def __init__(self, parameters: dict[str, object], **kwargs):
        configuration = dict(parameters)
        # In case the user has a custom VMUSBConfig file
        # we only set the program_path if it's not set by the
        # caller.
        if 'program_path' not in configuration:
            try:
                configuration['program_path'] = _programPath()
            except KeyError as error:
                raise RuntimeError(
                    'DAQBIN must be set to locate VMUSBReadout'
                ) from error

        super().__init__(configuration, **kwargs)

    @classmethod
    def parameters(cls) -> dict[str, type]:
        '''
        We need to add the VMUSBReadout parameters to the configuration:
        
        daqconfig  - path to the daq config file.
        ctlconfig  - path to the ctl config file.
        serial     - Serial string to connect to.
        control_port - Slow controls server port.
        timestamplib - Library to extract timestamps from
        quickstart  - Use quick start.
        
        @return dict[str, type] - updated parameterization
        '''
        params = super().parameters()
        params['daqconfig'] = str
        params['ctlconfig'] = str
        params['serial']    = str
        params['timestamplib'] = str
        params['quickstart']   = str
        return params 
    
    
    def createCommandLine(self) -> str:
        '''
            @return str - the command line with which to run the program.
            
        ''' 
        command = super().createCommandLine()
        
        # Now we need to add any additional options:
        
        command += f' --daqconfig={self.cget("daqconfig")}'
        command += f' --ctlconfig={self._configuration.get("ctlconfig", "/dev/null")}'
        
        #  --serialno is not on the line if not defined:
        
        serial  = self._configuration.get('serial')
        if serial:
            command += f' --serialno={serial}'
        
        command += f' --port={self._configuration.get("control_port", 27000)}'
        
        # IF there's no timestamp lib, that too is not used:
        
        tslib = self._configuration.get('timestamplib')
        if tslib:
            command += f' --timestamplib={tslib}'
        
        # Quickstart - must be present in the config to be used and the value
        # must be 'on' or 'off
        
        quickstart = self._configuration.get('quickstart')
        if quickstart:
            if quickstart in ('on', 'off'):
                 command += f' --quickstart={quickstart}'             
            else:
                raise ValueError(
                    f'"quickstart" configuration parameter must be "on" or "off", was {quickstart}')
        
        # return the command.
        
        return command

# Configuration display and configuration methods:

class ConfigurationDisplay(nscldaq.readoutgui.FRIBDAQDataSource.ConfigurationDisplay):
    '''
        We just have to display the base class configuration and then
        add to it the additional parameters we have.
    '''
    def __init__(self, source : VMUSBDataSource, parent : QWidget | None = None):
        super().__init__(source, parent)
        
        config = source.getConfig()
        self._addRow('Readout Configuration:', config['daqconfig'])
        
        if 'ctlconfig' in config:
            self._addRow('Slow Control configuration', config['ctlconfig'])
        
        if 'serial' in  config:
            self._addRow('Connect to VMSUB: ', config['serial'])
        
        
        if 'timestamplib' in config:
            self._addRow('Extract timestamps with', config['timestamplib'])
        
        quickstart = config.get('quickstart', 'off')
        self._addRow('Quick start is', quickstart)
    

class ConfigureSource(nscldaq.readoutgui.FRIBDAQDataSource.ConfigureSource):
    '''
    Instantiate this in a dialog to cofigure a data sourc.
    Note that this just adds extra fields to the FRIBDAQDataSource
    configuration form for the extra parameters the VMUSBReadout uses.
    The program remains editable in case someone's done a custom extended
    VMUSB Readout...not currently supported but maybe possible.
    
    We completely re-implement makeSource, however to instantiate the
    correct data source.
    
    For load, we load the base and our additional stuff.
    '''
    
    # New set of mandatory_parmaeters...daqconfig is also needed.
    _mandatory_parameters : ClassVar[tuple[str]] = ('host', 'program_path', 'daqconfig')

    def __init__(self, source : VMUSBDataSource | None = None, parent : QWidget | None = None):
        super().__init__(source, parent)   # Layout base class form.
        
        layout : QVBoxLayout = self.layout()     
        
        # Daqconfig -  no default.
        
        dclayout = QHBoxLayout()
        dclayout.addWidget(QLabel('Readout Configuration', self))
        self._daqconfig = QLineEdit(self)
        dclayout.addWidget(self._daqconfig)
        
        self._browsedaqconfig = QPushButton('Browse...', self)
        dclayout.addWidget(self._browsedaqconfig)
        self._browsedaqconfig.clicked.connect(
            lambda: self._browseConfigFile(self._daqconfig)
        )
        
        layout.addLayout(dclayout)
        
        # Ctlconfig - default /dev/null:
        
        cclayout = QHBoxLayout()
        cclayout.addWidget(QLabel('Control configuration', self))
        
        self._ctlconfig = QLineEdit(self)
        cclayout.addWidget(self._ctlconfig)
        self._ctlconfig.setText('/dev/null')
        
        self._browsectlconfig = QPushButton('Browse...')
        cclayout.addWidget(self._browsectlconfig)
        self._browsectlconfig.clicked.connect(
            lambda: self._browseConfigFile(self._ctlconfig)
        )
        
        layout.addLayout(cclayout)
        
        # Serial number connection:
        
        snolayout = QHBoxLayout()
        self._enablesno = QCheckBox('Connect By serial', self)
        snolayout.addWidget(self._enablesno)
        
        snolayout.addWidget(QLabel('Serial String:', self))
        self._serial = QLineEdit(self)
        snolayout.addWidget(self._serial)
        self._serial.setInputMask('VM9999')
        self._serial.setEnabled(False)
        self._serialno_widgets = [self._serial,]
        self._enablesno.clicked.connect(
            lambda: self._setWidgetStates(self._enablesno, self._serialno_widgets)
        )
        layout.addLayout(snolayout)
        
        
        # Optional timestamp extraction lib:
        
        tslayout = QHBoxLayout()
        self._enablets = QCheckBox('Extract Timestamps:', self) 
        tslayout.addWidget(self._enablets)
        tslayout.addWidget(QLabel('Extraction library', self))
        
        self._tslib = QLineEdit(self)
        self._tslib.setEnabled(False)
        tslayout.addWidget(self._tslib)
        
        self._browsets = QPushButton('Browse...', self)
        tslayout.addWidget(self._browsets)
        self._browsets.setEnabled(False)
        self._browsets.clicked.connect(self._browseSo)
        
        self._tswidgets = [self._tslib, self._browsets]
        self._enablets.clicked.connect(
            lambda: self._setWidgetStates(self._enablets, self._tswidgets)
        )
        
        layout.addLayout(tslayout)
        
        # Quick start:
        
        layout.addWidget(QLabel('Quick Start options:', self))
        qslayout = QHBoxLayout()
        self._qsOn = QRadioButton('On', self)
        self._qsOn.setChecked(False)
        
        self._qsOff = QRadioButton('Off (recommended)', self)
        self._qsOff.setChecked(True)
        
        qslayout.addWidget(self._qsOn)
        qslayout.addWidget(self._qsOff)
        
        layout.addLayout(qslayout)
        
        # Patch the program path:
        
        self._program.setText(_programPath())
        
        # If a source was provided, load the form:
        # We do things this way to allow derivation of
        # this class forcing our _loadFOrm method to be called.
        #  .. since this is __init__
        if source:
            ConfigureSource._loadForm(self, source)
            

    # Utilities:
    
    def _loadForm(self, source : VMUSBDataSource) -> None:
        # Load the form from the existing VMUSB Data Source object.
        
        super()._loadForm(source)     # Load the superclass form.
        config = source.getConfig()   # Configuration to load from.
        # Load the stuff that the base class didn't.
        # 'daqconfig', 'ctlconfig', 'serial', 'timetamplib', and 'quickstart'
        
        self._daqconfig.setText(config.get('daqconfig', ''))
        self._ctlconfig.setText(config.get('ctlconfig', ''))
        
        if 'serial' in config:
            self._enablesno.setCheckState(Qt.CheckState.Checked)
            self._serial.setText(config['serial'])
        else:
            self._enablesno.setCheckState(Qt.CheckState.Unchecked) # Asume nothing.
        
        self._setWidgetStates(self._enablesno, self._serialno_widgets)   # Update state of widgets.
        
        if 'timestamplib' in config:
            self._enablets.setCheckState(Qt.CheckState.Checked)
            self._tslib.setText(config['timestamplib'])
        else:
            self._enablets.setCheckState(Qt.CheckState.Unchecked)
        self._setWidgetStates(self._enablets, self._tswidgets)
        
        # Who to check:
        
        if 'quickstart' not in config:
            qs = self._qsOff
        else:
            if config['quickstart'] == 'on':
                qs = self._qsOff
            elif config['quickstart'] == 'off':
                qs - self._qsOn
            else:
                raise ValueError(
                    f'"quickstart" configuration parameter must be either "on" or "off" was {config["quickstart"]}'
                )
        
        
        
    
    # Private slots:
    
    def _browseConfigFile(self, line : QLineEdit) -> None:
        # Browse for a .tcl/.config file and, on accepted, fill in
        # the line widget with the selected file:
        
        file, _ = QFileDialog.getOpenFileName(
            self, 'Choose Config File', '.', 
            'Tcl Scripts (*.tcl);;Config Files (*.cfg);; All Files (*)'
        )
        
        if file.strip():
            line.setText(file)
    def _browseSo(self) -> None:
        # Browse for the timestamp extraction library .so:
        
        file, _  = QFileDialog.getOpenFileName(
            self, 'Choose tslib', '.',
            'Shared libs (*.so);;All files (*)'
        )
        if file.strip():
            self._tslib.setText(file)
        
    def _setWidgetStates(self, check : QCheckBox, widgets : list[QWidget]) -> None:
        # Set the enabled state of a list of widgets based on the state of a
        # checkbox.
        
        state = self._isChecked(check)     # From base class.
        
        for w in widgets:
            w.setEnabled(state)
            
#  Test code:

if __name__ == '__main__':
    import sys
    from nscldaq.readoutgui import ReadoutGuiView
    from PyQt6.QtWidgets import QApplication
    app = QApplication(sys.argv)
    dummyGui = ReadoutGuiView.ReadoutGuiMainWindow()
    
    src = VMUSBDataSource({
        'host'      : 'localhost',
        'daqconfig' : '/home/ron/daqtest/daqconfig.tcl',
        'ctlconfig' : '/home/ron/daqtest/ctlconfig.tcl',
        'quickstart' : 'on'
        
    })
    
    win = ConfigureSource(src)
    win.show()
    
    sys.exit(app.exec())
        