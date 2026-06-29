'''
    Provides a controller that mediates between the readout status MV
    and the managed experiment environment.
    
    Note that this controller, needs to know the ReST services
    of each Readout, as well as the names of the Readout programs in the
    manager.  
    
    @file ReadoutStatusControler.py
    @brief controller between the manager server/readout ReST servers and the ReadoutStatus MV.
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



from nscldaq.manager_client import Programs
import nscldaq.readoutREST.rdo_utils as rdo_utils
import nscldaq.readoutREST.readoutRestClient as readoutRest

from PyQt6.QtCore import QObject, QTimer

import getpass

class ReadoutStatusController(QObject):
    '''
        The controller object.  We assume there's a view object
        that is compatible with QAbstractView and that its model
        is compatible with ReadoutStatusModel.  
    '''
    
    def __init__(self,
        view     : QObject,
        readouts : list[tuple[str, str | None]],
        mgr_host : str, 
        mgr_user : str | None = None,
        mgr_service : str = 'DAQManager',
        parent : QObject | None = None
    ):
        '''
            @param view - the view object.. It's model() method returns the model.
            @param readouts - a list of two element tuplse. The first element of each
                    tuple is a Readout program name.  The second is the ReST service if not
                    ReadoutREST None if it is that default value.
            @param mgr_host - the host in wich the manager is running.
            @param mgr_user - The user running the manager if not the current user, None if it is.
            @param mgr_service - The ReST service of the manager, defaults to DAQManager, the default
                service name.
            @param parent - our parent object if there is one. None if not.
            
            @note we'll create the manager client and ReST client each poll interval so here
                all that's needed is to save the paramters.
        '''
    
        super().__init__(parent)
        
        self._view = view
        self._readouts = readouts    
        
        self._mgrhost = mgr_host
        self._user = mgr_user if not None else getpass.getuser()  # Readouts run on the manager's user.
        self._mgrsvc = mgr_service

        self._poll()                  # Update is just as good as load...get the initial model load.
        
        # Set up timed updates.
        
        self._timer = QTimer(self)
        self._timer.setInterval(rdo_utils.CONSTANTS.POLL_MS)
        self._timer.setSingleShot(False)
        self._timer.timeout.connect(self._poll)
        self._timer.start()
        
        
    def _poll(self) -> None:
        # Handle the periodic update.
        
        # Collect the information about the readouts in a dict keyed by their names.
        
        # First get the programs nad their data:
        
        mgr_client = Programs(self._mgrhost, self._user, self._mgrsvc)
        info =  mgr_client.status()
        if info['status'] != 'OK':
            raise RuntimeError('Did not get an OK response from the manager listing programs')
        all_programs = info['programs']

        
        # Generate the  list of readout programs:
        
        readout_programs = list()
        for program in all_programs:
            for name, service in self._readouts:
                if program['name'] == name:
                    program['service'] = 'ReadoutREST' if service is None else service
                    readout_programs.append(program)
                    break
        
        
        # Now get the state element of all the readouts.
        
        for readout in readout_programs:
            readout_client = readoutRest.ReadoutClient(readout['host'], readout['service'], self._user )
            try:
                info = readout_client.getState()
                if info['status'] == 'OK':
                    state = info['state']
                else:
                    state = 'error'
            except Exception as e:
                state = 'unresponsive'
            readout['state'] = state
        
        # Now update the model:
        
        self._view.model().update(readout_programs)
    
# Test code:

if __name__ == '__main__':
    from nscldaq.readoutREST.ReadoutStatus import ReadoutStatus
    from PyQt6.QtWidgets import QApplication
    import sys
    import getpass

    
    readouts = [('Readout_readout', None), ]   # OUr readout definition tuple.
    user = getpass.getuser()
    
    app = QApplication(sys.argv)
    widget = ReadoutStatus()
    widget.show()
    
    controller = ReadoutStatusController(widget, readouts,'localhost', user, parent=widget)
    
    
    sys.exit(app.exec())
    
    
    