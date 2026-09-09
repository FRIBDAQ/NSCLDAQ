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

from PyQt6.QtWidgets import QMessageBox
from PyQt6.QtCore import QObject, pyqtSignal
import pyUI
import daqformat
from datetime import datetime
import time
import struct

from  nscldaq.pyscaler.datasource import FileDataSource
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
        self._plugins       = None
        self._filter       = None
        self._version       = 12
        self._eventbuilt    = False

        # Hook in to the signals the view will emit:
        #
        self._view.open.connect(self._openEventFile)
        self._view.plugin.connect(self._loadPlugin)
        self._view.exit.connect(self._cleanup)
        self._view.filter.connect(self._setFilter)
        self._view.clearfilter.connect(self._clearFilter)
        self._view.next.connect(self._nextItem)
        
        
    # Slots that are internal (private) to the controller:
    
    def _openEventFile(self, path : str) -> None:
        #  Open a new event file:
        
        try:
            self._eventfile = FileDataSource(path, self._version, set(), set())
            self._eventfileName = path
        except Exception as e:
            QMessageBox.warning(
                None, 'Failed Event Source',
                f'Failed to create an event source for {path}, {e}'
            )

     
    def _loadPlugin(self, pluginPath : str) -> None:
        # Load a formatting plugin  @todo
        pass   
     
    def _cleanup(self) -> None:
        # Cleanup before exiting:
        # If there is an event file, close it:
        
        if self._eventfile:
            self._eventfile.close()
            self._eventfile = None
            self._eventfileName = None

    def _setFilter(self, filter : list) -> None:
        # set the list of acceptable ring item types:
        
        self._filter = filter
        
    def _clearFilter(self) -> None:
        # Clear any ring item type filter:
        
        self._filter = None
    
    
    def _nextItem(self) -> None:
        if self._eventfile:    # Nothing if no event file.
            while True:
                item = self._eventfile.next()
                if not item:
                    # End of file:
                    self.endFile.emit()
                    self._eventfile.close()
                    self._eventfile = None
                    self._eventfilename = None
                    break
                else:
                    # Skip the item?
                    
                    if self._filter and item.type() not in self._filter:
                        continue
                    else:
                        text = self._format(item)
                        self._view.dumpWidget().setText(text)
                        break
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
            result += f'  Source Id   : {item.sourceid()}\n'
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
        result += f'From original source id: {item.originalSource()}\n\n'     
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
        result += f'Fragment Source id : {item.source()}\n'
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
        building = 'Bulding' if item.isBuiding() else 'Not Building'
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
        result += f'Original Source Id: {item.getOriginalSourceId()}\n'
        incr = 'Incremental'  if item.isIncremental() else 'Not Incremental'
        result += f'Readout is {incr}\n'
        result += 'Counters:\n'
        for chan, value in enumerate(item.getScalers()):
            result += f'Channel: {chan} : {value}\n'
        result += '\n'
        
        return result
    
    
    def _formatTextItem(self, item: daqformat.stringlistitem) -> str:
        result = ('Monitored Variables\n' if item.type() == daqformat.MONITORED_VARIABLES 
            else 'Packet Types\n')
        result += self._formatBodyHeader(item)
        result += f'{item.getElapsedTime():.2f} seconds into the run at {self._timestring(item.getTime())}\n'
        result += f'Original Source id: {item.originalSource()}\n'
        result += 'Strings:\n'
        for string in item.getStrings():
            result += f'{string}\n'
        result += '\n'
        
        return result
    
    def _formatPhysicsEvent(self, item : daqformat.physicsevent) -> str:
        result = 'Physics event\n'
        result += self._formatBodyHeader(item)
        result += 'Body\n'
        result += self._formatByteArray(item.getbody())
        result += '\n'
        
        return result
    
    
    def _formatEventCount(self, item : daqformat.eventcountitem) -> str:
        result = 'Trigger/Event count\n'
        result += self._formatBodyHeader(item)
        result += f'{item.timeOffset():.2f} seconds into the run at {self._timestring(item.time())}\n'
        result += f'Original Source id: {item.originalSource()}\n'
        result += f'{item.eventCount()} triggers accepted\n' 
        result += '\n'

        return result