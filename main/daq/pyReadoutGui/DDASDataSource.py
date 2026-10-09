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

import os
import getpass
import nscldaq.readoutgui.FRIBDAQDataSource
from PyQt6.QtWidgets import QWidget

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
            try :
                configuration['program_path'] = os.path.join(
                    os.environ['DAQBIN'], 'ddasReadout'
                )
            except KeyError as e:
                raise RuntimeError('DAQBIN ust be defined to locate ddasReadout, setup a version of FRIB/NSCLDAQ')
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
    win = ConfigurationDisplay(source)
    
    win.show()
    
    sys.exit(app.exec()) 
        
    
