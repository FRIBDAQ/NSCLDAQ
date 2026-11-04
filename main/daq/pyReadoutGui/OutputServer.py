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
@file OutputServer.py
@brief Serve output to clients.
@author Ron Fox.
 
 This advertises a serivce which, when connected to, relays output sent to the various
 tabs of the OutputMonitor to clients.  Unlike the slave server and output server of the
 Tcl readout gui, we support an arbitrary number of clients.
 
When output is signalled from the OutputManager we relay it to clients in the form:
tabname:whatever the output was.

'''
import io
import traceback
import types
from typing import ClassVar, Self

from nscldaq.mg_configutils import SaveDialog
from nscldaq.portmanager.PortManager import PortManager
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from nscldaq.readoutgui.StateSaveRestore import SerializationManager, Serializer
from PyQt6.QtCore import QByteArray, QObject, Qt
from PyQt6.QtNetwork import QTcpServer, QTcpSocket
from PyQt6.QtWidgets import (
    QCheckBox,
    QDialog,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMessageBox,
    QWidget,
)

DEFAULT_OUTPUT_SERVICE : str = 'ReadoutGUIOutput'

class OutputServer(QTcpServer):
    '''
    Provides a server that handles relaying output to the tabbed output
    widget to any set of clients desired.
    '''
    
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # List of connection objects
        
        self._connections : list[QTcpServer] = []
        
        # Connect to handle new connections.
        
        self.newConnection.connect(self._onConnect)
        
        # Connect to the ouptut manager:
        
        mainWindow().centralWidget().Outputs().output.connect(self._relayOutput)
    
    def __del__(self):
        #shut down any connections we have - hard.:
        
        for connection in self._connections:
            connection.abort()
    
    # Internal Slots:
    
    def _onConnect(self) -> None:
         # Drain all pending connections:
         
         while self.hasPendingConnections():
            socket : QTcpSocket = self.nextPendingConnection()
            self._connections.append(socket)
            # Remove from list when disconnected.
            socket.disconnected.connect(
                lambda : self._connections.remove(self._connections[-1])
            )

    def _relayOutput(self, tabname : str, output : str) -> None:
        #  Relay output to the connections.
        #  The assumption is that a connection loss will fire
        #  the signal needed to drop it from the list.
        outputText = f'{tabname} : {output}\n'
        outputTextq= QByteArray(outputText.encode('utf-8'))
        for connection in self._connections:
            connection.write(outputTextq)
            connection.flush()
            
class OutputServerController:
    '''
    Controller that configures and maintains an output server.
    This connects to the configuration menu entry on the main gui
    and uses it to configure the output server and start it listening.
    Listening starts when any of the following happens:
    -  The server is configured via the menu on the GUI.
    -  The server's configuration is read in by the serializer below.
    
    both of these will invoke the 'listen' method which will
    kill any existing server, work with the port manager to advertise
    the new service and create an OutputServer to start listening/servicing 
    connections.
    
    Note that this is a singleton class.
    '''
    _instance : ClassVar[Self | None] = None
    def __init__(self):
        # Enforce that only this file can construct us:
        
        caller = traceback.extract_stack()[-2]
        if caller.filename != __file__:
            raise RuntimeError('Attempted to construct singleton OutputServerController extenrally')
    
        self._server : OutputServer | None = None  # Construct on Listen.
        self._service : PortManager.PortManager | None = None
        self._serviceName : str  = DEFAULT_OUTPUT_SERVICE
        self._running : bool = False
        # Connect to the configuration menu entry:
        
        mainWindow().outputServerConfigure.connect(self._configure)
    
    @classmethod
    def instance(cls) -> Self:
        ''' @return OutputServerController - The singleton instance'''
        if not cls._instance:
            cls._instance = cls()
        return cls._instance
    
    def listen(self) -> None:
        # New port manager, new server:
        
        if self._server :
            # There already is one, shut down it's listener.
            self._server.close()
        self._server = OutputServer()
        self._service = PortManager('localhost')  # Drops any old service.
        port = self._service.getPort(self._serviceName)
        self._server.listen(port= port)      # Actually kicks off the server.
        self._running = True
        
    def disable(self) -> None:
        ''' Turn off the outut listener if it's running.'''
        if self._server:
            self._server.close()
            self._server = None
        self._running = False
        
    def getConfig(self) -> dict[str, str | bool]:
        '''
        Used by the serializer to get the configuration.
        @return dict[str, str | bool]:  Keys/values are:
            'service' : [str] - name of output service
            'enabled' : [bool] - True if the server should be enabled.
        '''
        return {
            'service' : self._serviceName,
            'enabled' : self._running
        }
    def setServiceName(self, name : str) -> None:
        '''
        @param name : str - new service name.
        @note   if the server is runnning it is shutdown and restarted with the new name.
                otherwise, the name is just stored for later use.
        '''
        self._serviceName = name
        if self._running:
            self.listen()
        
        
    def _configure(self) -> None:
            
        #  Configure the output service name - First we need a form to wrap in the SaveDialog:
        class ConfigureForm(QWidget):
            def __init__(self, parent : QWidget | None = None):
                super().__init__(parent)
                self.setLayout(QHBoxLayout())
                
                self.layout().addWidget(QLabel('Service name: ', self))
                self._service = QLineEdit(self)
                self.layout().addWidget(self._service)    
                self._enable = QCheckBox('Enable', self)
                self.layout().addWidget(self._enable)
                
            def service(self) -> str:
                # Get the service name:
                
                return self._service.text()
            def setService(self, name : str) -> None:
                # Set the service name:
                
                self._service.setText(name)
                
            def enable(self) -> bool:
                # get enable state.
                return self._enable.checkState() == Qt.CheckState.Checked
            def setEnable(self, enable : bool) -> None:
                # set the checkstate:
                self._enable.setCheckState(
                    Qt.CheckState.Checked if enable else Qt.CheckState.Unchecked
                )
                
        form = ConfigureForm(mainWindow())
        form.setService(self._serviceName)
        form.setEnable(self._running)
        
        dialog = SaveDialog(form, mainWindow())
        if dialog.exec() == QDialog.DialogCode.Accepted:
            service = form.service()
            enabled = form.enable()
            if not service.strip():
                QMessageBox.warning(dialog, 'Missing service', 'You must supply a service name')
                return
            
            # first turn off any existing service:
                
            self.disable()
            self.setServiceName(service)
            if enabled:
                self.listen()   # Will update self._running.
                
class SaveRestoreConfig(Serializer):
    '''
    Serializer for the configuration of the output server.
    The configuration is stored as the OutputServer variable which is
    given a dict value with the keys:
    - service : str - service name.
    - enabled : bool - True if the server should run.
    '''
    def serialize(self, destination : io.TextIOBase) -> None:
        '''
        Serialize the configuration.  See the class docstring for what is serialized.
        
        
        '''
        controller = OutputServerController.instance()
        config     = controller.getConfig()
        destination.write(f'OutputServer = {config}\n')
    
    def deserialize(self, mod : types.ModuleType) -> None: 
        controller = OutputServerController.instance()
        try :
            config = mod.OutputServer
            controller.setServiceName(config['service'])
            if config['enabled']:
                controller.listen()
            else:
                controller.disable()    # In case it was running.
        except AttributeError:
            pass                        # Ignore the lack of a config.
        
# Register our serializer:

SerializationManager.instance().addSerializer(SaveRestoreConfig())