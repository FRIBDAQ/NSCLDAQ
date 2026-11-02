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
@file SlaveController.py
@brief Mediates between the user interface and an instance of the Slave class.
@author Ron Fox.

This file has a bit of mixing in that out of sheer laziness we put the configuration
view and invocation of it here as well. The mainpurpose of this
is to catch menu signals that configure/enable the slave object. 
Our controller also encapsulates the slave object.
'''
import io
import traceback
import types
from typing import ClassVar, Self

from nscldaq.mg_configutils import SaveDialog
from nscldaq.readoutgui.StateSaveRestore import Serializer, SerializationManager
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from nscldaq.readoutgui.Slave import Slave
from PyQt6.QtWidgets import QDialog, QHBoxLayout, QLabel, QLineEdit, QWidget


class SlaveConfigDialog(SaveDialog):
    '''
    Configuration dialog for the slave.  Currently there's only one
    configuration parameter, the Service name.  For simplicity,
    we just wrap a form in a SaveDialog and call it a day.
    
    Attributes:
      service - The value of the service entry from the user.
    '''
    def __init__(self, initialValue : str = 'ReadoutGuiRemotControlService', parent : QWidget | None = None):
        
        
        self._form = QWidget()
        self._form.setLayout(QHBoxLayout())
        self._form.layout().addWidget(QLabel('Service Name:', self._form))
        
        self._service = QLineEdit(self._form)
        self._form.layout().addWidget(self._service)
        self.setService(initialValue)
        
        super().__init__(self._form, parent)
    
    def service(self) -> str:
        '''
        Return the current service name
        @return str
        '''
        return self._service.text()
    
    def setService(self, service :str) -> None:
        '''
        @param service - value to put in the service line entry.
        '''
        self._service.setText(service)
    
        

class SlaveController:
    '''
    This class mediates between a slave server and the graphical
    controls that manage it.. Specifically, its configuration and 
    enable/disable controls.   The main window has two menu items
    that control the slave configuration.  They signal:
    - slaveConfigure - indicating the user wants to configure the slave server
      (set its advertised service name).
    - slaveEnable - Indicating the enable state of the server just changed.
    
    Each configuration of the slave, destroys any existing slave server and
    Note this is a singleton class.
    
    '''
    _instance : ClassVar[Self | None]  = None
    
    def __init__(self) :

        # Only allow construction from this file:
        
        caller = traceback.extract_stack()[-2]
        if caller.filename != __file__:
            raise RuntimeError('Attempted to contruct singleon SlaveController')
    
        self._slave : Slave | None = None
        
        # Hook in our menu respoders:
        
        mw = mainWindow()
        mw.slaveConfigure.connect(self._configureSlave)
        mw.slaveEnable.connect(self._toggleEnable)
    
        self._serviceName = 'ReadoutGuiRemoteControl'
    
    @classmethod
    def instance(cls) -> Self:
        '''
        @return SlaveController - the singleton instance of the class.
        '''
        if not cls._instance:
            cls._instance = cls()
        return cls._instance
    
    def configuration(self) -> dict[str, object]:
        return {
            'service': self._serviceName,
            'enabled':  mainWindow().slaveEnabled()
        }
    
    # Internal slots:
    
    def _configureSlave(self) -> None:
        # Called in response to the menu entry to configure
        # a slave.  If the slave gets a service it is created
        # The enable is read from the enable checkbox menu entry.
        mw = mainWindow()
        prompt = SlaveConfigDialog(initialValue = self._serviceName, parent = mw)
        if prompt.exec() == QDialog.DialogCode.Accepted:
            service = prompt.service()
            if service != self._serviceName:     # Only act if it changed:
                self._serviceName = service
                self._createSlave()
                
                
    def _toggleEnable(self, enable : bool) -> None:
        # Called when the enable menu button changes state.
        # If there's a server, change its enable. If not, 
        # Create one with the current configuration.
        
        if enable:
            if not self._slave:
                self._createSlave()
            self._slave.enable()
        else:
            if self._slave:
                self._slave.disable()
    
    # Utilities:
    
    def _createSlave(self) -> None:
        # Create a new slave using the service name we have
        # and the status of the enable checkbutton menu entry.
        
        mw  = mainWindow()
        
        config = {
            'service' : self._serviceName,
            'enabled'  : mw.slaveEnabled()
        }
        self._slave = Slave(config)
        


class Configuration(Serializer):
    '''
    Serialization of the configuration.  Note this will also 
    (on desrialization) instantiate the singleton.
    '''
    def serialize(self, destination : io.TextIOBase) -> None:
        '''
        Serialize our configuration as:
        SlaveServerConfiguration = {'service' : str, 'enabled' : bool}
        @param destination - a file handle describing where to write the config.
        '''
        controller = SlaveController.instance()
        config = controller.configuration()
        destination.write(f'SlaveServerConfiguration =  {config}\n')
        
    def deserialize(self, mod : types.ModuleType):
        '''
        Deserialize - note that it's not a fatal error for the
          configuration not to exist...that'll give us a singleton
          SlaveController with the default servicename and disabled state.
          We do do a bit of dirtiness - reaching into the controller
          to configure it.
            @para mod : typeModuleType - the module with the configuration.
        '''
        controller = SlaveController.instance()   # Create default configured controller.
        try:
            config = mod.SlaveServerConfiguration
                
        except AttributeError:
            return
        
        # configure the controller .
        
        controller._serviceName = config['service']     
        controller._toggleEnable(config['enabled'])    # Starts the Slave server if it's enabled.
        
# Register the serlializer so it's called.

SerializationManager.instance().addSerializer(Configuration())