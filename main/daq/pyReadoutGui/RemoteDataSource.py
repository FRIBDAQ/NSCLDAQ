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
@file RemoteDataSource.py
@brief Data source to remote control another readout gui.
@author Ron Fox

This remote data source can remote control both Tcl and Python Reaout
shells through the common protocol documented in e.g. 
https://docs.frib.msu.edu/daq/newsite/nscldaq-12.2/index.html
see the 5daq manpage in that document (Readout GUI Remote control protocol ).
'''

import getpass

from nscldaq.portmanager.PortManager import PortManager
from nscldaq.readoutgui.DataSource import DataSource
from nscldaq.readoutgui.DataSourceRegistry import DataSourceRegistry
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from PyQt6.QtCore import QCoreApplication, QIODeviceBase, QObject
from PyQt6.QtNetwork import QAbstractSocket, QTcpSocket
from PyQt6.QtWidgets import QHBoxLayout, QLabel, QLineEdit, QVBoxLayout, QWidget

DEFAULT_CONTROL_SERVICE : str = 'ReadoutGUIRemoteControl'
DEFAULT_OUTPUT_SERVICE  : str = 'ReadoutGUIOutput'
DEFAULT_USER            : str = getpass.getuser()


def _applyDefaults(config : dict[str, object]) -> dict[str, object]:
    # Given a config, applies the defaults:
    # Raises a key error if host is not set.
    return {
        'host'            : config['host'],      # No default
        'control_service' : config.get('control_service', DEFAULT_CONTROL_SERVICE),
        'output_service'  : config.get('output_service', DEFAULT_OUTPUT_SERVICE),
        'user'            : config.get('user', DEFAULT_USER)
        
    }

class RemoteDataSource(DataSource):
    '''
    See the module docstrings.  This class implements the data source.
    It's parameters are:
    host:  - The host in which the remote is running.
    control_service - The service advertised by the remote for control operations.
    output_service  - The service advertised by the remote that relays output.
    user            - The user running the remote GUI.
    
    Defaults:
    host   - None, required.
    control_service -  'ReadoutGUIRemoteControl'
    output_service  -  'ReadoutGUIOutput'
    user            - getpass.getuser()

    '''
    def __init__(self, parameters : dict[str, object],  parent : QObject | None = None):
        '''
        Construction parameters:
        parameters - the configuration of the client.  A dict of string -> value mappings.
        parent     - Optional Qt parent object.
        '''
        
        super().__init__(parameters = parameters, parent = parent)
        self._validate_configuration()
        self._remoteClient : QTcpSocket | None = None
        self._outputClient : QTcpSocket | None = None
    
        
    @classmethod
    def parameters(cls) -> dict[str, type]:
        '''
        @return dict[str, type]  - the keys are parameter names, the value the allowed parameter type.
        '''
        return {
            'host'            :  str,
            'control_service' : str,
            'output_service'  : str,
            'user'            : str
        }
    
    def capabilities(self) -> dict[str, bool]:
        '''
        We need to flag that remote data sources don't in general, support pause.
        @return dict[str, bool]  keys are capabilities, values are True if supported.
        '''
        return {
            'canPause'        : True, 
            'runsHaveTitles'   : True, 
            'runsHaveNumbers' : True
        }
    def canBegin(self) -> bool:
        '''
        @return bool - True if the remote can start a run.
        '''
    
        state = self._getstate()    # Returns None if not connected.
        return state == 'inactive' or state == 'paused'
    
    def start(self) -> None:
        '''
        Create and connect the client sockets....
        '''
        config = self._applyDefaults()
        remcon, output = self._getServices(config)
        
        # Destroy any old sockets and connect to the services:
        
        self._remoteClient = QTcpSocket(self)
        self._remoteClient.connectToHost(config['host'], remcon )
        if not self._remoteClient.waitForConnected():
            print(f"Connection to {config['host']} failed!!")
        
        self._outputClient = QTcpSocket(self)
        self._outputClient.connectToHost(
            config['host'], output, QIODeviceBase.OpenModeFlag.ReadOnly
        )
        # Catch readable 
        
        self._outputClient.readyRead.connect(self._relayOutput)
        
        
        # Just in case connection wasn't enough:
        
        result = self._enslave()  # should not have an error so:
        if result != 'OK':
            raise RuntimeError(f'Expected OK from set slave 1 got {result}')
        
    def check(self) -> bool:
        '''
        @return bool - True if the remote is responsive and alive.
        '''
        
        response = self._transact('get state\n')
        if response is None:
            return False
        status = response.split()[0]
        return status == 'OK'
        
    def stop(self) -> None:
        '''
        Close down the services...
        '''

        if self._remoteClient:
            self._remoteClient.disconnectFromHost()
        if self._outputClient:
            self._outputClient.disconnectFromHost()
        
        
        self._remoteClient = None
        self._outputClient = None
        
    def begin(self, run : int, title : str) -> None:
        '''
        Try to start a run in the remote.
        @param run : int - the run number to set.
        @param title : str - The title of the run.
        @todo figure out more graceful error handling.
        '''
        
        # Why starts with... because the event loop runs in the middle of
        # waiting for the response and this can cause check to interleave.
        # mostly harmless but....
        
        host = self.getConfig()['host']
        if not self._transact(f'set title {{{title}}}\n') == 'OK':
            raise RuntimeError(f'Failed to set the remote title in {host}')
        
        if not self._transact(f'set run {run}\n') == 'OK':
            raise RuntimeError('Failed to set the remote run number in {host}')

        if not self._transact('begin\n') == 'OK':
            raise RuntimeError(f'Failed to start remote in {host}')
    
    def end(self) -> None:
        host = self.getConfig()['host']
        if self._transact('end\n') != 'OK':
            raise RuntimeError(f'Failed to end the run in {host}')
    
    def pause(self) -> None:
        # Ask the remote to pause  a run.
        
        host = self.getConfig()['host']
        response = self._transact('masterTransition Paused\n')
        if response != 'OK':
            raise RuntimeError(f'Failed to pause the run in {host} : {response}')
        
    def resume(self) -> None:
        # Ask the remote to resume a run (make it active again:)
        
        host = self.getConfig()['host']
        response = self._transact('masterTransition Active\n')
        if response != 'OK':
            raise RuntimeError(f'Failed to resume the run in {host} : {response}')
        
    # Utilities:
    
    def _applyDefaults(self) -> dict[str, object]:
        #  Apply defaults to the configuration.
        config = self.getConfig()
        return _applyDefaults(config)
    
    def _getServices(self, config) -> tuple[int, int]:
        # using the configuraiton (with defaults applied), looks
        # up the services and returns a tuple containing the
        # remote control and output relay ports in that order.
        
        mgr = PortManager(config['host'])
        remcon = mgr.find(service = config['control_service'], user = config['user'])
        if len(remcon) == 0:
            raise ConnectionRefusedError(f'No service {config["control_service"]} advertised by {config["user"]}')
        
        output = mgr.find(service = config['output_service'], user = config['user'])
        if len(output) == 0:
            raise ConnectionRefusedError(f'No service {config["control_service"]} advertised by {config["user"]}')
        
        #  This may be cruft from bygone days we don't need?
        
        
        return (remcon[0]['port'], output[0]['port'])
    
    def _enslave(self):
        # Write 'set slave 1\n' and flush to the control port
        
        return self._transact('set slave 1\n')
    
    def _getstate(self):
        # Do a get state transaction and, on success, return the state.
        reply = self._transact('get state\n')
        words = reply.split(' ')
        if words[0] != 'OK':
            raise RuntimeError(f'_getstate expected OK and state got {reply}')
        return words[1]
    def _transact(self, operation : str) -> str | None:
        # Send and operation to the selave and accept the reply.
        # the response is returned stripped of \n.  None is returned if
        # there was no response (lost connection e.g.)

        self._remoteClient.write(operation.encode('utf-8'))
        self._remoteClient.flush()
        while self._remoteClient.bytesAvailable() == 0:
            self._remoteClient.waitForReadyRead(1000)
            if self._remoteClient.state() != QAbstractSocket.SocketState.ConnectedState:
                break                   # Connection lost.
        response = self._remoteClient.readLine()
        if response.isEmpty():
            print("transact got an empty reponse!!!")
            return None
        else:
            string = response.data().decode('utf-8')
            return string.strip('\r\n')
            
        
    def _relayOutput(self) -> None:
        text = self._outputClient.readAll().data().decode('utf-8')
        if text:
            config = _applyDefaults(self.getConfig())
            tab = f'{config["user"]}@{config["host"]}'
            output = mainWindow().centralWidget().Outputs().addOutput(tab)
            output.append(text)
        
    

# Report the data source configuration:

class DisplayConfig(QWidget):
    '''
    Widget to display the configuration of a slave:
    '''
    def __init__(self, source : RemoteDataSource, parent : QWidget | None = None):
        super().__init__(parent)
        self.setLayout(QVBoxLayout())
        
        config = _applyDefaults(source.getConfig())   # Config with defaults applied.
        
        # THe host
        
        hostLayout = QHBoxLayout()
        hostLayout.addWidget(QLabel('Host', self))
        hostLayout.addWidget(QLabel(config['host'], self))
        self.layout().addLayout(hostLayout)

        # The remote control service:
        
        rctlLayout = QHBoxLayout()
        rctlLayout.addWidget(QLabel('Control Service', self))
        rctlLayout.addWidget(QLabel(config['control_service'], self))
        self.layout().addLayout(rctlLayout)


        # The output relay service
        
        outputLayout = QHBoxLayout()
        outputLayout.addWidget(QLabel('Output relay service', self))
        outputLayout.addWidget(QLabel(config['output_service'], self))
        self.layout().addLayout(outputLayout)
        
        # The user:
        
        userLayout = QHBoxLayout()
        userLayout.addWidget(QLabel('User', self))
        userLayout.addWidget(QLabel(config['user'], self))
        self.layout().addLayout(userLayout)
    
    
class ConfigurationPrompter(QWidget):
    ''''
    Instantiate this in e.g. a SaveDialog to prompt for the 
    configuration of a remote controlled data source.
    Once the form has been filled in, call makeSource
    to create the data source or a string error text.
    '''        
    def __init__(self, source : RemoteDataSource | None = None, parent : QWidget | None = None):
        '''
        @param source - optional source, fills in the form from its configuration 
               with defaults applied.
        @param parent - parent of this widget.
        '''
        super().__init__(parent)
        
        # Just like DisplayConfig, but the 'values' are LineEdits
        
        self.setLayout(QVBoxLayout())
        
        # Host:
        hostLayout = QHBoxLayout()
        hostLayout.addWidget(QLabel('Host', self))
        self._host = QLineEdit(self)
        hostLayout.addWidget(self._host)
        # There's no default for the host.
        self.layout().addLayout(hostLayout)
        
        # Remote control service:
        
        rctlLayout = QHBoxLayout()
        rctlLayout.addWidget(QLabel('Control Service', self))
        self._rctl = QLineEdit(self)
        rctlLayout.addWidget(self._rctl)
        self._rctl.setText(DEFAULT_CONTROL_SERVICE)
        self.layout().addLayout(rctlLayout)
        
        # output service:
        
        outLayout = QHBoxLayout()
        outLayout.addWidget(QLabel('Output service', self))
        self._output = QLineEdit(self)
        outLayout.addWidget(self._output)
        self._output.setText(DEFAULT_OUTPUT_SERVICE)
        self.layout().addLayout(outLayout)
        
        #  User running the remote:
        
        userLayout = QHBoxLayout()
        userLayout.addWidget(QLabel('User', self))
        self._user = QLineEdit(self)
        userLayout.addWidget(self._user)
        self._user.setText(DEFAULT_USER)
        self.layout().addLayout(userLayout)
        
        if source:
            config = _applyDefaults(source.getConfig())
            self._load(config)
            
    
    def makeSource(self) -> RemoteDataSource | str:
        # Gather the configuration:
        config = {}
    
        for name,widget in [('host', self._host), 
                            ('control_service', self._rctl), 
                            ('output_service', self._output), 
                            ('user', self._user)]:
            text = widget.text().strip()
            if text:
                config[name] = text
        
        if not 'host' in config:
            return 'The host configuration paramter is mandatory'
    
        return RemoteDataSource(parameters=config)
            
        
        # Host is mandatory.
        
    def _load(self, config : dict[str, str]) -> None:
        # Load the line edits from the config.  The config has had
        # defaults applied
        
        host = config.get('host')   # The only one that might be (in)conceivably None:
        if host:
            self._host.setText(host)
        
        self._rctl.setText(config['control_service'])
        self._output.setText(config['output_service'])
        self._user.setText(config['user'])
    
# Register the data source type:

DataSourceRegistry.instance().add('Slave GUI', RemoteDataSource, DisplayConfig, ConfigurationPrompter)

#  Test code:

if __name__ == "__main__":
    import sys
    from PyQt6.QtWidgets import QApplication
    from nscldaq.mg_configutils import SaveDialog, DismissDialog
    
    app = QApplication(sys.argv)
    
    # Prompt for a source:
    
    prompt = SaveDialog(ConfigurationPrompter())
    prompt.exec()
    src = prompt.workarea().makeSource()
    del prompt
    if isinstance(src, str):
        print(src)
    else:
        list = DismissDialog(DisplayConfig(source=src))
        list.exec()
        del list

        p = SaveDialog(ConfigurationPrompter(src))
        p.exec()
        del p
        
    
