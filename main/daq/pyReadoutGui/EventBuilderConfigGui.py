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
from PyQt6.QtGui import QIntValidator
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
        self._entryValidator = QIntValidator(self._entry)
        self._entryValidator.setBottom(0)
        self._entry.setValidator(self._entryValidator)
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
        #  Note our validator takes care of this being integer and
        #  positive.
        id = self._entry.text()
        
        # Must be a positive integer:
        
        try: 
            intid = int(id)
        except ValueError:
            # Defensive code that should not be needed.
            return QMessageBox.warning(self, 'Bad Id', f'Source ids must be integers you tried {id}')
        
        # Source ids must be unique as well:
        
        if intid in self.get():
            return QMessageBox.warning(self, 'Duplicate ID', f'{intid} is already in the list of ids.')
        
        # Finally we can add it:
        
        self._list.appendItem(id)     # Must append the textual item <sigh>

class FragmentSourceEditor(QWidget):
    '''
    This edits all of the parameters for a ring fragment source.
    The assumption is that this is part of a larger configuration system
    that is configuring an event bulder >and< that the host and service on
    which the event builder are listening for fragment source connections
    is already known and not editable...these are displayed a labels.
    The remaninder of the configuration are described in EventBuilder.py
    under the docstring for the FragmentSource class.
    
    Methods:
        load - loads an exiting configuration dict into the widget.
        get  - retrieves the configuration as a usable configuration dict.
    '''
    def __init__(self, evbhost : str, evbservice : str, parent : QWidget | None = None):
        '''
        Construction parameters are:
        @param evbhost - the host in which the event builder will run.
        @param evbservice - the service the event builder will advertise in that
                            host for fragment connections.
        @parent - the optional paranet widget (defaults to None).
        '''
        super().__init__(parent)
        
        #  The user interface is a bunch of vertical strips of stuff on the left and
        #  a SourceIdListEditor on the right.
        
        self.setLayout(QHBoxLayout())
        
        leftLayout  = QVBoxLayout()       # For the strips.
        
        
        # Bulid up the strips on the left.
        
        evbinfoStrip = QHBoxLayout()
        evbinfoStrip.addWidget(QLabel('EVB Host:', self))
        self._evbhost = QLabel(evbhost, self)
        evbinfoStrip.addWidget(self._evbhost)
        evbinfoStrip.addWidget(QLabel('Service', self))
        self._evbservice = QLabel(evbservice, self)
        evbinfoStrip.addWidget(self._evbservice)
        leftLayout.addLayout(evbinfoStrip)
        
        # Where the source lives:
        
        hostStrip = QHBoxLayout()
        hostStrip.addWidget(QLabel('Source Host:', self))
        self._host = QLineEdit(self)
        hostStrip.addWidget(self._host)
        leftLayout.addLayout(hostStrip)
                               
        # Description:
        
        descStrip = QHBoxLayout()
        descStrip.addWidget(QLabel('Description:', self))
        self._description = QLineEdit(self)
        descStrip.addWidget(self._description)
        leftLayout.addLayout(descStrip)
        
        # Default source id:
        
        defidStrip = QHBoxLayout()
        defidStrip.addWidget(QLabel('Default Id:', self))
        self._defaultId = QSpinBox(self)
        self._defaultId.setMinimum(0)
        self._defaultId.setMaximum(1 << 30)
        defidStrip.addWidget(self._defaultId)
        leftLayout.addLayout(defidStrip)
        
        # Source ring:
        
        ringStrip = QHBoxLayout()
        ringStrip.addWidget(QLabel('Source ring URI', self))
        self._ring = QLineEdit(self)
        ringStrip.addWidget(self._ring)
        leftLayout.addLayout(ringStrip)
        
        # End run timeout:
        
        eortmoStrip = QHBoxLayout()
        eortmoStrip.addWidget(QLabel('End Run Timeout', self))
        self._endRunTimeout = QSpinBox(self)
        self._endRunTimeout.setMinimum(1)
        self._endRunTimeout.setValue(10)
        self._endRunTimeout.setMaximum(1 << 30)
        eortmoStrip.addWidget(self._endRunTimeout)
        leftLayout.addLayout(eortmoStrip)
        
        # End run count:
        
        eorcountStrip = QHBoxLayout()
        eorcountStrip.addWidget(QLabel('End runs expected', self))
        self._eors = QSpinBox(self)
        self._eors.setMinimum(1)
        self._eors.setMaximum(1 << 30)
        self._eors.setValue(1)
        eorcountStrip.addWidget(self._eors)
        leftLayout.addLayout(eorcountStrip)
        
        # TImestamp offset:
        
        offsetTsStrip = QHBoxLayout()
        offsetTsStrip.addWidget(QLabel('Time stamp offset:', self))
        self._tsOffset = QSpinBox(self)
        self._tsOffset.setMinimum(0)
        self._tsOffset.setMaximum(1 << 30)
        self._tsOffset.setValue(0)
        offsetTsStrip.addWidget(self._tsOffset)
        leftLayout.addLayout(offsetTsStrip)
        
        # Timestamp divisor:
        
        tsDivStrip = QHBoxLayout()
        tsDivStrip.addWidget(QLabel('Time stamp divisor', self))
        self._tsdivisor = QSpinBox(self)
        self._tsdivisor.setMinimum(1)
        self._tsdivisor.setMaximum(1 << 30)
        self._tsdivisor.setValue(1)
        tsDivStrip.addWidget(self._tsdivisor)
        leftLayout.addLayout(tsDivStrip)
    
        self.layout().addLayout(leftLayout)
   
        # Stuff on the right side:
        
        self._sourceIds = SourceIdListEditor(self)
        self.layout().addWidget(self._sourceIds)
    
    def load(self, config : dict[str, object]) -> None:
        '''
        Load the  widget with a configuration.
        Note that items that are not in the configuration are left emtpy.
        All this is take care of by _loadText and _loadInt and using
        get to pull things from the configuration.
        
        @param config : dict[str, object] - a fragment source configuration
            dict.  See the docstring in EventBuilder.FragmentSource for more
            about what that can be.
        '''
        
        self._loadText(self._evbhost, config.get('evbhost'))
        self._loadText(self._evbservice, config.get('evbservice'))
        self._loadText(self._host, config.get('host'))
        self._loadText(self._description, config.get('description'))
        self._loadInt(self._defaultId, config.get('default-id'))
        self._loadText(self._ring, config.get('ring'))
        self._loadInt(self._endRunTimeout, config.get('timeout'))
        self._loadInt(self._eors, config.get('end-runs'))
        self._loadInt(self._tsOffset, config.get('offset'))
        self._loadInt(self._tsdivisor, config.get('ts-divisor'))
        
        ids = config.get('ids')
        if ids:
            self._sourceIds.load(ids)

    
    def get(self) -> dict[str, object] :
        '''
        @return dict[str, object] - The configuration of the event builder.
             Note that empty string items are None...Constructing a fragment
             source with an invalid configuration will raise a ValueError which
             can be caught and reported by the higher level stuff.
        '''
        return {
            'evbhost'    : self._getText(self._evbhost),
            'evbservice' : self._getText(self._evbservice),
            'host'       : self._getText(self._host),
            'description': self._getText(self._description),
            'ids'        : self._sourceIds.get(),
            'default-id' : self._defaultId.value(),
            'ring'       : self._getText(self._ring),
            'timeout'    : self._endRunTimeout.value(),
            'end-runs'   : self._eors.value(),
            'offset'     : self._tsOffset.value(),
            'ts-divisor' : self._tsdivisor.value()
        }
    
    # Utilities: 
    
    @staticmethod
    def _loadText(w : QLineEdit | QLabel, value :str | None) -> None:
        if value:
            w.setText(value)

    @staticmethod
    def _loadInt(w : QSpinBox, value : int | None) -> None:
        if value:
            w.setValue(value)
    @staticmethod 
    def _getText(w : QLineEdit | QLabel) -> str | None:
        value = w.text().strip()
        return value if value else None
    
    
# Test code:

if __name__ == '__main__':
    import sys

    from PyQt6.QtWidgets import QApplication, QPushButton

    app = QApplication(sys.argv)
    
    win = QWidget()
    win.setLayout(QVBoxLayout())
    editor = FragmentSourceEditor('localhost', 'Orderer:ron', win)
    win.layout().addWidget(editor)
    
    editor.load({
        'host' : 'spdaq32',
        'evbhost' : 'daqcompute01',
        'description' : 'This comes from spdaq32',
        'ids' : [1,2,3,4],
        'default-id' : 1,
        'ring' : 'tcp://spdaq32/ron',
        'timeout' : 5,
        'offset' : 100,
        'ts-divisor': 2,
        'end-runs' : 4
    })
    
    get = QPushButton('Get', win)
    get.clicked.connect(lambda: print(editor.get()))
    win.layout().addWidget(get)
    
    win.show()
    
    sys.exit(app.exec())