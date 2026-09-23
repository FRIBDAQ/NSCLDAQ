#!/usr/bin/env python3

# The shebang allows running this for unittests

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
@file DataSourceManager.py
@brief Provides a singleton to manage all data sources.
@author Ron Fox
'''
import traceback
from collections.abc import KeysView
from operator import methodcaller
from typing import Self

from nscldaq.readoutgui import DataSource
from PyQt6.QtCore import QObject, pyqtSignal

# Exceptions this module can raise:

class SourceManagerException(Exception):
    ''' Base class to allow generic exception handling.'''
    def __init__(self, msg : str) :
        super().__init__(msg)
    
class DuplicateName(SourceManagerException):
    ''' Attempting to add a data source with a name that already exists'''
    def __init__(self, name : str):
        super().__init__(f'{name} already exists')

class SingletonViolation(SourceManagerException) :
    ''' Attempted to construct the data source manager from outside this module. '''
    def __init__(self, filename: str):
        super().__init__(f'Bad intsantiation from {filename}')
        
class NoSuchSource(SourceManagerException):
    ''' Attempted a look up of a data source that does not exist.'''
    def __init__(self, name: str):
        super().__init__(f'{name} is not a data source')
 
class NotCapable(SourceManagerException):
    '''  Attempted to perform an action  at least one source could not do'''       
    def __init__(self, action : str):
        super().__init__(f'At least one data source is not capable of: {action}')
# The manager.
class DataSourceManager(QObject):
    '''
    At its heart, this class provides a singleton that manages
    the data sources the pyReadoutGui has.  It provides a named set of
    data sources.
    
    Major methods include:
    addSource    - Add a new data source.
    removeSource - Remove a data source
    sourceNames  - List the names of all data sources.
    mergedCapabilities - Merge the data source capabilities by logically anding them all.
    
    Slots(?)
    start    - start all data sources
    precheck - check that it's likely a run will be able to start.
    begin    - Start runs in all data sources.
    end      - End the active run in all data sorces.
    pause    - Pause runs in all data sources
    resume   - Resume a paused run.
    stop     - Stop all data sources.
    live     - Check data sources for liveness.
    
    Signals emitted by the manager are:
    starting - start called.
    started  - Start completed.
    
    prechecking - Precheck started.
    prechecked  - Precheck completed
    
    beginning   - Starting a run.
    begun       - run  begun.
    
    ending      - ending a run
    ended       - run ended.
    
    pausing     - pausing a run
    paused      - Run paused.
    
    resuming    - resuming a run.
    resumed     - run resumed.
    
    stopping    - Stop all data sources.
    stopped     - All deata srouces were stopped.
    
    failed(str, str) - operation failed operation/sourcename
    
    sourceDied(str, str) - Source named,type died.
    
    '''
    starting = pyqtSignal()
    started  = pyqtSignal()
    prechecking = pyqtSignal()
    prechecked  = pyqtSignal()
    beginning   = pyqtSignal()
    begun       = pyqtSignal()
    ending      = pyqtSignal()
    ended       = pyqtSignal()
    pausing     = pyqtSignal()
    paused      = pyqtSignal()
    resumeing   = pyqtSignal()
    resumed     = pyqtSignal()
    stopping    = pyqtSignal()
    stopped     = pyqtSignal()
    
    failed      = pyqtSignal(str, str)
    sourceDied  = pyqtSignal(str, str)
    
    def __init__(self, parent : QObject | None = None) -> None:
        # Validate singleton-ness:
        caller = traceback.extract_stack()[-2]
        if caller.filename != __file__:
            raise SingletonViolation(caller.filename)
    
        super().__init__(parent)
        self._sources = {}    # dict[str, DataSource.DataSource]
        
        
    def instance() -> Self:
        '''
        @return DataSourceManager - the singleton instance.
        @note External clients should use this.  External construction will result in
              an exception being thrown.
        '''
        return _instance
    
    #  Public methods:
    def addSource(self, name : str, source : DataSource.DataSource) -> None:
        '''
            @param  name : str - name of the new data source.
            @param  source : DataSource.DataSource - the data source to add.
            @throws DuplicateName exception if name is already in use.
        '''
        
        if name in self._sources:
            raise DuplicateName(name)
        
        self._sources[name] = source
    
    def removeSource(self, name : str) -> DataSource.DataSource:
        '''
            @param name -name of the data source to remove.
            @return DataSource.DataSource, the actual data source removed.
            @throw NoSuchSource if 'name' is not a data source name.
        '''
        result = self._sources.pop(name, None)
        if not result:
            raise NoSuchSource(name)

        return result
    
    def sourceNames(self) -> KeysView:
        '''
            @return KeysView - the names of all data sources.
        '''
        
        return self._sources.keys()
    
    def sources(self) -> dict[str, DataSource.DataSource]:
        '''
        @return - a name/data source dict.  This is a shallow copy
        of our internal dict,so the data sources are references to the
        actual data sources in the manager.
        '''
        return dict(self._sources)
        
    
    def mergedCapabilities(self) -> dict[str, bool]:
        '''
         Analyzes the capabilities of all of the data sources and
         returns a dict that provides the least capable values.
         
         @return dict[str, bool] - Capability name/value dict.
        '''
        result : dict[str, bool] = {
            'canPause' : True, 'runsHaveTitles': True, 'runsHaveNumbers': True
        }
        for source in self._sources.values() :
            source_caps = source.capabilities()
            for cap in source_caps:
                result[cap] &= source_caps[cap]
            
        return result
    
    # Public slots:
    
    def start(self) -> None:
        '''
            Start all data sources.
            'starting' is emitted before iterating over the sources.
            'started' is emitted when iteration completes.
            
            @note it is possible a source can throw an exception starting.
                  This is not captured by us but propagated up the call stack.
            @note started data sources are initialized.
        '''
        self.starting.emit()
        
        if self._iterateAction('start'):  # noqa: SIM102
            #@todo - is a delay appropriate here?
            if self._iterateAction('init'):
                self.started.emit()
    
    
    def precheck(self) -> bool:
        '''
            Ask all the data sources to perform a check to see if a run is
            likely to be startable.
            
            @return bool - True if all sources agreed, False if any disagreed.
            @note prechecking is signaled before beginning the iteration over sources.
            @note If successful (would return true, prechecked is emitted).
            @note All sources are prechecked even if one has failed earlier in the iteration.
            @note This does not emit failed if the precheck failed.  The return value
            is sufficient information for the caller....for now.
        '''
        # Can't use _iteraateAction because we need to collect the results.
        
        result : bool = True
        self.prechecking.emit()
        for name in self._sources:
            result &= self._sources[name].canBegin()
        
        if result:
            self.prechecked.emit()
    
        return result
    
    def begin(self, run : int, title: str) -> None:
        '''
            emits the beginning signal.  
            Once that's done, iterates over the data sources,
            asking them to begin a run. If none of those raises an 
            exception begun is emitted when the iteration is completed.
            
            @param run : int - the run number, passed to all source begin methods.
            @param title :str - The run title, passed to all source begin methods
            
            @note if a source is incapable of run numbers or titles, it will ignore
            those items.
        '''
        self.beginning.emit()
        for name in self._sources:
            try:
                self._sources[name].begin(run, title)
            except Exception as _e:  # noqa: BLE001
                self.failed.emit('begin', name)
                return
                
        
        self.begun.emit()
        
    
    def end(self) -> None:
        '''
        Emits ending then iterates over all data sources invoking end()
        if not exceptions were raised in this process, ended is emitted.
        '''   
        self.ending.emit()
        if self._iterateAction('end'):
            self.ended.emit()   
        
    def pause(self) -> None:
        '''
        emits pausing, tries to pause all data source then, if successful,
        emits paused.
        
        @throws NotCapabile without emitting any signals if at least one data source
        does not support pausing.
        '''
        if self.mergedCapabilities()['canPause']:
            self.pausing.emit()
            if self._iterateAction('pause'):
                self.paused.emit()
        else:
            raise NotCapable('pausing')
    
    def resume(self) -> None:
        '''
            See pause above, except the data sources are resumed if they can be.
        '''
        if self.mergedCapabilities()['canPause']:
            self.resuming.emit()
            if self._iterateAction('resume'):
                self.resumeded.emit()
        else:
            raise NotCapable('pausing/resuming')
        
    
    def stop(self) -> None:
        ''''
        emits the stopping signal, attempts to stop all data sourcdes
        and emits stopped if that was successful.
        
        '''
        self.stopping.emit()
        if self._iterateAction('stop'):
            self.stopped.emit()
    
    def live(self) -> bool | None:
        '''
        Polls all of the data sources to see if they are live.  For each
        sourc reporting it is dead, sourceDied is emitted.
        @return bool - True if all sources are live, False if at least one is dead.
        '''
        result = True
        for name in self._sources:
            if not self._sources[name].check():
                result = False
                self.sourceDied.emit(name, type(self._sources[name].__name__))
                
        return result
        
    # Utilities:
    
    def _iterateAction(self, action : str) -> bool:
        # Iterates over all data sources calling the named action with no
        # parameters.
        # emits failed if a data source raised an exception.
        # and returns False.  If all succeeded, returns True.
        
        for name in self._sources:
            func = methodcaller(action)
            try:
                func(self._sources[name])
            except Exception :  # noqa: BLE001
                self.failed.emit(action, name)
                return False
        return True
        
            
            
_instance : DataSourceManager = DataSourceManager()

if __name__ == '__main__':
    # Tests
    
    import unittest
    import sys
    from PyQt6.QtCore import QCoreApplication
    
    class NullDataSource(DataSource.DataSource):
        # Data source that doesn't do much.
        def __init__(self, **kwargs) :
            # Init our configuration to defaults:
            super().__init__({'anint' : 1, 'astring' : 'hello'}, **kwargs)
            self._run = None
            self._title = None

        def parameters(self) -> dict[str, type]:
            return {'anint': int, 'astring' : str}
        
        def start(self) -> None:
            pass
        
        def check(self) -> True:
            return True
        
        def stop(self) -> None:
            pass
        
        def begin(self, run : int, title : str) -> None:
            self._run = run
            self._title = title

        def end(self) -> None:
            pass

    class FailDataSource(DataSource.DataSource):
        # Data source that fails at everything it's asked to do.
        def __init__(self, **kwargs) :
            # Init our configuration to defaults:
            super().__init__({'anint' : 1, 'astring' : 'hello'}, **kwargs)
            self._run = None
            self._title = None

        def parameters(self) -> dict[str, type]:
            return {'anint': int, 'astring' : str}
        
        def start(self) -> None:
            raise Exception("not Important'")
        
        def check(self) -> True:
            raise Exception("not Important'")
        
        def stop(self) -> None:
            raise Exception("not Important'")
        
        def begin(self, run : int, title : str) -> None:
            self._run = run
            self._title = title
            raise Exception("not Important'")

        def end(self) -> None:
            raise Exception("not Important'")
        # Precheck:
        def canBegin(self) -> bool:
            return False
    
        def pause(self) -> None:
            raise Exception('Not important')
        def resume(self) -> None:
            raise Exception('Not important')

    class NoPauseSource(DataSource.DataSource):
        #  A data source that can't pause.
        def __init__(self, **kwargs) :
            # Init our configuration to defaults:
            super().__init__({'anint' : 1, 'astring' : 'hello'}, **kwargs)
            self._run = None
            self._title = None
 
        def parameters(self) -> dict[str, type]:
            return {'anint': int, 'astring' : str}
         
        def start(self) -> None:
            pass
         
        def check(self) -> True:
            return True
         
        def stop(self) -> None:
            pass
         
        def begin(self, run : int, title : str) -> None:
            self._run = run
            self._title = title

        def end(self) -> None:
            pass
      
        def capabilities(self) -> dict[str, bool]:
            return {
                'canPause' : False, 'runsHaveTitles' : True, 'runsHaveNumbers' : True
            }
        
    class Tests(unittest.TestCase):
        def setUp(self):
            global _instance
            # New data source manager each test... we can do this
            # because we are in the same file
            _instance = DataSourceManager()
            
            
        def test_instance(self):
            self.assertEqual(_instance, DataSourceManager.instance())
            
        def test_addSource_1(self):
            # Add a single source is ok:
            
            src = NullDataSource()
            DataSourceManager.instance().addSource('src', src) 
            self.assertEqual(1, len(_instance._sources))
            self.assertTrue('src' in _instance._sources)
            
        def test_addSource_2(self):
            # Different names can be added:
            
            src = NullDataSource()
            i   = DataSourceManager.instance()
            i.addSource('src1', src)
            i.addSource('src2', src)
            
            self.assertEqual(2, len(_instance._sources))
            self.assertTrue('src1' in _instance._sources)
            self.assertTrue('src2' in _instance._sources)
        
        def test_addSource_3(self):
            # Two sources with the same name is illegal.
            
            src = NullDataSource()
            i   = DataSourceManager.instance()
            i.addSource('src', src)
            
            with self.assertRaises(DuplicateName):
                i.addSource('src', src)   # Duplicate name fails.
        
        def test_removeSource_1(self):
            # Can remove an existing source and it happens.
            
            src = NullDataSource()
            i   = DataSourceManager.instance()
            i.addSource('src', src)
            self.assertEqual(src, i.removeSource('src'))
            self.assertEqual(0, len(i._sources))
            
        def test_removeSource_2(self):
            # The right source is removed:
            
            src = NullDataSource()
            i   = DataSourceManager.instance()
            
            i.addSource('src1', src)
            i.addSource('src2', src)
            
            i.removeSource('src1')
            self.assertTrue('src2' in i._sources)
            
        def test_removeSource_3(self):
            # NoSuchSource thrown if removing nonexistent source.
            
            src = NullDataSource()
            i   = DataSourceManager.instance()
            
            i.addSource('src', src)
            
            with self.assertRaises(NoSuchSource):
                i.removeSource('srcccccccc')   # bad spelling.
        
        def test_names_1(self):
            # No names initially.
            
            self.assertEqual(0, len(_instance.sourceNames()))
        
        def test_names_2(self):
            # Inserting a source allows names to name it:
            src = NullDataSource()
            i   = DataSourceManager.instance()
            
            i.addSource('src', src)
            
            self.assertEqual(1, len(i.sourceNames()))
            self.assertTrue('src' in i.sourceNames())
                           
        def test_sources_1(self):
            # initially empty sources are returned.
            
            self.assertEqual(0, len(DataSourceManager.instance().sources()))
            
        def test_sources_2(self):
            # Inserting a source gets it out again.
            
            src = NullDataSource()
            i   = DataSourceManager.instance()
            i.addSource('src', src)
            
            sources = i.sources()
            self.assertEqual(1, len(sources))
            self.assertTrue('src' in sources)
            self.assertIs(src, sources['src'])
            
        def test_caps_1(self):
            #  Nosources have full merged capabilities:
            
            caps = DataSourceManager.instance().mergedCapabilities()
            
            self.assertTrue(caps['canPause'])
            self.assertTrue(caps['runsHaveTitles'])
            self.assertTrue(caps['runsHaveNumbers'])
            
        def test_caps_2(self):
            #  Null data source inserted has full caps:
            
            src = NullDataSource()
            i   = DataSourceManager.instance()
            i.addSource('src', src)
            
            caps = i.mergedCapabilities()
            self.assertTrue(caps['canPause'])
            self.assertTrue(caps['runsHaveTitles'])
            self.assertTrue(caps['runsHaveNumbers'])
            
        def test_caps_3(self):
            # NoPauseSource added pause is not possible.
            
            src = NoPauseSource()
            i   = DataSourceManager.instance()
            i.addSource('src', src)
            
            caps = i.mergedCapabilities()
            self.assertFalse(caps['canPause'])
            self.assertTrue(caps['runsHaveTitles'])
            self.assertTrue(caps['runsHaveNumbers'])
            
        def test_caps_4(self):
            # Both null and no pause inserted pause is not possible.
            
            src1 = NullDataSource()
            src2 = NoPauseSource()
            i    = DataSourceManager.instance()
            
            i.addSource('pausable', src1)
            i.addSource('not-pausable', src2)
            
            caps = i.mergedCapabilities()
            self.assertFalse(caps['canPause'])
            self.assertTrue(caps['runsHaveTitles'])
            self.assertTrue(caps['runsHaveNumbers'])
        
        def test_start_1(self) :
            #  No sources can start with no errors...and will emit both signals.
            
            starting = False
            started  = False
            def starting_slot():
                nonlocal starting
                starting = True
            def started_slot():
                nonlocal started
                started = True

            i = DataSourceManager.instance()
            i.starting.connect(starting_slot)
            i.started.connect(started_slot)
            
            i.start()
            
            self.assertTrue(starting)
            self.assertTrue(started)
            
        def test_start_2(self):
            # If NullSource is registered, start works:
            starting = False
            started  = False
            def starting_slot():
                nonlocal starting
                starting = True
            def started_slot():
                nonlocal started
                started = True

            i = DataSourceManager.instance()
            i.addSource('src', NullDataSource())
            
            i.starting.connect(starting_slot)
            i.started.connect(started_slot)
            
            i.start()
            
            self.assertTrue(starting)
            self.assertTrue(started)            
            
        def test_start_3(self):
            # If FailingSource is added, the started signal won't be emitted
            # because its start raises an exception.
            
            starting = False
            started  = False
            failed   = False
            failop   = None
            failsrc = None
            
            def starting_slot():
                nonlocal starting
                starting = True
            def started_slot():
                nonlocal started
                started = True
            def failed_slot(op : str, src : str):
                nonlocal failed, failop, failsrc
                failed = True
                failop = op
                failsrc = src
                
            i = DataSourceManager.instance()
            i.addSource('src', FailDataSource())
            i.starting.connect(starting_slot)
            i.started.connect(started_slot)
            i.failed.connect(failed_slot)
            
            i.start()
            
            self.assertTrue(starting)
            self.assertFalse(started)            
            self.assertTrue(failed)
            self.assertEqual('start', failop)
            self.assertTrue('src', failsrc)
            
        def test_precheck_1(self):
            # Precheck with no source if fine:
            
            pc  = False
            pcd = False
            def precheck_slot():
                nonlocal pc
                pc = True
            
            def prechecked_slot():
                nonlocal pcd
                pcd = True

            i = DataSourceManager.instance()
            i.prechecking.connect(precheck_slot)      
            i.prechecked.connect(prechecked_slot)
            
            self.assertTrue(i.precheck())
            self.assertTrue(pc)
            self.assertTrue(pcd)
            
        def test_precheck_2(self):
            #  PRecheck with null data source works too:
            
            pc  = False
            pcd = False
            def precheck_slot():
                nonlocal pc
                pc = True
            
            def prechecked_slot():
                nonlocal pcd
                pcd = True

            i = DataSourceManager.instance()
            i.addSource('src', NullDataSource())
            i.prechecking.connect(precheck_slot)      
            i.prechecked.connect(prechecked_slot)
            
            self.assertTrue(i.precheck())
            self.assertTrue(pc)
            self.assertTrue(pcd)
            
        def test_precheck_3(self):
            # Test with failing precheck fails.   
            pc  = False
            pcd = False
            def precheck_slot():
                nonlocal pc
                pc = True
            
            def prechecked_slot():
                nonlocal pcd
                pcd = True

            i = DataSourceManager.instance()
            i.addSource('src', FailDataSource())
            i.prechecking.connect(precheck_slot)      
            i.prechecked.connect(prechecked_slot)
            
            
            self.assertFalse(i.precheck())
            self.assertTrue(pc)
            self.assertFalse(pcd)
        
        def test_begin_1(self):
            beginning = False
            begun     = False
            def beginning_slot() :
                nonlocal beginning
                beginning = True
            def begun_slot():
                nonlocal begun
                begun = True
            # Begin with no data sources works.
            i = DataSourceManager.instance()
            i.beginning.connect(beginning_slot)
            i.begun.connect(begun_slot)
            
            i.begin('some title', 1234)
            self.assertTrue(beginning)
            self.assertTrue(begun)
            
        def test_begin_2(self):
            # Test with null data source:
            beginning = False
            begun     = False
            def beginning_slot() :
                nonlocal beginning
                beginning = True
            def begun_slot():
                nonlocal begun
                begun = True
            # Begin with no data sources works.
            i = DataSourceManager.instance()
            i.addSource('src', NullDataSource())
            i.beginning.connect(beginning_slot)
            i.begun.connect(begun_slot)
            
            i.begin('some title', 1234)
            self.assertTrue(beginning)
            self.assertTrue(begun)
        
        def test_begin_3(self):
            #  Test begin with fail data source:
            beginning = False
            begun     = False
            def beginning_slot() :
                nonlocal beginning
                beginning = True
            def begun_slot():
                nonlocal begun
                begun = True
                
            failed = False
            op    = None
            src   = None

            def failed_slot(fop, fsrc):
                nonlocal failed, op, src
                failed =True
                op = fop
                src = fsrc
            
            # Begin with no data sources works.
            i = DataSourceManager.instance()
            i.addSource('fail', FailDataSource())
            i.beginning.connect(beginning_slot)
            i.begun.connect(begun_slot)
            i.failed.connect(failed_slot)
            
            i.begin('some title', 1234)
            self.assertTrue(beginning)
            self.assertFalse(begun)
            self.assertTrue(failed)
            self.assertEqual('begin', op)
            self.assertEqual('fail', src)
            
        def test_end_1(self):
            # End with no sources is ok.
            
            ending = False
            ended  = False
            
            failed = False
            fop    = None
            fsrc   = None

            def ending_slot():
                nonlocal ending
                ending = True
            def ended_slot():
                nonlocal ended
                ended = True
            
            def failed_slot(op, src):
                nonlocal failed, fop, fsrc
                failed = True
                fop = op
                fsrc = src

            i = DataSourceManager.instance()
            i.ending.connect(ending_slot)
            i.ended.connect(ended_slot)
            i.failed.connect(failed_slot)
            
            i.end()
            
            self.assertTrue(ending)
            self.assertTrue(ended)
            self.assertFalse(failed)
        
        def test_end_2(self):
            # with no-p data source also works:
            
            ending = False
            ended  = False
            
            failed = False
            fop    = None
            fsrc   = None

            def ending_slot():
                nonlocal ending
                ending = True
            def ended_slot():
                nonlocal ended
                ended = True
            
            def failed_slot(op, src):
                nonlocal failed, fop, fsrc
                failed = True
                fop = op
                fsrc = src

            i = DataSourceManager.instance()
            i.ending.connect(ending_slot)
            i.ended.connect(ended_slot)
            i.failed.connect(failed_slot)
            i.addSource('src', NullDataSource())            
            
            i.end()
            
            self.assertTrue(ending)
            self.assertTrue(ended)
            self.assertFalse(failed)
        
            
        def test_end_3(self):
            # With failing data source...
            
            
            ending = False
            ended  = False
            
            failed = False
            fop    = None
            fsrc   = None

            def ending_slot():
                nonlocal ending
                ending = True
            def ended_slot():
                nonlocal ended
                ended = True
            
            def failed_slot(op, src):
                nonlocal failed, fop, fsrc
                failed = True
                fop = op
                fsrc = src

            i = DataSourceManager.instance()
            i.ending.connect(ending_slot)
            i.ended.connect(ended_slot)
            i.failed.connect(failed_slot)
            i.addSource('fail', FailDataSource())            
            i.end()
            
            self.assertTrue(ending)
            self.assertFalse(ended)
            self.assertTrue(failed)
            self.assertEqual('end', fop)
            self.assertEqual('fail', fsrc)
        
        # Need extra tests for pause to cover the case where a data source does not support
        # pausing.
        
        def test_pause_1(self):
            # NO sources
            pausing = False
            paused  = False
            failed  = False
            fop     = None
            fsrc    = None

            def pausing_slot():
                nonlocal pausing
                pausing = True
            def paused_slot():
                nonlocal paused
                paused = True
            def failed_slot(op, src):
                nonlocal failed, fop, fsrc
                failed = True
                fop = op
                fsrc = src
                
            i = DataSourceManager.instance()
            i.pausing.connect(pausing_slot)
            i.paused.connect(paused_slot)
            i.failed.connect(failed_slot)
            
            i.pause()
            
            self.assertTrue(pausing)
            self.assertTrue(paused)
            self.assertFalse(failed)
        
        def test_pause_2(self):
            #Null source:
            pausing = False
            paused  = False
            failed  = False
            fop     = None
            fsrc    = None

            def pausing_slot():
                nonlocal pausing
                pausing = True
            def paused_slot():
                nonlocal paused
                paused = True
            def failed_slot(op, src):
                nonlocal failed, fop, fsrc
                failed = True
                fop = op
                fsrc = src
                
            i = DataSourceManager.instance()
            i.pausing.connect(pausing_slot)
            i.paused.connect(paused_slot)
            i.failed.connect(failed_slot)
            i.addSource('src', NullDataSource())            
            i.pause()
            
            self.assertTrue(pausing)
            self.assertTrue(paused)
            self.assertFalse(failed)
        
        def test_pause_3(self):
            #  Fail source:
            pausing = False
            paused  = False
            failed  = False
            fop     = None
            fsrc    = None

            def pausing_slot():
                nonlocal pausing
                pausing = True
            def paused_slot():
                nonlocal paused
                paused = True
            def failed_slot(op, src):
                nonlocal failed, fop, fsrc
                failed = True
                fop = op
                fsrc = src
                
            i = DataSourceManager.instance()
            i.pausing.connect(pausing_slot)
            i.paused.connect(paused_slot)
            i.failed.connect(failed_slot)
            i.addSource('fail', FailDataSource())            
            i.pause()
            
            self.assertTrue(pausing)
            self.assertFalse(paused)
            self.assertTrue(failed)
            self.assertEqual('pause', fop)
            self.assertEqual('fail', fsrc)
        
        def test_pause_4(self):
            # Data source does not support pausing.
            
            pausing = False
            paused  = False
            failed  = False
            fop     = None
            fsrc    = None

            def pausing_slot():
                nonlocal pausing
                pausing = True
            def paused_slot():
                nonlocal paused
                paused = True
            def failed_slot(op, src):
                nonlocal failed, fop, fsrc
                failed = True
                fop = op
                fsrc = src
                
            i = DataSourceManager.instance()
            i.pausing.connect(pausing_slot)
            i.paused.connect(paused_slot)
            i.failed.connect(failed_slot)
            i.addSource('nopause', NoPauseSource())            
            
            
            with self.assertRaises(NotCapable):
               i.pause()
            
            # NO signals fired.
            self.assertFalse(pausing)
            self.assertFalse(paused)
            self.assertFalse(failed)



        
    app = QCoreApplication(sys.argv)     # Needed for signal to work I think.
    unittest.main()