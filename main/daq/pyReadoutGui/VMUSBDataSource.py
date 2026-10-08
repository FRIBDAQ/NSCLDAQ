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

import nscldaq.readoutgui.FRIBDAQDataSource
from PyQt6.QtWidgets import QWidget

class VMUSBDataSource(nscldaq.readoutgui.FRIBDAQDataSource.FRIBDAQSource):
    """FRIBDAQ source configured to run VMUSBReadout by default."""

    def __init__(self, parameters: dict[str, object], **kwargs):
        configuration = dict(parameters)
        # In case the user has a custom VMUSBConfig file
        # we only set the program_path if it's not set by the
        # caller.
        if 'program_path' not in configuration:
            try:
                configuration['program_path'] = os.path.join(
                    os.environ['DAQBIN'], 'VMUSBReadout'
                )
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
        params['control_port'] = int
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
        
        self._addRow('Slow controls server port', str(config.get('port', 2700)))
        
        if 'timestamplib' in config:
            self._addRow('Extract timestamps with', config['timestamplib'])
        
        quickstart = config.get('quickstart', 'off')
        self._addRow('Quick start is', quickstart)
    
    
#  Test code:

if __name__ == '__main__':
    import sys
    from nscldaq.readoutgui import ReadoutGuiView
    from PyQt6.QtWidgets import QApplication
    app = QApplication(sys.argv)
    dummyGui = ReadoutGuiView.ReadoutGuiMainWindow()
    
    source = VMUSBDataSource({
        'daqconfig' : '/home/ron/daqtest/daqconfig.tcl',
        'ctlconfig' : '/home/ron/daqtest/ctlconfig.tcl',
        'serial'    : 'VM0123',
        'timestamplib' : '/home/ron/daqtest/tslib.so',
        
    })
    win = ConfigurationDisplay(source)
    win.show()
    
    sys.exit(app.exec())
        