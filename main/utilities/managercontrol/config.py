
import tomllib
import os

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

class Configuration:
    # Configuration defaults
    default_host='localhost'
    default_rest_service='DAQManager'
    default_monitor_service =  "DAQManager-outputMonitor"
    default_user = _getlogin()
    
    """Provides configuration file support via a .toml file.
        The toml file provides two sections:
        
        [connection] - which provides the manager connection information via
            host  - host where the manager lives
            rest_service - the rest service name advertiesd by the manager in host.
            monitor_service - the output monitor service name advertised by the manager in host.
            user  - THe user running the manager.
            
        [programs]
            readouts - names of the readout programs in the database.
            
        
        Defaults are defined (we use the notation section.key below):
        
        connection.host - "localhost"
        cpmmection.rest_service - DAQManager
        connection.monitor_service - DAQManager-outputMonitor
        connection.user - current user.
        
        programs.readouts - []
        
    """
    def __init__(self, filename):
        """Constructor.

        Args:
            filename (str): path to the configuration file.
        """
        with open(filename, "rb") as f:
            self.config = tomllib.load(f)
     
    def _set(self, group, key, value):
        if not group in self.config:
            self.config[group] = {key:value}
        else:
            self.config[group][key] = value

    # Attribute methods.
    
    def host(self):
        """Returns:
             (str) - host if present in the file or default_host if not.
        """
        try:
            return self.config['connection']['host']
        except:
            return self.default_host     # return default if missing key.
        
    def setHost(self, host):
        """Change the value of the host.

        Args:
            host (str): New host name string.
        """
        self._set("connection", "host", host)
        
    
    def rest_service(self):
        """Returns:
              (str)  - The service name advertised for the ReST interface.
        """
        
        try:
            return self.config['connection']['rest_service']
        except:
            return self.default_rest_service
    def setRest_service(self, service):
        """Set the connection.rest_service value

        Args:
            service (str): New REST service value
        """
        self._set('connection', 'rest_service', service)
    
    def monitor_service(self):
        """Returns:
            (str) - the service advertisef for the monitor service.
        """
        try:
            return self.config['connecton']['monitor_service']
        except:
            return self.default_monitor_service
    def setMonitor_service(self, service):
        """Set the value of the monitor service.

        Args:
            service (str): New service name.
        """
        self._set('connection', 'monitor_service', service)
    
    def user(self):
        """Returns:
            (str) - Name of the user the manager is running under.
        """
        try:
            return self.config['connection']['user']
        except:
            return self.default_user
        
    def setUser(self, user):
        """Change the username in the configuration.

        Args:
            user (str): New username.
        """
        self._set('connection', 'user', user)
    
    def readouts(self):
        """Returns:
              Array of the readouts in the system.
        """
        try :
            return self.config['programs']['readouts']
        except:
            return []
    def setReadouts(self, readouts):
        """Set a new set of readout programs. N

        Args:
            readouts ([str,]): New list of readout programs.
            
        """
        self._set('programs', 'readouts', readouts)
        
    def addReadout(self, readout):
        """Append a new readout to the list.

        Args:
            readout (str): Readout program namee.
        """
        if 'programs' not in self.config.keys():
            self.config['programs'] = {'readouts': []}
        if 'readouts' not in self.config['programs'].keys():
            self.config['programs'] = {'readouts': []}
        
        
        # Now we have a readouts array which might be empty but so what.
        
        self.config['programs']['readouts'].append(readout)
        
    def dump(self, file):
        """Dump the configuration to file.
        We recognize:
           - The special key 'title' which has a string value.
           - All other top level keys are sections 
           - All values in sections are strings or lists of strings.

        Line endings will be \n (unix)

        Args:
            file - Name of the file to write the configuration to.
            
        
        """
        with open(file, 'w') as f:
            if 'title' in self.config.keys():
                f.write( f'title = "{self.config["title"]}"\n')
            for section in self.config.keys():
                if section != 'title':
                    f.write(f'\n[{section}]\n')
                    for key in self.config[section].keys():
                        value = self.config[section][key]
                        if isinstance(value, str):
                            f.write(f'{key} = "{value}"\n')
                        elif isinstance(value, list):
                            # Wrap each item value in "" and join them with ','
                            f.write(f'{key} = {value}\n')
                                
                           
                        
        