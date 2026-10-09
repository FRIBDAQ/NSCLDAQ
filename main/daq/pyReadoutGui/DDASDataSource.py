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
    @file DDASDataSource.py
    @brief Provide a data source for an XIA/DDAS crate
    @author Ron Fox
'''

import getpass
import os

import nscldaq.readoutgui.FRIBDAQDataSource
from PyQt6.QtGui import QIntValidator
from PyQt6.QtWidgets import (
    QCheckBox,
    QFileDialog,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QSpinBox,
    QWidget,
)

def _readoutProgram() -> str:
    try:
        return os.path.join(
                    os.environ['DAQBIN'], 'ddasReadout'
                )
    except KeyError as e:
        raise RuntimeError('DAQBIN ust be defined to locate ddasReadout, setup a version of FRIB/NSCLDAQ')

class DDASDataSource(nscldaq.readoutgui.FRIBDAQDataSource.FRIBDAQSource):
    '''
        FRIBDAQ data sourcde that runs ddasReadout to start both
        the readout for an XIA/DDAS crate and its hit sort program.
        see the parameters method for more about the
        additional parameters that are possible.
        
    '''
    def __init__(self, parameters : dict [str, object], **kwargs):
        configuration = dict(parameters)
        #  If the user has a custom program we will support that but
        #  will default the program to $DAQBIN/ddasReadout
        
        if 'program_path' not in configuration:
            configuration['program_path'] = _readoutProgram()
        super().__init__(configuration, **kwargs)
        
    @classmethod
    def parameters(cls) -> dict[str, type]:
        '''
            Define the additional configuration parameters we support note that readouthost is
            taken from 'host'....and that's where the script is run. and sortring is taken from ring
            (the final output ring).
            Additional parameters:
            
            rawring  - The ringbuffer URI  Readout puts its data in, default is tcp://$host/user-name
            sorthost - THe the host in which the hit sorter is run.
            crate_directory - the directory in which the crate definition files are in the readout host.
            fifo_threshold - defaults to 20480
            readout_buffersize - Bytes of buffering in the Readout.  defaults to 16384
            infinity_clock - defaults to off - if true infinity clock is used by the readout.
            clock_multiplier - Defaults to 1 what the clock needs to be multiplied by to get ns/tick
                               this is mostly for externally clocked data.
            scaler_period  - Seconds between scaler readouts. defaults to 2
            sort_window    - defaults to 10.0 - the sorting window for the sort program.
            fast_boot      - Defaults to off if on, the Readout can assume the crates is already initialized
                             and firmware loaded in all devices.
            

        '''
        
        # See above for the meaning of additions:
        
        base_params = super().parameters()     # Start with the standard parameters:
        additional_params = {
            'rawring'            : str, 
            'sorthost'           : str,
            'crate_directory'    : str,
            'fifo_threshold'     : int,
            'readout_buffersize' : int,
            'infinity_clock'     : bool, 
            'clock_multiplier'   : int,
            'scaler_period'      : int,
            'sort_window'        : int,
            'fast_boot'          : bool
        }
        params = base_params | additional_params
        
        return params
        
        
    def createCommandLine(self) -> str:
        '''
            @return str - the command line string to run over SSH.
            @note We have to do it all from scratch without the help of the
                  base class because:
                    -   Some options have differnet names and, even more importantly,
                    -   The ddasRadout script uses single dashes for the option strings
                        (e.g. -port not --port)
        '''
        command = self.create_command_env()   # Set up the environment.
        
        # We need to keep the stdin of Readout open when it starts the
        # ReST server so that the script does not thing it exited.
        
        command += 'RDOREST_KEEPSTDIN=1'
        
        # Now the program and its options.
        
        command += self.cget('program_path')
        command += f' -readouthost {self.cget("host")}'
        
        rawring = self._configuration.get('rawring', f'tcp://localhost/{getpass.getuser()}')
        command += f' -readoutring {rawring}'
        command += f' -sorthost {self.cget("sorthost")}'
        command += f' -sortring {self.cget("ring")}'
        command += f' -cratedir {self.cget("crate_directory")}'
        
        # These have defaults in the ddasReadout script and we defer to those
        # so we don't have to change code if those defaults should change.
        
        if 'source_id' in self._configuration:     # Defaults to 0.
            command += f' -sourceid {self.cget("source_id")}'
        if 'fifo_threshold' in self._configuration:   # Defaults to 20480
            command += f' -fifothreshold {self.cget("fifo_threshold")}'
        if 'readout_buffersize' in self._configuration:   # Defaults to 16934
            command += f' -buffersize {self.cget("readout_buffersize")}'
        if 'infinity_clock' in self._configuration:    # Defaults to false.
            value  = 'on' if self.cget('infinity_clock') else 'off'
            command += f' -infinity {value}'
        if 'clock_multiplier' in self._configuration:
            command += f' -clockmultiplier {self.cget("clock_multiplier")}'
        
        # We want our own default scaerl period to be more like 2:
        
        command += f' -scalerseconds {self._configuration.get("scaler_period", 2)}'
        
        if 'sort_window' in self._configuration:
            command += f' -window {self.cget("sort_window")}'
        
        if 'fast_boot' in self._configuration:
            value = 'on' if self.cget('fast_boot') else 'off'
            command += f' -fastboot {value}'
        if 'port' in self._configuration:
            command += f' -port {self.cget("port")}'
        if 'log' in self._configuration:
            command += f' -log {self.cget("log")}'
            if 'debug' in self._configuration:       # Only meaningful for log.
                command += f' -debug {self.cget("debug")}'
                
        try:
            init_script = os.path.join(os.environ['DAQSHARE'], 'scripts', 'rest_init_script.tcl')
        except KeyError:
            raise RuntimeError(
                'DAQSHARE must be defined to locat the rest init script.  Setup an FRIB/NSCLDAQ'
            )
        command += f' -init-script {init_script}'
        
        return command
            
                    
class ConfigurationDisplay(nscldaq.readoutgui.FRIBDAQDataSource.ConfigurationDisplay):
    '''
    Displays the configuration parameters of a DDASDataSource in a widget.
    The base paramters are displayed by the base class but we add lines below that
    for each of the configuration parameters we added:
                'rawring'            : str, 
                'sorthost'           : str,
                'crate_directory'    : str,
                'fifo_threshold'     : int,
                'readout_buffersize' : int,
                'infinity_clock'     : bool, 
                'clock_multiplier'   : int,
                'scaler_period'      : int,
                'sort_window'        : int,
                'fast_boot'          : bool
            }
    '''
    def __init__(self, source : DDASDataSource, parent : QWidget | None = None):
        super().__init__(source)
        
        config = source.getConfig()
        
        self._addRow('Raw Readout ring', config['rawring'])
        self._addRow('Host sorting hits', config['sorthost'])
        self._addRow('Crate file directory', config['crate_directory'])
        self._addRow('FIFO Threshold', str(config.get('fifo_threshold', 20480)))
        self._addRow('Raw Readout Buffer Size', str(config.get('readout_buffersize', 16934)))
        self._addRow('Infinity clock', 'Enabled' if config.get('infinity_clock', False) else 'Disabled')
        self._addRow('Clock multiplier', str(config.get('clock_multiplier', 1)))
        self._addRow('Scaler read period (secs)', str(config.get('scaler_period', 2)))
        self._addRow('Hit sort window (secs)', str(config.get('sort_window', 10)))
        self._addRow('Fast Boot', 'Enabled' if config.get('fast_boot', False) else 'Disabled')

class  ConfigureSource(nscldaq.readoutgui.FRIBDAQDataSource.ConfigureSource):
    '''
    Widget to configure and create XIA/DDAS data sources.  The idea is to display this in a
    dialog and then ask the widget to create the data source for you.
    Example:
    from nscldaq.mg_configutils import SaveDialog
    from PyQt6.QtWidgets import QDialog, QMessageBox
    from nscldaq.readoutgui import DDASDataSource
    ...
    
    dialog = SaveDialog(DDASDataSource.ConfigureSource())
    source : DDASDataSource.DDASDataSource | str | None = None
    while dialog.exec() == QDialog.DialogCode.Accepted:
        source  = dialog.workarea().makeSource()
        if isinstance(source, str):
            QMessageBox.warning(dialog, 'Missing parameters', source)
        else:
            break    
    # If source is None, the dialog was rejected, otherwise it's a data source,
    # ready to go.
    
    '''
    def __init__(self, source : DDASDataSource | None = None, parent : QWidget | None = None):
        super().__init__(source, parent)
        
        layout = self.layout()
        
        # Prompt for:
        # 'rawring' 
        
        rawlayout = QHBoxLayout()
        rawlayout.addWidget(QLabel('Raw Ring:', self))
        self._rawring = QLineEdit(self)
        rawlayout.addWidget(self._rawring)
        
        layout.addLayout(rawlayout)
        
        
        # Prompt For   'sorthost'
        
        sorthlayout = QHBoxLayout()
        sorthlayout.addWidget(QLabel('Hit sorting host', self))
        self._sorthost = QLineEdit(self)
        sorthlayout.addWidget(self._sorthost)
        
        layout.addLayout(sorthlayout)
        
        # Prompt forcrate_directory .. with browse button.
        
        cdirlayout = QHBoxLayout()
        cdirlayout.addWidget(QLabel('Crate file directory', self))
        
        self._cratedir = QLineEdit(self)
        cdirlayout.addWidget(self._cratedir)
        
        self._browsecdir = QPushButton('Browse..', self)
        cdirlayout.addWidget(self._browsecdir)
        self._browsecdir.clicked.connect(self._browseCrateDir)
        
        layout.addLayout(cdirlayout)
            
        #  prompt for  'fifo_threshold'  
       
        fifolayout = QHBoxLayout()
        fifolayout.addWidget(QLabel('Fifo Threshold', self)) 
        
        self._fifothreshold = QSpinBox(self)        
        fifolayout.addWidget(self._fifothreshold)
        self._fifothreshold.setMinimum(1024)    # Pretty small.
        self._fifothreshold.setMaximum(128*1024)
        self._fifothreshold.setSingleStep(1024)
        self._fifothreshold.setValue(20480)
        layout.addLayout(fifolayout)
        
        # prompt for  'readout_buffersize' 
        
        bsizelayout = QHBoxLayout()
        bsizelayout.addWidget(QLabel('Readout buffersize', self))
        
        self._buffersize = QSpinBox(self)
        bsizelayout.addWidget(self._buffersize)
        self._buffersize.setMinimum(8*1024)
        self._buffersize.setMaximum(128*1024)
        self._buffersize.setSingleStep(1024)
        self._buffersize.setValue(16*1024)
        
        layout.addLayout(bsizelayout)
        
        # Prompt for infinity clock on/off 'infinity_clock' 

        self._infinity = QCheckBox('Infinity clock', self)
        layout.addWidget(self._infinity)
        
        
        # Prompt for   'clock_multiplier'   : int,
        # Line edito with int validator.
        
        ckmullayout = QHBoxLayout()
        ckmullayout.addWidget(QLabel('Clock multiplier', self))
        
        self._clockmult = QLineEdit(self)
        ckmullayout.addWidget(self._clockmult)
        self._clockmult.setText(str(1))
        
        posintvalidator = QIntValidator(self)
        posintvalidator.setBottom(1)        # Validate to positive integers.
        self._clockmult.setValidator(posintvalidator)
        
        layout.addLayout(ckmullayout)
        
        # Prompt for         'scaler_period'      : int,
        # spinbox 1-3600   hour between scaler reads _ought_ to be sufficient.
        
        swlayout = QHBoxLayout()
        swlayout.addWidget(QLabel('Scaler period'))
        
        self._scalerperiod = QSpinBox(self)
        swlayout.addWidget(self._scalerperiod)
        self._scalerperiod.setMinimum(1)
        self._scalerperiod.setMaximum(3600)
        self._scalerperiod.setValue(2)                                     
        
        layout.addLayout(swlayout)
        
        #  Propmt for + integer       'sort_window'        : int,
        
        
        swinlayout = QHBoxLayout()
        
        swinlayout.addWidget(QLabel('Sort window (secs)', self))
        self._sortwindow = QLineEdit(self)
        swinlayout.addWidget(self._sortwindow)
        self._sortwindow.setText(str(10))
        self._sortwindow.setValidator(posintvalidator)
        
        
        layout.addLayout(swinlayout)
        
        #  Prompt for       'fast_boot'          : bool
        
        self._fastboot = QCheckBox('Fast Boot', self)
        layout.addWidget(self._fastboot)
        
        
        # Set the program name to the ddas readout script.
        # readonly.
        #  This reaches into the base class widget
        
        self._program.setText(_readoutProgram())
        self._program.setStyleSheet("""
            QLineEdit:disabled {
                color: #333333;        /* Dark gray/black text instead of faint gray */
                background-color: #F0F0F0; /* Light gray background to still indicate it's disabled */
                border: 1px solid #CCCCCC;
            }
        """)
        self._program.setEnabled(False)
        self._browse.setEnabled(False)
        
        if source:
            self._loadForm(source)
    
    
    # private utilities:
    
    def _loadForm(self, source : DDASDataSource) -> None:
        pass
    
    # Internal (private) slots.
        
    def _browseCrateDir(self) -> None:
        dir = QFileDialog.getExistingDirectory(self, 'Choose Crate Directory', '.')
        if dir.strip():
            self._cratedir.setText(dir)
        
# Test code for configuration classes:

if __name__ == '__main__':
    import sys
    from nscldaq.readoutgui import ReadoutGuiView
    from PyQt6.QtWidgets import QApplication, QDialog, QMessageBox
    from nscldaq.mg_configutils import SaveDialog
    
    
    app = QApplication(sys.argv)
    dummyGui = ReadoutGuiView.ReadoutGuiMainWindow()
    
    # Make a data source and display its configuration:
    
    config = {
        'host' : 'localhost', 
        'ring' : 'sorted',
        'rawring' : 'raw',
        'sorthost' : 'daqcompute001',
        'crate_directory' : '/home/ron/crate_1',
        
    }   
    source = DDASDataSource(config)
    win = ConfigureSource()
    
    win.show()
    
    sys.exit(app.exec()) 
        
    
