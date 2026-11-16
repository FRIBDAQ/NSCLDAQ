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
@file EventBuilderConfigGui.py
@brief GUI Tools to configure an event builder.

We really have a couple of things to configure:
*  Event builder fragment sources and
*  The event builder itself - which makes use of event builder fragment source
    builds.
    
There are many other minor bits and piecdes (a list view/model of the fragment sourcdes e.g)

The final, top level gui is a wizard that lets you configure the event builder and
its fragment sources...all with a great deal of guidance.

Since editing the event builder is a needed operation, each
GUI allows itself to be loaded from an existing configuration.

'''

from nscldaq.editablelist6 import EditableList
from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import (
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMessageBox,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)


class SourceIdListEditor(QWidget):
    '''
    This widget allows the set of source ids to be edited.
    It is composed of an EditableList widget and a labeled
    lineedit.  The user can add new source id number from the 
    lineedit which are checked to ensure they are positive integers.
    
    We emit no signals of our own, but we provide the following public
    methods:
    
    load - load a set of source ids into the list box.
    get  - Gets the contents of the listbox as a list of ints.
    '''
    def __init__(self, parent : QWidget | None = None):
        '''
        @param parent : QWidget | None - the parent widget if any.
        
        '''
        super().__init__(parent)
        
        # Define the subwidgets laid out primarly horizontally:
        
        self.setLayout(QHBoxLayout())
        
        # The left side is a vertical label and editor:
        
        entryLayout = QVBoxLayout()
        entryLayout.addWidget(QLabel('Source id:', parent))
        self._entry = QLineEdit(self)
        entryLayout.addWidget(self._entry)
        self.layout().addLayout(entryLayout)
        
        self.layout().setAlignment(entryLayout, Qt.AlignmentFlag.AlignVCenter)
        
        # The right side is just the editable list:
        
        self._list = EditableList('Sourcde Ids:', self)
        self.layout().addWidget(self._list)
        
        # Set up the list box:

        self._list.add.connect(self._validatedAdd)   # Add with validation.
        

    
    # Public methods:
    
    def load(self, ids : list[int]) -> None:
        ''' Load the list.  The contents of the list are cleared.
            @param ids - the list of source ids to load into the list box.
        '''
        # Convert the ids to strings:
        
        strIds = [str(x) for x in ids]
        self._list.setList(strIds)
        
    def get(self) -> list[int]:
        '''
        @return list[int] - the contents of the list box as ints.
        '''
        return [int(x) for x in self._list.list()]
    
    # Internal (private) slots).
    
    def _validatedAdd(self) -> None:
        #  If the contents of self._entry are a valid source id
        #  and not already in the list, add it to the list.
        id = self._entry.text()
        
        # Must be a positive integer:
        
        try: 
            intid = int(id)
        except ValueError:
            return QMessageBox.warning(self, 'Bad Id', f'Source ids must be integers you tried {id}')
        
        if intid < 0:
            return QMessageBox.warning(self, 'Bad Id', f' Source ids must be positive, you tried {intid}')
        
        # Source ids must be unique as well:
        
        if intid in self.get():
            return QMessageBox.warning(self, 'Duplicate ID', f'{intid} is already in the list of ids.')
        
        # Finally we can add it:
        
        self._list.appendItem(id)     # Must append the textual item <sigh>
    

# Test code:

if __name__ == '__main__':
    import sys

    from PyQt6.QtWidgets import QApplication

    app = QApplication(sys.argv)
    
    win = SourceIdListEditor()
    win.load([1,2,3])
    win.show()
    
    sys.exit(app.exec())