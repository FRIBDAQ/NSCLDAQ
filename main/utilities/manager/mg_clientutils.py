'''
   Provides some utility functions for python clients of the 
   manager ReST server.
   
   @brief mg_kvclient.py
   @brief Common  utility methods for managed experiment python clients.
   @author Ron Fox
'''



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

def getServerPort(host: str, user: str, service: str='DAQMANAGER') -> int:
    '''
        Return the port on which the manager is listening for
        ReST connections/requests.
        
        @param host - host in which the manager is running.
        @param user - User running the manager.
        @param service -  (optional defaults to 'DAQMANAGER')
                       The service that is advertised by the manager.
        @exeption IndexError - the service is not advertised.
        @exception OsException derived errors if the request could not be completed.
    '''
    manager = PortManager(host)
    service_list = manager.find({'service': service, 'user':user })
    if len(service_list) == 0:
        raise IndexError(f'The service "{service}" is not advertised by the manager')                            
    return service_list['port']
    

def makeUrl(
    host: str, user: str, domain: str, 
    subdomain: str = None, service:str ='DAQMANAGER'
) -> str:
    '''
       Construct the URL part of a request.  This is suitable for use
       with e.g. requests and a query parameter dict.
       
        @param host - host running the server.
        @param user - User running the server.
        @param domain - The request domain (this is the part after the / in the URI.)
        @param subdomain (optional defaults to None) - Some domains have sub domains
            e.g. the KVStore domain has subdomains that are commands to perform
            within that domain.  If supplied, the subdomain is
            appended to the URI constructed separated by a / and if not,
            nothing is appended.
        @param service (optional defaults to 'DAQMANAGER') the service advertised by
           the manager server's ReST server. 
    '''
    port = getServerPort(host, user, service)
    base_uri = f'http://{host}:{port}/{domain}'
    if subdomain is not None:                   ## append the subdomain if given.
        base_uri += '/' + subdomain
    return base_uri
    