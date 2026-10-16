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
@file DataSourceMenuController.py
@brief Provides the logic for the Data Source -> list and Data Source -> delete menus
@author Ron Fox

'''


from nscldaq.readoutgui import DataSourceManager, DataSource, DataSourceRegistry, DataSourceViews
from nscldaq.mg_configutils import OkDialog

from PyQt6.QtWidgets import QWidget
class ListController:
    '''
    This should be instantiated by the Data Source -> List method.
    It instantiates a  DataSourceMode, stocks it from the manager,
    Instantiates an OkDialog with  a workarea that consists of  a 
    DataSourceTable.  We connect to the table's detail signal
    When it fires, we bring up an OkDialog wrapping the 
    detail view of the data source.
    
    Our instantiation of the DataSourceTable hides the trash can column.
    
    '''
    def __init__(self, parent : QWidget | None = None):
        '''
        @param parent - the parent of the table widget ok dialog wrapper.
        '''
        self._parent = parent        # So we can properly parent child dialogs.
        model = self._makeDataSourceModel()
        view  = DataSourceViews.DataSourceTable(model, parent)
        view.hideDelete()
        view.detail.connect(self._showDetail)
        dialog = OkDialog(view, parent)
        
        dialog.exec()
        
    # Utilties and internal (private) slots:
    
    def _makeDataSourceModel(self) -> DataSourceViews.DataSourceModel:
        #  Make the data source model:
        
        sourceList : list[tuple[str, DataSource.DataSource]] = []
        sourceDict = DataSourceManager.DataSourceManager.instance().sources()

        # Flatten the dict:
        
        sourceList = [(name, source) for name,source in sourceDict.items()]

        return DataSourceViews.DataSourceModel(sourceList)
    
    def _showDetail(self, sourceName):
        # Called to show the details of a data sourcde.
        # this will be wrapped in an OkDialog.
        
        sourceType = sourceName.split(':')[0]
        source = DataSourceManager.DataSourceManager.instance().sources()[sourceName]
        displayer = DataSourceRegistry.DataSourceRegistry.instance().sourceDisplayer(sourceType)
        dialog    = OkDialog(displayer(source), self._parent)
        dialog.exec()
        
        
        
    