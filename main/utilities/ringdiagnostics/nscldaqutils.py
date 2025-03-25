'''
    NSCLDAQ utility methods
'''
import subprocess
import shutil
import os
import socket
import time
from pyparsing import nestedExpr

from nscldaq.portmanager.PortManager import PortManager 
sshcmd=shutil.which('ssh')
cutText='---------- cut'

#------------- Internal utilities.

def _command(pipe, command):
    
    command = command + "\n"
    pipe.stdin.write(command)
    pipe.stdin.flush()
    
def _iscontainer():
    # Return True if we are running in a container
    try:
        container = os.environ['SINGULARITY_NAME']
        return True
    except:
        return False

def _containerName():
    #  If SINGULARITY_CONTAINER is defined, we use that, 
    #  If not, try KeyError else  raise KeyError
    
    try:
        return os.environ['SINGULARITY_CONTAINER']
    except:
        return os.environ['SING_IMAGE']
 
def _containerBindings():
    # Return a string that can be handed to the --bind option
    # for singularity/apptainer.
    
    return os.environ['CONTAINER_BINDINGS']

def _rebuildContainer(pipe):
    #  Rebuild our container environment in the remote:
    
    sing_cmd = f"SINGULARITY_SHELL='/bin/bash' singularity shell --no-home --bind {_containerBindings()} {_containerName()}"
    _command(pipe, sing_cmd)
    
def _setupEnvironment(pipe):
    # Push our environment into the ssh shell.
    # This is really intended to get people other than us to have an easier time.
    
    for name in os.environ:
        value = os.environ[name]
        cmd = f'export "{name}"="{value}"'  # Quotes because spaces might be there.
        _command(pipe, cmd)

def _setcwd(pipe):
    #  Set the current directory to our current directory:
    
    command = f'cd "{os.path.abspath(os.curdir)}"'  # Quotes protect against spaces in the dirname.
    _command(pipe, command)


def _afterCut(l):
    global cutText
    #  Returns all the lines in a list of lines after the line with
    #  
    
    while l[0].count(cutText) == 0 and len(l) > 0:
        l.remove(l[0])
        
    if (len(l)) == 0:
        return ''     # Nothing.
    # Get rid of one more element... the cut line"
    
    l.remove(l[0])
    
    # There's always an empty line after this:
    
    l.pop()
    
    return l

def _getRingMasterPort(host):
    # Return the port the ring master is running in on the host
    # else throw an exception if it, or the port manager is not running.
    
    pm = PortManager(host)
    info = pm.find(service='RingMaster')
    if len(info) == 0:
        raise KeyError('cannot find ring master')
    return info[0]['port']
    
        

#--- Public entries.
def ssh(host, command):
    global sshcmd
    global cutText
    """ 
        Performs an ssh command in the current environment
        on a remote host.  
        
        *  The user must have set up passwordless ssh -> host.
          Their shell must be bash or some other sh compatible shell.
        *  We attempt to restore the environment in the remote host
        this includes
           -  THe container being run if any and its filesystem bindings.
           -  The environemt we run under.
           -  Our current working directory.
           
        Parameters:
            host - host in which the command will run. 
            command - the command to run.
            
        Returns:
            A two element list.  The first element contains stdout
            captured by the entire sequence of command required to 
            do the command.  That might be a lot of commands.
            A line containing
            
            ---------- cut
            
            is added to the output just prior to the output for the command.
            
           The second element of the list is the captured stderr.
        Note: The ssh remote has exited when we return.

    
    """
    #  Establish the connection:
    
    pipe = subprocess.Popen(
        [sshcmd, host], 
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE, text=True, encoding='utf-8')
    
    # Get the banner:
    
        
    if _iscontainer() :

        # IF we are in a container set up the container environment in the remote host.
        _rebuildContainer(pipe)
    
    # Send our environment to the remote.
    
    _setupEnvironment(pipe)
    _setcwd(pipe)
    
    #  Provide a marker to allow clients to pull their own output
    # from the aggregate output:

    _command(pipe, f"echo {cutText}")
    _command(pipe, f"echo >&2 {cutText}")

    # Send the command return the response.

    _command(pipe, command)
    
    if _iscontainer():
        _command(pipe, "exit")
    
    # Exit the ssh.
    
    _command(pipe, "exit")
    
    result = pipe.communicate()
    
    return result

def getSshOutput(result):
    '''
    Given the result from an ssh operation, returns the output from only 
    the command as a list of lines.
    '''
    
    stdout = result[0].split('\n')
    return _afterCut(stdout)

def getSshError(result):
    '''
    Given the restul of an ssh operation, returns the error from only the command.
    as a list of lines.
    '''
    
    stderr = result[1].split('\n')
    return _afterCut(stderr)
    
class RingMaster:
    ''' 
     This class is a proxy for a ring master in some host.
     On creation, a connection is formed with the ring master
     and maintained (if possible) for the life of the object.
     
     What is a returned is an iterable/indexable (array) of dicts.
     This could be empty.
     
     Each dict has the following key/values:
     
     * 'name' - name of the ringbuffer.
     * 'size' - number of bytes in the data area of the ring buffer.
     * 'free' - Number of free bytes in the data area.
     * 'maxconsumers' - maximum number of consumers that can connect to the ring.
     * 'producer_pid' - The pid of the ringbuffer producer.
     * 'maxget' - Largest get that could be made (biggest backlog).
     * 'minget'  - smallest backlog.
     * 'consumers' - ringbuffer consumers.  The value is an interable containing dicts that have the keys:
         * 'consumer_pid'  - PID of the consumer process.
         * 'backlog'      - data backlog in bytes.
     
    '''
    def __init__(self, host):
        '''
        Initialization attempts to connect to the ring master to the
        host.
        '''
        port = _getRingMasterPort(host)
        self._socket = socket.socket()
        self._socket.connect((host, port))
        self._host = host
    
    def __del__(self):
        #self._socket.close()
        #self._socket = None
        pass
        
    # Public interface:
    
    def list_rings(self):
        '''
        Return a list of the ringbuffers, their characteristics and 
        usage
        '''
        
        self._socket.sendall(b'LIST\n')
        time.sleep(0.1)
        raw_list = self._socket.recv(1024*1024)
        lines = raw_list.decode('utf-8').split('\r\n')
        
        if len(lines) == 0:
            # Lost connection.
            self._socket.close()   # Render us unusable.
            self._socket = None
            raise ConnectionAbortedError('lost connection with ringmaster')
        
        # If the first line is not OK raise.
        
        if lines[0] != 'OK':
            raise SystemError(f'Error response from Ringmaster: {lines[0]}')
        
        # Parse line 1 as a list of Tcl lists:
        
        tclList = nestedExpr("{", "}")
        result =  []
        for item in tclList.searchString(lines[1]):
            l = list(item)
            result.append(self._makeRingDict(l[0]))
            

        return result
    
    def host(self):
        ''' Return the host we're talking to: '''
        return self._host
    
    # Internal methods
    
    def _makeRingDict(self, item):
        # Given a ring item defintion in raw form turn it into a dict.
        
        result =dict()
        
        result['name'] = item[0]
        
        properties = item[1]
        result['size'] = int(properties[0])
        result['free'] = int(properties[1])
        result['maxconsumers'] = int(properties[2])
        result['producer_pid'] = int(properties[3])
        result['maxget'] = int(properties[4])
        result['minget'] = int(properties[5])
        
        consumers = properties[6]
        consumerdicts = []
        for c in consumers:
            consumerdicts.append({
                'consumer_pid' : int(c[0]),
                'backlog'     : int(c[1])
            })
        result['consumers'] = consumerdicts
    
        return result