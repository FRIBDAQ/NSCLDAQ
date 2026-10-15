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

from PyQt6.QtWidgets import QTableView, QComboBox, QWidget
from PyQt6.QtGui     import QStandardItemModel
from nscldaq.mg_configutils import SaveDialog


class TypeChooser(SaveDialog):
    '''
    Selects a type (in this case a data source type).  Intended for
    use with a list of data source types but, I suppose it might be more
    generally useful.  The types are presented as a combobox.
    '''
    
    def __init__(self, typeList : list[str],  parent : QWidget | None = None):
        super().__init__(QComboBox(), parent)
        
        # Now add the items to the box:
        
        self.workarea().setParent(self)
        self.workarea().addItems(typeList)
        
    def selected(self) -> str:
        '''
        Convenience method to get the selection from the combobox.
        @return str
        
        '''
        return self.workarea().currentText()     


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
    
    