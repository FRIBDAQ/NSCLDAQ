'''
    This module provides a class and methods for
    manipulating the key/value part of the manager database through
    ReST requests.
    
    @file mg_kvclient.py
    @brief Provide an ReSt interface to the Key value store in the manager database.
    @author Ron Fox.
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

import getpass   # os.getlogin fails in containers but getpass.getuser() works.
import requests


from nscldaq import mg_clientutils
class KvStore_ReST:
    '''
        Provide ReST interface access to the key/value store.
    '''
    
    def __init__(self, host: str, user:str = getpass.getuser(), service = 'DAQMANAGER') :
        '''
        Construct the client. Note that since this is a ReST client, no
        actual network operations are done until requests are made.
        
        @param host - host in which the manager is running.
        @param user - User running the manager (optional defaults to current user).
        @param service - Service name the manager advertises with the port manager.
                (optional defaults to 'DAQMANAGER').
        '''
        self._host = host
        self._user = user
        self._service = service
        
        
    def getValue(self, key : str) -> str:
        '''
            Returns the value of a key.
            @param key - the key to return.
            @return value - if the key is defined
            @exception any of a number of exceptions from mg_clientutils.makeUrl
            @exception IndexError if the key does not exist.
        
        '''
        uri = mg_clientutils.makeUrl(self._host, self._user, 'KVStore', 'value', self._service)
        response = requests.get(uri, {'name': key})
        json = response.json()
        self.__class__._checkError(json, IndexError)
        return json['value']
    
    def setValue(self, key : str, value : str) -> None:
        '''
          Set the value of a key in the KV dtaabase:
          
          @param key - name of the key to set.
          @param value - new value for the key.
          @exception IndexError - if the response returned an error.
          
        '''
        uri = mg_clientutils.makeUrl(self._host, self._user, 'KVStore', 'set', self._service)
        postdata = {'user': getpass.getuser(), 'name': key, 'value': value}
        response = requests.post(uri, postdata)
        self._class__._checkError(response.json(), IndexError)
    
    def listNames(self) -> list[str]:
        '''
            Returns a list of all of the keys in the KV store. Note you should not
            assume the listing is in any specific order.
            
            @return list(str).
            @exception RuntimeError if the request failed.
        '''
        uri = mg_clientutils.makeUrl(self._host, self._user, 'KVStore', 'listnames', self._service)
        response = requests.get(uri)               # No query params.
        json = response.json()
        self.__class__._checkError(json, RuntimeError)
        
    
    def list(self) -> dict:
        '''
            @return a dict of the name/values of the KV store.  The 
                dict is keyed by key names with values the values of 
                the associated keys.
                
        '''
        uri = mg_clientutils.makeUrl(self._host, self._user, 'KVSTORE', 'list')
        json = uri.get(uri).json()
        self.__class__._checkError(json, RuntimeError)
        
        return json['variables']
    
    #----------------------- private utility methods -----------------------------------------
    @classmethod
    def _checkError(json, etype):
        # check the response JsON for error and raise etype with the reason text if so.
        if json['status'] != 'OK':
            raise etype(json['message']) 