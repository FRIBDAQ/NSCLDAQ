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
@file EventBuilderController.py
@brief Code that links together the user interface for event builders and the manager.
@author Ron Fox

This file contains code that can respond to menu selections in the 
Readout gui.  What we can do is
*  Run a wizard to make a new event builder.
*  Run a wizard to modify an existing builder.
*  Delete a selected event builder.

'''
import nscldaq.readoutgui.EventBuilderConfigGui  # The management views.
from nscldaq.readoutgui.EventBuilder        import EventBuilder
from nscldaq.readoutgui.EventBuilderManager import EventBuilderManager, EVBManagerException


from PyQt6.QtWidgets import QDialog, QMessageBox


def MakeEventBuilder():
    '''
    Controller to create an event builder.
    - Fires up a wizard and, if the wizard is accepted,
    - Pulls info from the wizard, makes a new event builder
      and stuffs it into the manager.
      
    @todo - maybe make failure a bit simpler, since really the only failure
            is a duplicate name-- could ask if we want to overwrite e.g
            and, if not, return to the wizard....
    '''

    wizard = EventBuilderConfigGui.EVBWizard()
    if wizard.exec() == QDialog.DialogCode.Accepted:
        evbDefinition = wizard.get()
        evb = EventBuilder(evbDefinition[2], evbDefinition[1])
        try:
            EventBuilderManager.instance().add(evbDefinition[0], evb)
        except EVBManagerException as e:
            QMessageBox.warning(wizard, "Can't register", f'Failed to add Event builder {evbDefinition[0]} : {e}')
                
    
