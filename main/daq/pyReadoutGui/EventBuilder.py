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
@file EventBuilder.py
@brief Classes to support event building.
@author Ron Fox

This class provides the classes needed to run the programs
that make up an event builder:

FragmentSource - The source of data fragments ($DAQBIN/RingFragmentSource)
EventBuilderPipeline   - The event builder pipeline consisting of 
      $DAQBIN/Orderer | $DAQBIN/glom | $DAQBIN/stdintoring
       
       And their command line and live configuration options
       (pushed at Orderer via its stdin).
EventBuilder - Consisting of an EventBuilderPipeline and the sources that feed it.

Note:  Unlike the Tcl readout GUI, the fragment sources and event builder pipline are run
        in pySSHProcess objects which allow them to be allocated to any system
        that participates in the experiment.
'''
import getpass
from abc import abstractmethod

from PyQt6.QtCore import QObject, QProcess, pyqtSignal
from pySSHProcess import SSHProcess

class EncapsulatedCommand(QObject):
    '''
    Common code for ssh encapsulation:
    derive from this and implement your own _generateCommand method which
    is intended to provide the command that will run in the shell
    started.
    
    Signals:
    finished - Relays the finished signal from the SSHProcess.
    outputLine     - Signals a line of text was output/errored.
    '''
    finished = pyqtSignal(int, QProcess.ExitStatus)
    line     = pyqtSignal(str)

    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        self._process : SSHProcess | None  = None
    
    def start(self) -> None:
        '''
        Starts the process and sets up the signal relay. Relies on the concrete
        class to implement two methods:
        _getHost - returns the host in which the command runs.
        _generateCommand - returns the command to run in that host.
        '''
        
        command = self._generateCommand()
        host    = self._getHost()
        self._process    = SSHProcess()
        self._process.finished.connect(self.finished)
        self._process.finished.connect(self._exited)
        self._process.readyReadStandardInput.connect(self._relayOutput)
        self._process.readyReadStandardOutput.connect(self._relayOutput)
        self._process.spawnRemote(host, command)
    
    def process(self) -> SSHProcess | None:
        '''
        @return the process or None if start has not been called.
        '''
        return self._process
    
    # Abstract methods:
    
    @abstractmethod
    def _getHost(self) -> str:
        '''
        @return str - the host in which to run the program.
        '''
        ...
        
    @abstractmethod
    def _generateCommand(self) -> str:
        '''
        @return str - the comman to run in the shell on the other end of the ssh pipe.
        '''
        ...
        
    
    # Internal slots:
    
    def _exited(self, status : int, reason : QProcess.ExitStatus) -> None:
        # The process exited, destroy the ssh process:
        # anyone else who cares can catch the signal as well.
        
        self._process = None   # Eventually gets garbage collected.

    def _relayOutput(self) -> None:
        '''
        The ssh is readable, read a line and signal it:
        '''
        line = self._process.ReadLine()
        self.line.emit(line)

class FragmentSource(EncapsulatedCommand):
    '''
    This class is responsible for running on $DAQBIN/ringFramgentSource.
    It is configured with a dict that has the following keys:
    - host - the host in which the source will run.
    - evbhost - The event builder host.
    - evbservice - The service the event builder is advertising with the port manager.
    - description - The description string the orderer displays for the source.
    - ids     - List of source ids expected in the data.
    - default-id-  Source id to assign data without a predefined
                    source id
    - ring   - URI of the data source that will be pushed into the event builder.
    - timeout - Seconds the orderer hangs around looking for end runs before bailing.
                                  (default=`10')
    - offset  - Signed offset to add to the timestamps in the data to compensate for known skew
    - ts-divisor - integer divisor to apply to the timestamp to compensate for different but 
                  synchronized clock frequencies.  This is applied prior to the offset.
                e.g the final timestamp of an item is (original/ts-divisor + offset).
    - end-runs - Number of end runs to expect from the source. Note that this is to support
              hierarchical event building.
        See the class method applyDefaults for the defaults for these if they have them.
        See the class method checkConfig to get an error message for missing but needed configuration values.
    
    Signals:
    finished   - Relay of the finished signal from QProcess that runs the source.
    line(str)  - A line of ouptut/error has been captured.
    
    '''
    finished = pyqtSignal(int, QProcess.ExitStatus)
    line     = pyqtSignal(str)
    #--------------------------------- Class (static) methods:
    @staticmethod
    def configTypes() -> dict[str, type]:
        '''
        @return dict[str, type] - The values of the dict are the valid type for that config param (key)
        '''
        return {
            'host' : str,
            'evbhost' : str,
            'evbservice' : str,
            'description' : str,
            'ids' : list[int],
            'default-id' : int,
            'ring' : str,
            'timeout' : int,
            'offset' : int,
            'ts-divisor' : int,   
            'end-runs'   : int
        }
    @staticmethod    
    def applyDefaults(config : dict[str, object]) -> dict[str, object]:
        '''
            Applies default values for configuration values that have them.
            @param config - the configuration, a dict keyed as shown in the 
                class docstring with values the values of that configuration
                parameter.
            @return - A new dict which is the config with defaults applied.
            @note - if there are missing values that are have not defaults, they will
            remain missing, see checkConfig which shoulid be called after us.
        '''
        result = config

        # Apply the defaults to config.
        
        result['evbservice'] = config.get('evbservice', f'ORDERER:{getpass.getuser()}')
        result['ring']       = config.get('ring', f'tcp://localhost{getpass.getuser()}')
        result['default-id'] = config.get('default-id', 0)
        result['timeout']    = config.get('timeout', 10)
        result['offset']     = config.get('offset', 0)
        result['ts-divisor'] = config.get('ts-divisor', 1)
        result['end-runs']   = config.get('end-runs', 1)
    
    @classmethod
    def checkConfig(cls, config : dict[str, object]) -> str | None:
        '''
        Check that a configuration has all its parameters.
        @param config - The configuratino to check.
        @retun str | None - A string if there is a problem, None on success.
        '''
        
        required_options = ['host', 'evbhost', 'description', 'ids']
        for option in required_options:
            if option not in config:
                return f'The {option} configuration parameter is missing.'
        
        # Check that all the configuration values are the right type:
        
        typedict = cls.configTypes()
        for name,t in typedict.items():
            if not isinstance(config[name],t):
                return f'Configuration param {name} must by of type {t.__name__} was {type(config[name]).__name__}'
        
        return None                        # Be explicit.
    
    def __init__(self, config : dict[str, object], parent : QObject | None = None):
        '''
        Construct a FragmentSource object. Note that the source is not started
        until the 'start' method is invoked.
        @param config : dict[str, object] - the configuration dict.
        @param parent : QObject | None = None - the parent object or None if not supplied.
        
        '''
        
        super().__init__(parent)
        
        # IF there is a problem with the configuration, after the
        # defaults are applied, that's a ValueError
        
        finalConfig = self.applyDefaults()
        errorMsg    = self.checkConfig(finalConfig)
        if isinstance(errorMsg, str):
            raise ValueError(f'Error in FragmentSource configuration: {errorMsg}')
    
        self._config = finalConfig
    
            
    def getConfig(self) -> dict[str, object]:
        '''
        @return dict[str, object] - The configuration of the source.
        '''
        return self._config
    
    
    
    # Utilities:
    
    def _getHost(self) -> str:
        # Return the host torun in:
        
        return self.config['host']
    def _generateCommand(self) -> str:
        # Based on the configuration, gernerates and
        # returns the command to run
        
        command = ['$DAQBIN/ringFragmentSource', ]
        command += f'--evbhost={self._config["evbhost"]}'
        command += f'--evbname={self._config["evbservice"]}'
        command += f'--info="{self._config["description"]}"'   # " for when join is done.
        
        # one --ids for each id:
        
        for id in self._config['ids']:
            command += f'--ids={id}'
        
        command += f'--default-id={self._config["default-id"]}'
        command += f'--ring={self._config["ring"]}'
        command += '--expectbodyheaders'
        command += f'--oneshot={self._config["end-runs"]}'
        command += f'--timeout={self._config["timeout"]}'
        command += f'--offset={self._config["offset"]}'
        command += f'-ts-divisor={self._config["ts-divisor"]}'


        return command
    
        
class EventBuilderPipeline(EncapsulatedCommand):
    '''
    This class runs an event builder pipline consisting of
    the Orderer, the configuration options are:
    General:
    - host           - host in which to run the pipeline.
    Orderer:
    - service-suffix - Suffix to add to ORDERER:username e.g. ORDERER:ron:service-suffix
    - orderer-window - Orderer flush window.
    - XoffThreshold  - Threshold queued bytes to xoff.
    - XonThreshold   - Threshold queued byts to xon (should be smaller than XoffThreshold)
    - perQXoffThreshold - Xoff threshold but per queue.
    - perQXonThreshold  - Xon threshold but per uquqe.
    Glom.
    - dt            -  Event building coincidence interval.
    - tspolicy      -  How the timestamp is assigned (average, earliest, latest)
    - sourceid      - Output source id.
    - maxfragments  - max fragments in queue.
    stdintoring
    - ring          - Where event built data goes.
    
    @note as with the FragmentSource, we relay the SSHProcess finished signal
    
    '''
    finished = pyqtSignal(int, QProcess.ExitStatus)
    @staticmethod
    def configTypes() -> dict[str, type]:
        '''
        @return  dict[str, type] for each configuration parameter, the type of the parameter.
        '''
        return {
            'host'           : str,
            'service-sufix'  : str | None,       # Union types sure are cool.
            'orderer-window' : int | None,
            'XoffThreshold'  : int | None,
            'XonThreshold'   : int | None,
            'perQXoffThreshold' : int | None,
            'perQXonThreshold'  : int | None,
            'dt'              : int,
            'tspolicy'        : str,
            'sourceid'        : 0,
            'maxfragments'    : int | None,
            'ring'            : str
        }
        
    @staticmethod
    def applyDefaults(config : dict[str, object]) -> dict[str,object]:
        '''
            Apply default configuration options to an EventBuilderPipeline
            configuration dict for any missing parameters. See the class
            docstring.
            
            @aram config : dict[str, object] - The input configuration dict.
            @return dict[str, object]  - The configuration with defaults applied to missing values.
            
            The defaults are:
        - host           - The localhost.
        - service-suffix - None - yielding no parameter to orderer
        - orderer-window - None - not pushed to the orderer so the default orderer window.
        - The various Xoff/Xon Thresholds None - not pushed to the orderer.
        - dt             - No default value must be supplied.
        - tspolicy       - latest
        - sourceid       - 0
        - maxfragments   - None - not set on command line.
        - ring           - f'{getpass.getuser()}_built}
        
        @note there are two types of defaults, 'active' and 'passive'. Active defaults
            must be asserted by us and passive are properties of the programs which
            we don't override if defaulted.  Passive defaults are represented by None values.
        '''
        # This dict captures the defaults:
        
        defaults = {
            'host'           : 'localhost',
            'service-suffix' : None,
            'orderer-window' : None,
            'XoffThreshold'  : None,
            'XonThreshold'   : None,
            'perQXoffThreshold' : None,
            'perQXonThreshold'  : None,
            'tspolicy'       : 'latest',
            'sourceid'       : 0,
            'maxfragments'   : None,
            'ring'           : f'{getpass.getuser()}_built'
        }
        result = config       # The starting point.
        for name,value in defaults.items():
            result[name] = result.get(name, value)     # Apply defaults to keys not present.
        
        
        return result
    
    @classmethod
    def checkConfig(cls, config : dict[str, object]) -> str | None:
        '''
        Check the legality of a configuration.  First that 'dt' the only 
        reqauired parameter is present and then that all parameters have the
        correct type.  The defaults should have been applied.  Every
        configuration item not present is treated as None.
        
        @param config - the configuration to check.
        @return str | None : None means goot configuration a string is an error message.
        '''
        # 'dt' is mandatory with no default.
        if not config.get('dt'):
            return 'The "dt" configuration parameter is required'
        
        # Validate the timstamp policy.
        
        validTsPolicies = ['average', 'earliest', 'latest', None]
        if config['tspolicy'] not in validTsPolicies:
            return f'tspolicy was {config.get["tspolicy"]} must be one of {validTsPolicies}'
        for name,configType in cls.configTypes().items():
            if not isinstance(config.get(name), configType):
                return f'The configuration parameter {name} must be of type {configType} was {type(config.get(name)).__name__}'
        return None
    
    def __init__(self, config : dict[str, object], parent : QObject | None = None):
        '''
        @param config : dict[str, object] - the configuration of the event builder pipeline.
        @parent parent : QObject | None = None - the parent object, if supplied.
        @note the event builder pipeline is not started, see the start and stop methods
            run and kill the pipeline.  The configuration, does, however, have
            default values applied and is type checked.
        @throws ValueError if the configuration is invalid.
        '''
        super().__init__(parent)
        fullConfig = self.applyDefaults(config)
        valid      = self.checkConfig(fullConfig)
        if valid:
            # valid is an error string about why the config is invalid.:
            
            raise ValueError(f'Invalid configuration: {valid}')

        # Configuration is ok:
        
        self._config = config
    
    def getConfig(self) -> dict[str, object]:
        '''
        @return dict[str, object] - The configuration of the event builder.
        '''
        return self._config
 
 
    def start(self) -> None:
        # We have to override the base class so we can poke stuff down
        # the pipe.
        
        super().start()     # kick things off.
        
        self._setOrdererParms()  # poke the orderer parameters down the pipe.
    
   
    #  Implementation of protected methods:
    
    def _getHost(self) -> str:
        # Return the host in which to run:
        
        return self._config['host']
    def _generateCommand(self) -> str:
        # Return the command to run.
        
        orderer = self._generateOrderer()
        glom    = self._generateGlom()
        stdinToRing = self._generateStdinToRing()
        
        return f'{orderer} | {glom} | {stdinToRing}'
    
    def _generateOrderer(self) -> str:
        # Generate the orderer command.  This is $DAQBIN/startOrderer
        # and, if a service-suffix was provided that as a parameter.
        # The remainder of the configuration is pushed through the
        # orderer's stdin.
        
        command = '$DAQBIN/startOrderer'
        suffix = self._config['service-suffix']
        if suffix:
            command += f' {suffix}'
        
        return command
    
    def _generateGlom(self) -> str:
        # Generate the glomc command fromt he configuration.
        
        command = '$DAQBIN/glom '
        
        # Map of option to glom switches:
        # Yes,  I know that the mapping just pre-pends '--' but maybe
        # later we'll do things like 'timestamp-policy' instead of 'tspolicy'.
        glomOptionMap = {
            'dt' : '--dt',
            'tspolicy' : '--tspolicy',
            'sourceid' : '--sourceid',
            'maxfragments' : '--maxfragments'
        }
        # Fold in the command line options
        for name, option in glomOptionMap.items():
            if self._config[name]:
                command += f' {option}={self._config[name]}'
    
        return command

    def _generateStdinToRing(self) -> str:
        # Generate the StdinToRing command:
        
        return f'$DAQBIN/stdintoring {self._config["ring"]}'
        
    def _setOrdererParms(self) -> None:
        # Some orderer commands get pushed down into the stdin of 
        # The pipeline (orderer).
        # This method does that.
        
        # again we'll have a mapping between our parameter names
        # and the parameter names the orderer has
        paramNameMap  = {
            'orderer-window' : 'window',
            'XoffThreshold'  : 'XoffThreshold',
            'XonThreshold'   : 'XonThreshold',
            'perQXoffThreshold' : 'perQXoffThreshold',
            'perQXonThreshold'  : 'perQXonThreshold'
        }
        pipe = self.process()
        for configName, paramName in paramNameMap.items():
            value = self._config[configName]
            if value:                      # None means default:
                pipe.WriteLine(f'EVB::config set {paramName} {value}')
                pipe.flush()
                