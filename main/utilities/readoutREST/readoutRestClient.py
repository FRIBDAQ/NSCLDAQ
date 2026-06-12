'''
    readoutRestClient provides a Python client for the Readout Rest server.
    The client has functionality very similar to that of the Tcl rest client.
    
'''

import requests



from nscldaq.portmanager.PortManager import PortManager
import requests
class ReadoutClient:
    
    def __init__(self, host, serviceName, readoutUser):
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
        response = requests.post(f'http://{self.host}:{port}/setparam', {'name': 'run', 'value': value})
        return response.json()
        
        
    #----------------------- public methods ---------------------------------
    
    #    Run control
    
    def begin(self):
        '''Request that a run begin'''
        
        return self._transition("BEGIN")
    
    def end(self):
        ''' Request that a run end'''
        
        return self._transition("END")
    
    def init(self):
        ''' Request an initialization set of actions '''
        return self._transition("INIT")
    
    def shutdown(self):
        ''' request a shutdown of the server '''
        
        return self._transition("SHUTDOWN")
    
    #     Fetch state items:
    
    def getState(self) :
        ''' Return the run state text '''
        result = self._getStatusItem("state")
        return result['state']
    
    def getTitle(self):
        ''' Return the current title: '''
        
        result = self._getStatusItem("title")
        return result['title']

    def getRunNumber(self):
        ''' return the current run number'''
        result = self._getStatusItem("runnumber")
        return result['run']

    def getStatistics(self):
        '''
            Returns a statistics dict.  The dict has two keys, 'cumulative' and 'perRun'
            The perRun statistics are reset at the beginning of a run the cumulative statistics
            are summed from when  readout started.
            
            Each statistics entry has the following keys:
            
            * triggers - the number of triggers.
            * acceptedTriggers - the number of accepted triggers.
            * bytes - the number of bytes worth of event body.
            
            
        '''
        return  self._getStatusItem("statistics")
        
            # Set parameters
            
    def setTitle(self, newTitle):
        ''' set a new title string. '''
        return self._setParameter("title", newTitle)
    
    def setRunNumber(self, newRun):
        ''' Set a new run number '''
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

