'''
   Provides a controller class that integrates the ReadoutStatistcs Model/View
   with the managed experiment environment given a Readout in it runs the
   ReST server.
   
   @file ReadoutStatisticsController.py
   @brief Controller mediating between the readout statistics MV and the manager and a Readout.
   @author Ron Fox
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


from nscldaq.readoutREST.readoutRestClient import ReadoutClient
from nscldaq.readoutREST.rdo_utils         import getReadoutHost
from nscldaq.readoutREST.rdo_utils         import CONSTANTS as RDO_CONSTS
from nscldaq.manager_client import CONSTANTS as MGRCONSTS


from PyQt6.QtCore import QObject, QTimer
from collections import namedtuple
import getpass


Constants = namedtuple('Constants', ['POLL_MS',])
CONSTANTS = Constants(POLL_MS = 1000)             # Milliseconds between polls.
class ReadoutStatisticsController(QObject):
   '''
      Provides a controller to mediate between a model and 
      view that are compatible with RunStatisticsModel/RunStatistics.
      Note that all that is required is that the view implement the model method
      and that the model implement the setStatistics method in a manner compatible
      with those classes.
      
      We periodically get the statistics from the named Readout and and 
      push it into the model.  The MV interaction takes care of the rest.
      We are tolerant of Readout agility in the sense that for each 
      poll, we locate the Readout again and translate the service into a port.
   '''
   
   def __init__(
      self, view : QObject,  rdo_name : str,
      mgr_host : str, mgr_user : str | None = None,
      mgr_service : str = MGRCONSTS.DEFAULT_MANAGER_REST_SERVICE,
      rdo_service : str = RDO_CONSTS.DEFAULT_READOUT_REST_SERVICE, 
      parent : QObject |None = None
   ): 
      '''
         @param view     - The view object that's connected to the model.
         @param rdo_name - Name of the Readout program we're connecting to.
         @param mgr_host - host in which the manager is running.
         @param mgr_user  - User running the manager, the default value of None will user the current user.
         @param mgr_service - The ReST service advertised by the manager, defaults to  the manager's normal value.
         @param rdo_service - The ReST service advertised by the  readout program, defaults to 'ReadoutREST'
                        which is the default value.
         @param parent    - The parent of this QObject, if not supplied defaults to None.
         
         Only view, rdo_name and mgr_host are mandatory.
      '''
      
      super().__init__(parent)   
      
      self._view   = view    # In the unlikely event the view's model changes with time.
      
      # Save the readout parameters:
      
            
      self._rdo_name = rdo_name
      self._rdo_service = rdo_service

      # Save the manager parameters:
      
      self._mgr_host = mgr_host
      self._mgr_user = mgr_user
      self._mgr_service = mgr_service

      #   Initially populate the model.
      
      self._update()
      
      # Set up the periodic update:
      
      self._timer = QTimer()
      self._timer.setInterval(CONSTANTS.POLL_MS)
      self._timer.setSingleShot(False)
      self._timer.timeout.connect(self._update)
      self._timer.start()
      
   def _update(self) -> None:
      #  This method does all the work:
      #  - Figure out the user name to use:
      #  - figure out where the Readout is running, 
      #  - generate a ReST client for it
      #  - get the statistics dict 
      #  - pass it into the model.
      #
      username = self._mgr_user if self._mgr_user is not None else getpass.getuser()  
      readout_host = getReadoutHost(self._mgr_host, self._mgr_user, self._mgr_service, self._rdo_name)
      client       = ReadoutClient(readout_host, self._rdo_service, username)
   
      # If we can't get the statistics we might be in the shutdown state
      # so try again next poll interval.
      try:
         statistics = client.getStatistics()
      except Exception:
         return
      #
      #  If the respons was Ok, then we can marshall it off to the model.
      #  Otherwise, we'll asssume that this will all work next time...
   
      if statistics['status'] == 'OK':
         modelstats = dict()
         modelstats['cumulative'] = statistics['cumulative']
         modelstats['perRun']     = statistics['perRun']
         self._view.model().setStatistics(modelstats)
      
      
      
      
# Testing code:

if __name__ == '__main__':
   from PyQt6.QtWidgets import QApplication, QMainWindow
   import nscldaq.readoutREST.ReadoutStatistics as ReadoutStatistics
   import sys

   
   app  = QApplication(sys.argv)
   win  = QMainWindow()
   
   view = ReadoutStatistics.RunStatistics(win)
   win.setCentralWidget(view)
   controller = ReadoutStatisticsController(
      view, 'Readout_readout', 'localhost', parent=view
   )
   
   
   win.show()
   sys.exit(app.exec())