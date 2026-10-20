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
@file DataSourceCofig.py
@brief Save/restore the set of data sources that are defined.
@author Ron Fox

Auto registers a Serializer with the SerializationManager to 
serialize and deserialize the set of data sources that are
defined.   

@note deserialization requires that any data sources that were
added when this ReadoutGui was run also have been registered
at startup via the user's initialization script.  The intial
deserialization of the configuration happens _after_ that script
is run so all is well.
'''
import io
import types

from nscldaq.readoutgui.StateSaveRestore import Serializer, SerializationManager
from nscldaq.readoutgui.DataSourceRegistry import DataSourceRegistry
from nscldaq.readoutgui.DataSourceManager import DataSourceManager

class DataSourceConfig(Serializer):
    def serialize(self, destination : io.TextIOBase) -> None:
        '''
        We are going to create a list of dicts, one dict
        for each data source that is known to the manager.  Each dict
        has the following keys:
        name - the name of the source
        config - The configuration of the source (a nested dit).
        
        @note the names of sources have the form type:uuid where
        type is the type of the source. Therefore, we can derive
        the source type on deserialization just from the name.
        Since the UUID is a UUID, we don't have to worry about
        name collisions on desrialization with any subsequently 
        added sources.
        
        
        @param destination : io.TextIOBase - where to write our configuratino
        variable which will be called, unoriginally enough, dataSources
        '''
        sources = DataSourceManager.instance().sources()
        
        configuration = []     #  We'll build the configuration here.
        for name, source in sources.items():
            aConfig = source.getConfig()
            configuration.append({
                'name' : name,
                'config' : aConfig
            })
        
        #  Ready to write:
        
        destination.write(f'dataSources = {configuration}')
        

    def deserialize(self, mod : types.ModuleType) -> None:
        '''
        For each item in the configuration list, make a new
        data source,  We'll try to do a bit of clever 
        try/except to give some good error messages:
        '''
        registry = DataSourceRegistry.instance()
        manager  = DataSourceManager.instance()
        defined_types = registry.listSourceTypes()
        try:
            sources = mod.dataSources
        except AttributeError as a:
            print(f'Unable to get the data source configuration: {a}')
            return
        
        for source in sources:
            full_name  = source['name']
            aconfig = source['config']
            source_type = full_name.split(':')[0]
            if source_type in defined_types:
                sourceObject = registry.sourceClass(source_type)(aconfig)
                manager.addSource(full_name, sourceObject)
            else:
                print(
                    f'''Data source type {source_type} has not been registered and will not be restored - 
be sure readoutguiinit.py imports and registers it''')
                
        
        


SerializationManager.instance().addSerializer(DataSourceConfig())



