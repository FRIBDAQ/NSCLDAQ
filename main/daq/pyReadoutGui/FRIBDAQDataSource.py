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

from nscldaq.readoutgui.DataSource import DataSource
from nscldaq.readoutgui.pySSHProcess import SSHProcess
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from nscldaq.readoutREST.readoutRestClient import ReadoutClient
from PyQt6.QtCore import QProcess
from PyQt6.QtWidgets import (
    QCheckBox,
    QGridLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
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

    from nscldaq.readoutgui import DataSourceManager, ReadoutGuiView, StateMachine
    from PyQt6.QtCore import QTimer
    from PyQt6.QtWidgets import QApplication, QMessageBox, QDialog, QDialogButtonBox
    
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
            
    
    app = QApplication(sys.argv)
    gui = ReadoutGuiView.ReadoutGuiMainWindow()
    
    # Make our data source and add it to the data source manager:
    
    source = FRIBDAQSource({
        'host' : 'localhost',
        'program_path' : '~/daqtest/readout/Readout',
        'port' : 1234,
        'log'  : '~/readout.log'
    })
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
    
    