'''
The RingMaster module provides access to the RingMaster LIST operation.
This operation allows clients to determine the set of ring buffers and
current usage statistics.
'''
from nscldaq.portmanager import  PortManager
import socket
from tkinter import Tcl



#----------------  private methods ----------------------------------------------
def _make_ring_dict(interp, info):
    # Given a tcl interpreter, and a ring information tcl list,
    # return a dict for the ring:
    
    info = f'"{info}"'
    print(info)
    ring_name = interp.eval(f'lindex {info} 0')
    ring_stats = interp.eval(f'lindex {info} 1')
    ring_stats = f'"{ring_stats}"'
    ring_size = (int)(interp.eval(f'lindex {ring_stats} 0'))
    free = int(interp.eval(f'lindex {ring_stats} 1'))    
    max_consumers = int(interp.eval(f'lindex {ring_stats} 2'))
    producer    = int(interp.eval(f'lindex {ring_stats} 3'))
    maxget      = int(interp.eval(f'lindex {ring_stats} 4'))
    minget      = int(interp.eval(f'lindex {ring_stats} 5'))
    consumer_list = interp.eval(f'lindex {ring_stats} 6')
    
    return {
        'name': ring_name, 'size' : ring_size, 'free': free,
        'maxconsumers': max_consumers, 'producer': producer,
        'maxget': maxget, 'minget': minget, 
        'consumers': consumer_list    
    }
def _create_list(response):
    # Given a response string, returns the array of dicts
    # described in the usage Return value.
    # We use tkinter's tcl interpreter mechanism to to do the decode.
    
    tcl = Tcl()                      # Instantiate an interpreter.
    tcl.eval(f'set response "{response}"')   # The Tcl variable response has the full respones
    nrings = (int)(tcl.eval("llength $response"))
      
    result = []
    #  Process each list item:
    
    for i in range(0, nrings):
        ring_info = tcl.eval(f'lindex $response {i}')
        print(ring_info)
        result.append(_make_ring_dict(tcl, ring_info))
    return result  


#--------------------------- public methods -------------------------------------
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
    result = _create_list(response)
    
    return result
    
    
    
    