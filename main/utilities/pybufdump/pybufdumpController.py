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
                    self.endfile.emit()
                    self._eventfile.close()
                    self._eventfile = None
                    self._eventfilename = None
                else:
                    # Skip the item?
                    
                    if self._filter and item.type() not in self._filter:
                        continue
                    else:
                        text = self._format(item)
                        self._view.dumpWidget().setText(text)
                        break

    def _format(self, item : daqformat.ringitem) -> str:
        match item.type():
            case daqformat.ABNORMAL_ENDRUN:
                return self._formatabend(item)
            case daqformat.BEGIN_RUN | daqformat.END_RUN | daqformat.PAUSE_RUN | daqformat.RESUME_RUN:
                return self._formatStateChange(item)
            case _:
                return f'Unhandled item type: {item.type()}\n'

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
            result += f'  Timestamp   : {ts:016x}\n'
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