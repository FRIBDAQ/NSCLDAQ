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
@file RunConfig.py
@brief Save/Restore the run configuration.
@author Ron Fox.

The configuration saved includes the following:
Title, Run NUmber, Length of timed run, if the run is timed and the recording state.


'''
import io
import types

from nscldaq.readoutgui.StateSaveRestore import Serializer, SerializationManager
from nscldaq.readoutgui.ReadoutGuiView import mainWindow

from PyQt6.QtCore  import Qt



class RunConfigSerializer(Serializer):
    '''
    See the file docstring.  We're gong to write/read a dict with the keys:
    * 'title'  - the run title  string
    * 'run'    - The run number.
    * 'timedRun' - Bool True if the run is timed.
    * 'runDuration' - Length of the run if timed.
    * 'recording'  - Bool, true if the run is recording.
    
    '''
    def serialize(self, destination : io.TextIOBase) -> None:
        '''
        Form and write the dict to file:
        @param destination : io.TextIOBase - where to write the data.
        '''
        # Fetch the stuff we need to save:
        main = mainWindow().centralWidget()
        
        runparams = main.RunParameters()
        title = runparams.title()
        runNumber = runparams.run()
        
        timedrun = main.RunTimer()
        isTimed = timedrun.timed()
        duration = timedrun.plannedDuration()   # In seconds.
        
        recording = main.Recording().checkState() == Qt.CheckState.Checked
        
        # form the dict:
        
        configuration = {
            'title': title,
            'run'  : runNumber,
            'timedRun' : isTimed,
            'runDuration' : duration,
            'recording'   : recording
        }
        destination.write(f'runParameters = {configuration}\n')
    
    def deserialize(self, module : types.ModuleType) -> None:
        '''
            The marshall the read back dict into the GUI:
            @param module : types.ModuleType - A module that has been
                  loaded that, among other things, contains the runParametersDict.
        '''
        # For now ignore errors (e.g. partial config file)
        try:
            config = module.runParameters    # reduce typing.
            main = mainWindow().centralWidget()
            runparams = main.RunParameters()
            runparams.setTitle(config['title'])
            runparams.setRun(config['run'])
            
            timedrun = main.RunTimer()
            timedrun.setTimed(config['timedRun'])
            timedrun.setPlannedDuration(config['runDuration'])
            
            recordingState = Qt.CheckState.Checked if config['recording'] else Qt.CheckState.Unchecked
            main.Recording().setCheckState(recordingState)
            
        except Exception as e:  # noqa: BLE001
            print(f'Warning Exception caught deserializing run configuration {e}')
            
        
        
        
        
# Register ourselves with the manager:

SerializationManager.instance().addSerializer(RunConfigSerializer())              