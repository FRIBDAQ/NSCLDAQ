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

import os
import getpass
import time
import traceback
from nscldaq.readoutgui.pySSHProcess import SSHProcess
from nscldaq.readoutgui.DataSource import DataSource
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from nscldaq.readoutREST.readoutRestClient import ReadoutClient

from PyQt6.QtCore import QProcess


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
        print("Output window: ", self._outputWin)
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
            
        }
        

    def start(self) -> None:
        '''
        Start the processon the SSH Pipe.
        If one is already running, shut it down.
        When the process is started we connect
        '''
        print('Start in frib src')
        self._outputMsg('Start\n')
        if self._ssh is not None and self._ssh.state != QProcess.ProcessState.NotRunning: 
            print('calling stop')
            self.stop()
            print('called')
        self._ssh = SSHProcess()
        print('made ssh')
        self._ssh.readyReadStandardOutput.connect(self._relayOutput)
        print('connected output')
        self._ssh.readyReadStandardError.connect(self._relayOutput)
        print('and error')
        self._ssh.finished.connect(self._relayExit)
        print('and exit')
        try:
            command = self.createCommandLine()
            self._outputMsg(f'Staring "{command}"')
            host = self._configuration.get('host', 'localhost')
            self._outputMsg(f' in {host}')
            self._ssh.spawnRemote(host, command)
            self._client = self._make_client()    # Since translation doesn't happen until requests are done.
        except Exception as e:
            print(f'{e} at\n {traceback.format_exc()}')
            raise
    
    def check(self) -> bool: 
        if self._client is not None and  (self._ssh is None or self._ssh.state() != QProcess.ProcessState.NotRunning):
            return False
        # See if we can poll the status from the ReST interface
        
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
        self._require_client()
        self._client.setRunNumber(run)
        self._client.setTitle(title)
        self._client.begin()
    
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
        command = f'TCLLIBPATH={os.environ["DAQTCLLIBS"]} '
        if 'service' in self._configuration:
            command=f'SERVICE_NAME={self.cget("service")} '
        command += self.cget('program_path') 
        command += ' --ring=' + self._configuration.get('ring', getpass.getuser())
        command += ' --sourceid=' + str(self._configuration.get('source_id', 0))
        
        # If logging add thast stuff too:
        
        if 'log' in self._configuration:
            command += f' --log {self.cget("log")}'
            command += f' --debug_level={self._configuration.get("debug_level", 0)}'
        
        # We need to add an initscript so that the ReST server starts.
        # Since daqsetup will not necessarily have been run in the target host:
        
        daqshare = os.environ['DAQSHARE']
        command += f' --init-script={daqshare}/scripts/rest_init_script.tcl'
        
        return command
    
    
    # Slots for signals from the process:
    
    def _relayOutput(self) -> None:
        #  ouptut is available to be added to the output window.
        print('Relay Output called')
        self._outputMsg(self._ssh.ReadAll() + '\n')
    def _outputMsg(self, msg : str) -> None:
        self._outputWin.append(msg)
    def _relayExit(self, exitCode : int, status : QProcess.ExitStatus) -> None:
        # Called on process exit, make an suitable message
        # for the output window.
        print('_relayExit called')
        match status:
            case QProcess.ExitStatus.NormalExit:
                strStatus = 'Normally'
            case QProcess.ExitStatus.CrashExit:
                strStatus = 'By crashing'
            case _:
                strStatus = "In an unknown way"
        
        self._outputMsg(f'Exited with code {exitCode}, exited {strStatus}\n')
    
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
        
        self._client = ReadoutClient(host, service, user)

    
    def _require_client(self) -> bool:
        if not self._client:
            raise RuntimeError('Attempting to do a client request but no ReST client was instantiated.')
        
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
    from PyQt6.QtWidgets import QApplication
    from  nscldaq.readoutgui import ReadoutGuiView
    from  nscldaq.readoutgui import StateMachine
    from  nscldaq.readoutgui import DataSourceManager
    
    def start(sm : StateMachine.ReadoutStateMachine) -> None:
        sm.transition('Starting')
        
    
    def transitionDataSources(fromState : str, toState : str) :
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
                case 'Starting' :
                    print('Starting in signal handler')
                    # Note on success, we have to drive the state-manager to the halted state.
                    
                    mgr.start()
                    sm.transition('Halted')   # Will signal us again for halted.
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
            
    
    app = QApplication(sys.argv)
    gui = ReadoutGuiView.ReadoutGuiMainWindow()
    
    # Make our data source and add it to the data source manager:
    
    source = FRIBDAQSource({
        'host' : 'localhost',
        'program_path' : '~/daqtest/readout/Readout'
    })
    DataSourceManager.DataSourceManager.instance().addSource('DataSource', source)
    
    # The GUI signals make state transitions happe in the state machine
    # and the state machine signals make things happen in the data source:
    
    controlGui = gui.centralWidget().StateControls()
    sm   = StateMachine.ReadoutStateMachine.instance()
    controlGui.setState(sm.state())
    
    # Drive the state machine from the GUI:
    
    controlGui.start.connect(lambda : start(sm))
    controlGui.begin.connect(lambda : sm.transition('Active'))
    controlGui.end.connect(lambda : sm.transition('Halted'))
    controlGui.pause.connect(lambda : sm.transition('Paused'))
    controlGui.resume.connect(lambda : sm.transitino('Active'))
    
    # State machine drives the GUI appearance:
    
    sm.newstate.connect(controlGui.setState)
    
    # State machine drives the data source manager:
    # We transition the sources on enter so everyone else can prepare
    
    sm.enter.connect(transitionDataSources)
    
    # Add a liveness timer check.. ...every second.
    
    
    
    gui.show()
    sys.exit(app.exec())
    
    