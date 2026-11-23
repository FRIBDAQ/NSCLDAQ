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
import getpass
import urllib.parse

from nscldaq.editablelist6 import EditableList
from nscldaq.mg_configutils import SaveDialog
from nscldaq.readoutgui.ReadoutGuiView import mainWindow
from PyQt6.QtCore import QModelIndex, QObject, Qt, pyqtSignal
from PyQt6.QtGui import QIntValidator, QStandardItem, QStandardItemModel
from PyQt6.QtWidgets import (
    QCheckBox,
    QComboBox,
    QDialog,
    QGridLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMessageBox,
    QPushButton,
    QSpinBox,
    QStyle,
    QTableView,
    QVBoxLayout,
    QWidget,
    QWizard,
    QWizardPage,
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
        self._entry.returnPressed.connect(self._validatedAdd) # enter is also add.
        
    
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
        self._entry.setText('')       # Clear the  entry for the user.

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
    
class FragmentSourceListModel(QStandardItemModel):
    '''
    This standard item model provides a model for a list of
    fragment source configuration dicts.  The model provides
    a view only the host, and ringbuffer and description for each 
    configuration.  Given a Model index, however, it is 
    possible to pull out the entire configuration for that fragment.
    The idea is that this allows interaction with the view to 
    initiate a FragmentSourceEditor already loaded with that configuration,
    for editing.
    Methods:
       load - loads the model.
       getConfig - Given a model index, gets the configuration of that row.
       
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # Set up the model with an empty list of sources:
        # We do this to set up the table headers so we don't just have
        # an initially empty widget.    
        self.load({})

    def load(self, configs : list[dict[str, object]]) -> None:
        self.clear()
        self._configs = []
        self.setHorizontalHeaderLabels(['Host', 'Ring URI', 'Description'])
        
        for config in configs:
            self.addRow(config)

    def getConfigs(self) -> list[dict[str, object]]:
        '''
        @return list[dict[str, object]] - All of the configurations in the model.
        '''
        return self._configs
        
    
    def getConfig(self, index :QModelIndex) -> dict[str, object] | None:
        '''
        @param index - the model index whose configuration we'll return.
        @return dict[str, object] - The configuration for that row.
        @retval None - the row is out of range.
        '''
        
        row = index.row()
        if len(self._configs) > row:
            return self._configs[row]
        else:
            return None
    
    def replaceRow(self, row : int, config : dict[str, object]):
        '''
        Repaces  the configuration in a row with a new config and
        refreshes. 
        @note we assume there are not that many fragment sources.
        
        @param row :  int - the row number to replace.
        @param config : dict[str, object]- new config for that row.
        '''
        newconfig = self._configs
        newconfig[row] = config
        self.load(newconfig)
               
    
    def addRow(self, config : dict[str, object]) -> None:
        '''
        Appends a row to the model:
        @param config : dict[str, object] - the configuraition to add.
        '''
        
        self._configs.append(config)
        host = QStandardItem(config.get('host', ''))
        ring = QStandardItem(config.get('ring', ''))
        desc = QStandardItem(config.get('description', ''))
    
        self.appendRow([host, ring, desc])

class FragmentSourceListView(QTableView):
    '''
    Intended to be used with a FragmentSourceListModel to list
    pertinent chunks of a list of fragment sources.  
    Signals:
        selected(int, dict[str, object]) - a double click the parameters
                      are  the  row and configuration of that row.
    '''
    selected = pyqtSignal(int, dict)    # Can't give full type hint.
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        self.doubleClicked.connect(self._select)
    
    def _select(self, index : QModelIndex) -> None:
        # Relay the doublie clicked signal to selected:
        
        row = index.row()
        config = self.model().getConfig(index)
        if config:
            self.selected.emit(row, config)
    
class FragmentSourceDefiner(QWidget):
    '''
    This class provides a fragment source editor.  Given a  list of
    fragment source descriptions it supports:
    - Double-clicking a source to edit its definition.
    - Clicking a + button to create a new source definition and add it to
      the table.
    - Fetch the  list of configuration from the table of sources.
    Methods:
    load - loads the definitions in to the table.
    get  - gets the final definitions back out
    
    @note the intent is for this to be a page in a wizard that
          defines or edits event builders and their sources.
    No signals are emitted from this widget.
    '''
    def __init__(self,  parent : QWidget | None = None):
        '''
        @param parent : QWidget|None - The parent widget if supplied, defaults to None.
        '''
        
        super().__init__(parent)
        
        
        
        # We have a table on the left and a + button below it.
        
        self.setLayout(QVBoxLayout())
        
        self._table = FragmentSourceListView(self)
        self._table.setModel(FragmentSourceListModel(self))
        self.layout().addWidget(self._table)
        
        self._addButton = QPushButton('Add Source', self)
        self.layout().addWidget(self._addButton)
    
        # Connect the signals to internal slots:
        
        self._table.selected.connect(self._editSource)
        self._addButton.clicked.connect(self._addSource)
    
    # In a wizard, we might need to set the evb host and service externally
    
    def setEvbParameters(self, evbhost : str, evbservice : str) -> None:
        '''
        @param evbhost - where the event buildeer pipeline is running.
        @param evbservice - the service the orderer is advertising there.
        '''
        
        self._evbhost = evbhost
        self._evbservice = evbservice
                
        
    def load(self, configurations : list[dict[str, object]]) -> None:
        '''
        Load the table with configurations.
        @param configurations - a list of fragment source definitions.
            
        '''
        self._table.model().load(configurations)
    
    def get(self) -> list[dict[str, object]]:
        '''
        @return list[dict[str, obect]] - The list of configurations
                in the table
        '''
        return self._table.model().getConfigs()
    
    # Utilities:
    
    def _configureSource(self, config : dict[str, object] | None = None) -> dict[str, object] | None:
        # Sets of a fragment source editor in a Save dialog.
        # - If the dialog is saved, and there's a valid configuration, 
        #  it is returned to the caller. 
        # - If the dialog is not saved, None is returned.
        
        form = FragmentSourceEditor(self._evbhost, self._evbservice)  # always use _ours_
        if config:
            form.load(config)      # IF there is a configuration load it.
        
        dialog = SaveDialog(form, self)
        while dialog.exec() ==  QDialog.DialogCode.Accepted:
            result =  form.get()
            if not self._validRing(result):
                QMessageBox.warning(self, 'Bad ring', 
                    'The ring buffer specification must be a URI  of the form "tcp://host/ringname"')
            else:
                return result
        return None        # Not accepted.
    
    def _validRing(self, config : dict[str, object]) -> bool:
        # Validate a configuration's ring buffer:
        
        ring = config['ring'].strip()
        if not ring:
            return False
        # Must have tcp scheme.
        
        return urllib.parse.urlsplit(ring).scheme.lower() == 'tcp'
        
    
    # The slots below are for editing can existing or new configuration.
    # They just figure out how to call _configureSource and what to do if
    # it returns a configuration:
    
    def _editSource(self, row, config) -> None:
        # Edit an existing source:
        
        modifiedConfig = self._configureSource(config)
        if modifiedConfig:
            self._table.model().replaceRow(row, modifiedConfig)
    
    def _addSource(self) -> None:
        #  Same as _editSource but we have no starting point and
        # the configuration is appended to the list.
        
        newConfig = self._configureSource()
        if newConfig:
            self._table.model().addRow(newConfig)
    
# Ok, so here's the event builder wizard pages and the wizard itself.
# There is conditional branching in the wizard as there are some
# advanced event builder configuration parameters that are only
# presented to the user if requested.
# pages are:
#    10 - Main EVBPipe page 
#    20 - EVBPipeAdvanced page [only seen if "Edit Advanced Parameters checkbox set in page 10"]
#    30 - DataSource page (the last page).

class EvbMainParameters(QWizardPage):
    '''
    This page accepts the base event builder parameters which are fields like:
    'host'  - where the event builder lives.
    'service_suffix' - The event builder service suffix.
    'dt'    - The glom build window.
    'sourceid' - The output source id.
    'ring'  - The event builder output ring.
    
    Thereis a checkbox which, if checked sends us to page 20 as the next page
    otherwise, the next page is 30.
    
    Note that the 'load' method can be called from the wizard's currentIdChanged
    the first time this page is displayed to pre-load the widgets with
    values from an event builder configuration.
    
    '''
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        self.setTitle("Event Builder parameters")
        self.setSubTitle('Main parameters set')
        self._initialized = False
        
    def initializePage(self) -> None:
        '''
        Initializes the form for the page, and a few other things.
        We'll use a grid layout so stuff lines up nicely.
        '''
        if not self._initialized:
            self.setLayout(QGridLayout())
            
            # The event builder has a name, since the application can manage
            #  more than one event builder.  The name is mandatory.
            
            self._name = QLineEdit(self)
            self.layout().addWidget(self._name, 0,0)
            self.layout().addWidget(QLabel('Event Builder name', self), 0,1 )
            self.registerField('name', self._name)
            
            # Up at the top, the host
            # This is a mandatory field named host
            
            self._host = QLineEdit(self)
            self.layout().addWidget(self._host, 1,0)
            self.layout().addWidget(QLabel('Event Builder host', self), 1, 1, )
            self.registerField('host', self._host)
            
            # Now the service suffix, which is not mandatory.  It allows
            # multiple event builder pipelines to run in the same host
            # with unique service advertisements
            
            self._suffix = QLineEdit(self)
            self.layout().addWidget(self._suffix, 2,0, )
            self.layout().addWidget(QLabel('Service suffix (optional)', self), 2,1, )
            self.registerField('service_suffix', self._suffix)
            
            # The event builder coincidence window (glom)
            
            self._dt = QSpinBox(self)
            self._dt.setMinimum(0)
            self._dt.setMaximum(1 << 30)
            self._dt.setValue(100)
            self.layout().addWidget(self._dt, 3,0, )
            self.layout().addWidget(QLabel('Event build window in ticks', self), 3,1, )
            self.registerField('dt', self._dt)
            
            #  The output source id (glom):
            
            self._sourceid = QSpinBox(self)
            self._sourceid.setMinimum(0)
            self._sourceid.setMaximum(1 << 30)
            self._sourceid.setValue(0)
            self.layout().addWidget(self._sourceid, 4,0, )
            self.layout().addWidget(QLabel('Source id of built data'), 4,1, )
            self.registerField('sourceid', self._sourceid)
            
            # Output ring (mandatory):
            
            self._ring = QLineEdit(self)
            self.layout().addWidget(self._ring, 5,0, )
            self.layout().addWidget(QLabel('Output ringbuffer name for event built data', self), 5,1, )
            self.registerField('ring', self._ring)
            
            # Checkbox that allows presentation of advanced parameters.
            
            self._advanced = QCheckBox(self)
            self._advanced.setCheckState(Qt.CheckState.Unchecked)
            self.layout().addWidget(self._advanced, 6,0, )
            self.layout().addWidget(QLabel('Configure advanced parameters', self), 6,1, )
            
            self._initialized = True
        
    def load(self, name: str, config : dict[str, object]) -> None:
        '''
        Should be called after initializePage - this can load the
        values of the input widget from a EventBuilderPipeline configuration.
        Note that missing configuration items are not loaded.
        @param name : str - the event builder name.
        @param config : dict[str, object] the event builder configuration.
        '''
        
        self._name.setText(name)
        
        # The textual items:
        for textItem in [('host', self._host), ('service-suffix', self._suffix), ('ring', self._ring)]:
            value = config.get(textItem[0])
            if value:
                textItem[1].setText(value)
        for intItem in [('dt', self._dt), ('sourceid', self._sourceid)]:
            value = config.get(intItem[0])
            if value:
                intItem[1].setValue(value)
    
    def nextId(self) -> int:
        '''
        Only present the advanced parameters page if the advanced checkbox
        is checked.
        '''
        return 20 if self._advanced.checkState() == Qt.CheckState.Checked else 30
        
    def validatePage(self) -> bool:
        '''
        Since changing an initial value with a default might leave empty something mandator:
        @return bool - True if it's ok to move on to the next page
        '''
        if not self.field('name').strip():
            QMessageBox.warning(self, 'Missing name', 'You must give the event builder a non-empty name')
            return False
        
        if not self.field('host').strip():
            QMessageBox.warning(self, 'Missing host', 'The host field must not be empty!')
            return False
        
        if not self.field('ring').strip():
            QMessageBox.warning(self, 'Missing ringbuffer', "The output ring buffer must have a URL")
            return False
        

        return True
        
         
                
class EvbAdvancedParameters(QWizardPage):
    '''
    Optional page to prompt for advanced event builder parameters.
    IMPORTANT This must have page id 20, the next page must be page 30.
    The advanced parameters are:
    flush_window   - the tick window for which fragments are flushed even if not all queues have data.
    xoff_threshold - The global backlog fragment threshold at which the orderer stop accepting fragments
    xon_threshold  - The global backlog fragment threshold at which the orderer is allowed to accept frags.
    perqxon_threshold - Same but in any queue for that queue.
    perqxoff_threshold - Same but in any queue for that queue.
    tspolicy     - Policy used to assign timestamps to the built events.
    maxfragments - Maximum number of fragments that can build up in glom before it flushes an event.
    
    All of these are given default values, which may be overidden by callling load from the 
    wizard's currentChanged slot.  All are optional.
    '''
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        
        self.setTitle('Event Builder parameters')
        self.setSubTitle('Advanced parameters')
        self._initialized = False

    def initializePage(self) -> None:
        '''
        Initialize the form... the class comments describe the fields we'll create.
        I think the most common things modified might be 'maxfragments' and
        tspolicy so those will be at the top:
        '''
        if not self._initialized:
                
            self.setLayout(QGridLayout())
            
            # The timestamp policy first:
            
            self._tspolicy = QComboBox(self)
            self._tspolicy.addItems(['latest', 'earliest', 'average'])
            self._tspolicy.setCurrentText('latest')
            self.layout().addWidget(self._tspolicy, 0,0, )
            self.layout().addWidget(QLabel('Timestamp policy', self), 0,1, )
            self.registerField('tspolicy', self._tspolicy, 'currentText')  # Policy name not index.
            
            #  maxfragments next:
            
            self._maxFrags = QSpinBox(self)
            self._maxFrags.setMinimum(100)
            self._maxFrags.setMaximum(1 << 20)
            self._maxFrags.setValue(1000)
            self.layout().addWidget(self._maxFrags, 1,0, )
            self.layout().addWidget(QLabel('Maximum fragments in an event', self), 1,1, )
            self.registerField('maxfragments', self._maxFrags)
            
            #  Flush tick window
            
            self._window = QSpinBox(self)
            self._window.setMinimum(10)
            self._window.setMaximum(3600)
            self._window.setValue(30)
            self.layout().addWidget(self._window, 2,0, )
            self.layout().addWidget(QLabel('Maximum fragment retention in ticks'), 2,1, )
            self.registerField('window', self._window)
            
            # The flow control parameters.
            #  Global flow control:
            
            self._globalxoff = QSpinBox(self)
            self._globalxoff.setMinimum(200000)    # Pretty arbitrary guesses of 1 million
            self._globalxoff.setMaximum(100000000) # to 100million fragments
            self._globalxoff.setValue(4000000)     # this is the current default value.
            self._globalxoff.setSingleStep(100000) # up/down by 100000 else they'll get bored.
            self.layout().addWidget(self._globalxoff, 3,0, )
            self.layout().addWidget(QLabel('Global Xoff threshold'), 3,1, )
            self.registerField('globalxoff', self._globalxoff)
            
            self._globalxon = QSpinBox(self)
            self._globalxon.setMinimum(100000)    # Pretty arbitrary guesses of 1 million
            self._globalxon.setMaximum(100000000) # to 100million fragments
            self._globalxon.setValue(3000000)     # this is the current default value.
            self._globalxon.setSingleStep(100000) # up/down by 100000 else they'll get bored.
            self.layout().addWidget(self._globalxon, 4,0, )
            self.layout().addWidget(QLabel('Global Xon threshold'), 4,1, )
            self.registerField('globalxon', self._globalxon)
            
            #   Per Queue flow control
            
            self._perqxoff = QSpinBox(self)
            self._perqxoff.setMinimum(20000)
            self._perqxoff.setMaximum(100000000)
            self._perqxoff.setValue(400000)
            self._perqxoff.setSingleStep(10000)
            self.layout().addWidget(self._perqxoff, 5,0, )
            self.layout().addWidget(QLabel('Per Queue Xoff threshold', self), 5,1, )
            self.registerField('perqxoff', self._perqxoff)
            
            self._perqxon = QSpinBox(self)
            self._perqxon.setMinimum(10000)
            self._perqxon.setMaximum(100000000)
            self._perqxon.setValue(50000)
            self._perqxon.setSingleStep(10000)
            self.layout().addWidget(self._perqxon, 6,0, )
            self.layout().addWidget(QLabel('Per queue Xon threshold', self), 6,1, )
            self.registerField('perqxon', self._perqxon)
        
            self._initialized = True
    def load(self, config : dict[str, object]) -> None:
        '''
        The wizard can call this to pre-load the form from an event builder pipeline configuratin
        dict.  Note that this should not be called more than once due to the ability of users to 
        go backwards through the wizard.
        '''
        # set the timestamp policy if that's given
        
        tspolicy = config.get('tspolicy')
        if tspolicy:
            self._tspolicy.setCurrentText(tspolicy)
            
        # The rest are spinboxes and can be set using this clever little loop:
        
        for name, widget in [('orderer-window', self._window),
                             ('maxfragments', self._maxFrags), 
                             ('XoffThreshold', self._globalxoff),
                             ('XonThreshold', self._globalxon),
                             ('perQXoffThreshold', self._perqxoff),
                             ('perQXonThreshold', self._perqxon)
                            ]:
            value = config.get(name)
            if value:
                widget.setValue(value)

        # defaultPerQXonLimit (50000)
    def nextId(self) -> int:
        '''
        We unconditionally go to page 30.
        '''
        return 30
    
    def validatePage(self) -> bool:
        '''
        The xoff thresholds _must_ be higher than the xon thresholds, else this makes no
        sense.  If that's not, the case pop up a message box and return False else True.
        '''
        if self.field('globalxoff') <= self.field('globalxon'):
            QMessageBox.warning(self, 'Bad xon/xoff values', 
                'The global xoff value must be greater than the global xon value')
            return False
        
        if self.field('perqxoff') <= self.field('perqxon'):
            QMessageBox.warning(self, 'Bad per queue xon/xoff values',
                    'The per queue xoff value must be larger than the per queue xon value')
            return False
        
        return True
    
class DataSourceParameters(QWizardPage):
    '''
    This one is a bit more exciting.  We hae a FragmentSourceDefiner as our sole
    contents.
    IMPORTANT NOTE:  this must be page id 30. 
    
    This page:
        Declares no fields. The wizard has to pull the configuration list
        from us directly.

        In InitializePage, we pull in the event builder host and construct the
        full service name.  These are from the registered fields:
        'host' and 'service_suffix'

    '''
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        self.setTitle('Configure Data Sources')
        self._initialized = False

    def initializePage(self) -> None:
        
        if not self._initialized:
            self.setLayout(QGridLayout())
            self._editor = FragmentSourceDefiner(self)
            self.layout().addWidget(self._editor, 0,0, )
            
            self._initialized = True
        # Update the host and service, even on subsequent calls
        
        evbhost = self.field('host')
        service = self._computeEvbService()
        self._editor.setEvbParameters(evbhost, service)
        
    def load(self, configs: list[dict[str, object]]) ->None:
        '''
        Loads the fragment source configurations into the editor:
        @param configs - list of fragment source configuration dicts.
        @note - before loading the configurations, the evbhost and evbservice 
            parameters are set to match that of the wizard.
        '''
        evbhost = self.field('host')
        evbservice = self._computeEvbService()
        finalConfigs = configs
        for index, config in enumerate(finalConfigs):
            finalConfigs[index]['evbhost'] = evbhost
            finalConfigs[index]['evbservice'] = evbservice
        
        
        self._editor.load(finalConfigs)
    
    def getConfiguration(self) -> list[dict[str, object]]:
        '''
        @return list[dict[str, object]] the configurations of the data sources:
        '''
        return self._editor.get()

    # Utilities
    def _computeEvbService(self) -> str:
        # Compute the event builder full service name:
        
        result = f'Orderer:{getpass.getuser()}'
        suffix = self.field('service_suffix').strip()
        if suffix:
            result += f':{suffix}'
        
        return result
        
        
class EVBWizard(QWizard):
    '''
    This wizard allows you to prompt or edit the configuration of an
    event builder and its data sources.  It consists of three pages:
    id                        purpose
    10           General event building pipeline parameters.
    20           (optional) advanced event building pipeline parameters.
    30           Fragment source configuration
    
    Methods:
        load - Allows an event builder configuration to be loaded into the
              wizard.  This is how the wizard can be used to edit event builder
              and fragment source configurations.
        get  - If the wizard is completed and accepted, this allows you to fetch
               the event builder pipline configuration and list of fragment
               source configurations.
    '''
    def __init__(self, parent : QWidget | None = None):
        '''
        Configure the wizard.  Normally the order of operations is
        1. Instantiate the wizard.
        2. Optionally, load a configuration into the wizard.
        3. exec the wizard to prompt the user.
        4. If the result of execution was QDialog.DialogCode.Accepted, 
           fetch the configuration and use it to create an EventBulder.EventBuilder object
           attaching it to the state machine so that it starts up at the beginning of runs.
        
        @param parent : QWidget | None (default None) - Parent of the wizard. This determines
             where the wizard dialog posts so it is imoprtant.
        
        '''
        super().__init__(parent)
        
        self.setPage(10, EvbMainParameters(self))
        self.setPage(20, EvbAdvancedParameters(self))
        self.setPage(30, DataSourceParameters(self))

        
        # We need to set up to load pages on their first display if we've been loaded:
        
        self._loadedPages : set[int] = set()                  # Set of pages we've loaded.
        self._name : str | None = None
        self._evbconfig   : dict[str, object] | None = None   # Loaded event builder configuration
        self._sourceConfigs : list[dict[str, object]] | None = None # loaded fragment source configurations.
        self.currentIdChanged.connect(self._loadPageConfig)
    
    def load(self, name : str, pipeConfig : dict[str, object], sourceConfigs : list[dict[str, object]]) -> None:
        '''
        Load the event builder configuration:
        @param name - name of the event builder.
        @param pipeConfig - Configuration of the event builder pipeline
        @param sourceConfigs - list of fragment source configurations.
        '''
        
        self._name = name
        self._loadedPages = set()        # Reset the pages that have been loaded.
        self._evbconfig   = pipeConfig
        self._sourceConfigs = sourceConfigs
        
    def get(self) -> tuple[str, dict[str, object], list[dict[str, object]]]:
        '''
        @return tuple(dict[str, object], list[dict[str, object]]) 
            Element 0 of the tuple is the event builder name.
            Element 1 of the tuple is the event builder pipeline
            configuration.  element 2 of the tuple is the list of 
            event builder fragment sources.
        '''
        return (str(self.field('name')), self._getPipeConfig(), self._getSourceConfigs())
    
    
    # Utilities.
    
    def _getPipeConfig(self) -> dict[str, object]:
        #  Pull the configuration of the event buider from the 
        #  fields that were registered.  Note that
        # the fields in the EvbAdvancedParameters page (page 20) 
        # may not be defined, so we check to be sure that page
        # has been visited before adding them to the configuration.
        # We add a special configuration item:
        # 'name'
        config = {
            'host': str(self.field('host')),
            'dt'  : int(self.field('dt')),
            'sourceid' : int(self.field('sourceid')),
            'ring' : str(self.field('ring'))
        }
        # If the service suffix is not blank, it needs to be configured too:
        
        svcSuffix = self.field('service_suffix').strip()
        if svcSuffix:
            config['service-suffix'] = svcSuffix
        
        if self.hasVisitedPage(20):
            # The advanced parameters have been set:
            
            advancedParams = {
                'orderer-window' : int(self.field('window')),
                'XoffThreshold'  : int(self.field('globalxoff')),
                'XonThreshold'   : int(self.field('globalxon')),
                'perQXoffThreshold' : int(self.field('perqxoff')),
                'perQXonThreshold' : int(self.field('perqxon')),
                'tspolicy'       : str(self.field('tspolicy')),
                'maxfragments'   : int(self.field('maxfragments'))
            }
            config = config | advancedParams

        return config
    
    def _getSourceConfigs(self) -> list[dict[str, object]]:
        # Returns the list of fragment source configurations.
        
        page = self.page(30)     # We can get it from the page.
        return page.getConfiguration()
    
    # Local slots.
    
    def _loadPageConfig(self, pageId : int) -> None:

        # Called when the page has changed.  If there are configurations,
        # and if the page has not been loaded by us previously,
        # load the page from the configurations.
        # We only want to load pages the first time they are displayed so that
        # the user can modify them, go back and forward with impunity and
        # without their changes being lost...hence this scoreboard
        
        
        # We need direct comparisons to none in case the configuration
        # has been laoded but is empty (e.g. no sources).
        if pageId not in self._loadedPages and \
            self._evbconfig is not None and self._sourceConfigs is not None:
            
            page = self.page(pageId) 
            match pageId:
                case 10:
                    page.load(self._name, self._evbconfig)
                case 20:
                    page.load(self._evbconfig)
                case 30:
                    page.load(self._sourceConfigs)
            
            self._loadedPages.add(pageId)      
        
class EVBTableModel(QStandardItemModel):
    '''
    This model gets loaded with a list of named event builder configurations.
    The items exported to an e.g. QTableView are the event builder
    name, the host it runs in and the output ringbuffer it creates and a trashcan
    icon in case the client wants to make a table from which event builders can be
    deleted.  
    After instantiating the model, invoking 'load' loads the model with
    named event builders.
    '''
    def __init__(self, parent : QObject | None = None):
        super().__init__(parent)
        
        # This is cleverly designed to be the same as what comes back from 
        # EventBuilderManager.configs.  Indices of this list will also
        # match the rows we crate in load.
        
        self._buidlers : list[tuple[str, dict[str, object], list[dict[str, object]]]] =  []
        
        # For the trash can column.
        
        icon_attr = QStyle.StandardPixmap.SP_TrashIcon
        self._trashIcon = mainWindow().style().standardIcon(icon_attr)
        
    
    def load(self, configs : list[tuple[str, dict[str, object], list[dict[str, object]]]]) -> None:
        '''
        Loads the model after clearing it and sets up the model and headers.
        @param configs :list[tuple[str, dict[str, object], list[dict[str, object]]]] 
            - The list of configurations from which to stock the model:
               [0] - Is a name to associate with the event buidler.
               [1] - Is the pipeline configuration.
               [2] - Is the list of fragment source configurations.
        @note This is identical to the list returned by the EventBuilderManaer's singleton
              configs method.
        '''
        self.clear()
        self.setHorizontalHeaderLabels(['Name', 'Host', 'Output Ring', 'Delete'])
        
        self._builders = configs         # Sam struct after all.
        for config in self._builders:
            self._addRow(config[0], config[1]['host'], config[1]['ring'])
    
    def getDefinition(self, idx: QModelIndex) ->  tuple[str, dict[str, object], list[dict[str, object]]]:
        '''
        Given a model index, return the configuration for the row that contains
        that index
        @param idx: QModelIndex - The index of the row to return.
        @return tuple[str, dict[str, object], list[dict[str, object]]]
            - The information that corresponds to that row. See load for information about
              the contents of the returned tuple.
        @throw IndexError  - if the index is not in the model somehow.
        '''
        return self._builders(idx.row())
        
    
    #  Utilities
    
    def _addRow(self, name : str, host : str, ring : str) -> None:
        # Add a row to the model.
        
        nameItem = QStandardItem(name)
        hostItem = QStandardItem(host)
        ringItem = QStandardItem(ring)
        trashItem = QStandardItem(self._trashIcon, '')
        
        self.appendRow([nameItem, hostItem, ringItem, trashItem])

class EVBTable(QTableView):
    '''
    Displays the data in an EVBTableModel in a table.  Normally,
    the trash can (delete) column is displayed, however,
    the hideDelete and showDelete methods can control
    the visibility of this column.
    Signals:
    
    delete(QModelIndex)  Trash can was clicked.
    selected(QModelIndex) An item was selected via double click.
    '''
    delete   = pyqtSignal(QModelIndex)
    selected = pyqtSignal(QModelIndex)
    def  __init__(self, model : EVBTableModel, parent : QWidget | None = None):
        super().__init__(parent)
        self.setModel(model)
        
        self.clicked.connect(self._clicked)    # Figure out if we need to emit delete.
        self.doubleClicked.connect(self.selected)
        
    
    #  Show/hide trash cans:
    
    def hideDelete(self) -> None:
        '''
        Hide the trashcan column.
        '''
        self.hideColumn(3)
    def showDelete(self) -> None:
        '''
        Show the trashcan column.
        '''
        self.showColumn(3)
        
    # Internal (private) slots.
    
    def _clicked(self, index : QModelIndex) -> None:
        # If the model index indicates the  the trash can icon
        # was what was clicked signal delete.
        if index.row() == 3:
            self.delete.emit(index)
        
        
# Test code:

if __name__ == '__main__':
    import sys

    from PyQt6.QtWidgets import QApplication, QPushButton

    app = QApplication(sys.argv)
    
    win = EVBWizard()
    
    pipeconfig = {
        'host' : 'localhost',
        'service-suffix' : 'builder',
        'ring' : 'ron',
        'orderer-window' : 60,
        'tspolicy' : 'earliest',
        'maxfragments' : 1000
    }
    sourceConfigs = [
        {
            'host' : 'spdaq30',
            'description' : 'Test source',
            'ids' : [1,2,3],
            'default-id': 1,
            'ring' : 'tcp://localhost/ron'
        },
    ]
    win.load('my builder', pipeconfig, sourceConfigs)
    win.show()
    
    if win.exec() == QDialog.DialogCode.Accepted :
        print(win.get())