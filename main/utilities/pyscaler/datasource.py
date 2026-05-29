'''
  This module provides classes that allow ring items to be
  read from an FRIB/NSCLDAQ data source.  Once created, a
  datasource looks like a file.  There are three public classes
  
  FileDataSource - reads data from a file.
  OnlineDataSource - reads data from a pipe attached to ringselector.
  DataSourceFactory -  Given a URI for a data source creates the
            appropriate data source object.

The data source classes are all iterable in the sense that given a data source
s

you can do:

for ring_item in iter(s):
   # do something with the next ring item from the source.
   

@file datasource.py
@brief Provide FRIB/NSCLDAQ data sources
@author Ron Fox
@note Using this the module path must include the unified format's pytyhon
subdirectory as the daqformat module is imported from there.
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

import subprocess
import select
import urllib.parse as urlparse
from os import environ
import struct
import daqformat
import typing

LONGWORD_SIZE=4
    

#
# Provides data and methods common to all
# data sources. 
class _DataSourceBase:
    _itemfactory : daqformat.ringitemfactory
    _version     : int
    #
    #  Construct the 'tentative' factory.
    #  Note that as we get ring items in,
    #  we may modify our idea of the actual
    # Factory.
    #  The caller can set a tentative factory or we
    #  will default to 12 if not.
    #  Note that format 10 must be explicitly selected.
    def __init__(self, version:int =12):
        self._version = version
        self._itemfactory = daqformat.ringitemfactory(version)
    #
    # Given a byte array that's supposed to hold
    # a ring item, 
    #  1. Alter our factory if the item is a version.
    #  2. Return the actual ring item type - which is a polymorphic
    #     type derived from daqformat.ringitem
    #
    def makeItem(self, ba: daqformat.ringitem) -> daqformat.ringitem:
        base_item = self._itemfactory.makeRingItem(ba)
        type = base_item.type()
        
        # Note that the factory throws an exception if
        # the item type says it's of the wrong format so:
        
        try:
            if type == daqformat.RING_FORMAT:
                item = self._itemfactory.makeDataFormatItem(base_item)
                self._itemfactory = daqformat.ringitemfactory(item.getMajor())
                return item
        except RuntimeError:
            if self._version == 11:
                self._version = 12
            else:
                self._version = 11
            self._itemfactory = daqformat.ringitemfactory(self._version)
            item = self._itemfactory.makeDataFormatItem(base_item)
            return item
    
        # Now we have the right format, match on the type to 
        # Create the actual ring item:
        
        match type:
            case daqformat.ABNORMAL_ENDRUN:
                return self._itemfactory.makeAbnormalEndItem(base_item)
            case daqformat.BEGIN_RUN | daqformat.END_RUN | daqformat.PAUSE_RUN | daqformat.RESUME_RUN:
                return self._itemfactory.makeStateChangeItem(base_item)
            case daqformat.EVB_FRAGMENT:
                return self._itemfactory.makeRingFragmentItem(base_item)
            case daqformat.EVB_GLOM_INFO:
                return self._itemfactory.makeGlomParameters(base_item)
            case daqformat.EVB_UNKNOWN_PAYLOAD:
                return base_item
            case daqformat.INCREMENTAL_SCALERS | daqformat.PERIODIC_SCALERS | daqformat.TIMESTAMPED_NONINCR_SCALERS:
                return self._itemfactory.makeScalerItem(base_item)
            case daqformat.MONITORED_VARIABLES | daqformat.PACKET_TYPES:
                return self._itemfactory.makeTextItem(base_item)
            case daqformat.PHYSICS_EVENT:
                return self._itemfactory.makePhysicsEventItem(base_item)
            case daqformat.PHYSICS_EVENT_COUNT:
                return self._itemfactory.makePhysicsEventCountItem(base_item)
            
        # If we fell through the match, the item is not a recognized type
        # and our library needs to be exteneded rather than us just
        # Passing back the raw item and hoping the client
        # to do with it.
        
        raise ValueError(
            f"makeItem got an item type {type}  which we don't (yet) know how to handle"
        )
        
        
    #
    # This is an internal utility method that
    # reads  a ring item and returns it in a byte array
    # from a file like object passed in.
    # The only thing it knows is that the first
    # 32 bits of a ring item is its total size in
    # bytes.
    #  Note - we assume the data are little endian
    # as all FRIB/NSCLDAQ data have been so far.
    #
    #  If the end of file is reached reading the
    #  size field, None is returned.
    #  If the end of file is readched in the middle
    #  of the ringitem, ValueError is raised as it would
    #  be for attempting to read from a closed file object.
    #
    @staticmethod
    def _read_item(source: typing.BinaryIO) -> bytearray:
        item = source.read(LONGWORD_SIZE)
        if len(item) == 0:
            return None                # end of file.
        if len(item) < LONGWORD_SIZE:
            raise ValueError('There is not even a size left in the input')
        
        # Read in the remaining ring item data:
        
        (full_length,) = struct.unpack('<i', item) 
        remaining_length = full_length- LONGWORD_SIZE
        item += source.read(remaining_length)
        if len(item) != full_length:
            raise ValueError('End of file found in the middle of reading the ringitem.')
        
        return item
    
            
            
class FileDataSource(_DataSourceBase):
    _skip_set : set
    _source   : typing.BinaryIO
    '''
        Data source hooked to a path in the filesystem.
    '''
    def __init__(self, path: str, format: int=12, skip: set={}, sample: set={}):
        '''
            Create the data source:
            
            @param path   - The filesystem path that is to be read.
            @param format - Optional ring item format to use, defaults to 12.
            @param skip   - Optional set of types to skip.  By default no types are
                            skipped.  The skipped set of types are not
                            returned to the caller.
            @param sample - For file data sources, no types are sampled.
            @note the special path '-' is assumed to be stdin.
        '''
        super().__init__(format)
        self._skip_set = skip
        if path == '-':
            file = 0
        else:
            file = path
        self._source   = open(file, "rb", buffering=0)  
      
    def __del__(self):
        # The try block is in case we get called after a filed construction?
        try:
            self._source.close()
        except Exception:
            pass
    
    def check(self) -> bool:
        '''
            Lets the program know if data is ready.  This is always
            true for files.
        '''
        return True
    def next(self) -> daqformat.ringitem:
        '''
            Return the next ring item or None.  You might well ask why publicize
            this when we're also going to implement the iterator protocol:
            The answer is to support programs that may have more than one
            data source.  For those data sources, we will need
            to multiplex amongst them.  
            
            @return - the actual ring item object (e.g. a scaler item)
            @retval None - if we read the last one  - in that case
                           the data source is closed so the next call will
                           raise ValueError.
        '''
        # The while loop allows us to implement the skip set.
        while True:
            item = self._read_item(self._source)
            
            if item is None:
                self._source.close()
                return None
            ring_item = self.makeItem(item)
            if ring_item.type() not in self._skip_set:
                return ring_item
     
    # Implement the iterator protocol:
    
    def __iter__(self):
        return self

    def __next__(self) :
        result = self.next()
        if result is None:
            raise StopIteration
        else:
            return result
        
    
class OnlineDataSource(_DataSourceBase):
    _source : subprocess.Popen
    '''
        This uses $DAQBIN/ringselector to 
        provide an online data source.  In this case, skipping and sampling
        are all handed by ringfragment source.  
        
    '''
    def __init__(self, uri: str, format: int=12, skip: set={}, sample: set={}):
        super().__init__(format)
        
        #  For this all to work, we need for DAQBIN to
        # be in the environment:
        
        if 'DAQBIN' not in environ.keys():
            raise LookupError('The DAQBIN environment variable must be defined and is not')
        
        #
        #  Construct the command and its arguments.
        
        args = [
            f'{environ["DAQBIN"]}/ringselector',
            f'--source={uri}',
            '--non-blocking',
        ]
        # Only add in the --sample and --exclude items if they are not empty:
        
        if len(skip) > 0 :
            args.append(f'--exclude={",".join(str(x) for x in skip)}')
    
        if len(sample) > 0 :
            args.append(f'--ample={",".join(str(x) for x in sample)}')
            
        # I think this will vector stderr to output.
        self._source = subprocess.Popen(
            args, stdout=subprocess.PIPE, stdin=subprocess.DEVNULL, stderr=2
        )
    
    def __del__(self):
        '''
            If possible kill and reap the subprocess:
            1. close its stdout (should sigpipe it).
            2. send SIGKILL to the process.
            3. wait _without_ timeout for the process to exit.
        '''
        try:
            self._source.stdout.close()
        except Exception:
            pass
        try:
            self._source.kill()
        except Exception:
            pass

        try :
            self._source.wait()
        except Exception:
            pass
        
    def check(self) -> bool:
        ''' 
            See if input is available on the process' pipe.  This is done
            by polling select.select.
        '''
        (readable, writeble, exceptions) = select.select([self._source.stdout,], [], [], 0)
        return len(readable) > 0
    
    def next(self) -> daqformat.ringitem:
        '''
            Return the next ring item from the data source, or None if the
            ringselector exited.  Note that this blocks until a full ring item
            has been read.
            
            @return actual final ring item type.
            @retval None - no more items on the source
        '''
        item =  self._read_item(self._source.stdout) 
        if item is None:
            return None
        else:
            return self.makeItem(item)
    
    #  Implement the iterator protocol
    def __iter__(self):
        return self
    def __next__(self):
        result = self.next()
        if result is None:
            raise StopIteration
        else:
            return result
        
        


class DataSourceFactory:
    '''
        Factory for creating data sources.
    '''
    
    @staticmethod
    def makeSource(uri: str, format: int=12, skip: set={}, sample: set={}) -> OnlineDataSource | FileDataSource:
        '''
            Create a new data source.
            
            @param uri - the URI that specifies the data source.
            @param version (optional) - NSCLDAQ version the data is believed to be
                         defaults to 12.
            @param skip - (optional) set of ring item data types to skip.  This is a set of 
                ring item types from the constants defined in daqformat.  defaults to empty.
            @param sample - (optional) set of ring item data types that can be sampled.
                this is a set of ring item types from the constants defined in daqaformat.
                defaults to empy.
                `
            URI's of the form tcp://host/ringname - make an online data source.
            URI's of the form file:///path  generate file data sources.
            URI's that are just plain filenames are file data sources.
            
            @returns an object that implmenets the data source protocol.
           
        '''
        parsed_uri = urlparse.urlsplit(uri, scheme='file')
        if parsed_uri.scheme == 'tcp':
            return OnlineDataSource(uri, format, skip, sample)
        elif parsed_uri.scheme == 'file':
            return FileDataSource(parsed_uri.path, format, skip, sample)
        else:
            raise ValueError('Only "file" and "tcp" URIs are supported')
        
        