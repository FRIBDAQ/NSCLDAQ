"""This file provides the elements of the main gui.  The main gui consist of a top part,
which contains the title, run number, and buttons for run control and event recording.
[for now timed runs are not supported]
Below that is a tabbed notebook.  The tabbed notebook has the following pre-built tabs:

*   Monitored programs which has a widget that shows the state of the monitored programs.
*   Eventloggers - shows the state of event loggers
*   Main output log  - which has to consolidated output log.

Note that the tabbed notebook is dynamic.  new tabs are added each time there's output from a program not yet seen.


"""

from PyQt5.QtWidgets import (
    QVBoxLayout, QHBoxLayout, QWidget, 
    QPushButton, QCheckBox, QLineEdit, QLabel, QTabWidget, 
    QTextEdit
    
)
from PyQt5.QtGui import ( QStandardItemModel, QStandardItem, QIntValidator)

from PyQt5.QtCore import pyqtSignal, Qt

#---------------------------------------------------------------------------------------------
#
#  THe widgets in this section have to do with the top part of the GUI.
#  There are widgets for the run information and managing state transitions,
#  as well as one widget to rule them all.
class RunInfoWidget(QWidget):
    """This class provides information about the run:
      - A text entry for the title. - with change button.
      - Actual title - title from the database
      - A Text entry for the run number.
      - A pushbutton to  increment the run number.
      - Push button to set the run number.
      - Actual run number,
      
      
      
    Attributes:
        title - the contents of the title widget Not it's possible this differs from the 
                title in the config data base if, e.g. more than one editor is involved.
        actualTitle Actual title string value.
        runNumber - contents of the run number text edit (this will be a positive integer).
        actualRunNumber - contents of the actual run number string.
        
    Signals:
        setTitle(str) - Set title button was clicked.,
        setRunNumber(int) - Set the run number button was clicked.
        incrRun(int)   - Increment run number was clicked Note:
              slots for this _probably_ should get the run number fromt he
              config database, increment it set it back inboth run number and a tual run number.

    """
    updateTitle = pyqtSignal(str)
    updateRunNumber = pyqtSignal(int)
    incRun = pyqtSignal(int)
    
    def __init__(self, *args):
        super().__init__(*args)
        
        layout = QHBoxLayout()
        
        #  title:
        user_title_layout = QHBoxLayout()
        
        title_layout = QVBoxLayout()
        
        self._titlelabel = QLabel('Title: ', self)
        self._title = QLineEdit(self)
        self._title.setMaxLength(80)
        self._title.setMinimumWidth(80*5)
        self._setTitle = QPushButton('Set')
        
        user_title_layout.addWidget(self._titlelabel)
        user_title_layout.addWidget(self._title)
        user_title_layout.addWidget(self._setTitle)
        
        title_layout.addLayout(user_title_layout)
        
        actual_title_layout = QHBoxLayout()
        self._actual_titlelabel = QLabel('Actual', self)
        self._actual_title      = QLabel('', self)
        actual_title_layout.addWidget(self._actual_titlelabel)
        actual_title_layout.addWidget(self._actual_title)
        title_layout.addLayout(actual_title_layout)
        
        layout.addLayout(title_layout)
        
        #  Now the run number - same layout structure as title...just other widgets.
        
        run_layout = QVBoxLayout()
    
        user_run_layout = QHBoxLayout()
        self._runlabel = QLabel('Run #', self)
        self._run = QLineEdit('0', self)
        self._run.setMaxLength(8)
        self._increment = QPushButton('+', self)
        self._setRun   = QPushButton('Set', self)
        
        user_run_layout.addWidget(self._runlabel)
        user_run_layout.addWidget(self._run)
        user_run_layout.addWidget(self._increment)
        user_run_layout.addWidget(self._setRun)
        
        run_layout.addLayout(user_run_layout)
        
        actual_run_layout = QHBoxLayout()
        self._actual_run_label = QLabel("Actual Run#", self)
        self._actual_run = QLabel('0', self)
        
        actual_run_layout.addWidget(self._actual_run_label)
        actual_run_layout.addWidget(self._actual_run)
        
        run_layout.addLayout(actual_run_layout)
        
        layout.addLayout(run_layout)
        
        
        
        # put a validator on the run number so the user can't have a non-integer.
        # for now just an integer -- TODO require it be >0.
        
        self._run.setValidator(QIntValidator())
        
        # Connect signals.
        
        self._setTitle.clicked.connect(self._setTitleRelay)
        self._setRun.clicked.connect(self._setRunNumberRelay)
        self._increment.clicked.connect(self._incRun)
        
        self.setLayout(layout)
    
    # Attributes:
    
    def title(self):
        """Return the user title string.
        """
        return self._title.text()
    def setTitle(self, title):
        """Set the user title string. The actual title string is not modified.

        Args:
            title (str): new _user_ title string.
        """
        self._title.setText(title)
        
    def runNumber(self):
        """Return the run number (integer).
        """
        return int(self._run.text())
    
    def setRunNumber(self, run):
        """
        Set the user run number.

        Args:
            run (int): New run number should be a positive int.
            
        """
        self._valid_run(run)
        self._run.setText(str(run))
        
    
    # THe actual parameter attributes:
    
    def actualTitle(self):
        """Retturn the value of the actual title.
        """
        return self._actual_title.text()
    
    def setActualTitle(self, title):
        """Set the value of the actual title.

        Args:
            title (str): new title.
        """
        self._actual_title.setText(title)
    
    def actualRunNumber(self):
        """Return actual run number value."""
        return int(self._actual_run.text())
    def setActualRunNumber(self, run):
        """Set the new actual run number value

        Args:
            run (int): _description_

        Raises:
            TypeError: If run isn't an int.
            ValueError: run is an int but is negative.
        """
        self._valid_run(run)
        self._actual_run.setText(str(run))
        
    # private methods.
    
    def _valid_run(self, run):
        # Throw an appropriate exceptionm if a run number is rinvalid.
        
        if not isinstance(run, int):
            raise TypeError(f'{run} must be a positive integer but was a {type(run)}')
        r = int(run)
        if r < 0:
            raise ValueError(f'Run number must be a positive integer, was {run}')
    
    # Signal relays:
    
    def _setTitleRelay(self):
        # SIgnal relay called when the set title button was clicked. - 
        # emit setTitle passing the current title string
        
        self.updateTitle.emit(self._title.text())
    def _setRunNumberRelay(self):
        # SIgnal relay for the run number
        
        try:
            self.updateRunNumber.emit(int(self._run.text())) 
        except:
            # Probably integer conversion of run failed:
            
            pass
     
    def _incRun(self):
        #   Relay the increment run  click:
         
        try:
            self.incRun.emit(int(self._run.text()))
        except:
            pass     #likely int conversion failed.
        

class RunControlWidget(QWidget):
    """Class that provides state transition controls. 
        This provides  buttons and internal state
        An internal state is used to control what the button look like:
        *  A shutdown button is provided that is enabled if the state is not SHUTDOWN
        *  A transition button is 
           - disabled if the state is SHUTDOWN
           - Labeled 'Initialize'  if the state is BOOT
           - BEGIN if the state is HWINIT or END
           - END if the state is BEGIN
        * A Checkbox that both shows the state of event logging and 
          changes it.  Note that if the state is BEGIN, we disable that widget
          as it should not be changed during the run (though changing it during the
          run is harmless since it's only looked at during transtions to BEGIN)
           

    Parent class:
        QWIdget 
    Attributes:
        state(readonly)  Provides a state e.g. RunControlWidget.SHUTDOWN
        
    Methods:
        boot - transition the widget to RunControlWidget.BOOT
        shutdown - Transition the Widget to RunControlWidget.SHUTDOWN
        hwinit - Transition the widget to RunControlWidget.HWINT
        begin - Transition the widget to RunControlWidget.BEGIN
        end   - Transition the widget to RunControl.END
        
        
        state_name - translates the state value (from state()) to a state name string.
        
    Signals:
        transition(str) - A transition is requested to the new named state.
           Note if the shtdown button is clicked transtion('SHUTDOWN') is emitteed.
        logtoggle(int) - Logbutton toggled.
        
    """
    # Class storage:
    
    # Any resemblance betwween these values and the id  field in the database
    # should be treated as purely conncidental.
    
    BOOT = 1
    SHUTDOWN = 2
    HWINIT = 3
    BEGIN = 4
    END = 5
    
    
    _StateNames = {
        BOOT: 'BOOT', SHUTDOWN: 'SHUTDOWN', HWINIT: 'HWINIT', BEGIN: 'BEGIN', END: 'END'
    }
    # Transition Button label table:
    
    _TransitionLabels = {
        SHUTDOWN : 'Boot', BOOT: 'HW Init', HWINIT: 'Begin', END: 'Begin', BEGIN: 'End'
    }
    # Given a button label the transition it requests:
    _TransitionRequests = {
        'Boot' : BOOT, 'HW Init': HWINIT, 'Begin': BEGIN, 'End': END, 
    }
    #  The Shutdown button can't be alive if we're already shutdown:
    
    _ShutdownDisabledStates = [SHUTDOWN,]
    
    transition = pyqtSignal(str)
    logtoggle  = pyqtSignal(int)
    def __init__(self, *args):
        super().__init__(*args)
        
        layout = QHBoxLayout()
        
        # CUrrent state:
        
        self._statelabel = QLabel('State: ', self)
        layout.addWidget(self._statelabel)
        self._statename = QLabel('SHUTDOWN', self)
        layout.addWidget(self._statename)
        
        # SHutdown button
        
        self._reqshutdown = QPushButton('Shutdown', self)
        layout.addWidget(self._reqshutdown)
        
        #  State transition button:
        
        self._transitionreq  = QPushButton('BOOT', self)
        layout.addWidget(self._transitionreq)
        
        # Event logging button:
        
        self._log = QCheckBox("Record", self)
        layout.addWidget(self._log)
        
        self._stateid = self.SHUTDOWN
        self._update_buttons()
        
        self.setLayout(layout)
        
        # Hook in buttons:
        
        self._reqshutdown.clicked.connect(self._signalShutdown)
        self._transitionreq.clicked.connect(self._signalChangeReq)
        self._log.stateChanged.connect(self.logtoggle)
    
    # Attributes
    
    def state(self):
        """Return the current state id. 
        """
        return self._stateid
    def setStateName(self, name):
        if not name in self._StateNames.values():
            raise ValueError(f'{name} is not a valid state name.')
        for state, label in self._StateNames.items():
            if name == label:
                self._stateid = state
                self._statename.setText(label)
                self._update_buttons()
        
    def state_name(self):
        """return the name of the currdnt state.  This attribute has no setter.
        """
        return self._StateNames[self._stateid]
    
    # Public methods:
    
    def boot(self):
        """ Transition to the 'BOOT' state._
        """
        self._transition(self.BOOT)
    def shutdown(self):
        """Transitino to SHUTDOWN:
        """
        self._transition(self.SHUTDOWN)
    def hwinit(self):
        """Transition to the HWINIT state:
        """
        self._transition(self.HWINIT)
    def begin(self):
        """Transition to the BEGIN state:
        """
        self._transition(self.BEGIN)
    def end(self):
        """Transition to endid.
        """
        self._transition(self.END)
    def setLoggerState(self, state):
        """Set the state of the logger checkbox:

        Args:
            state (bool): global logger state from e.g. Logger.isRecording
        """
        if state:
            self._log.setCheckState(Qt.Checked)
        else:
            self._log.setCheckState(Qt.Unchecked)
        
    # Signal relays:
    
    def _signalShutdown(self):
        #  Shutdown button was clicked, emit
        
        self.transition.emit(self._StateNames[self.SHUTDOWN])
    
    def _signalChangeReq(self):
        # figure out what the button click means and signal the related transition request.
        # the state requested is the state that is the key to the label on the button in
        # self._TransitionLabels
        
        button_text = self._transitionreq.text()
        if button_text in self._TransitionRequests.keys():
            self.transition.emit(self._StateNames[self._TransitionRequests[button_text]])
        else:
            raise AssertionError(
                f' The button label: {button_text}, could not be translated to a transiton state.'
            )    
        
        
                
                    
    # private methods
    
    def _transition(self, newid):
        # Tranition to the new state in the newid:
        
        self._stateid = newid
        self._update_buttons()
        
    
    def _update_buttons(self):
        # If necessary disable the SHUTDOWN button:
        
        if self._stateid in self._ShutdownDisabledStates:
            self._reqshutdown.setDisabled(True)
        else:
            self._reqshutdown.setDisabled(False)
        
        # Set the label on the transition button:
        
        self._transitionreq.setText(self._TransitionLabels[self._stateid])
        
        #  IF the state is BEGIN, then we must turn off the log checkbutton.
        
        if self._stateid == self.BEGIN:
            self._log.setDisabled(True)
        else:
            self._log.setDisabled(False)

class RunControls(QWidget):
    """Container widget for the RunInfoWidget and RunControlWidgets.  These are the top part of the
    main GUI

    SuperClass
        QWidget
        
    Attributes (delegated to the subwidgets):
    From RunInfoWidget
        title - the contents of the title widget Not it's possible this differs from the 
                title in the config data base if, e.g. more than one editor is involved.
        actualTitle Actual title string value.
        runNumber - contents of the run number text edit (this will be a positive integer).
        actualRunNumber - contents of the actual run number string.
    From RunControlWidget:
        state(readonly)  Provides a state e.g. RunControlWidget.SHUTDOWN
        
    Signals:
    From RunInfoWidget:
        updateTitle(str) - Set title button was clicked.,
        updatetRunNumber(int) - Set the run number button was clicked.
        incRun(int)   - Increment run number was clicked Note:
              slots for this _probably_ should get the run number fromt he
              config database, increment it set it back inboth run number and a tual run number.
        transition(str) - A transition is requested to the new named state.
           Note if the shtdown button is clicked transtion('SHUTDOWN') is emitteed.

    The following methods are delegated to contained objects:
    RunControlWidget:
        boot - transition the widget to RunControlWidget.BOOT
        shutdown - Transition the Widget to RunControlWidget.SHUTDOWN
        hwinit - Transition the widget to RunControlWidget.HWINT
        begin - Transition the widget to RunControlWidget.BEGIN
        end   - Transition the widget to RunControl.END
        
        state_name - translates the state value (from state()) to a state name string.
    
    Additionally, if clients prefer to interact with the subwidgets directly:
        runInfo - Returns the RunInfoWidget object
        runControls - Returns the RunControlWidget 
        
       
        
    """
    updateTitle = pyqtSignal(str)
    updateRunNumber = pyqtSignal(int)
    incRun = pyqtSignal(int)
    
    transition = pyqtSignal(str)
    
    def __init__(self, *args):
        super().__init__(*args)
        
        layout = QVBoxLayout()
        
        self._info = RunInfoWidget(self)
        layout.addWidget(self._info)
        
        self._controls = RunControlWidget(self)
        layout.addWidget(self._controls)
        
        self.setLayout(layout)
        
        # Hook signals to our signals:
        
        self._info.updateTitle.connect(self.updateTitle)
        self._info.updateRunNumber.connect(self.updateRunNumber)
        self._info.incRun.connect(self.incRun)
        
        self._controls.transition.connect(self.transition)
    
    #  Attributes (including exposing the subwidgets):
    
    def runInfo(self):
        """

        Returns:
            RunInfoWidget: The run information subwidget.
        """
        return self._info
    def runControls(self):
        """

        Returns:
           RunControlWidget: The run controls subwidget.
        """
        return self._controls
    
    # Attributes from RunInfoWidget:
    
    def title(self):
        """Return the user title string.
        """
        return self._info.title()
    
    def setTitle(self, title):
        """Set the user title string. The actual title string is not modified.

        Args:
            title (str): new _user_ title string.
        """
        self._info.setTitle(title)
        
    def runNumber(self):
        """Return the run number (integer).
        """
        return self._info.runNumer()
    
    def setRunNumber(self, run):
        """
        Set the user run number.

        Args:
            run (int): New run number should be a positive int.
            
        """
        self._info.setRunNumber(run)
        
        
    
    # THe actual parameter attributes:
    
    def actualTitle(self):
        """Retturn the value of the actual title.
        """
        return self._info.actualTitle()
    
    def setActualTitle(self, title):
        """Set the value of the actual title.

        Args:
            title (str): new title.
        """
        self._info.setActualTitle(title)
    
    def actualRunNumber(self):
        """Return actual run number value."""
        return self._info.actualRunNumber()
    def setActualRunNumber(self, run):
        """Set the new actual run number value

        Args:
            run (int): _description_

        Raises:
            TypeError: If run isn't an int.
            ValueError: run is an int but is negative.
        """
        self._info.setActualRunNumber(run)
        
    # Delegations from self._controls:
    
    def state(self):
        """Return the current state id. This attribute has no direct setter.
        """
        return self._controls(state)
    def setStateName(self, name):
        self._controls.setStateName(name)
        
    def state_name(self):
        """return the name of the currdnt state.  This attribute has no setter.
        """
        return self._controls()
    
    # Public methods:
    
    def boot(self):
        """ Transition to the 'BOOT' state._
        """
        self._controls.boot()
    def shutdown(self):
        """Transitino to SHUTDOWN:
        """
        self._controls.shutdown()
    def hwinit(self):
        """Transition to the HWINIT state:
        """
        self._controls.hwinit()
    def begin(self):
        """Transition to the BEGIN state:
        """
        self._controls.begin()
    def end(self):
        """Transition to endid.
        """
        self._controls.end()


#-------------------------------------------------------------------------------------------------------
#
#  The widgets in this section  have to do with the bottom part of the GUI:
#   Specifically, the contents of and the tabbed widget.

class LogWindow(QTextEdit):
    """This class provides a scrolling text widget that's suitable for logging stuff.
    Specifically, the log is a limited number of lines of text and is readonly.
    Text may only be appended and, if the number of lines of text is larger than the limit the
    top lines of text are removed.
    
    Attributes:
        maxlines - Maximum number of lines of text.
        lines (readonly) maximum line count.
    Methods:
        append - Add a new line of text.
        clear  - Clears the text completely.
        
        
    Base class::
        QTextEdit
    Internals Note:
    """
    DEFAULT_MAX_LINES=1000
    def __init__(self, *args):
        super().__init__(*args)
        self._maxlines = self.DEFAULT_MAX_LINES
        
        self._lines = []                 # The text.
        
        self.setReadOnly(True)
        
    def append(self, line):
        self._lines.append(line)
        if len(self._lines) > self._maxlines:
            self._lines.pop(0)
        
        self.setPlainText('\n'.join(self._lines))
    
    def maxlines(self):
        return self._maxlines
    def setMaxLines(self, n):
        self._maxlines = n
        if len(self._lines) > n:
            num_to_delete = len(self._lines) - n
            del self._lines[0:num_to_delete]            # Kill the first few lines.
            self.setPlainText('\n'.join(self._lines))   # Update the widget.
    def clear(self):
        self._lines = []
        self.setPlainText('\n'.join(self.lines))                
        
        
class LogCollection:
    """Not really a widget, this  forms a collection of LogWindows that live in a tabbed widget parent.
    Log messages are issued to sources.  Each source gets a LogWindow which, if necessary, is created dynamically.
    The parent tab widget has tabs that are named after the data sources.
    For now, the numbger of lines in each log window is the same.
    
    Methods:
        log - Logs a message to a data source.  The message may be multiline in which case it is parsed up into
            lines and each line appended to the approriate window.  If the source is new a new log is created and
            added as a tab to the parent.
        clear - Clears a data source or all.  Note that if needed a new data source is created.
        maxlines - Returns the current maximum line count.
        setMaxlines - sets the maximum line count.
        
    """
    def __init__(self, tabwidget):
        """Construction

        Args:
            tabwidget (QTabWidget): The tab widget in which the log widgets  are put.
        """
        self._tabs = tabwidget
        self._maxlines = LogWindow.DEFAULT_MAX_LINES     # Usee the default for everyone to start.
        self._logs = dict()                              # Dict of source:widget
        self._tabs.tabCloseRequested.connect(self._closeTab)
    
    def log(self, source, message):
        """Add a log message from a source.

        Args:
            source (str): Identifies the data source.
            message (str): Possibly multiline message to add.
        """
        src = self._getSource(source)
        lines = message.split('\n')
        for line in lines:
            src.append(line)
    
    def clear(self):
        """clears all widgets:
        """
        
        for w in self._logs.values():
            w.clear()
    
    def maxlines(self):
        """

        Returns:
            int: MNaximum number of text lines retaineed by the logs.
        """
        return self._maxlines
    def setMaxlines(self, n):
        """Set the maximum lines.

        Args:
            n (int): New max line count.
        """
        self._maxlines = n
        for w in self._logs.values():
            w.setMaxlines(n)
            
    # ---  Slots
    
    def _closeTab(self, index):
        source = self._tabs.tabText(index)
        if source in self._logs.keys():
            self._tabs.removeTab(index)
            
            # Just in case
            
            try:
                del self._logs[source]
            except:
                pass
        
    # --- Private methods:
    
    def _getSource(self, src):
        if not src in self._logs.keys():
            
            # Have to make a new one:
            
            log = LogWindow(self._tabs)
            self._tabs.addTab(log, src)
            self._logs[src] = log
            
        # We're going to allow for movable tabs so find the right tab:
        
        for index in range(self._tabs.count()):
            if src == self._tabs.tabText(index):
                # Matches so give the widget:
                
                return self._tabs.widget(index)
        #This is a bug:
        
        raise AssertionError("Could not find log tab even after creating!")
    
#----------------------------------------------------------------------------------------------------------
#  Tying this all together:

class MainGui(QWidget):
    """ This is the main GUI widget for the managed experiment run control system.  It has a 
        RunControls widget at the top and a tabbed widget that's managed (mostly) by a log collection at the
        bottom.  Rather than doing the painful explicit delegation, we provide accessors for the components:
        
        controls - returns the RunControls widget.
        tabs     - Returns the tabbed widget.
        log      - Returns the log manager.
        
        This should provide all the support we need (e.g. we can put a program status widget into tabs)
        
    Inherits from:
        QWidget
    """

    def __init__(self, *args):
        super().__init__(*args)
        
        layout = QVBoxLayout()
        
        self._controls = RunControls(self)
        layout.addWidget(self._controls)
        
        self._tabs = QTabWidget(self)
        self._tabs.setMinimumHeight(50*8)
        self._tabs.setMovable(True)
        self._tabs.setTabsClosable(True)
        layout.addWidget(self._tabs)
        
        self._log = LogCollection(self._tabs)
        
        self.setLayout(layout)
    
    def controls(self):
        return self._controls
    def tabs(self):
        return self._tabs
    def log(self):
        return self._log
        
    
    
#----------------------------------------------------------------------------------------------------------
#
# Test code.

    
if __name__ == "__main__":
    test_widget = None
    
    def updateTitle(title): 
        test_widget.setActualTitle(title)
    def updateRunNumber(n):
        test_widget.setActualRunNumber(n)
    def incrun(n):
        n += 1
        test_widget.setActualRunNumber(n)
        test_widget.setRunNumber(n)
        
    def transition(name):
        if name == 'SHUTDOWN':
            test_widget.shutdown()
        elif name == 'BOOT':
            test_widget.boot()
        elif name == 'HWINIT':
            test_widget.hwinit()
        elif name == "BEGIN":
            test_widget.begin()
        elif name == "END":
            test_widget.end()
        else:
            print('unrecognized state', name)

        test_widget.setStateName(name)
        
    def append_text():
        log.append(text.text())
    
    def logmsg():
        source = src.text()
        msg    = line.text()
        logs.log(source, msg)
    
    from PyQt5.QtWidgets import (QApplication, QMainWindow)
    import sys
    app = QApplication(sys.argv)
    main_window = QMainWindow()
    test_widget = MainGui()
    
    main_window.setCentralWidget(test_widget)
    

    main_window.show()
    app.exec()