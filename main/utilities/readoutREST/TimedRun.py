'''
    One important capability is to perform a timed run.  A timed run
    is a run that has a specified duration.  The view provides
    both the duraction and a checkbutton that allows timed run
    to be enabled/disabled.   The model maintains the
    enable state and the elapsed run time.  It also emits a 
    signal when the elapsed run time extends beyond the
    desired elapsed run time..if a timed run is enabled.
    
    An external controller is presumed to hook into the
    DAQ to know when state changes to active have
    occured and to maintained the elapsed run time.
    
    The external controller, presumable hooks onto the 
    model's time to end run signal and attempts to end
    the run when that signal is received.
    
@file TimedRun.py
@brief Model and view to support timed runs.
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

