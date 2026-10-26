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
@file EventLog.py
@brief Class that encapsulate eventlog

Note there are two types of event loggers that can be run.
Partial and Full.   

Partial event loggers just make a soup
of event files in the target directory.
Full event loggers maintain the directory structure users have
come to know and love/hate in the Tcl ReadoutGUI primary
event logger. See the individual class descriptions
for more.

Note as well, that unlike the ReadoutGUI event loggers can
run in any host that can be a client to the DAQ system.
They are therefore run on the end of a pySSHProcess.
This implies the container we might run in is reconstructed
in the target system.  

This code also encapsulates an EventLogManager that
contains a list of event loggers constructed externally
by the user interacting with, e.g. the ReadoutGui.

We include a ReadoutConfiguration saver to allow the
set of event loggers and their configurations to be saved
and restored on restart.
'''
import datetime
import getpass
import os
import pathlib
import subprocess
from abc import ABC, ABCMeta, abstractmethod
from typing import ClassVar, Self

from nscldaq.readoutgui.EventLogManager import EventLogRegistry
from nscldaq.readoutgui.pySSHProcess import SSHProcess
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from PyQt6.QtCore import QObject, QProcess, Qt, pyqtSignal
from PyQt6.QtWidgets import (
    QCheckBox,
    QFileDialog,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)


class QObjectMeta(type(QObject), ABCMeta):
    pass

class EventLog(QObject, metaclass=QObjectMeta):
    '''
    Abstract base class that defines the methods
    every event logger must implement.
    We also provide for configuration.  At this
    level, we don't assume much of anything
    about the event logger that's being run to allow
    foreignn event loggers to work.
    '''
    def __init__(self, configuration : dict[str, object], **kwargs):
        '''
        @param configuration : dict[str, object] - the initial 
            configuration of the event.  It should
            conform to the types returned by the
            parameters abstractmethod.
        '''
        super().__init__(**kwargs)
        self._configuration = configuration
    
    @classmethod
    @abstractmethod
    def parameters(cls) -> dict[str, type]:
        '''
        @return dict[str,type] - specifies the configuration parameter names and types.
            e.g. {'ringbuffer' : str'}  there is a confiiguration parameter named 'ringbuffer' 
            that has str type.
        '''
        ...
        
    def configure(self, name : str, value : object) -> None:
        '''
        Modifies a configuration parameter (or sets it if not yet set).
        @param name : str - name of the parameter.
        @param value : object - The new value to give the parameter,
            must be consistent with the type of that parameter from the
            parameters method.
        @throws IndexError if there's no matching parameter name.
        @throws TypeError the value is not of the required type.
        '''
        
        required_type = self.parameters()[name]
        if  isinstance(value, required_type):
           self._configuration[name] = value
        else:
            raise TypeError(
                f'''Type {value} for parameter {name} must be of type {required_type.__name__} but was of {type(value).__name}'''
            ) 
    def getConfig(self) -> dict[str, object]:
        '''
        @return dict[str, object] - the configuration of the logger.
        '''
        
        return self._configuration
    
    @abstractmethod
    def start(self) -> None:
        '''
        For concrete classes, this should start the event logger.
        '''
        ...
    @abstractmethod
    def end(self) -> None:
        '''
        For concrete classes, this should do what's done at the end of
        a run for the event logger.  This includes ensuring the
        eventlogger actually exits at some point.
        '''
        ...
    @abstractmethod
    def stop(self) -> None:
        '''
        Called to stop an event logger (e.g when the run ends). This is a best effort attempt.
        '''
        ...
        
    # Proctected methods
    def _validateConfig(self) -> bool:
        '''
        Validate the configuration against parameter types.
        @return bool - True if all elements validate, False if not.
        '''
        return  len(self._listInvalidParameters()) == 0
            
        
    def _listInvalidParameters(self) -> list[str]:
        '''
        @return list[str] - list of parameter names that are of invalid type.
        '''
        parameters = self.parameters()
        result = []
        for name, value in self._configuration.items():
            if not isinstance(value, parameters[name]):
                result.append(name)
            
        
        return result
        
class FRIBEventLogProgram(SSHProcess):
    '''
        This is a utility class that runs an event log
        on the end of an SSHPipe.  Since we are
        derived frrom SSHProcess and that, in turn is
        derived from QProcess, all of the signals/slots
        are available.
        
        We do wrap spawnRemote in a nicer call interface
        that's a bit kinder. 
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
    
    def run(self, remote : str, bindir : str, options : list[str]) -> None:
        '''
        Wraps the spawn remote of our base class:
        @param remote  - the host in which the event logger is run.
                         our local container environment is re-created.
        @param bindir  - The DAQBIN in which to find 'eventlog'.
        @param options - A list of options e.g. ['-oneshot', '-source=tcp://localhost/ron',...]
        '''
        
        # Locate the program name:
        
        program_path = pathlib.Path(bindir) / 'eventlog'
        if not program_path.exists():
            raise FileNotFoundError(f'{bindir} does not have an eventlog program!')
    
        command = str(program_path)
        
        # Now fold in the options:
        
        for option in options:
            command += ' ' + option

        # Spawn:
        
        self.spawnRemote(remote, command)
        

class MultiLogEventLog(EventLog):
    '''
    This class handles a multilog style event logger.  Specificaily,
    it does no post processing when an event logger exits.
    It prefixes the event files with some unique string (the time of day)
    so that name collisions are avoided....though the user
    is responsible for not having two multiloggers pointing to the same
    destination.
    
    Signals:
        started - the process started in the remote system.
        finished - The process completed. (see QProcess).
        readable - The pipe is readable.
    '''
    started  = pyqtSignal()
    finished = pyqtSignal(int, QProcess.ExitStatus)
    readable = pyqtSignal()
    @classmethod
    def parameters(cls) -> dict[str, type]:
        '''
        Returns the set of configuration parameters that are provided:
        
        host        : str - in which host to run the eventlog (default localhost)
        bindir      : str - In which bindir to find eventlog (default = $DAQBIN)
        source      : str - ringbuffer url from which to take data (default = tcp://host/username)
        destination : str  - Where to put the data. no default
        checksum    : bool - If checksum files should be created for each run. Defaults to True
        source_count: int  - Number of data sources to expect begin/ends for. Defaults to 1.
        '''
        return {
            'host'        : str,
            'bindir'      : str,
            'source'      : str,
            'destination' : str,
            'checksum'    : bool,
            'source_count' : int,
        }
    
    def __init__(self, **kwargs) -> None:
        '''
        Note that the parameters you'll need to pass in are
        @param configuration : dict[str, object] - the configuration of the source
        @param parent        : optional parent of this QObject.
        '''
        super().__init__(**kwargs)
        if not self._validateConfig():
            raise RuntimeError(
                f'The following parameters are invalid: {self._listInvalidParameters()}'
            )
        self._logger =  FRIBEventLogProgram(self)
        self._logger.started.connect(self.started)
        self._logger.finished.connect(self.finished)
        self._logger.readyReadStandardError.connect(self.readable)
        self._logger.readyReadStandardOutput.connect(self.readable)
       
        # Set our default configuration values:
        #  With the exception of source that is special, the
        #  Missing items have no default values and are required at the
        # time things start.
        #
        self._defaults = {
            'host' : 'localhost',
            'bindir' : os.environ.get('DAQBIN', ''),
            'checksum': True,
            'source_count' : 1
        }
    
        
    def start(self) -> None:
        '''
        Start the event logger.  Other objects are responsible for
        any output/error monitoring or exit monitoring.
        '''
        options = self._makeStandardOptions()   # We will build the program options here
        host    = self._configuration.get('host', self._defaults['host'])
        bindir  = self._configuration.get('bindir', self._defaults['bindir'])
        
        
        #  Options we always have:
        
        options += f'--prefix={datetime.now.isoformat()}--'
            
                
        self._logger.run(host, bindir, options)
    
    def end(self) -> None:
        '''
        End of run actions...we don't have to do anything.
        '''
        pass
    def stop(self) -> None:
        '''
        Stop the program (if possible).
        '''
        self._logger.kill()       
        
        
    def read(self) -> str | None:
        '''
        Read the output from the ssh pipe
        @return str | None - if there's nothing to read this can return None.
        '''
        return self._logger.ReadAll()   # this one changes the QBytesArray into str.
        
     
    # Utilities:
    
    def _makeStandardOptions(self) -> list[str]:
        # Process the configuration to get the starting option set:
        
        options = []
        
        default_source = f'tcp://{self._configuration["host"]}/{getpass.getuser()}'
        options += f'''--source={self._configuration.get('source', default_source)}'''
        options += f'''--path={self._configuration['destination']}''' # Error on default.
        if self._configuration.get('checksum', self._defaults['checksum']):
            options += '--checksum'
        options += f'''--number-of-sources={self._configuration.get('source_count', self._defaults['source_count'])}'''
        options += '--oneshot'
        
        return options   

    
class FullEventLog(MultiLogEventLog):
    '''
    For this we just need to
    override start and catch the finished signal so that
    we do the end of run organization stuff.
    We support the4 same configuration parameters but
    don't apply a prefix.
    '''
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        
        self._logger.finished.connect(self._endRunActions)
        self._runNumber = None
        
        
    def start(self) -> None:
        '''
        Start the logger:
        '''
        self._createDirectoryStructure()         # We're only sure the config is complete here.
        
        options = self._makeStandardOptions()    # This is ours...cooks destination.
        host    = self._configuration.get('host', self._defaults['host'])
        bindir  = self._configuration.get('bindir', self._defaults['bindir'])
        
        self._logger.run(host, bindir, options)

        self._runNumber = mainWindow().centralWidget().RunParameters().run()  # This run number.

    # Private/internal Slots
    
    def _endRunActions(self) -> None:
        # Do the stuff needed when a logger exits to build the
        # directory stuff.
        #  mv the event file(s) and checksums from current -> experiment/runxyz
        #  tar (following links) everything else from current -> experiment/runxyz
        #  make a link in experiment/complete that poits to each event file in runxyz.
        #  set permissions on the event files and checksums in runxyz to prevent writing
        #  and the perms  on the runxyz directory to prevent content modification.
        #
        # Note permissions are a bit more open than in the Tcl version.  We don't touch
        # The perms on the top  level directories.
        
        # Given the run number, construct a path glob search for the event files/checksums
        # these are of the form run-nnnn-*
        
        topdir = pathlib.Path(self._configuration(['destiniation']))
        current = topdir / 'current'
        evfilelist = current.glob(f'run-{self._runNumber:04d}*')   # Should catch .evt and checksums too.
        
        final = topdir / 'experiment' / f'run{self._runNumber}'
        final.mkdir()
        #
        # Move the event files:
        #
        for file in evfilelist:
            file.move_into(final)
        
        # Everything elses gets tarred -H to follow links.
        
        subprocess.call(f'cd {str(current)}; tar czhf - .|(cd {str(final)}; tar --warning=no-timestamp -xpzf -)')
        
        # The directory and all files in final get chmod a-w:
        
        subprocess.call(f'chmod -R a-w {str(final)}')   # Also does -w on the directory.
    
    # utilties
    
    def _makeStandardOptions(self) -> list[str]:
        #  Make the standar options (base class) and modify the
        #  --path option to add a /current element to tit.
        
        options = super()._makeStandardOptions()
        for index, option in options.enumerate():
            if option.startswith('--path='):
                options[index] += '/current'
                break                       # There can be only one.
        
        return options
    
    def __createDirectoryStructure(self) -> None:
        #  Given the destiniation of the configuration, dest
        # Create:
        #    dest/current
        #    dest/complete
        #    dest/experiment
        #
        # if they don't exist.
        
        dest = pathlib.Path(self._configuration['destination'])    # No default for this.
        current = dest /  'current'
        complete = dest / 'complete'
        experiment = dest / 'experiment'
        
        for dir in [current, complete, experiment]:
            dir.mkdir(parents = True, existss_ok = True)

class LoggerConfigDisplay(QWidget):
    '''
    Display the configuration of one of the logger types in this file.
    The class type is enough to know if the logger being displayed is
    'full' logger.
    '''
    
    def __init__(self, logger : EventLog, parent : QWidget | None = None):
        super().__init__(parent)
        
        self.setLayout(QVBoxLayout())      # Bunch of strips.
        config = logger.getConfig()
        
        loggerType = 'full' if isinstance(logger, FullEventLog) else 'multilog'
        self.layout().addWidget(QLabel(f'This is a {loggerType} event logger', self))
    
        # Where does the logger run:
        
        hostlayout = QHBoxLayout()
        hostlayout.addWidget(QLabel('Runs in', self))
        hostlayout.addWidget(QLabel(config.get('host', 'localhost'), self))                                    
        
        self.layout().addLayout(hostlayout)
        
        # Which logger runs:
        
        whichlayout = QHBoxLayout()
        whichlayout.addWidget(QLabel('Event log program:', self))
        whichlayout.addWidget(QLabel(f'{config.get("bindir", os.environ["DAQBIN"])}/eventlog', self))
        
        self.layout().addLayout(whichlayout)
        
        # Logging from what ringbuffer?
        
        ringlayout = QHBoxLayout()
        ringlayout.addWidget(QLabel('Data source:', self))
        ringbuffer = config.get('source', f'tcp://localhost/{getpass.getuser()}')
        ringlayout.addWidget(QLabel(ringbuffer, self))
        
        self.layout().addLayout(ringlayout)
        
        #  Where does the data go:
        
        destlayout = QHBoxLayout()
        destlayout.addWidget(QLabel('Logged to', parent))
        destlayout.addWidget(QLabel(config.get('destination', '<not set>'), parent))
        
        self.layout().addLayout(destlayout)
        
        # Checksum?
        
        cksumlayout = QHBoxLayout()
        cksumlayout.addWidget(QLabel('Checksumming', self))
        checksums = 'Enabled' if config.get('checksum', True) else 'Disabled'
        cksumlayout.addWidget(QLabel(checksums, self))
        
        self.layout().addLayout(cksumlayout)
    
        # Number of sources:
        
        countlayout = QHBoxLayout()
        countlayout.addWidget(QLabel('Number of sources: ', self))
        count = config.get('source_count', 1)
        countlayout.addWidget(QLabel(str(count), self))
        
        self.layout().addLayout(countlayout)
        
class LoggerConfigure(QWidget):        
    '''
    Configure a logger (multilogger or complete).
    Normally this  is made the work area of a SaveDialog
    and, on accept, the client invokes makeLogger
    to create the logger.  That produces, either
    a logger, or a string error that is normally
    displayed in a message box subsequently giving
    the user another go at the dialog.
    
    '''
    def __init__(self, logger : EventLog | None = None, parent : QWidget | None = None):
        '''
        WHen instantiating the widget:
        @param logger : EventLog | None = None - if supplied, the form is loaded with
            the configuration of this logger. This allows a logger to be reconfigured later on.
        @param parent : QWidget | None = None - If supplied, provides the form's parent.
        
        '''
        super().__init__(parent)
        
        # Layout the form:
        
        layout = QVBoxLayout()    # Main layout is strips arranged vertically
        self.setLayout(layout)
        
        # Full or partial logger:
        
        self._full = QCheckBox('Full logger', self)
        layout.addWidget(self._full)
        
        # Run the logger in which host?
        
        hostlayout = QHBoxLayout()
        hostlayout.addWidget(QLabel('Run in host:', self))
        self._host = QLineEdit(self)
        hostlayout.addWidget(self._host)
        self._host.setText('localhost')    # Default value.
        
        layout.addLayout(hostlayout)
        
        # Which Logger?  Can browse for that:
        
        daqlayout = QHBoxLayout()
        daqlayout.addWidget(QLabel('DAQ bin directory with eventlog'))
        self._bindir = QLineEdit(self)
        daqlayout.addWidget(self._bindir)
        self._bindir.setText(os.environ.get('DAQBIN', ''))
        self._browsebindir = QPushButton('Browse...', self)
        daqlayout.addWidget(self._browsebindir)
        self._browsebindir.clicked.connect(lambda: self._browseDir('/usr/opt/daq', self._bindir))   # Browse handler.
        
        layout.addLayout(daqlayout)
    
        # Source ring:
        
        ringlayout = QHBoxLayout()
        ringlayout.addWidget(QLabel('Ring Buffer to log (URL)', self))
        self._ringuri = QLineEdit(self)
        ringlayout.addWidget(self._ringuri)
        self._ringuri.setText(f'tcp://localhost/{getpass.getuser()}')
        
        layout.addLayout(ringlayout)
        
        # Destination (where to log)
        
        destlayout = QHBoxLayout()
        destlayout.addWidget(QLabel('Logging Directory', self))
        self._destination = QLineEdit(self)
        destlayout.addWidget(self._destination)
        self._destination.setText(str(pathlib.Path.home()))
        self._browsedest = QPushButton('Browse...')
        destlayout.addWidget(self._browsedest)
        self._browsedest.clicked.connect(lambda : self._browseDir(str(pathlib.Path.home()), self._destination))
        
        layout.addLayout(destlayout)
        
        #  Do checksums:
        
        self._checksum = QCheckBox('Compute checksum files', self)
        layout.addWidget(self._checksum)
        self._checksum.setCheckState(Qt.CheckState.Checked)     # Default is on.
        
        # Source Count:
        
        sourceslayout = QHBoxLayout()
        sourceslayout.addWidget(QLabel('Number of event sources ', self))
        self._sources = QSpinBox(self)
        sourceslayout.addWidget(self._sources)
        self._sources.setMinimum(1)
        self._sources.setMaximum(10000)   # a lot (I hope).
        
        layout.addLayout(sourceslayout)
        
        # If the user supplied a logger, load the form from it:
        
        if logger :
            self._loadForm(logger)
    
    
    def setFull(self) -> None:
        ''' Set the full logger checkbox.'''
        self._full.setCheckState(Qt.CheckState.Checked)
    def setMultilog(self) -> None:
        ''' Unset the full logger checkbox. '''
        self._full.setCheckState(Qt.CheckState.Unchecked)
    def makeLogger(self) -> EventLog | str:
        '''
        Either create a logger configured as per the form or
        return an error message describing why that could not be done.
        
        @return Eventlog | str
        '''
        # Dict of mandatory parameters that must be present that the form
        # would allow to be absent and their error messages:
        mandatoryParameters = {
            'host' : 'You must specify a host for the event logger to run in',
            'bindir' : 'You must specify the bin subdirectory of an installed version of FRIB/NSCLDAQ so the eventlogger can be located', 
            'source' : 'You must specify a ringbuffer URI from which the data are logged.',
            'destination' : 'You must specify a destination directory in which loggin is done',
        }
        # CHeck that the mandator parameters are present and not empty.
        
        configuration = self._makeConfig()       # Make config dict from the form
        for param, msg in mandatoryParameters.items():
            if not configuration[param].strip():
                return msg
        # Futhermore, there must be an eventlog program in bindir:
        
        logger  = pathlib.Path(configuration['bindir']) / 'eventlog'
        if not logger.exists():
            return f'The directory {configuration["bindir"]} does not have an eventlog executable.'
        
        # make and return the appropriate logger type:
        
        if self._full.checkState() == Qt.CheckState.Checked:
            return FullEventLog(configuration=configuration)
        else:
            return MultiLogEventLog(configuration=configuration)
    
    def _makeConfig(self) -> dict[str, object]:
        # Create the configuration from the form.  Note that
        # We ignore the state of self._full:
        
        return {
            'host' : self._host.text(),
            'bindir' : self._bindir.text(),
            'source' : self._ringuri.text(),
            'destination' : self._destination.text(),
            'checksum' : self._checksum.checkState() == Qt.CheckState.Checked,
            'source_count' : self._sources.value()
        }
    
    def _loadForm(self, logger : EventLog) -> None:
        # What type of logger:
        
        full_state = Qt.CheckState.Checked if isinstance(logger, FullEventLog) else Qt.CheckState.Unchecked
        self._full.setCheckState(full_state)
        
        # Load the form from the EventLog ... assumed to have what we need
        
        config = logger.getConfig()
        # This code  assumes the default values are already loaded into the 
        # form
        textFields = {
            'host': self._host,
            'bindir' : self._bindir,
            'source' : self._ringuri,
            'destination' : self._destination    # Text fields and corresponding QLineEdits
        }
        for param,field in textFields.items():
            value = config.get(param, None)
            if value:
                field.setText(value)
        
        # that leaves the checksu7m and source count:
        
        checksum = config.get('checksum', None)
        if checksum is not None:
            state = Qt.CheckState.Checked if checksum else Qt.CheckState.Unchecked
            self._checksum.setCheckState(state)
        
        sources = config.get('source_count', None)
        if sources is not None:
            self._sources.setValue(sources)
    # Internal slots (private).
    
    def _browseDir(self, base: str, target : QLineEdit) -> None:
        # Browse for a directory and put it in the target line edit when found.
        # The directory string for the dialog is set to base.
        
        dir = QFileDialog.getExistingDirectory(self, 'Choose directory', base)
        if dir.strip():
            target.setText(dir)
            
            
# Register our loggers:

EventLogRegistry.instance().add(
    'multi logger', MultiLogEventLog, LoggerConfigDisplay, LoggerConfigure, 
    lambda prompt: prompt.setMultilog()
)
EventLogRegistry.instance().add(
    'Full logger', FullEventLog, LoggerConfigDisplay, LoggerConfigure,
    lambda prompt: prompt.setFull()
)

# Test code:

if __name__ == '__main__':
    import sys

    from nscldaq.mg_configutils import SaveDialog
    from PyQt6.QtWidgets import QApplication, QDialog, QMessageBox
    
    app = QApplication(sys.argv)
    
    # Prompt with save dialog for now:
    
    dlg =  SaveDialog(LoggerConfigure()) 
    status =  dlg.exec()
    while status == QDialog.DialogCode.Accepted:
        msgOrLogger = dlg.workarea().makeLogger()
        if isinstance(msgOrLogger, EventLog):
            logger = msgOrLogger
            break
        else:
            QMessageBox.warning(dlg, 'Invalid configuration', msgOrLogger)
            status = dlg.exec()
    
    if status != QDialog.DialogCode.Accepted:
        sys.exit(-1)
    
    cfgwin = LoggerConfigure(logger)
    cfgwin.show()
    win = LoggerConfigDisplay(logger)
    win.show()
    
    sys.exit(app.exec())
        
    