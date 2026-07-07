'''
    readoutRestClient provides a Python client for the Readout Rest server.
    The client has functionality very similar to that of the Tcl rest client.

@file readoutRestClient.py
@brief Provide a ReST client for readouts runing the ReST server plugin.
@author Ron Fox.    
'''

import requests



from nscldaq.portmanager.PortManager import PortManager

class ReadoutClient:
    '''
      This is a ReST client of the Readout's ReST server control server.
      It is a stateless ReST client leting the server maintain the Readout 
      program's state.  In order to be agile across restarts of the Readout
      progra, which might have been assigned a new port name, each transaction
      -  Interacts with the Port manager in the server's host to translate the
         service name into a port and
      -  Connects with the ReST server to send a ReST requrest and recieve its
         response.
      This hand shaking may make this class not suitable for high frequency
      continuous requests but is probably good enough for normal use cases.
      
      Each method returns the JsON reponse decoded as a Dict.  All response dicts
      include a key 'status' whose value is 'OK' if the request succeeded and
      'error' if the request could not be performed (for example attempting to begin
      a run when the Readout already has an active run).  If the 'status' key's
      value is 'ERROR', ther wil be a 'message' field that will contain a 
      string that is a human readable error message.  
      
      Some request may set other keys in the dict. These are described for
      each of the methods for which this is the case.  If a method does not
      describe additional keys, you can assume the response will only be
      'status'.
    '''
    
    
    def __init__(self, host, serviceName, readoutUser):
        '''
          Constructor.  Note that this does not perform any
          network operations.   To be agile in the presenece of
          port number changes, each actual operation on the server
          translates the port and makes the connection needed to
          perform the http request of the server.
          
          @param host - host the readout ReST server is runnin gin.
          @param serviceName - the name of the service the ReST server advertises 
              with it's host port manager.
          @param readoutUser - the username under which the ReST server is running.
             this qualifies the service name allowing identical serviced names
             with different users.
        '''
        
        self.host = host
        self.serviceName = serviceName
        self.readoutUser = readoutUser
        
    #  Internal, resolve the port on which the service
    #  is being run in the host.
    #   Returns the host number or raises something.
    
    def _port(self) :
        pm = PortManager(self.host)
        info = pm.find(service=self.serviceName, user = self.readoutUser)    
        if len(info) == 1:
            return info[0]["port"]
        else:
            raise KeyError(
                f"Unable to unambiguously resolve {self.serviceName} in host {self.readoutUser}"
            )
    
    #  Internal function to construct a URI given the subdomain and request within that domain.
    #  returns the full URI string.
    #  Note that the suffix might be empty in some cases and that's just fine.
    def _constructUri(self, service, suffix):
        port = self._port()
        uri = f"http://{self.host}:{port}/{service}/{suffix}"
        return uri
    
    def _transition(self,  transition):
        port = self._port()
        response = requests.post(f'http://{self.host}:{port}/control', params={'operation': transition})
        return response.json()
    
    
    #
    #   Get a status item  sub is the subdomain with in /status.
    
    def _getStatusItem(self, sub):
        uri = self._constructUri("status", sub)
        r   = requests.get(uri)
        return r.json()
    
    # Set a parameter value
    
    def _setParameter(self, what, value):
        port = self._port()
        response = requests.post(f'http://{self.host}:{port}/setparam', {'name': what, 'value': value})
        return response.json()
        
        
    #----------------------- public methods ---------------------------------
    
    #    Run control
    
    def begin(self) -> dict:
        '''Request that a run begin'''
        
        return self._transition("BEGIN")
    
    def end(self) -> dict:
        ''' Request that a run end'''
        
        return self._transition("END")
    
    def init(self) -> dict:
        ''' Request an initialization set of actions '''
        return self._transition("INIT")
    
    def shutdown(self) -> dict:
        ''' request a shutdown of the server '''
        
        return self._transition("SHUTDOWN")
    
    #     Fetch state items:
    
    def getState(self) -> dict:
        ''' Return the run state text 
            On success, the returned dict will contain
            a key 'state' which containst the run state.
            This will be one of 'idle' - the run is inactive.
            'active' the run is active.  Note that the
            ReST server does _not_ support paused runs, however if another
            entitity that does pauses the run, the state will be
            'paused'  in that state the run can be ended.
        '''
        return self._getStatusItem("state")
        
    
    def getTitle(self) -> dict:
        ''' Return the current title: 
        
            On sucess, the dict returned will have a 'title' key
            which contains the text of the current title. Note that
            in state transition items, the title gets truncated, if necessary
            to 80 bytes that are guaranteed to be null terminated.  
            While e.g. UTF-8 title's are fine, there's no
            guarantee that this truncation won't occur in the middle
            of a multi-byte character (similarly for UNICODE).
        '''
        
        return  self._getStatusItem("title")
        

    def getRunNumber(self) -> dict:
        ''' return the current run number
             The 'run' key of the dict has the run number
             as an integer if successful.
        '''
        return self._getStatusItem("runnumber")
        

    def getStatistics(self) -> dict:
        '''
            Returns a statistics dict.  On success,
            The dict has two keys, 'cumulative' and 'perRun'
            The perRun statistics are reset at the beginning of a run the cumulative statistics
            are summed from when  readout started.
            
            Each statistics entry has the following keys:
            
            * triggers - the number of triggers.
            * acceptedTriggers - the number of accepted triggers.
            * bytes - the number of bytes worth of event body.
            
            
        '''
        return  self._getStatusItem("statistics")
        
            # Set parameters
            
    def setTitle(self, newTitle: str) -> dict:
        ''' set a new title string. 
            @param newTitle the new title string.  Note that in state transition
                ring items, the title is truncated to 80 bytes but is always
                null terminated
        '''
        return self._setParameter("title", newTitle)
    
    def setRunNumber(self, newRun: int) -> dict:
        ''' Set a new run number 
            @param newRun - the new run number requested.  This
                is rejected by the server if negative (0 is ok).
        '''
        return self._setParameter("run", newRun)
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

