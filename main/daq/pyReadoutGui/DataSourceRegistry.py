#!/usr/bin/env python3 

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
@file DataSourceRegistry.py
@brief Registry of data source _types_ and their creators.
@author Ron Fox

The pyReadoutGui needs to have a registry of the types of data sources 
it can use, how to create them and how to display them.  This provides that
registry.  The data source modules that are baked into the GUi.  Additional,
external modules can be registered with this registry to extend the set of
data sources supported by the GUI.

This can be done in two ways:
- Automatically at startup through a pyReadoutGuiInit.py script located in the
  directory in which the pyReadoutGui program is run.
- Manually by the File->Load... operation specifying a file that will
  interact with this registry to load additional specfications.
  
This module provides a singleton class.  Modules implementing
data sources will need lines of code like:

from nscldaq.readoutgui.DataSourceRegistry import DataSourceRegistry
from nscldaq.readougui.DataSource          import DataSource
# Other imports omitted for clarity be sure to import
# qt stuff from PyQt6 *not* PyQt5.

class MyDataSource(DataSource):
    .... # Implement the data source here....
    def __init__(self, parameters : dict[str, object], **kwargs):
        super().__init__(parameters, **kwargs)
        ... # Implement your data source
    ...

class MyDataSourceConfigurationDisplay(QWidget):   
    
    def __init__(self, source : MyDataSource, parent : QWidget | None = None):
        ....#Implement a widget to display the configuration of the data source
    
class MyDataSourceMaker(QWidget):
    def __init__(self, source : MyDataSource | None = None, parent : QWidget | None = None):
        ... #Implement widget to prompt for the data source configuration.
        ... # If a sourcde is provided, it should pre-stock the widget.
    def makeSource(self) -> MyDataSource | str:
        ... #Implement code to pull the data from your widget and create a data source
            # or return a string that says why you could not.
            

DataSourceRegistry.instance().add(
    'My Data Source', MyDataSource, MyDataSourceConfigurationDisplay, MyDataSourceMaker
)

The last line associates a name with your data source, it's displayer, the maker so that
the GUI can instantiate those classes as needed in whatever surrounding GUI context
required.  For example, the data source name will be in the submenu for Data Source ->Add ...
as well as in other contexts used to identify the data source (e.g. the listing of data sources).
'''


import traceback
from typing import Self, ClassVar



#  Exception classes:

class DataSourceRegistryException(Exception):
    '''
    Base class for our exception hierarchy:
    '''
    def __init__(self, msg : str):
        super().__init__(msg)

class DuplicateName(DataSourceRegistryException):
    '''
    Attempt to register two source types with the same name
    '''
    def __init__(self, name : str):
        super().__init__(f'{name} has already been registered')
        
class NoSuchName(DataSourceRegistryException):
    '''
        Attempted to fetch a data source name that has not been registered.
        
    '''
    def __init__(self, name : str):
        super().__init__(f'No data source type named {name} has been registered.')
    
class SingletonViolation(DataSourceRegistryException):
    ''''
    Someone from outside the module has attempted to instantiate the registry.
    
    '''    
    def __init__(self, filename : str):
        super().__init__(f'Attempted to instantiate the data source registry singleton from {filename} not allowed')
    

#  The registry.

class DataSourceRegistry:
    _instance : ClassVar[Self | None] = None

    def __init__(self) -> None:
        '''
        Constructor.  Note that if a client attempts to run this,
        a SingletonViolation is raised.  Clients should use the instance method to get
        a reference to the singleton instance
        '''
        
        caller = traceback.extract_stack()[-2]    # Our caller.
        if caller.filename != __file__:
            raise SingletonViolation(caller.filename)
                          
        self._registry : dict[str, (type, type, type)]= {}
    
    @classmethod
    def instance(cls) -> Self:
        '''
        @return DataSourceRegistry - the singleton instance of the registry.
        '''
        if not cls._instance:
            cls._instance = cls()
        return cls._instance
    
    #  Public object methods:
    
    def add(self, typename : str, sourceClass : type, sourceDisplayClass : type, sourceCreatorClass : type) -> None:
        '''
            Register a new data source type.
            @param typename - the uniqe name the data source type will be known by.
            @param sourceClass - The data source class.
            @param sourceDisplayClass - the class that displays the properties of a source of that type.
            @param sourceCreatorClass  - The prompter/creator of sources of this type.
            @throws DuplicateName - if that typename already exists.
        '''
        if typename in self._registry:
            raise DuplicateName(typename)

        self._registry[typename] = (sourceClass, sourceDisplayClass, sourceCreatorClass)

    def registerDataSourceType(self, typename : str, sourceClass : type,
                              sourceDisplayClass : type, sourceCreatorClass : type) -> None:
        '''
        Compatibility alias for add().
        '''
        self.add(typename, sourceClass, sourceDisplayClass, sourceCreatorClass)
    
    def listSourceTypes(self) -> list[str]:
        '''
        @return list[str] - list of source type names.
        '''
        return list(self._registry.keys())

    def sourceClass(self, typename : str) -> type:
        '''
        This s used when reconstituting a data source given its configuration 
        (e.g. you're restarting from the saved configuration).
        
        @param typename : str - name of a data source type.
        @return  type   : the class of the data source. 
        @throws NoSuchName - if the typename represents a data source you've not  registered.
        
        '''
        if typename not in self._registry:
            raise NoSuchName(typename)
        return self._registry[typename][0]

    def sourceCreator(self, typename : str) -> type:
        '''
        @param typename : str - Name of the source type whose creator you want.
        @return type:  The creator class - you must instantiate it in the context you want (e.g. a dialog or wizard).
        @throws NoSuchName - if that typename doe nost exist.
        '''
        if typename not in self._registry:
            raise NoSuchName(typename)
        return self._registry[typename][2]
        
    def sourceDisplayer(self, typename : str) -> type:
        '''
        @param typename : str - name of a data source type whose displayer class you want.
        @return type : The displayer class.
        @throws NoSuchName - if that typename does not exist.
        '''
        if typename not in self._registry:
            raise NoSuchName(typename)
        return self._registry[typename][1]

    @classmethod
    def _reset(cls) -> None:
        # Provided for testing:
        #  just destroys the instance:
        cls._instance = None

    

        
# Unit tests  These get run if the module is main.
# Note that our creators/listers/datasources are really
# silly and would never work.

if __name__ == '__main__':
    import unittest
    
    class TestDataSourceRegistry(unittest.TestCase):
        # Written mostly by Copilot  
        def setUp(self):
            DataSourceRegistry._reset()

        def tearDown(self):
            DataSourceRegistry._reset()

        def test_instance_is_singleton(self):
            first = DataSourceRegistry.instance()
            second = DataSourceRegistry.instance()

            self.assertIs(first, second)

        def test_register_and_list_types(self):
            class Source:
                pass

            class Display:
                pass

            class Creator:
                pass

            registry = DataSourceRegistry.instance()
            registry.add("Alpha", Source, Display, Creator)

            self.assertEqual(registry.listSourceTypes(), ["Alpha"])
            self.assertIs(registry.sourceClass("Alpha"), Source)
            self.assertIs(registry.sourceDisplayer("Alpha"), Display)
            self.assertIs(registry.sourceCreator("Alpha"), Creator)

       
        def test_duplicate_name_raises(self):
            class Source:
                pass

            class Display:
                pass

            class Creator:
                pass

            registry = DataSourceRegistry.instance()
            registry.add("Gamma", Source, Display, Creator)

            with self.assertRaises(DuplicateName):
                registry.add("Gamma", Source, Display, Creator)

        def test_missing_name_raises(self):
            registry = DataSourceRegistry.instance()

            for getter in (registry.sourceClass, registry.sourceDisplayer, registry.sourceCreator):
                with self.assertRaises(NoSuchName):
                    getter("DoesNotExist")

        

    unittest.main()





        
        



