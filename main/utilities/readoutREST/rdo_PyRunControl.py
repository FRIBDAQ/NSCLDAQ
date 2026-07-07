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


''''
   This program replaces the rdo_RunControl.tcl run control script.  It's used a bit differently 
   than rdo_RunControl.tcl which only supports default ReST services for the 
   Readouts and, hence, would not be usable if more than one Readout was running in a host as is,
   for example, the case in the S800 or maybe in test lab systems.
'''
pgm_usage= \
'''   
   Usage:
      $DAQBIN/rdo_PyRunControl mgr_host mgr_user /path/to/readout_descriptions [mgr_service]
      
      Where:
         mgr_host is the host on which the manager is running.
         mgr_user is the user that's running the manager.
         /path/to/readout_descriptions is a readout description file, see below.
         mgr_service if present is the ReST service advertised by the manager, defaults to 
             the manager default service.
         The readout description file is a CSV file. Each line in the file describes
         a Readout program.
         *  If there is one field in a line, that's just the name of a Readout program
         *  If there are two fields, the second field is the name of that Readout's ReST service
         For example:
      
Readoutone_readout
Reeadouttwo_readout,MyService

         Describes two Readouts.  'Readoutone_readout 'uses the default ReST service of ReadoutREST while 
         'Readouttwo_readout' advertises its ReST server on 'MyService'

'''

import sys
import csv
from nscldaq.readoutREST import rdo_utils
from nscldaq.manager_client import KVStore, Logger
from nscldaq.manager_client import CONSTANTS as MGRCONSTS
from nscldaq.readoutREST import (
   RunInfo, RunState, Eventlog, ReadoutStatus, ReadoutStatistics,
   TimedRun
)

from nscldaq.readoutREST import (
   RunInfoController, RunStateController, EventlogController,
   ReadoutStatusController, ReadoutStatisticsController, TimedRunController)

from PyQt6.QtWidgets import (QMainWindow, QApplication, QWidget, 
                             QVBoxLayout, QHBoxLayout, QTabWidget)
from PyQt6.QtCore import QObject


# Global data:

run_number : int = -1               # Not known yet.

# This is the megawidget that is the user interface: It contains all the views
# and accessors for them to allow controllser to be establisehd for them.
#

class GUI(QWidget):
   def __init__(self, parent :QObject | None  = None):
      super().__init__(parent)
      self._layout = QVBoxLayout()
      self.setLayout(self._layout)
      
      self._runInfo = RunInfo.RunInfo(self)
      self._layout.addWidget(self._runInfo)
      
      self._stateloggerlayout = QHBoxLayout()
      self._runState = RunState.RunState(self)
      self._stateloggerlayout.addWidget(self._runState)
      
      self._loggerEnable = Eventlog.Logger(self)
      self._stateloggerlayout.addWidget(self._loggerEnable)
      self._layout.addLayout(self._stateloggerlayout)
      
      self._timedRuns = TimedRun.TimedRunView(self)
      self._layout.addWidget(self._timedRuns)
      
      
      # In the very bottom, we need a tabbed widget
      # That has in the first tab a
      # an eventlog configuration controller.
      # In the second tab, a list of the readouts and their states,
      # Clients can add tabs for each Readout to show its
      # triggers statistics.
      
      self._tabs = QTabWidget(self)
      self._layout.addWidget(self._tabs)
      
      self._logConfig = Eventlog.LoggerConfig(self)
      self._tabs.addTab(self._logConfig, 'EventLogging')
   
      self._readoutStatuses = ReadoutStatus.ReadoutStatus(self)
      self._tabs.addTab(self._readoutStatuses, 'Readout Summary')
      
      
      self._statisticsViews = list()
      self._originalStyle = self._tabs.styleSheet()

   def setRecordingBackground(self):
      self._tabs.setStyleSheet('background-color: green;')
   def setNormalBackground(self):
      self._tabs.setStyleSheet(self._originalStyle)

   def runInfo(self) -> RunInfo.RunInfo:
      return  self._runInfo
   
   def runState(self) -> RunState.RunState:
      return self._runState
     
   def timedRuns(self)-> TimedRun.TimedRunView:
      return self._timedRuns
     
   def loggerEnable(self) -> Eventlog.Logger:
      return self._loggerEnable
   
   def logConfig(self) -> Eventlog.LoggerConfig:
      return self._logConfig

   def readoutStatuses(self) -> ReadoutStatus.ReadoutStatus:
      return self._readoutStatuses
   
   def addReadoutStatisticsView(self, name : str) -> ReadoutStatistics.RunStatistics:
      # Add a new Readout statistics view
      # tab and return the view.
      
      statView = ReadoutStatistics.RunStatistics(self)
      self._statisticsViews.append(statView)
      self._tabs.addTab(statView, name)
      
      return statView
      

def createControllers(
   gui : GUI, mgr_host : str, mgr_user : str, mgr_service: str, 
   readout_list : list[tuple[str, str]]
) -> tuple[QObject]:
   # Create the controllers for all the views in the GUI:
   # I think the parameters are self explanatory except maybe for the
   # readout_list which is a list of 2 element tuples containing
   # in order, the names of readouts and their ReST service names.
   # we return the tuples we created to prevent them from being
   # garbage collected.
   
   # Run info controller:
   
   run_info_controller = RunInfoController.RunInfoController(
      gui.runInfo(), KVStore(mgr_host, mgr_user, mgr_service)
   )
   # Run state controller:
   run_state_controller = RunStateController.RunStateController(
      readout_list, gui.runState(), mgr_host, mgr_user, mgr_service
   )
   run_state_controller.start()
   
   # Event logging global enable
   
   log_enable = EventlogController.LoggerEnableController(
      gui.loggerEnable(), mgr_host, mgr_user, mgr_service
   )
   
   # Timed runs:
   timed_runs = TimedRunController.TimedRunController(
      gui.timedRuns(), mgr_host, mgr_user, mgr_service
   )
   
   # Event log configuration
   
   log_config = EventlogController.LoggerConfigController(
      gui.logConfig(), mgr_host, mgr_user, mgr_service
   )
   
   #  Readout status:
   
   readout_summary = ReadoutStatusController.ReadoutStatusController(
      gui.readoutStatuses(), readout_list, mgr_host, mgr_user, mgr_service
   )
   
   # Add readout statistics tabs for each Readout:
   
   statistics_controllers = list()
   for (name, service) in readout_list:
      view = gui.addReadoutStatisticsView(name + ' stats')
      controller = ReadoutStatisticsController.ReadoutStatisticsController(
         view, name, mgr_host, mgr_user, mgr_service, service
      )
      statistics_controllers.append(controller)
   
   return (run_info_controller, run_state_controller, timed_runs,
           log_enable, log_config,
           readout_summary, statistics_controllers)
   

def Usage() -> None:
   # Print the program usagbe on stderr.
   
   global pgm_usage
   print(pgm_usage, file = sys.stderr)


def process_arguments(arglist : list[str]) -> tuple[str, str, list[str]]:
   
   mgr_host = sys.argv[1]
   mgr_user = sys.argv[2]

   with open(sys.argv[3], newline='' ) as readout_file:
      readouts = csv.reader(readout_file)
      readout_list = list()
      for line in readouts:
         if len(line) < 2:
            line.append(rdo_utils.CONSTANTS.DEFAULT_READOUT_REST_SERVICE)
         readout_list.append(line)
      
   mgr_service = arglist[4] if len(arglist) == 5 else MGRCONSTS.DEFAULT_MANAGER_REST_SERVICE
      
   return (mgr_host, mgr_user, mgr_service, readout_list)     
      

def state_transition(
   newstate :str, gui : GUI, host: str, user: str, service: str
) -> None:
   global run_number
   
   # This handles stuff that can't be easily done in the multi
   # MVC world.
   
   # If we went to BEGIN state with recording active, 
   # tell the GUI to set the background color else,
   # normal color:
   
   logger = Logger(host, user, service)
   kvstore = KVStore(host, user, service)
   if run_number < 0:
      run_number = kvstore.run()    # Initial run number.
   if newstate == 'BEGIN':
      
      if logger.isRecording():
         gui.setRecordingBackground()
      else:
         gui.setNormalBackground()
      # Save the run number for transitions to idle:
      run_number = kvstore.run()
   else:
      gui.setNormalBackground()   
   
   if newstate == 'END':
      
      # increment the run number if recording:
      # We joined in the middle if run_number == -1:
      if logger.isRecording() and run_number >= 0:
         run_number += 1
         kvstore.setRun(run_number)
         gui.runInfo().setRun(run_number)
         gui.runInfo().model().setRequestedRun(run_number)
         kvstore.setRun(run_number)


def main():
   if len(sys.argv) < 4 or len(sys.argv) > 5:
      Usage()
      sys.exit(-1)

   mgr_host, mgr_user, mgr_service, readout_list = process_arguments(sys.argv)
         
   # Set up the GUJI:
   
   app = QApplication(sys.argv)
   main_win = QMainWindow()
   
   gui = GUI(main_win)
   main_win.setCentralWidget(gui)
   
   # Create the controllers for the  views.
   # We hold them here to prevent them from being garbage collected
   # away...though we don't need to do anything with them:
   
   
   _controllers = createControllers(
      gui, mgr_host, mgr_user, mgr_service, readout_list
   )
   
   # We want to monitor state transitions to do some stuff
   # that's cross controller..
   
   state_poller = rdo_utils.StatePollFactory.getInstance(
      mgr_host, mgr_user, mgr_service
   )
   state_poller.stateChanged.connect(
      lambda newstate, gui= gui, host=mgr_host, user=mgr_user, service=mgr_service:
         state_transition(newstate, gui, host, user, service)
   )
   
   # @todo - figure out how to increment the runnumber if
   # event recording was active.
   
   main_win.show()
   return(app.exec())
   

if __name__ == '__main__':
   sys.exit(main())