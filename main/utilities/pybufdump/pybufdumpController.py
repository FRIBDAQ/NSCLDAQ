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
@file pybufdumpControler.py
@brief contains the program logic that connects the UI to the data.
@author Ron Fox
'''

import struct
from datetime import datetime
import sys
import daqformat
from nscldaq.pydumper import pyUI
import tabulate
import tomllib
from nscldaq.pyscaler.datasource import FileDataSource
from PyQt6.QtCore import QObject, Qt, pyqtSignal
from PyQt6.QtWidgets import QApplication, QMessageBox
import importlib.util        # For plugin loading.
import uuid
import types

def _generate_plugin_name() -> str:
    # Uses uuid4 to generate a random, unique plugin\
    # module name for importlib loads:
    
    suffix  = str(uuid.uuid4())
    module  = 'plugin' + suffix
    return module

def _load_plugin(path : str) -> types.ModuleType:
    # Loads a plugin and returns the module object.
    # path - is the path to the plugin file.
    # returns the module that was loaded.
    
    module_name = _generate_plugin_name()
    spec = importlib.util.spec_from_file_location(module_name, path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    spec.loader.exec_module(module)    # Runs outer code if there is some.

    return module

class Fragment:
    '''
    This class represents an event builder fragment.
    '''
    def __init__(self, data : bytearray):
        self._data = data
    
    def timestamp(self) -> int:
        '''
        @return int (unsigned int64) the fragment timestamp.
        
        '''
        # The and below makes it appear unsigned.
        
        return int.from_bytes(self._data[0:8], byteorder='little') & 0xffffffffffffffff
    def sourceId(self) -> int:
        '''
        @return int (unsigned int32) the fragment source id.
        '''
        
        return int.from_bytes(self._data[8:12], byteorder='little') & 0xffffffff
    
    def payloadSize(self) -> int:
        '''
        @return int (unsigned int32) the size of the fragment payload.
        '''
        return int.from_bytes(self._data[12:16], byteorder='little') & 0xffffffff
    def barrierId(self) -> int:
        '''
        @return int (unsigned int32) - The barrier id of the fragment.
        
        '''
        return int.from_bytes(self._data[16:20], byteorder='little') & 0xffffffff
    
    def payload(self) -> bytearray:
        '''
        @return bytearray - the payload sliced from the fragment
        '''
        return self._data[20:]
        
        
    
    

class BuiltEventIterator:
    '''
        This class (might eventually want to go elsewhere like ufmt)
        provides a mechanism to iterate through the fragments in an
        event that came out of an event builder.
    '''
    header_size = 20         # Bytes in the fragment headers.
    
    def __init__(self, event : bytearray):
        self._data = event
        # Self inclusive fragmet size.
        self._size = int.from_bytes(self._data[0:4], byteorder='little') &  0xffffffff 
        self._offset = 4   # First fragment offset.
        
        
    def __iter__(self):
        return self

    def __next__(self) -> Fragment:
        if self._offset < self._size:
            offset = self._offset
            size_offset = offset + 12    # Where the payload size is for the fragment.
            payload_size = int.from_bytes(self._data[size_offset:size_offset+4], byteorder='little') & 0xffffffff
            total_size   = payload_size + self.header_size
            self._offset += total_size         # offset to next fragment.
            return Fragment(self._data[offset:self._offset])
        else:
            raise StopIteration
        
    

class BufDumpController(QObject):
    '''
        The controller has public entry points for each of the
        signals the UI can emit.  In addition it emits:
        
        endFile - on end of file.
    '''
    endFile = pyqtSignal()
    
    def __init__(self, view : pyUI.MainWindow, parent : QObject | None = None):
        '''
        @param view : pyUI.MainWindow - the UI widget.
        @param parent : QObject | None - the parent object, if there is one.
        '''
        
        super().__init__(parent)
        
        
        # Initialize member data.
        
        self._view          = view
        self._eventfile     = None
        self._eventfileName = None
        self._plugins       = {}
        self._filter       = None
        self._version       = 12
        self._eventbuilt    = False
        self._statusText    = ''
        self._scaler_file   = None
        self._scaler_map  = None
        self._sid_file      = None
        self._sid_map       = {}      # source id - > name map

        # Hook in to the signals the view will emit:
        #
        self._view.open.connect(self._openEventFile)
        self._view.plugin.connect(self._loadPlugin)
        self._view.exit.connect(self._cleanup)
        self._view.filter.connect(self._setFilter)
        self._view.clearfilter.connect(self._clearFilter)
        self._view.next.connect(self._nextItem)
    
    # public methods:
    
    def registerFormatter(self, srcid : int, formatter : object) -> None:
        '''
            Register a formatter for a specific source id.  Any
            prior formatter registered for that source id
            is removed.
            
            @param srcid - fragments from this source id will be
                           formatted by this formatter.
            @param formatter - the object whose format method will be
                           called to format that srcid.
                
        '''
        self._plugins[srcid] = formatter
        
    # attributes:
    def formatVersion(self) -> int:
        '''@return int - ring item format version.'''
        return self._version
    def setFormatVersion(self, vsn : int) -> None:
        '''
        @param vsn : int - DAQ format version to apply to the next
              input event file.  
        @note setting this does not change the version of the current file
              data source.
        '''
        self._version = vsn
    
    def eventbuild(self) -> bool:
        '''@return bool - true if event build data should be unpacked at fragment level.'''
        return self._eventbuilt
    def setEventBuilt(self, built : bool ) -> None:
        '''
        @param built : bool If true, the formatting is done assuming tyhe data are
                    event built and fragments are broken out for events.
        '''
        self._eventbuilt = built
        
    def scalerFile(self) -> str | None:
        ''' @return str | None - path to scaler toml file or None if there isn't one'''
        return self.scaler_file
    def setScalerFile(self, path : str) -> None:
        ''' @param path : str - Path to a scaler toml file from which the scaler
                        name  map is built
        '''
        # Note the map format is keyed by source id  
        # and each map then contains a list of channel names.
        # If defined the dump of scalers will use those names rather than channel
        # number to label the scalers.
        self._scaler_file = path
        with open(self._scaler_file, "rb") as f:
            raw_toml = tomllib.load(f)
        
        self._scaler_map = self._makeScalerMap(raw_toml)
        
    def sidFile(self) -> str | None:
        ''' @return str | None - the name of the source id definition file if present. '''
        return self._sid_file
    def setSidFile(self, path : str) -> None:
        ''' 
            Update the source id map from a new source id file.
            @param path - name of a new sourcde id definition file.  
        '''
        self._sid_file = path
        self._sid_map  = self._makeSidMap()
        
        
    # Utilities for interacting with the view:
    
    def _setStatusBar(self, text : str) -> None:
        # Put text in the status bar:
        sb = self._view.statusBar()
        sb.showMessage(text)
        self._statusText = text
    def _refreshStatusBar(self) -> None:
        self._setStatusBar(self._statusText)
    def _clearStatusBar(self) -> None:
        sb = self._view.statusBar()
        sb.clearMessage()
    
    
        
            
    # Slots that are internal (private) to the controller:
    
    def _openEventFile(self, path : str) -> None:
        #  Open a new event file:
        
        try:
            self._eventfile = FileDataSource(path, self._version, set(), set())
            self._eventfileName = path
            self._setStatusBar(f'Reading data from {path}')
            self._view.dumpWidget().setText('')     # Clear any done message.
        except Exception as e:
            QMessageBox.warning(
                None, 'Failed Event Source',
                f'Failed to create an event source for {path}, {e}'
            )

     
    def _loadPlugin(self, pluginPath : str) -> None:
        self._refreshStatusBar()
        
        # Load the plugin and call its registerFormatters function
        #
        
        
        plugin = _load_plugin(pluginPath)   
        plugin.registerFormatters(self)
        
     
    def _cleanup(self) -> None:
        # Cleanup before exiting:
        # If there is an event file, close it:
        
        if self._eventfile:
            self._eventfile.close()
            self._eventfile = None
            self._eventfileName = None

    def _setFilter(self, filter : list) -> None:
        self._refreshStatusBar()
        # set the list of acceptable ring item types:
        
        self._filter = filter
        
    def _clearFilter(self) -> None:
        self._refreshStatusBar()
        # Clear any ring item type filter:
        
        self._filter = None
    
    
    def _nextItem(self) -> None:
        if self._eventfile:    # Nothing if no event file.
            # Filters can cause a long delay t othe next item so
            # set an wait cursor.
            
            QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)
            while True:
                item = self._eventfile.next()
                if not item:
                    # End of file:
                    self.endFile.emit()
                    self._eventfile.close()
                    self._eventfile = None
                    self._eventfilename = None
                    self._clearStatusBar()
                    self._view.dumpWidget().setHtml('<h1>No more items</h1>')
                    break
                else:
                    # Skip the item?
                    
                    if self._filter and item.type() not in self._filter:
                        continue
                    else:
                        text = self._format(item)
                        self._view.dumpWidget().setText(text)
                        self._refreshStatusBar()
                        break
        QApplication.restoreOverrideCursor()
    #  Formatting methods:  In general, these take a ring itemand 
    # return a string that is stuffed into the dumper widget.

    
    def _format(self, item : daqformat.ringitem) -> str:
        
        match item.type():
            case daqformat.ABNORMAL_ENDRUN:
                return self._formatabend(item)
            case daqformat.BEGIN_RUN | daqformat.END_RUN | daqformat.PAUSE_RUN | daqformat.RESUME_RUN:
                return self._formatStateChange(item)
            case daqformat.RING_FORMAT:
                return self._formatRingVersion(item)
            case daqformat.EVB_FRAGMENT:
                return self._formatEvbFragment(item)
            case daqformat.EVB_GLOM_INFO:
                return self._formatGlomParameters(item)
            case daqformat.EVB_UNKNOWN_PAYLOAD:
                return self._formatUnknown(item)
            case daqformat.INCREMENTAL_SCALERS | daqformat.PERIODIC_SCALERS | daqformat.TIMESTAMPED_NONINCR_SCALERS:
                return self._formatScalerItem(item)
            case daqformat.MONITORED_VARIABLES | daqformat.PACKET_TYPES:
                return self._formatTextItem(item)
            case daqformat.PHYSICS_EVENT:
                return self._formatPhysicsEvent(item)
            case daqformat.PHYSICS_EVENT_COUNT:
                return self._formatEventCount(item)
            case _:
                return self._formatUnknown(item)

    def _formatByteArray(self, data : bytearray) -> str:
        # Format a byte array like the body of an event or
        # the body of a fragment item.
        
        nBytes = len(data)
        result = f'{nBytes} Bytes:\n'
        
        # We run hex on 16 byte slices with a space separation every two bytes.
        
        
        
        start = 0
        for (word,) in struct.iter_unpack('<H', data):
            result += f'{word:04x} '
            start +=1
            if start %8 == 0:
                result += '\n'
            
        if nBytes % 16:
            result += '\n'
        
        return result
    
    
    def _timestring(self, stamp : int) -> str:
        '''
        Convert a unix timestamp in to a time string in the current zone.
        '''

        timestamp = datetime.fromtimestamp(  # noqa: DTZ006
            stamp, tz=None
        )                                                  # in local time.
        return  timestamp.strftime('%c')
    def _formatBodyHeader(self, item : daqformat.ringitem) -> str:
        # If the item has a body header, return its formatted equivalent:
        
        result = ''
        ts     = item.timestamp()
        if ts is not None:
            # Have a body header:
            # Force timestamp to unsigned 64 bits:
            ts &= 0xfffffffffffffff
            result += 'Body header:\n'
            result += f'  Timestamp   : 0x{ts:016x}\n'
            result += f'  Source Id   : {self._makeSidString(item.sourceid())}\n'
            result += f'  Barrier Type: {item.barriertype()}\n\n'
        return result
    
        
    def _formatabend(self, item : daqformat.abnormalenditem) -> str:
        # Format an abnormal end item.
        
        result = 'Abnormal end item\n'
        result += self._formatBodyHeader(item)
        return result
    
    def _formatStateChange(self, item : daqformat.statechangeitem) -> str:
        state_change_names  = {
            daqformat.BEGIN_RUN : 'Begin Run', daqformat.END_RUN : 'End Run',
            daqformat.PAUSE_RUN : 'Pause Run', daqformat.RESUME_RUN : 'Resume Run'
        }
        result = f'{state_change_names[item.type()]} \n'
        result += self._formatBodyHeader(item)
        result += f'For run {item.getRunNumber()}, {item.getElapsedTime():.2f} into the run, at {self._timestring(item.getTime())}\n'
        result += f'Title: {item.getTitle()}\n'
        if self._version >= 12:
            result += f'From original source id: {self._makeSidString(item.originalSource())}\n\n'     
        return result
    
    def _formatRingVersion(self, item: daqformat.ringformatitem) -> str:
        result = 'Ring Format item: \n'
        result += f'  FRIB/NSCLDAQ version: {item.getMajor()}\n\n'
        return result
    
    def _formatEvbFragment(self, item : daqformat.ringfragmentitem) -> str:
        result = 'Ring Fragment Item\n'
        result += self._formatBodyHeader(item)
        fts = item.timestamp()
        fts &= 0xffffffffffffffff     # Makes it unsigned 64 bit.
        result += f'Fragment Timestamp : {fts:016x}\n'
        result += f'Fragment Source id : {self._makeSidString(item.source())}\n'
        result += f'Fragment Barrier id: {item.barrierType()}\n'
        result += 'Payload:\n'
        payload = item.payload()     # Byte array of the body.
        # @todo - this could be submitted to plugins for formatting.
        
        result += self._formatByteArray(payload)
        result += '\n'
        return result
    
    def _formatGlomParameters(self, item: daqformat.glomparameters) -> str:
        result = 'Event builder Glom parameters:\n'
        result += self._formatBodyHeader(item)
        result += f'Coincidence Ticks: {item.coincidenceTicks()}\n'
        building = 'Building' if item.isBuilding() else 'Not Building'
        result += f'Glom is          :  {building}\n'
        result += f'Timestamp policy :  {item.policy()}\n\n'
        return result
    
    def _formatUnknown(self, item : daqformat.ringitem) -> str:
        # Unknonwn payload.. just provide the body after the 'normal' stuff.
        
        result = 'Unknown payload/item type:\n'
        result += self._formatBodyHeader(item)
        payload = item.body()
        result += self._formatByteAarray(payload)
        result += '\n'
        return result
    
    def _formatScalerItem(self, item :daqformat.scaleritem) -> str:
        result = 'Periodic Scaler: \n'
        result += self._formatBodyHeader(item)
        result += f'Readout at {self._timestring(item.absoluteTime())}'
        result += f'accumleted from {item.startTime():02f} to {item.endTime():02f} seconds into the run.\n'
        if self._version >= 12:
            result += f'Original Source Id: {self._makeSidString(item.getOriginalSourceId())}\n'
        incr = 'Incremental'  if item.isIncremental() else 'Not Incremental'
        result += f'Readout is {incr}\n'
        result += 'Counters:\n'
        data_list = []
        interval = item.endTime() - item.startTime()
        for chan, value in enumerate(item.getScalers()):
            name = self._scaler_Name(item.getOriginalSourceId(), chan)
            rate = value/interval if interval != 0.0 else '***'
            data_list.append([name, value, rate])
    
        result += tabulate.tabulate(data_list, headers = ['Name', 'Value', 'Rate'])
        result += '\n'
        
        return result
    
    
    def _formatTextItem(self, item: daqformat.stringlistitem) -> str:
        result = ('Monitored Variables\n' if item.type() == daqformat.MONITORED_VARIABLES 
            else 'Packet Types\n')
        result += self._formatBodyHeader(item)
        result += f'{item.getElapsedTime():.2f} seconds into the run at {self._timestring(item.getTime())}\n'
        if self._version >= 12:
            result += f'Original Source id: {self._makeSidString(item.originalSource())}\n'
        result += 'Strings:\n'
        for string in item.getStrings():
            result += f'{string}\n'
        result += '\n'
        
        return result
    
    def _formatPhysicsEvent(self, item : daqformat.physicsevent) -> str:
        result = 'Physics event\n'
        result += self._formatBodyHeader(item)
        result += 'Body\n'
        if self._eventbuilt:
            result += self._formatFragments(item.getbody())
        else:
            result += self._formatByteArray(item.getbody())
            
        result += '\n'
        
        return result
    
    
    def _formatEventCount(self, item : daqformat.eventcountitem) -> str:
        result = 'Trigger/Event count\n'
        result += self._formatBodyHeader(item)
        result += f'{item.timeOffset():.2f} seconds into the run at {self._timestring(item.time())}\n'
        if self._version >= 12:
            result += f'Original Source id: {self._makeSidString(item.originalSource())}\n'
        result += f'{item.eventCount()} triggers accepted\n' 
        result += '\n'

        return result
    
    def _formatFragment(self, fragment : Fragment) -> str:
        # Format a single fragment from event build data
        sourceid = fragment.sourceId()
        
        result =  f'Fragment Timestamp : {fragment.timestamp():016x}\n'
        result += f'Fragment Source    : {self._makeSidString(sourceid)}\n'
        result += f'Fragment Size      : {fragment.payloadSize()}\n'
        result += f'Fragment Barrier id: {fragment.barrierId()}\n'
        
        # If there's a plugin formatter us it otherwise,
        # just render the byte array:
        
        formatter = self._plugins.get(sourceid, None)
        if formatter:
            result += formatter.format(sourceid, fragment.payload())      
        else:
            result += self._formatByteArray(fragment.payload()) 
            result += '\n'
        
        return result
        
    
    def _formatFragments(self, body : bytearray) -> str:
        # Format fragments in an event built body:
        result = 'Fragments: \n'
        fragments = BuiltEventIterator(body)
        for fragment in fragments:
            result += self._formatFragment(fragment)
        return result
    # Other utility methods:
    
    def _makeScalerMap(self, toml : dict[str]) -> dict[int, list[str]]:
        # Convert the raw toml of a parsed scaler def into a scaler map
        #
        
        scaler_map = {}
        for source in toml['datasource'].values():
            id = source.get('sourceid', 0)
            names = source['scalers']
            scaler_map[id] = names
        
        return scaler_map

    def _scaler_Name(self, source : int, channel : int) -> str:
        # Given a scaler channel and its original source id,
        # rerturn a name for that scaler;
        
        # If we can't map it's just the channel number:
        
        result = f'Channel {channel}'
        if self._scaler_map and source in self._scaler_map:
            names = self._scaler_map[source]
            result = names[channel] if channel < len(names) else result
        
        return result

    def _makeSidMap(self) -> dict[int, str]:
        # Generate an sid to name map. and return it.
        
        with open(self._sid_file, "r") as f:
            lines = f.readlines()
        
        result = {}
        for line in lines:
            info = line.split(maxsplit=1)
            sid = int(info[0])
            name = info[1][:-1]
            result[sid] = name
        
        return result

    def _makeSidString(self, sid : int) -> str:
        # IF there's an sid map and the source id
        # is in it then make a nice string for it:
        
        name = f'{sid} '
        if self._sid_map and sid in self._sid_map:
            name = f'{self._sid_map[sid]} ({sid}) '
        
        return name
        
        