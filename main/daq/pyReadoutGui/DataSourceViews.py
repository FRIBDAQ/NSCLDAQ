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
@file DataSourceViews.py
@brief Provide megawidgets in support of the Data Source menu operations.
@author Ron Fox
'''

from nscldaq.mg_configutils import SaveDialog
from nscldaq.readoutgui.DataSource import DataSource
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from PyQt6.QtCore import QModelIndex, QObject, Qt, pyqtSignal
from PyQt6.QtGui import QStandardItem, QStandardItemModel
from PyQt6.QtWidgets import QComboBox, QStyle, QTableView, QWidget, QHBoxLayout, QLabel


class TypeChooser(SaveDialog):
    '''
    Selects a type (in this case a data source type).  Intended for
    use with a list of data source types but, I suppose it might be more
    generally useful.  The types are presented as a combobox.
    

    '''
    
    def __init__(self, typeList : list[str],  parent : QWidget | None = None):
        self._form = QWidget()    # Will be the workarea.
        self._form.setLayout(QHBoxLayout())
        self._form.layout().addWidget(QLabel('Choose Data Source Type', self._form))
        self._selection = QComboBox(self._form)
        self._form.layout().addWidget(self._selection)
        
        super().__init__(self._form, parent)
        
        # Now add the items to the box:
        
        self.workarea().setParent(self)
        self._selection.addItems(typeList)
        
    def selected(self) -> str:
        '''
        Convenience method to get the selection from the combobox.
        @return str
        
        '''
        return self._selection.currentText()     


class DataSourceModel(QStandardItemModel):
    '''
    This is a model containing the data for a list of data sources.
    Each row of a model contains:
    - Data source type (with data containing the full data source name)
    - Data source host - where appropriate.
    - Data source program - where appropriate
    - Deletion icon which may or may not  be displayed depending on the view.
      this is SP_TrashIcon.
    '''
    
    def __init__(self, sources : list[tuple[str, DataSource]], parent : QObject | None = None) :
        '''
        @param sources : list[tuple[str, DataSource]] - List of sources.
        @param parent : QObject | None = None optional parent model.
        '''
        
        
        super().__init__(parent)
        
        # Save the trashcan icon since we'll us it in more than one row:
        
        icon_attr = getattr(QStyle.StandardPixmap, 'SP_TrashIcon')
        self._trashIcon = mainWindow().style().standardIcon(icon_attr)
        self.load(sources)
        
    # Utilities:
    
    def load(self, sources : list[tuple[str, DataSource]]) -> None:
        '''
        Clear the model and load with new data
        
        @param sources : list[tuple[str, DataSource]] - List of sources.
        '''
           
        self.clear()
    
        for name, source  in sources:
            self._addRow(name, source)
            
        self._setHeaders()
        
                
    def _setHeaders(self) -> None:
        # Set the model header lables.
        
        self.setHorizontalHeaderLabels(['Type', 'Host', 'Program', 'Output Ring', ''])
        
    def _addRow(self, source_name : str, source : DataSource):
        source_type = source_name.split(':')[0]
        stype = QStandardItem(source_type)
        stype.setData(source_name)
        
        # Figure out the strings for host and program:
        
        config = source.getConfig()
        host   = QStandardItem(config.get('host', 'N/A'))
        pgm    = QStandardItem(config.get('program_path', 'N/A'))
        output = QStandardItem(config.get('ring', 'N/A'))
        trashcan = QStandardItem(self._trashIcon, '')
        
        self.appendRow([stype, host, pgm, output, trashcan])
        
    
class DataSourceTable(QTableView):
    '''
    View for a list of data sources.  This can be used both for the delete
    and list operations.  If used for list, invoke hideDelete to remove the 
    column contaning trashcan icons.
    
    Signals:
      delete(str)  - Trash can clicked, The string passed is the name of the source.
      detail(str)  - Anything on the line was double clicked (except the trash can),
                     The string is the name of the data source, typically this is used
                     to bring up the details of the source.
                     
    Note that names here means names of the form sourcetype:uuid 
    (assumed to be storedd in the user data for column 0).
    '''
    delete = pyqtSignal(str)
    detail = pyqtSignal(str)
    
    def __init__(self, model : DataSourceModel, parent : QWidget | None = None):
        '''
        @param model  : DataSourceModel- A data source model containing the data source info.
        @param parent : QWidget | None = None- Optional parent widget.
        '''
        
        super().__init__(parent)
        self.setModel(model)
        
        # Setup signal relays to our private slots which will map them
        # to our delete and detail signals.
        
        self.clicked.connect(self._clicked)
        self.doubleClicked.connect(self._doubleClicked)
        
        # Make the window a bit wider than default:
        
        self.resize(800, self.height())
    
    def hideDelete(self):
        '''
        Hides column number 4 - which holds the trashcan icon:
        '''
        self.hideColumn(4)
        
    def showDelete(self):
        '''
        Shows the delete column:
        '''
        self.showColumn(4)
    
        
      
    #  Private (internal) slots:
    
    def _clicked(self, index : QModelIndex) -> None:
        #  Single click slot.  If the 
        #  index is column 4, fetch the name of the
        #  object and emit delete.
        
        if index.column() == 4:
            self.delete.emit(self._getSourceName(index))
    
    def _doubleClicked(self, index : QModelIndex) -> None:
        #   Double clicked.  Emit the detail signal for this row.
        
        self.detail.emit(self._getSourceName(index))
    
    # Utitlities:
    
    def _getSourceName(self, index : QModelIndex) -> str:
        # Return the name of the row of index:
        
        name_index =  index.siblingAtColumn(0)
        return self.model().itemFromIndex(name_index).data(Qt.ItemDataRole.UserRole+1)
        
# Test code
if __name__ == '__main__':
    import sys
    from PyQt6.QtWidgets import QApplication, QDialog
    
    app = QApplication(sys.argv)
    
    dlg = TypeChooser(['a', 'b', 'c' ,'d'])
    
    if dlg.exec() == QDialog.DialogCode.Accepted:
        print(dlg.selected())
    else:
        print('rejetcted')
    
    