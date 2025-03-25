'''
This modlue provides client software for python appolications that interact with the manager.
Note that the following classes are intended to be public:

*  State - interact with the state machine and transitions.
*  Programs - Check the status of the various programs.
*  KVStore - interact with the Key value store including some convenience methods.
*  Loggers - interact with the event logging subsystem
*  OutputMonitor - connect to and get information from the output mirror.

See the rctl_rest module in the source directory main/utilities/readoutREST for clients to the
REST control provided by Readout programs.

'''
import os
import requests
import socket
import errno
from nscldaq.portmanager.PortManager import PortManager



def _getlogin():
    """Return the login name.
    Note that in a shell under WSL with MSVisual code, os.getlogin() fails with the exception:
    
    Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
OSError: [Errno 6] No such device or address

    Therefore we use os.getlogin() and if it throws an exception fallback to os.getenv('USER').
    
    
    """
    try :
        return os.getlogin()
    except:
        return os.getenv('USER')

def _service_port(host, name, port=30000, user=None):
    """Determines the port associated with a service both in the presence of the port manager.
    

    Args:
        host (str): Host on which the service is running
        name (str): Service name advertised by the port manager.
        port (int): Port number on which the port manager is running  Defaults to 30000 - the normal port for the manager.
        user (_type_, optional): Username - None means use the logged in user.
        
    Returns:
        On success the number of the port allocated to that servicde.
        On failure, NameError is raised.
        
        
    """
    
    # User if not given:
    
    if user is None:
        user = _getlogin()
    pm = PortManager(host, port)
    matches = pm.find(service=name, user=user)
    if len(matches)  != 1:
        raise NameError(f'Cannot find a match for {name}@{host}')
    
    return matches[0]['port']

class _Client:
    #  This is a base class for REST clients, containing, as it does,
    # the utilities that make REST requests easy
    def __init__(self, host, user=None, service="DAQManager"):
        """Construct the _Client object.

        Args:
            host (str): host running the manager program.
            user (str, optional): User running the state manager. Defaults to None - the current user.
            service (str, optional): Name of the advertised service. Defaults to "DAQManager" - the default manager service name.
            
        Raises:
            NameErorr if the manager is not running in the host.
        """
        self._port = _service_port(host, service, 30000, user)
        self._host = host
        
    def _create_uri(self, request):
        return f'http://{self._host}:{self._port}{request}'
    
    def _get(self, uri, parameters = {}):
        response = requests.get(uri, parameters)
        response.raise_for_status()
        try:
            json = response.json()
        except Exception as e:
            print('failed to parse\n', response.content)
            raise e
        if json['status'] != 'OK':
            raise RuntimeError(json['message'])
        return json
    
    def _post(self, uri, parameters):
        response = requests.post(uri, parameters)
        response.raise_for_status()
        try:
            json = response.json()
        except Exception as e:
            print('failed to parse\n', response.content)
            raise e

        if json['status'] != 'OK':
            raise RuntimeError(json['message'])
        return json
    
    
    

class State(_Client):
    """This class provides access to the /State part of a manager.
        Besides construction  we provide:
        
        status -Get the current state.
        allowed - Get the names of the states we are allowed to transition to from here.
        transition - performa a transition.
        elapsed   - Elapsed run time.
        shutdown - Transition to SHUTDOWN and exits the server.
        
        
    """
    def __init__(self, host, user=None, service='DAQManager'):
        super().__init__(host, user, service)
    
    def status(self):
        """Return the manager's  current state.  
        
        Raises:
            Whatever reqeusts.get and the json decoding might raise.
            
            
        """
        uri = self._create_uri('/State/status')
        json = self._get(uri)
        return json['state']
   
    def allowed(self):
        """Return an iterable containing the allowed next state.
        """
        uri = self._create_uri('/State/allowed')
        json = self._get(uri)
        return json['states']
        
    def transition(self, newstate):
        """Request a state transition.

        Args:
            newstate (str): Textual name of the state (e.g. "BOOT")

        Returns:
            The full JSON returned by the server.
        Note:
            SHUTDOWN when already SHUTDOWN seems to hang...does for Tcl client as well
        """
        parameters = {'user': _getlogin(), 'state': newstate}  
        uri = self._create_uri('/State/transition')
        json = self._post(uri, parameters)
        return json
    
    def elapsed(self):
        uri = self._create_uri('/State/elapsed')
        return self._get(uri)['elapsed']
    
    def shutdown(self):
        uri = self._create_uri('/State/shutdown')
        parameters = {'user' : _getlogin()}
        return self._post(uri, parameters)

class Programs(_Client):
    """This class provides support for the '/Programs' family of URIs.

    Args:
        _Client (_type_): The standard client class is our base.
    """
    def __init__(self, host, user=None, service='DAQManager'):
        super().__init__(host, user, service)
        
    def status(self):
        """status
              Returns the status of all defined programs.
        Retuns:
            A dict with the keys 'containers' and 'programs'.  Each is an array.
            
            'containers' has one element for each defined container.  The elements, 
            are themselves dicts with the keys:
            'name' - name of the container
            'image' - container image file.
            'bindings' - list of container bindings.   Each element of the list is a
            colon separated pair of strinngs.  The left string is a host path, the
            right where in the container that path is bound
            Examples:
                '/etc:/etc'  - host's /etc is bound into the containers /etc
                '/usr/opt/opt-bookworm:/usr/opt/bookworm'  - hosts /usr/opt/opt-bookworm
                  is bound into /usr/opt in the container.
            'activations' area list of the hosts in which the container is acstive.
            
            'programs' has one element for each defined program.   Each element is a
            dict with the following keys:
            'name' - the program name.
            'path'  - path of the program within the container it is to run.
            'type'  - Type of program e.g. 'Critical'
            'host'  - Host in which the container runs.
            'container' - if a non-empty string, the container in which the program 
               is run.
            'active' - 0 if the program is not running, 1 if it is.
        """
        uri = self._create_uri("/Programs/status")
        return self._get(uri)
    
class KVStore(_Client):
    """This class provides an interface to the key value store table.
    This is used to provide arbitrary associations between text strings and 
    time varying values.

    Args:
        _Client - generic client utility base class.
    """
    def __init__(self, host, user=None, service='DAQManager'):
        ''' See _Client.__init__'''
        super().__init__(host, user, service)
    
    def value(self, name):
        """Returns the value of a key in the KVStore.

        Args:
            name (str): Key whose value we'll return.
        """
        
        uri = self._create_uri('/KVStore/value')
        parameters = {'name': name}
        json = self._get(uri, parameters)
        return json['value'].strip('{').strip('}')
    
    def listNames(self):
        """Returns:
           The names of all of the keys in the KVStore.
        """
        uri = self._create_uri('/KVStore/listnames')
        json = self._get(uri)
        return json['names']
    
    def list(self):
        """Returns the list of {'name': varname, 'value': value} dicts
        """
        uri = self._create_uri('/KVStore/list')
        json = self._get(uri)
        return json['variables']
    
    def set(self, name, value):
        """Sets the value of a kvstore element.  Note it is an exception
        to set the value of a 'name' that has not been created.

        Args:
            name (str): Name of the variable.
            value (str): New value of the variable.
        """
        uri = self._create_uri('/KVStore/set')
        parameters = {'user': _getlogin(), 'name': name, 'value': value}
        json = self._post(uri, parameters)
        return json
    
    # These are convenince methods based on the fact that 'title'
    # and 'run' are always defined:
    
    def title(self):
        """Returns the title:
        """
        return self.value('title')
        
    def setTitle(self, title):
        """Set the new title value

        Args:
            title (str): new title string
        """
        self.set('title', title)
    
    def run(self):
        """
        Returns:
            The run number as an integer.
        """
        return int(self.value('run'))
    def setRun(self, runNumber):
        """Set the the run number

        Args:
            runNumber (int): New run number.
        """
        self.set('run', str(runNumber))
        

class Logger(_Client):
    """This class provides access to the event log subsystem.

    Args:
        _Client (class): Base class that provides common services for 
        all ReST clients.
    """
    def __init__(self, host, user=None, service='DAQManager'):
        super().__init__(host, user, service)
        
    def enable(self, destination):
        """Enables a logger

        Args:
            destination (str): Logger destination.
        """
        uri = self._create_uri('/Loggers/enable')
        parameters = {'logger': destination, 'user': _getlogin()}
        self._post(uri, parameters)
    
    def disable(self, destination):
        """Disable a loggers.

        Args:
            destination (str): Logger destination.
        """
        uri = self._create_uri("/Loggers/disable")
        parameters = {'logger': destination, 'user': _getlogin()}
        self._post(uri, parameters)
    
    def list(self):
        """Fetch a list of the loggers.

        Returns:
            Iterable (e.g. list) each item describes a logger using a dict with
            the following keys:
            
            id - Primary key of the logger.
            ring - the data source ring buffer.
            daqroot - The NSCLDAQ root that will be use to find 'eventlog'
            host  - Host the logger runs in.
            partial - nonzero if the logger is a partial logger otherwise 0.
            destination - where the data are logged (context of the container).
            critical - 1 if the logger is critical, 0 if not.
            enabled - 1 if the loggers is enabled else 0.
            container - name of the container the logger runs in.
            
        """
        uri = self._create_uri('/Loggers/list')
        return self._get(uri)['loggers']
    
    def record(self, state):
        """Enable/disable event recording.  Next time the run starts, all enabled loggers will record
        data from that run.
        
        Paramters:
          state (int)  - State of the recordinng, 0 not, 1 record.
        """
        
        uri  = self._create_uri('/Loggers/record')
        parameters = {'user': _getlogin(), 'state': 1 if state else 0}
        self._post(uri, parameters)
    
    def isRecording(self):
        uri = self._create_uri('/Loggers/isrecording')
        json = self._get(uri)
        
        return json['state'] != 0
    
    def start(self):
        """Start all enabled loggers.
        """
        
        uri = self._create_uri('/Loggers/start')
        parameters = {'user': _getlogin()}
        self._post(uri, parameters)
        


class Disconnected(Exception):
    def __init__(self):
        super().__init__("Connection dropped or never formed.")

class OutputMonitor:
    """Provides access to the output monitor service of the manager.
    In normal usage, construction implies connection.  What you'd then normally
    do is periodic 'read' calls to get the available data (if any).  
    The read call will raise manager_client.Disconnected if the connection
    is dropped (manager exits).  In that case, it's entirely up to the client of this
    class to decide what to do; but one possibility is to invoke reconnect to 
    attempt to reconnect...that too will raise Disconnected if the connection
    fails.
    
    Construction can also raise Disconnected if the initial connection fails.
    
    
    """
    def __init__(self, host, user=None, service='DAQManager-outputMonitor'):
        """
           We just resolve the service and store the host and port.
           Next we call our reconnect method to try to form a connection.
           
           Args:
            host (str)  - The DNS address or dotted IP string of the host running the
                        manager.
            user (str)  - If supplied the username the manager is running under.  If not,
                    the current user.
            service (str) - The service to connect to, if supplied, defaults to 'DAQManager-outputMonitor'
                    the default service name.
        """
        self._host = host
        self._port = _service_port(host, service, 30000, user)
        self._socket = None     # Not yet connected.
        self.reconnect()
        
    def reconnect(self):
        """Reconnect to the ouptput monitor service.
            We make use of self._host and self_port to know how to do this.
            As a side effect, self._socket will be modified.
        
        """
        
        # If necessary, shutdown the socket first.
        if self._socket is not None:
             self._socket.shutdown(socket.SHUT_RDWR)
             self._socket = None     # To be sure the socket destructs.
        
        try:
            s = socket.create_connection((self._host, self._port))
            s.setblocking(False)    # So we can poll for input.
            self._socket = s
        except:
            raise Disconnected
        
    def isConnected(self):
        """
            Returns (bool)
            True  - if the connected
            False - if not.
        """   
        return self._socket is not None
    
    def read(self):
        """Read data from the socket.  The data are assumed to be textual
        in nature and appropriately converted.  
        
        -  If no data are available, the underlying EWOULDBLOCK exception is
           turned into a return value of 0.
        -  If there's no exception but no data (connection closed), we 
           close the socket from our side and fraise Disconnected.
        
        all this is calculated to allow users to e.g:
        
            mon = OutputMonitor('somehost')
            from time to time:
                try:
                    data = mon.read()
                    if len(data) > 0:
                       do_something_with_the_data(data)
                except Disconnected:
                    mon.reconnect() - until successful or you give up.
        """
        if self._socket is None:
            raise Disconnected     # Because we already were.
        
        try:
            raw_data = self._socket.recv(1024)
            if len(raw_data) == 0:
                # Lost connection rais connnlost the exception
                # handler will take care of the rest.
                raise OSError(errno.ECONNRESET, 'Lost connection')
            return raw_data.decode('ascii')
        except OSError as e:
            reason = e.errno
            if reason == errno.EWOULDBLOCK or reason == errno.EINTR:
                return ""        # Empty string instead
            if reason == errno.ECONNRESET:
                # Lost the connection:
                self._socket.shutdown(socket.SHUT_RDWR)
                self._socket = None
                raise Disconnected     # Signal the caller we disconnected.
            raise e              # Something else, so raise it.