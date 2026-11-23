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
from nscldaq.mg_configutils import DismissDialog
from nscldaq.readoutgui.EventBuilder import EventBuilder
from nscldaq.readoutgui.EventBuilderManager import (
    EVBManagerException,
    EventBuilderManager,
)
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from PyQt6.QtCore import QModelIndex
from PyQt6.QtWidgets import QDialog, QMessageBox, QWidget


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

    wizard = nscldaq.readoutgui.EventBuilderConfigGui.EVBWizard(mainWindow())
    if wizard.exec() == QDialog.DialogCode.Accepted:
        evbDefinition = wizard.get()
        evb = EventBuilder(evbDefinition[2], evbDefinition[1])
        try:
            EventBuilderManager.instance().add(evbDefinition[0], evb)
        except EVBManagerException as e:
            QMessageBox.warning(wizard, "Can't register", f'Failed to add Event builder {evbDefinition[0]} : {e}')

class ListBasedEditor():
    def __init__(self):
        parent = mainWindow()
        
        self._model = nscldaq.readoutgui.EventBuilderConfigGui.EVBTableModel(parent)
        self._loadModel()
        view = nscldaq.readoutgui.EventBuilderConfigGui.EVBTable(self._model, parent)
        self._dialog = DismissDialog(view, parent)        
        
    def _loadModel(self)  -> None:    
        # load Self.model:
        
        eventBuilders = EventBuilderManager.instance().configs()
        self._model.load(eventBuilders)    
        
class EditBuilders(ListBasedEditor):
    '''
    This class edits one or more event builders.  When constructed,
    it pops up a dismiss dialog wraapping an EvTable listing the
    existing event builders.  The table will hide the delete trash can.
    The Selected signal will pop up a wizard to edit _that_ event builder.
    Accepting the wizard will modify that builder in the event builder
    manager - if it exists.  If it does not exist, the user is prompted
    about making a new one with that name...and if so, a new one is created.
    
    '''
    def __init__(self):
        super().__init__()
        
        # Build the table model:
        
        self._dialog.workarea().selected.connect(self._edit)
        self._dialog.workarea().hideDelete()
        self._dialog.exec()
        
    
        
    def _edit(self, index: QModelIndex) -> None:
        # Called when the user selected (via double click) a
        # builder to edit.
        
        name, pipeconfig, srcconfig = self._model.getDefinition(index)
        wizard  = nscldaq.readoutgui.EventBuilderConfigGui.EVBWizard(self._dialog)
        wizard.load(name, pipeconfig, srcconfig)
        if wizard.exec() == QDialog.DialogCode.Accepted:
            definition = wizard.get()
            builder = EventBuilder(definition[2], definition[1])  # We may toss this but...
            if definition[0] not  in EventBuilderManager.instance().names():
                accept =  QMessageBox.question(
                    wizard, 'New EVB', 
                    f'You changed the name of the event builder to {definition[0]}, make a new event builder?'
                )
                # Only make a new one if they say so.
                
                if accept == QMessageBox.StandardButton.Yes:
                    EventBuilderManager.instance().add(definition[0], builder)
            else:
                EventBuilderManager.instance().replace(definition[0], builder)
            
            self._loadModel()         # Load the model/table.
            self._dialog.workarea().hideDelete()  # Keep the delete function off.

class DeleteBuilders(ListBasedEditor):
    '''
    Instances of this class bring up the event editor list with the
    trash can and, when the trash can is clicked, deletes the corresponding
    event builder.  We cheat by overriding the _edit method of our
    base class to do nothing...and connecting yto the delete  method to do
    our business.
    '''
    def __init__(self):
        
        super().__init__()
        self._dialog.workarea().showDelete()    # show the trashcan.
        self._dialog.workarea().delete.connect(self._delete)  # connect to the trash can.
        self._dialog.exec()
        
    
    # Implement the deleter:
    
    def _delete(self, index : QModelIndex) -> None:

        name, _pipeconfig, _srconfig = self._model.getDefinition(index)
        EventBuilderManager.instance().remove(name)
        
        #  Update the list.
        
        self._loadModel()
        self._dialog.workarea().showDelete()