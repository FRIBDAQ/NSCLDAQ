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

import getpass
import os
import time
import traceback
from typing import ClassVar

from nscldaq.readoutgui.DataSource import DataSource
from nscldaq.readoutgui.pySSHProcess import SSHProcess
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from nscldaq.readoutREST.readoutRestClient import ReadoutClient
from PyQt6.QtCore import QProcess, Qt
from PyQt6.QtWidgets import (
    QCheckBox,
    QFileDialog,
    QGridLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)


class FRIBDAQSource(DataSource):
    def __init__(self, parameters: dict[str, object], **kwargs):
        super().__init__(parameters, **kwargs)
        self._ssh: SSHProcess | None = None
        self._client: ReadoutClient | None = None
        self._validate_configuration()
        
        # Find the output tabbed widget and make a tab for us
        # Named ring@host
    
        host = self._configuration.get('host', 'localhost')
        ring = self._configuration.get('ring', getpass.getuser())
        self._outputWin = mainWindow().centralWidget().Outputs().addOutput(f'{ring}@{host}')
        
    @classmethod        
    def parameters(cls) -> dict[str, type]:
        '''
        @return dict[str,type] - Dictionary of parameters we recognize and their types:        
        '''
        return {
            'host'        : str,     # Where we run.
            'program_path': str,     # what we run
            'service'     : str,     # ReST service name.
            'ring'        : str,     # where data are written.
            'source_id'   : int,     # Source id.
            'log'         : str,     # Log file path if should log.
            'debug_level' : int,     # Debugging level for logging.
            'port'        : int,     # TCL Server port.
            
        }
        

    def start(self) -> None:
        '''
        Start the processon the SSH Pipe.
        If one is already running, shut it down.
        When the process is started we connect
        '''
    
        if self._ssh is not None and self._ssh.state != QProcess.ProcessState.NotRunning: 
            self.stop()
        self._ssh = SSHProcess()
        self._ssh.readyReadStandardOutput.connect(self._relayOutput)
        self._ssh.readyReadStandardError.connect(self._relayOutput)
        self._ssh.finished.connect(self._relayExit)
        command = self.createCommandLine()
        self._outputMsg(f'Starting "{command}"')
        host = self._configuration.get('host', 'localhost')
        self._outputMsg(f' in {host}')
        self._ssh.spawnRemote(host, command)
        self._make_client()    # Since translation doesn't happen until requests are done.  

    def check(self) -> bool: 
        if self._client is None or   (
            self._ssh is None or 
            self._ssh.state() not in  
            [QProcess.ProcessState.Running, QProcess.ProcessState.Starting]):
            
            return False
        # See if we can poll the status from the ReST interface
        
        # If we are starting we're ok:
        
        if self._ssh.state() == QProcess.ProcessState.Starting:
            
            return True
        
        try:
            
            self._client.getState()
            return True
        except KeyError:
            
            return False
        
    
    
    
    def stop(self) -> None:
        if self._client is not None:
            try: 
                self._client.shutdown()
            except Exception: 
                pass
    def begin(self, run: int, title: str) -> None:
        try:
            self._require_client()
            self._client.setRunNumber(run)
            self._client.setTitle(title)
            self._client.begin()
        except Exception as e:
            print(f'{e} \n {traceback.format_exc()}')
    
    def end(self) -> None:
        self._require_client()
        self._client.end()
    
    def pause(self)->None:
        self._require_client()
        self._client.pause()
        
    
    def resume(self)->None:
        self._require_client() 
        self._client.resume()
        
    def canBegin(self) -> bool:
        try: 
            return self.check()
        except Exception: 
            return False
    def capabilities(self) -> dict[str, bool]:
        return {'canPause': True, 'runsHaveTitles': True, 'runsHaveNumbers': True}

    def createCommandLine(self):
        '''
            This will need to be overidden by subclasses, it will 
            create the command line given the configuration options. subclasses
            for e.g. VMUSBReadout will have additional paramters that result in additional
            command line options.
            
            @return str - the commandline string.
        '''
        command = self.create_command_env()
        
        # Now the program and its options
        
        command += self.cget('program_path') 
        command += ' --ring=' + self._configuration.get('ring', getpass.getuser())
        command += ' --sourceid=' + str(self._configuration.get('source_id', 0))
        
        # If logging add thast stuff too:
        
        if 'log' in self._configuration:
            command += f' --log {self.cget("log")}'
            command += f' --debug={self._configuration.get("debug_level", 0)}'
        
        # Tcl server?
        
        if 'port' in self._configuration:
            command += f' --port={self.cget("port")}'
        
        # We need to add an initscript so that the ReST server starts.
        # Since daqsetup will not necessarily have been run in the target host:
        
        daqshare = os.environ['DAQSHARE']
        command += f' --init-script={daqshare}/scripts/rest_init_script.tcl'
        
        
        
        return command
    
    def create_command_env(self) -> str:
            '''
            @return str - The environment setting part of the command.
                          separated out to allow re-use.
            '''
            env = f'TCLLIBPATH={os.environ["DAQTCLLIBS"]} '
            
            # Fold in all env vars that Start with DAQ
            
            for envname in os.environ:
                if envname.startswith('DAQ'):
                    env += f'{envname}={os.environ[envname]} '
                
            
            service = self._configuration.get('service', 'ReadoutREST')
            env +=f'SERVICE_NAME={service} '
            
            return env
    
    # Slots for signals from the process:
    
    def _relayOutput(self) -> None:
        #  ouptut is available to be added to the output window.
        self._outputMsg(self._ssh.ReadAll() + '\n')
    def _outputMsg(self, msg : str) -> None:
        self._outputWin.append(msg)
    def _relayExit(self, exitCode : int, status : QProcess.ExitStatus) -> None:
        # Called on process exit, make an suitable message
        # for the output window.
        match status:
            case QProcess.ExitStatus.NormalExit:
                strStatus = 'Normally'
            case QProcess.ExitStatus.CrashExit:
                strStatus = 'By crashing'
            case _:
                strStatus = "In an unknown way"
        
        self._outputMsg(f'Exited with code {exitCode}, exited {strStatus}\n')
        
        # Kill the process as well:
        
        self._ssh = None
    
    # Utilities:
        
    def _validate_configuration(self):
        for key in self._configuration:
            if key not in self.parameters():
                raise KeyError(f'Invalid parameter name: {key}')
            if type(self._configuration[key]) != self.parameters()[key]:
                raise TypeError(
                    f'Invalid type for parameter {key} was {type(self._configuration[key]).__name__} must be {self.parameters()[key].__name__}'
                )
    def _make_client(self):
        # Mote the program must be running for this to be called, else
        # the service -> port translation will fail.
        host = self._configuration.get('host', 'localhost')
        service =self._configuration.get('service', 'ReadoutREST')
        user = getpass.getuser()
        self._outputMsg(f'Creating ReST client for {service}@{host} user: {user}')
        self._client = ReadoutClient(host, service, user)

    
    def _require_client(self) -> bool:
        if not self._client:
            self._outputMsg('_require_client did not have a client!!')
            raise RuntimeError('Attempting to do a client request but no ReST client was instantiated.')


#   Data sources need two other things:
#   - The ablity to display their configuraiton in a QWidget
#   - The ability to configure themeselves.
#
#  These are used by the framework as a whole to list data sources and their
#  attributes and to create configured data sources.
#  To support modules that can be loaded to extend the set of
#  supported data source types, these must have the class names:
#
#  ConfigurationDisplay  - To display the configuration of a data source.
#  ConfigureSource       - To create a configured data source.


class ConfigurationDisplay(QWidget):
    '''
    Configuration display object for the FRIBDAQDataSource.  
    Instantiate this passing a data source instance and an
    optional parent.  The resulting widget will display
    (non modifyably) the configuration of the source.
    '''
    def __init__(self, source : FRIBDAQSource, parent : QWidget | None = None):
        '''
            Instantiate the data source:
            @param source : FRIBDAQSource - The data sourcde to describe.
            @param parent : QWidget | None = NOne - the parent widget if desired.
        '''
        super().__init__(parent)
        config = source.getConfig()
        self._row = 0
        
        self.setLayout(QGridLayout())
        
        self._addRow('Data Source type: ', 'Generic Readout')
        self._addRow('Run In:', config.get('host', 'localhost'))
        self._addRow('Program', config['program_path'])
        self._addRow('Output Ring:', config.get('ring', getpass.getuser()))
        self._addRow('Source Id', str(config.get('source_id', 0)))
        self._addRow('Rest Service', config.get('service', 'ReadoutREST'))
        
        # Now the optional stuff:
        
        
        if 'port' in config:
            self._addRow('Tcl server  port', str(config['port']))
            
        if 'log' in config:
            self._addRow('Logging to ', config['log'])
            self._addRow('Log level',  str(config.get('debug_level', 0)))
                             
                             
    # Utilities:
    def _addRow(self, title : str, value : str) -> None:
        # Fill in the next row of the widget:
        
        self.layout().addWidget(QLabel(title, self),  self._row, 0)
        self.layout().addWidget(QLabel(value, self),  self._row, 1)
        
        self._row += 1
    
        

class ConfigureSource(QWidget):
    '''
    Instantiate this in a dialog to configure a data source.
    WHen the dialog is accepted call the makeSource method.
    It will either return a FRIBDAQSource data source or
    an str if there are detected errors.  In that case,
    the string should be displayed in a messagebox and 
    the dialog re-execed until either it's cancelled
    or an FRIBDAQSource is returned from the makeSource
    method.
    '''
    # _mandatory_parmaeters is the set of parameters that must not be blank.
    
    _mandatory_parameters : ClassVar[tuple[str]] = ('host', 'program_path')
                                                        # Default values.
    def __init__(self, source : FRIBDAQSource | None = None, parent : QWidget | None = None):
        '''
        @param source - if provided, the source's configuration is used to pre-load the form.
                        this is to support modification/replacement of an existing source.
        @param parent - Parent widget.
        
        '''
        super().__init__(parent)
        
        # Layout the form:
        
        self.setLayout(QVBoxLayout())
        layout = self.layout()
        
        
        
        # TItle me:
        
        layout.addWidget(QLabel('Configure an FRIBDAQ generic data source', self))
        
        # Host and program path:
                
        host_program_layout = QHBoxLayout()
        host_program_layout.addWidget(QLabel('Host:', self))
        self._host = QLineEdit(self)
        host_program_layout.addWidget(self._host)
        
        host_program_layout.addWidget(QLabel('Program:', self))
        self._program = QLineEdit(self)
        host_program_layout.addWidget(self._program)
        self._browse = QPushButton('Browse...', self)
        host_program_layout.addWidget(self._browse)
        
        self._browse.clicked.connect(self._browseProgram)
        
        layout.addLayout(host_program_layout)
        
        # ReSt Service name:
        
        service_layout =  QHBoxLayout()
        service_layout.addWidget(QLabel('ReST service: ', self))
        self._restService = QLineEdit('ReadoutREST', self)
        service_layout.addWidget(self._restService)
        
        layout.addLayout(service_layout)
        
        # RingBuffer:
        
        ring_layout = QHBoxLayout()
        ring_layout.addWidget(QLabel('Ouput ring name: ', self))
        self._ring = QLineEdit(getpass.getuser(), self)
        ring_layout.addWidget(self._ring)
        
        layout.addLayout(ring_layout)
        
        # Source ID:
        
        sourceid_layout  = QHBoxLayout()
        sourceid_layout.addWidget(QLabel('Source Id', self))
        self._sourceid = QSpinBox(self)
        sourceid_layout.addWidget(self._sourceid)
        self._sourceid.setMinimum(0),
        self._sourceid.setMaximum(0x7fffffff)
    
        layout.addLayout(sourceid_layout)
        
        #Logging:
        
        self._enableLogging  = QCheckBox('Enable logging', self)  # Controls enable.
        layout.addWidget(self._enableLogging)
        self._enableLogging.clicked.connect(self._enableDisableLogWidgets)
    
        logging_layout = QHBoxLayout()
        logging_layout.addWidget(QLabel('Log File:', self))
        
        self._logfile = QLineEdit(self)
        logging_layout.addWidget(self._logfile)
        self._logfile.setEnabled(False)      # unles/until _enableLogging checked.
        self._browseLogfile = QPushButton('Browse...', self)
        logging_layout.addWidget(self._browseLogfile)
        self._browseLogfile.clicked.connect(self._browseLogFile)
        self._browseLogfile.setEnabled(False)
    
        logging_layout.addWidget(QLabel("Log level", self))    
        self._logLevel = QSpinBox(self)
        logging_layout.addWidget(self._logLevel)
        self._logLevel.setMinimum(0)
        self._logLevel.setMaximum(2)
        self._logLevel.setEnabled(False)
        
        self._logWidgets = (self._logfile, self._browseLogfile, self._logLevel)    # Enabled via checkbox.
    
        layout.addLayout(logging_layout)
        
        # TCL Server - 
        
        tcl_server_layout = QHBoxLayout()
        self._enableTclServer = QCheckBox('Enable Tcl Server', self)
        
        tcl_server_layout.addWidget(self._enableTclServer)
        self._enableTclServer.clicked.connect(self._enableDisableServerWidgets)
        tcl_server_layout.addWidget(QLabel('Port', self))

        self._tclport = QSpinBox(self)
        tcl_server_layout.addWidget(self._tclport)
        self._tclport.setMinimum(1024)         # unpriv port.
        self._tclport.setMaximum(29999)        # Below the port manager port pool.
        self._tclport.setEnabled(False)        # Unless enableTclServer is checked.
        
        self._tclServerWidgets = (self._tclport,)   # For now.

        layout.addLayout(tcl_server_layout)
        
        # If a source was provided, load the form from it:
        
        
        if source:
            self._loadForm(source)
        
        
    # Public methods:
    
    def makeSource(self) -> FRIBDAQSource | str:
        '''
            Attempts to construct a daq data source from the configuration
            in the widget.
            @return FRIBDAQSource - if the configuration allowed us to do that.
            @return str           - Error message to display if not.
            
        '''
        # pull the raw configuration out first.
        
        # Mandatory stuff:
        
        config = {
            'host'         : self._host.text(),
            'program_path' : self._program.text(),
            'source_id'       : self._sourceid.value(),
            
        }
        
        # Things with defaults:
        
        svc = self._restService.text()
        if svc.strip():
            config['service'] = svc
        
        ring = self._ring.text()
        if ring.strip():
            config['ring'] = ring
            
        if self._isChecked(self._enableLogging):
            # There must be a log file:
            
            logfile = self._logfile.text()
            if logfile.strip():
                config['log'] = logfile
            else:
                return "If you enable logging you must supply a log file as well."
            
            config['debug_level'] = self._logLevel.value()
        
        if self._isChecked(self._enableTclServer):
            config['port'] = self._tclport.value()
        
        # Be sure the mandatory parameters are set:
        
        for key in self._mandatory_parameters:
            if not config[key].strip():
                return f'The {key} configuration must be provided.'
        
        # Valid config so:
        
        return FRIBDAQSource(config)
    
    # Private methods:
    
    def _loadForm(self, source : FRIBDAQSource) -> None:
        # Load the contents of the form from the 
        # configuration of an existing data source:
        
        config = source.getConfig()
        
        # These two must be present.
        
        self._host.setText(config['host'])
        self._program.setText(config['program_path'])
        
        if 'service' in config:
            self._restService.setText(config['service'])
            
        if 'ring' in config:
            self._ring.setText(config['ring'])
        
        if 'source_id' in config:
            self._sourceid.setValue(config['source_id'])
            
        # Set logging if enabgled:
        
        if 'log' in config and config['log'].strip():
            self._enableLogging.setCheckState(Qt.CheckState.Checked)
            self._enableDisableLogWidgets()       # Should enable the widgets.
            self._logfile.setText(config['log'])
            if config['debug_level'] in config:
                self._loglevel.setValue(config['debug_level'])
        
        # Set Tcl server port if enabled:
        
        if 'port' in config:
            self._enableTclServer.setCheckState(Qt.CheckState.Checked)
            self._enableDisableServerWidgets()
            self._tclport.setValue(config['port'])
    
    # Private slots:
    
    def _browseProgram(self) -> None:
        # Browse for the program.  On accepted, set self._program from the resulting
        #path
        
        path, _ = QFileDialog.getOpenFileName(self, 'Choose Program', '.')
        if path.strip() :
            self._program.setText(path)
    
    def _enableDisableLogWidgets(self) -> None:
        # Depending on the state of self._enableLogging, turn on/off the
        # widgets in self._logWidgets:
        
        state = self._isChecked(self._enableLogging)
        for w in self._logWidgets:
            w.setEnabled(state)
            
    def _browseLogFile(self) -> None:
        # Set  self._logfile from the output of a file dialog borwser:
        
        path, _ = QFileDialog.getSaveFileName(
            self, 'Choose Log file', '.', 'Log Files (*.log);;All Files (*)'
        )
        if path.strip():
            self._logfile.setText(path)
        
    def _enableDisableServerWidgets(self) -> None:
        # Enable/disable the tcl server widgets depending on the state
        # of the enable chekcbutton:
        
        state = self._isChecked(self._enableTclServer)  
        for w in self._tclServerWidgets:
            w.setEnabled(state)
    
    # Utilities:
    
    def _isChecked(self, w : QCheckBox) -> bool:
        # Simplify checking box states for bistate.

        return w.checkState() == Qt.CheckState.Checked        
            

# Test code  
#   Note this is specific to my development env  because it assumes
#   there's a readout program in ~/daqtest/readout/Readout.

# the test is a mini readout GUI with:
#  The readout GUI.
#  The state manager.
#  The Data source manager
#  A single data FRIBDAQDataSource.

if __name__ == '__main__':
    import sys

    from nscldaq.mg_configutils import SaveDialog
    from nscldaq.readoutgui import DataSourceManager, ReadoutGuiView, StateMachine
    from PyQt6.QtCore import QTimer
    from PyQt6.QtWidgets import QApplication, QDialog, QDialogButtonBox, QMessageBox
    
    #  Toy Class to display the data source 'list'
    
    class ListSources(QDialog):
        def __init__(self, parent : QWidget| None = None):
            super().__init__(parent)
            
            self.setLayout(QVBoxLayout())
            sources = DataSourceManager.DataSourceManager.instance().sources()
            source = sources[list(sources.keys())[0]]
            self.layout().addWidget(ConfigurationDisplay(source, self))
            
            self._buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Close, self)
            self.layout().addWidget(self._buttons)
            self._buttons.rejected.connect(self.reject)
            
    
    
    def listSources(parent) -> None:
        lister = ListSources(parent)
        lister.exec()
        
    liveness = None    # Live timer when it's active.
    
    def start(sm : StateMachine.ReadoutStateMachine) -> None:
        sm.transition('Starting')
    
    def stopSources() -> None:
        # Normally called just before exit...we stop
        # all of the data sources via the source manager.
        
        DataSourceManager.DataSourceManager.instance().stop()
           
    def doExit() -> None:
        # Confirm:
        
        confirm = QMessageBox.question(
            mainWindow(), 'Really exit?', 'Are you sure you want to exit?'
        )
        if confirm == QMessageBox.StandardButton.Yes:
            stopSources()
            QApplication.instance().exit(0)
            
    
    def checkDataSources() -> None:
        # Check the liveness of data sources.
        # If one of them failed transition to NotReady which will
        # cause the data sources to be shutdown.
        
        mgr = DataSourceManager.DataSourceManager.instance()
        mw = mainWindow()
        sm = StateMachine.ReadoutStateMachine.instance()
        
        if sm.state() in ['Halted', 'Active', 'Paused'] and not mgr.live():
            sm.transition('Not Ready')
            
        
        
    
    def transitionDataSources(fromState : str, toState : str) :
        global liveness
        # Transition the data sources 
        
        mgr = DataSourceManager.DataSourceManager.instance()
        mw = mainWindow()
        sm =  StateMachine.ReadoutStateMachine.instance()
        if mw:
            ow = mw.centralWidget().Outputs()
            ow.addToMain(f'Transitioning to {toState} state')
        else:
            ow = None
        try:
            match toState:
                case 'Not Ready':
                    mgr.stop()
                    # If liveness is active, kill it off
                    
                    if liveness:
                        liveness.stop()
                        liveness = None
                case 'Starting' :
                    # Note on success, we have to drive the state-manager to the halted state.
                    
                    mgr.start()
                case 'Halted':
                    #  IF the from state was Starting we don't need to do anything.
                    if fromState != 'Starting':
                        # Halting a run:
                        
                        mgr.end()
                                
                case 'Active':
                    # Resuming or beginning:
                    
                    if fromState == 'Halted':
                        if mgr.precheck():
                            run = mw.centralWidget().RunParameters().run()
                            title = mw.centralWidget().RunParameters().title()
                            
                            mgr.begin(run, title)
                        else:
                            raise RuntimeError('BEGIN  failed precheck')
                    elif fromState == 'Paused':
                        mgr.resume()
                    else:
                        raise Exception(f'Transition to "Active" unrecognized from state: {fromState}')
                case 'Paused':
                    mgr.pause()
                case _ :
                    # Unrcognized state:
                    if ow:
                        ow.addToMain(
                            f'Unrecognized state transition from {fromState} to {toState}'
                        )        
                    mgr.failed()
        except Exception as e:
             if ow:
                ow.addToMain(f'Transition failed: {e} \n{traceback.format_exc()}')
                sm.failTransition()            
            
    
    def postTransition(fromstate : str, tostate: str):
        
        global liveness
        # IF we got into starting, we can now transition to Halted
        
        if tostate == 'Starting':
            StateMachine.ReadoutStateMachine.instance().transition('Halted')
            
            # Set up a timer to check data source liveness.
            
            liveness = QTimer(mainWindow())
            liveness.setInterval(1000)
            liveness.setSingleShot(False)
            liveness.timeout.connect(checkDataSources)
            liveness.start()
            
    def promptSource() -> FRIBDAQSource:
        dialog = SaveDialog(ConfigureSource())
        while dialog.exec() == QDialog.DialogCode.Accepted:
            source : FRIBDAQSource | str = dialog.workarea().makeSource()
            if type(source) == str:
                QMessageBox.warning(dialog, 'Missing parameters', source)
            else:
                return source
    
        # Exit for now if no source chosen.
        
        QApplication.instance().exit(-1)    
        
    
    
    app = QApplication(sys.argv)
    gui = ReadoutGuiView.ReadoutGuiMainWindow()
    
    # Make our data source and add it to the data source manager:
    
    #source = FRIBDAQSource({
    #    'host' : 'localhost',
    #    'program_path' : '~/daqtest/readout/Readout',
    #    'port' : 1234,
    #    'log'  : '~/readout.log'
    #})
    source = promptSource()
    DataSourceManager.DataSourceManager.instance().addSource('DataSource', source)
    
    # The GUI signals make state transitions happe in the state machine
    # and the state machine signals make things happen in the data source:
    
    controlGui = gui.centralWidget().StateControls()
    sm   = StateMachine.ReadoutStateMachine.instance()
    
    # Drive the state machine from the GUI:
    
    controlGui.start.connect(lambda : start(sm))
    controlGui.begin.connect(lambda : sm.transition('Active'))
    controlGui.end.connect(lambda : sm.transition('Halted'))
    controlGui.pause.connect(lambda : sm.transition('Paused'))
    controlGui.resume.connect(lambda : sm.transition('Active'))
    
    # State machine drives the GUI appearance:
    
    sm.newstate.connect(controlGui.setState)
    
    # State machine drives the data source manager:
    # We transition the sources on enter so everyone else can prepare
    
    sm.leave.connect(transitionDataSources)
    sm.enter.connect(postTransition)
    
 
    # If a data source livecheck fails, output an emergency message
    # to the output window:
    
    DataSourceManager.DataSourceManager.instance().sourceDied.connect(
        lambda srcname : gui.centralWidget().Outputs().emergencyMessage(
            f'Data source {srcname} just died.'
        )
    )
    # Set the GUI initial state:
    
    controlGui.setState(sm.state())
    
    # If the program is exiting, force the state to 
    # Not Ready to kill off the data sources.
    
    gui.fileExit.connect(doExit)
    gui.destroyed.connect(stopSources)
    
    gui.show()
    
    # Attach DataSource -> List  to displaying the  data source.
    # In a dialog with the Ok button to dismiss it.
    
    gui.dsListSources.connect(lambda: listSources(gui))
    
    sys.exit(app.exec())
    
    