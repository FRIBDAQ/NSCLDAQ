'''
The RingMaster module provides access to the RingMaster LIST operation.
This operation allows clients to determine the set of ring buffers and
current usage statistics.
'''
from nscldaq.portmanager import  PortManager
import socket
import tkinter      # Easiest way to get the Tcl list Ringmaster sends parsed.

def getPort(host, port=30000):
    '''
        Given a host and the port of the port manager, returns
        then port on which the RingMaster is running in that 
        host:
        
        Parameters:
          host - host name we want information about.
          port - Port on which the portmanager is running (defaults to 30000
                 which is the normal port).
        Returns:
            integer - port number on which the ring master is running.
            None    - The port manager is running but not the ringmaster on that host.
        Raises 
            RuntimerError - if the port manager did not like our request.
            Exceptions from the socket library (e.g. no Port manager in the host.)
    '''
    pm = PortManager.PortManager(host, port)
    matches = pm.find(service="RingMaster")
    if len(matches) == 0:
        return None
    else:
        return matches[0]['port']
    
    
    
def usage(host='localhost', portman=30000):
    '''
        Returns ringbuffer usage.
        
        Parameters:
           host - host the ringmaster is running in (defaults to 'localhost')
           portman - port the _port_manager_ (note ringmaster) is running on, (defaults to 30000)
        
        Returns:
            An array of dicts.  Each dict has the following keys:
            name - name of a ringbuffer.
            size - Size of the data region of the ringbuffer (bytes)
            free - Number of free bytes in the data region.
            maxconsumers - Maximum number of consumers supported by this ringbuffer.
            producer  - PID of the producer.  This is -1 if there is no producer.
            maxget    - Bytes in the worst case backlog.
            minget    - bytes in the best case backlog
            consumers - array of dicts describing consumers.  This dict has the following keys:
                pid   - process id of the consumer.
                backlog - number of bytes of backlog.
            
            If the ringmaster is not running but the port manager is, None is returned.
        Notes:
           * The default paramters are such that for typical FRIB/NSCLDAQ systems, this will
           return ring usage for the host that called it.
           * The consumers array can, of course be empty.  However if the ring monitor is running,
           there will be at least one element.
        Raises:
            Stuff from the socket library when communicating with the ringmaster.
            Anything from getPort.
    '''
    ringport = getPort(host, portman)
    if ringport is None:
        return None
    
    # Connect to the ring master:
    
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, ringport))
    
    # Send the request and get the response as a string:
    
    s.sendall(bytearray('LIST\n', 'utf-8'))
    
    fd     = s.makefile()
    ok   = fd.readline()
    if ok != 'OK\n':
        raise RuntimeError(f'RingMaster gave an error response: {ok}')
    
    response = fd.readline()
    
    return response
    
    
    
    
    