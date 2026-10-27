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
@file EventLogControllers.py
@brief controllers to handle the event log menu entries.
@author Ron Fox

Provides a base class event log controller (BaseController) and three derived classes:
The base class sets up the EventLogListView with the requested enables value
and loads the view with the loggers in the EventLogManager. The view is presented in a
SaveDialog.

The  controllers derived from  BaseControllers are

ListController - Derived from BaseController with enables false.  
    Att aches to the view's selected method and pops up an
    Wrapper dialog showing the characteristics of the logger seleted.
DeleteController - Derived from BaseController with enables false.
    The selected signal is connected to confirm the deletion of
    the selected logger.
EnableController - Derived from BaseController with enables true.
    This connects to the enableTOggled model signal changing
    the enables as needed.

A final controller:

AddController prompts for the type of logger to create, then brings up the prompter
    for that logger's configuration in a Save dialog and, if Save is done properly,
    creates the requested logger and enters it into the EventLogManager.
'''

from nscldaq.readoutgui import EventLogManager, EventLogViews
from nscldaq.mg_configutils import SaveDialog, OkDialog
from PyQt6.QtWidgets import QWidget, QMessageBox


class BaseController:
    '''   
    The base class for the controllers that present an event logger
    list table.
    '''
    def __init__(self, 
        dialogType : OkDialog | SaveDialog, 
        enables : bool = False, 
        parent : QWidget | None = None ):
        '''
        @param dialogType - Class for the dialog that will be created (Ok or Save accept buttons).
        @param enables    - Should the model be created with the enable/disable checks enabled?
        @param parent     - If provided the parent of the controller.  This will be the parent of the
                            dialog (which can control where it pops up).
        '''
        
        # Create the listing and stock its model
        
        self.view = EventLogViews.EventLogList(enables)
        self._loadModel()
        
        # Embed the listing in the requested dialog.
        
        self.dialog = dialogType(self.view, parent)
    
    # Protected methods:
    
    def dialog(self) -> OkDialog | SaveDialog:
        '''
        @return OkDialog | SaveDialog: - The dialog widget (so it can be execed e.g.)
        '''
        return self.dialog
    def model(self) -> EventLogViews.EventLogList:
        '''
        @return EventLogViews.EventLogList  - the model of the list view.
        '''
        return self.view.model()
    def listing(self) -> EventLogViews.EventLogList:
        '''
        @return EventLogViews.EventLogList - the list view widget.
        '''
        
        return self.view
    
    
    #  Utility methods:
    
    # protected:
    def _loadModel(self) -> None:
        # build the list expected by the model's load method and invoke it:
        
        manager = EventLogManager.EventLogManager.instance()
        registry = EventLogManager.EventLogRegistry.instance()
        
        # The listing we need to crate is a list of tuples where:
        # [0]  - Is the name of the logger.
        # [1]  - Is the name of the type of the logger.
        # [2]  - is the enable state of the logger.
        # [3]  - Is the logger object iself.
        
        listing : list[(str, str, bool, object)] = []
        for name, enable, logger in manager.list():
            loggerTypeName = registry.find(logger)
            listing.append((name, loggerTypeName, enable, logger))
        
        self.model().load(listing)
        
    
class ListController(BaseController):
    '''
    Used in listing loggers.  If a logger line is double clicked,
    its configuration is brought up.
    '''
    def __init__(self, parent : QWidget | None = None):
        super().__init__(OkDialog, False, parent)
        self.view().selected.connect(self._showDetail)  # Double click shows logger config details.
    
    #  Internal (private) slots:
    
    def _showDetail(self, name : str) -> None:
        #  Show the configuration details of the named logger.
        # 'name' is assumed to be in the manager as its type is assumed
        # to be int he registry:
        
        manager = EventLogManager.EventLogManager.instance()
        registry = EventLogManager.EventLogRegistry.instance()
        
        # Get the logger object:
        
        _, logger = manager.find(name)
        typeName = manager.find(logger)
        displayerClass = manager.displayer(typeName)
        
        displayer = displayerClass(logger)
        dialog = OkDialog(displayer, self.dialog())  # Place over/near the listing.
        
        dialog.exec()      # Display modally until dismissed
        
        
class DeleteController(BaseController):
    '''
    List the loggers, if the user double clicks and confirms,
    the selected logger is deleted and the list updated by invoking
    the superclass _loadMOdel again.
    '''
    def __init__(self, parent : QWidget | None = None):
        super().__init__(OkDialog, False, parent)
        self.view().selectec.connect(self._confirmDelete)
    
    
    # Internal (private) slots:
    
    def _confirmDelete(self, name : str):
        # Given the named logger, confirm that they want to delete it
        # and, if so, delete and re-load the list:
        
        confirm = QMessageBox.question(self.dialog(), 'Delete logger?', f'Are you sure you want to delete {name}')
        if confirm == QMessageBox.SandardButton.Yes:
            EventLogManager.EventLogManager.instance().remove(name)
            self._load()

def EnableController(BaseController):
    '''
    List the loggers respond to enable/deisable clicks by enabling/disabling
    the corresponding logger inthe
    manager.
    '''
    def  __init__(self, parent  : QWidget, | none = None):
        super().__init__(SaveDialog, True, parent)
        self.model().enableToggled.connect(self._setEnable)
    
    # Internal (private) slots;
    
    def _setEnable(self, name : str, state: bool) -> None:
        # Set the state of the named logger:
        
        manager = EventLogManager.EventLogManager.instance()
        if state:
            manager.enable(name)
        else:
            manager.disable(name)
            
    
#----- Handle adding a logger:
    
    
        
    
        
        

