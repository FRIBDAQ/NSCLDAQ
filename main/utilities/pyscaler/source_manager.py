'''
    This module provides a manager of data sources.  Each source is implemented
    as a QThread that reads data from a source created by the datasourcde module.
    That manager provides the following services:
    
    - adds a new data source; which means starting an new thread that, in tern
      creates the data source with the appropriate characteristics.
    - Providing a slot for the threads to publish their data when it's availalbe.
      This allows the threads to use blocking reads of their sources.
      The slot will provide a cooked signal for the main application.
    - Providing a slot to catch thread exit, clean up knowledge of that
      exit and signal the exit to the application.
      
    The data source characteristics are suitable for the scaler display, that
    is we only need state changes, scaler items, versions and we'll also
    add the value of accepting the trigger statistics as these might
    be displayed later.
    
    @file source_manager.py
    @brief Manage the scaler display's data sources.
    @author Ron Fox
    @note The python module path needs to include the unified format python module.
          The scaler program should add this to the search path by adding
          os.environ(DAQROOT)/unifiedformat/python to the module search path.
    
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

from PyQt5.QtCore import QObject, QThread, pyqtSignal, QEventLoop
import datasource
import daqformat

#  The default set of items to skip:

default_skip_list = {
        daqformat.EVB_FRAGMENT, daqformat.EVB_GLOM_INFO, daqformat.EVB_UNKNOWN_PAYLOAD,
        daqformat.MONITORED_VARIABLES, daqformat.PACKET_TYPES, daqformat.PHYSICS_EVENT
}
class DataSourceThread(QThread):
    '''
        This thread reads data from a data source.
        It provides the signals:
        
        - started (inherited from QThread)
        - finished (inherited from QThread)
        - newData(str, daqformat.ringitem) - The string is the name of a source
            that identified the data source (not the URI).  The ringitem is the data
            received.  The actual item will be derived from that base class depending on the
            type of the underlying ring item.
        
        @note, while this is intended to handle online data sources, in fact
               our  code can handle offline (file) data sources as well. This is,
               obviously intended for testing.
    '''
      
    
    #  instance data:
    _source  : datasource.OnlineDataSource | datasource.FileDataSource
    _source_name : str             
    
    # Signals 
    
    newData = pyqtSignal(str, daqformat.ringitem)

    #  This worker contains the actual data source logic and will
    #  be moved into self    
    
    class Worker(QObject) :
        newData = pyqtSignal(daqformat.ringitem)
        def __init__(self, source, parent):
            super().__init__(None)
            self._parent = parent
            self._source = source
        def process(self):
            # @todo - Play with data sources so 
            #         we can do a timed wait so that
            #         when e.g. a run is not active, the
            #         thread's event loop is not starved.
            dispatcher = self._parent.eventDispatcher()
            if  dispatcher is None:
                print("warning no event dispatcher!")
            for item in iter(self._source):
                if self._parent.isInterruptionRequested():
                    self._parent.exit()
                    return
                if dispatcher is not None:
                    dispatcher.processEvents(QEventLoop.AllEvents)
                self._parent.newData.emit(self._parent._source_name, item)
                
                

    def __init__(self, 
                 name: str, uri: str, 
                 format:int=12, skip:set = default_skip_list, 
                 sample:set={}, parent:QObject | None = None):
        '''
            Construct a new data source thread:
            
            @param name - name to be given to the data source - this is the str argument to the newData signal.
            @param uri  - The URI defining the data source.  
            @param format - The optional DAQ format version - note that receipt of a daqformat.RING_FORMAT  item
                            will transparently correct this if its wrong. Defaults to 12.
            @param skip - The optional set of types of data we are not interested in. 
               This defaults to a set containing:
                * daqformat.EVB_FRAGMENT, 
                * daqformat.EVB_GLOM_INFO, 
                * daqformat.EVB_UNKNOWN_PAYLOAD,
                * daqformat.MONITORED_VARIABLES, 
                * daqformat.PACKET_TYPES, 
                * daqformat.PHYSICS_EVENT
            @param sample - An optional set containing the ring item types that can be sampled. 
                This is only relevant if the uri is a tcp: uri.  By default this is empty, 
                no data are sampled, even online.
            @parent - The parent object for the thread.  Defaults to None
        '''
        super().__init__(parent)
        self._source_name = name
        self._source = datasource.DataSourceFactory.makeSource(uri, format, skip, sample)
        
        #  This rigmarole with a worker object that is moved to the thread
        #  with a slot connected to the thread is how we ensure
        #  the thread has an event loop and that, therefore,
        #  the attempts to exit the thread can be made.
        self._worker = self.Worker(self._source, self)
        self._worker.moveToThread(self)    # Not sure if I need to do ths?
        self.started.connect(self._worker.process)  # run process in worker when started.
        
            

class DataSourceManager(QObject):
    '''
        This class provides a manager for data sources.  Each data source is a DataSourceThread.
        
        Methods:
            addSource - add a new source to the manager, the source is started.
            killSource - Remove a source from the manager. The source thread is stopped and
                         I believe, sourceExited will be signalled.  This is normally not used.
        
        Attributes:
            sources    - readonly provides a list of the names of the sources being managed.
        
        Signals:
        
        newData - relayed from the threads.  This is parameterized by the source name and item.
        sourceExited - when a source has exited  this is parameterized by the source name.
        
    '''
    # Instance data:
    
    _sources : dict     # Dict of source keyed by source name.
    
    # Class data:
    
    newData       = pyqtSignal(str, daqformat.ringitem)
    sourceExited  = pyqtSignal(str)
    
    def __init__(self, parent=None):
        '''
            Construct the manager.
            @param parent - if desired, the parent of the QObject that is the manager.
                             defaults to None.
        '''
        super().__init__(parent)
        self._sources = dict()             # Initialize the sources dict.
        
        
    def addSource(self, 
                  name : str, uri : str, format:int = 12, 
                  skip:set = default_skip_list, sample:set={}) -> None:
        '''
            Create and manage a new data source.  
            
            @param name   - name of the data source. If this is not unique, ValueError is raised.
            @param uri    - URI specifying the data source.
            @param format - The optional argument specifying the format version ofthe ring items
                 expected from the source.  Note that if a daqformat.RING_FORMAT item is seen,
                 this is transparently corrected, if necessary.  Note that this is a darn good reason
                 not to have daqformat.RING_FORAMT in the skip list.
            @param skip  - Optional set of item types to filter out. 
                This defaults to a set containing:
                * daqformat.EVB_FRAGMENT, 
                * daqformat.EVB_GLOM_INFO, 
                * daqformat.EVB_UNKNOWN_PAYLOAD,
                * daqformat.MONITORED_VARIABLES, 
                * daqformat.PACKET_TYPES, 
                * daqformat.PHYSICS_EVENT
            @param sample - Optional set of items that can be sampled.   Sampling an item means you
               are not assured of seeing all items of that type.  This defaults to an empty set.
        '''
        if name in self._sources.keys():
            raise ValueError(f'There is already a source named "{name}"')
        
        source = DataSourceThread(name, uri, format, skip, sample, self)
        
        
        # If I understand Qt correctly, the default connection is Qt.AutoConnection
        # which for me means signals emitted in that thread get queued to my 
        # Event loop and executed in my thread. 
        #
        #  The trick below allows me to pass the name of the exiting source to the
        #  Actual slot _sourceExited
        #
        source.finished.connect(lambda : self._sourceExited(name))
        source.newData.connect(self.newData)    # Just aggregate from all sources.
        
        # Save  the thread in the dict and run it.
        
        self._sources[name] = source
        source.start()
    
    def killSource(self, name: str) -> None:
        '''
           Forces a data source to exit.  The method will block until the 
           exit completes... furthermore, the sourcExited signal with that
           source name is also emitted.  It is at that point that the 
           source is removed from the known sources list.
           
           @param name - name of the source to kill.
           @note IndexError is eraised if the source does not exist.
        '''
        source = self._sources[name]     # Here's where IndexError is raised.
        source.requestInterruption()     #try it the nice way.
        if not source.wait(3000):        # 3 seconds because 2 second scaler interval.
            # If the source didn't exit after a second, take harsh measures with
            # fingers crossed... see the warnings in https://doc.qt.io/qt-6/qthread.html#terminate
            # about doing this:
            
            source.terminate()
            
            

    ## Internal slots:
    def _sourceExited(self, name: str) -> None:
        # Called when a source exists.  In that case:
        # Remove the sourcde from the dict emit our sourceExited signal.
        
        self._sources.pop(name, None)   # Tolerate non such name.
        self.sourceExited.emit(name)
        
        
    
    
    
if __name__ == '__main__':
    items = 0
    def onData(name, item):
        global items
        print(f'Got type {item.type()} from source: {name}')
        items += 1
        if items == 10:
            print("killing 'ron'")
            mgr.killSource('ron')
    
    def onExit(name):
        print(f'Source {name} exited.')
    
    # Let's see if we can get some test code .
    
    from PyQt5.QtWidgets import QApplication, QMainWindow
    app = QApplication([])
    win = QMainWindow()
    
    #  Make a data source manager and manage data from
    # tcp://localhost/ron
    
    mgr = DataSourceManager()
    mgr.newData.connect(onData)
    mgr.sourceExited.connect(onExit)
    mgr.addSource('ron', 'tcp://localhost/ron')
    
    win.show()
    app.exec()