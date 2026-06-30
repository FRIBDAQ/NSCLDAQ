'''
 This file provides a controller that mediates between the 
 model and view supporting timed runs an the runs themselves.
 Specifically, we're responsible for:
 - Disabling the user interface when the run active 
 - Closing the loop between the model and view.
 - Catching the run end request from the model
   and turning it into a state transitino request for the manager.
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



from PyQt6.QtCore import QObject, QTimer
from nscldaq.readoutREST.rdo_utils import StatePollFactory
from nscldaq.manager_client        import State
from nscldaq.manager_client        import CONSTANTS as MGRCONSTS
from parse import parse
class TimedRunController(QObject):
    '''
        This controller handles the interaction between the
        TimedRun MV elements and connects this all to the
        manager state machine itself.
        
    '''
    def __init__(self, 
        view : QObject, 
        mgr_host : str, mgr_user : str|None = None ,
        mgr_service : str  = MGRCONSTS.DEFAULT_MANAGER_REST_SERVICE, 
        parent: QObject | None = None
    ):
        '''
         @param view - the view (which contains the model) must be compatible with 
                       TimedRunView.
        @param mgr_host - host in which the manager is running.
        @param mgr_user - (Optional) user running the manager, if omitted or None, the current user
                        is assumed.
        @param mgr_service (Optional) The ReST service advertised by the manager.
                        if omitted,  the default service is used.
        '''
        
        super().__init__(parent)
        
        # Save the parameters for later.
        
        self._view = view
        self._host = mgr_host
        self._user = mgr_user
        self._service = mgr_service
        
        # Make a state client.
        
        self._client = State(mgr_host, mgr_user, mgr_service)
        self._poller = StatePollFactory.getInstance(mgr_host, mgr_user, mgr_service)
        
        # Poll the state to handle widget ghosting.
        
        self._poller.stateChanged.connect(self._stateChanged)
        
        # Set the initial stae.
        self._stateChanged(self._client.status())

        # Connect to the required set of view signals:
        
        self._view.desiredChanged.connect(self._newLength)
        self._view.timedChanged.connect(self._newEnable)
        
        # The model will tell us when to end the run:
        
        self._view.model().runExpired.connect(self._endRun)
        
        # We need to update the elapsed time from time to time:
        
        self._timer = QTimer()
        self._timer.setSingleShot(False)
        self._timer.setInterval(500)    # So we're not lagging.
        self._timer.timeout.connect(self._setElapsed)
        self._timer.start()

    #  Private slots.
    
    def _setElapsed(self) -> None:
        # Update the elapsed time if it's not inactive:
        
        elapsedTime = self._client.elapsed()
       
        if elapsedTime != '*Inactive*':
        
            # Note elapsedTime has no days so we need to cook it a bit:
            
            result = parse('{hours:d}:{minutes:d}:{seconds:d}', elapsedTime)
            timeDict = result.named
            hrs    = timeDict['hours'] % 24
            days  = int(timeDict['hours']/24)
            
            elapsedTime = f'{days:02d} {hrs:02d}:{timeDict["minutes"]:02d}:{timeDict["seconds"]:02d}'
            
            self._view.model().setElapsedTime(elapsedTime)
    
    def _stateChanged(self, state : str) -> None:
        # Set the enable/disable for the view:
        
        if state == 'BEGIN':
            self._view.setEnabled(False)
        else:
            self._view.setEnabled(True)
            
        self._setElapsed()
        
            
    def _newLength(self, desired : dict) -> None:
        # set the new desired run length to the desired dict.
        
        timestr =  \
            f'{desired["days"]:02d} {desired["hours"]:02d}:{desired["minutes"]:02d}:{desired["seconds"]:02d}'
        self._view.model().setDesiredLength(timestr)
        
    def _newEnable(self, isTimed : bool) -> None:
        # the timed run toggle changed:
        
        self._view.model().setLimitRun(isTimed)
        
    def _endRun(self) -> None:
        # Note it's possible the state changed out from underneath us so tolerate
        # Failure:
        
        try:
            self._client.transition('END')
        except Exception:
            pass
        

## Test code:
if __name__ == '__main__':
    
    from PyQt6.QtWidgets import QApplication, QMainWindow
    from nscldaq.readoutREST.TimedRun import TimedRunView
    import sys

    app = QApplication(sys.argv)
    win = QMainWindow()
    
    view = TimedRunView(win)
    win.setCentralWidget(view)
    controller = TimedRunController(view, 'localhost')
    
    win.show()
    sys.exit(app.exec())
    



