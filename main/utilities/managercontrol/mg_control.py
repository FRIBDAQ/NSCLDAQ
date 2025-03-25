#! /usr/bin/env python3
"""This program provides the main program for the new control panel of the
    Managed experiment control panel.
"""

from nscldaq.manager_control.config import Configuration
from nscldaq.manager_control.programlist import ProgramView
from nscldaq.manager_control.cfgwizard import ConfigWizard
from nscldaq.manager_control.maingui import MainGui
from nscldaq.manager_control.loggerlist import LoggerTable
from nscldaq.manager_client import Programs, OutputMonitor, KVStore, State, Logger
from PyQt5.QtWidgets import (
    QApplication, QLabel, QLineEdit,
    QMainWindow
)
from PyQt5.QtCore import QTimer, Qt, QThread
import sys
import os
import datetime


CONFIGURATION_FILE="mg_guiconfig.toml"
UPDATE_INTERVAL=2        # Units are sedonds

Updater = None          # Early in initialization this becomes a QTimer. Updaters hook to the timeout signal.
outputMonitor = None



def get_configuration():
    global CONFIGURATION_FILE
    #   If the file exists we just use it.  Otherwise we must use a widget to instantiate it.
    
    if os.access(CONFIGURATION_FILE, os.R_OK):
        return Configuration(CONFIGURATION_FILE)
    else:
        c = Configuration ('/dev/null')   # All defaults.
        wiz = ConfigWizard(c)
        wiz.exec()
        c.dump(CONFIGURATION_FILE)
        return c

#----------------------------------------------------------------------------------------------------
#  Stuff for the programs display.

def addProgramDisplay(window):
    programList = ProgramView()
    tabs = window.tabs()
    tabs.addTab(programList, 'Programs')
    return programList
    

def updateProgramsTab():
    # Timeout handler to update the model driving the programs tab.
    
    programs = Programs(config.host(), config.user(), config.rest_service())

    programlist.model().update(programs.status())


#-------------------------------------------------------------------- 
#  Handle output logging.

def updateLogWindows():
    # called periodically to check for output. 
    
    # Reconnect if we can:
    
    if not  outputMonitor.isConnected():
        try:
            outputMonitor.reconnect()
        except:
            return     # reconnect failed this time.
    
    # continue if we're still connected:
    
    if outputMonitor.isConnected():
        full_output = ""
        output = outputMonitor.read()
        while len(output) > 0:
            full_output += output    # Aggregate the otputut.
            output = outputMonitor.read()
        # Now we have the full_output:
        # Put it all in aggregated output:
        
        if len(full_output) > 0:
            gui.log().log('Aggregate output', str(datetime.datetime.now()) + " : " + full_output)
            
            # Now log to the programs  tab:  Each line that has a ':' is split into 
            # program: stuff
            # and the stuff is logged to program.
            
            for line in full_output.split('\n'):
                if ':' in line:
                    broken_line = line.split(':')
                    program = broken_line.pop(0)
                    if program not in ["Completing transition", 'Stacktrace'] and program[0] != ' ':
                        log_line = ':'.join(broken_line)
                        final_line = str(datetime.datetime.now()) + ' : ' + log_line
                        gui.log().log(program, final_line)
                    
#----------------------------------------------------------------------------------------------
#   Slots to handle the signals from the controls par tof the widget:

def updateControls():
    # Timer handler to update the actuals in the controls widget:
    
    client = KVStore(config.host(), config.user(), config.rest_service())
    controls_widget.setActualTitle(client.title())
    controls_widget.setActualRunNumber(client.run())
    
    state_client = State(config.host(), config.user(), config.rest_service())
    state = state_client.status()
    controls_widget.setStateName(state)
    if state == 'BOOT':
        controls_widget.boot()
    elif state == 'SHUTDOWN':
        controls_widget.shutdown
    elif state == 'HWINIT':
        controls_widget.hwinit()
    elif state == 'BEGIN':
        controls_widget.begin()
    elif state == 'END':
        controls_widget.end()

def setTitle(newTitle):
    client = KVStore(config.host(), config.user(), config.rest_service())
    client.setTitle(newTitle)

def setRun(newRun):
    client= KVStore(config.host(), config.user(), config.rest_service())
    client.setRun(newRun)
    
def incRun(oldRun):
    new = oldRun+1
    setRun(new)
    controls_widget.setRunNumber(new)
    
def reqTransition(newstate):
    client = State(config.host(), config.user(), config.rest_service())
    try:
        # If the new state is BEGIN, then we must start the loggers:
        
        if newstate == 'BEGIN':
            lclient = Logger(config.host(), config.user(), config.rest_service())
            lclient.start()
            
            QThread.sleep(1)     # maybe we don't need this?
        
        # Start the run.
        client.transition(newstate)
    except Exception as e:
        gui.log().log('Aggregate output', str(e))
    # If transitioning to END and logging is enabled, increment the run numbers:
    #  Note everyone will see the actual run number change but 
    #  the editbox only gets updated on the gui that asked for the state change.
    
    if newstate == 'END':
        info = gui.controls().runInfo()
        nextrun = info.actualRunNumber() + 1
        info.setRunNumber(nextrun)
        kvclient = KVStore(config.host(), config.user(), config.rest_service())
        kvclient.setRun(nextrun)
        
    
    
    
def load_initial_controls(w):
    # Load the initial run, title, actuals of those from the KVStore:
    
    updateControls()   # Gest the actuals and as a bonus the state.
    client = KVStore(config.host(), config.user(), config.rest_service())
    title = client.title()
    run = client.run()
    
    w.setTitle(title)
    w.setRunNumber(run)
    
    
#---------------------------------------------------------------------
#  Event log management:

def UpdateLoggers():
    # Called periodicaly to update loggers:
    
    client = Logger(config.host(), config.user(), config.rest_service())
    listing = client.list()
    eventlogWidget.updateData(listing)
    gui.controls().runControls().setLoggerState(client.isRecording())
                    
def enableDisableLogger(info):
    #  Handle toggles from the logger list checkbuttons.
    # Change the state of the loggers to what's requested by the button:
    state = info[0]
    dest = info[1]
    client = Logger(config.host(), config.user(), config.rest_service())
    if state:
        client.enable(dest)
    else:
        client.disable(dest)

def eventlogToggle(state):
    # Set the global event log state
    client = Logger(config.host(), config.user(), config.rest_service())
    if state == Qt.Checked:
        client.record(True)
    else:
        client.record(False)
    
#---------------------------------- Entry point -----------------------
#


app = QApplication(sys.argv)
Updater = QTimer()
Updater.start(UPDATE_INTERVAL*1000)

# Updater is a timer that can be hooked into to do periodic updates... It will run every
# UPDATE_INTERVAL seconds.

# We need to either read or create the configuration:

config = get_configuration()

# Now the GUI:

main_window = QMainWindow()
gui = MainGui()

#  Add the dynamic program list:
programlist = addProgramDisplay(gui)
filter = config.readouts()
programlist.model().setFilter(config.readouts())
Updater.timeout.connect(updateProgramsTab)

# Set up output logging:

outputMonitor = OutputMonitor(config.host(), config.user(), config.monitor_service())
Updater.timeout.connect(updateLogWindows)

# set up the run info connections to the gui: 

controls_widget = gui.controls()
load_initial_controls(controls_widget)
controls_widget.updateTitle.connect(setTitle)
controls_widget.updateRunNumber.connect(setRun)
controls_widget.incRun.connect(incRun)
controls_widget.transition.connect(reqTransition)
Updater.timeout.connect(updateControls)


# Set up the event log manager window:

eventlogWidget = LoggerTable(gui)
gui.tabs().addTab(eventlogWidget, 'Loggers')

# Hook in slots for updating the eventlog widget and
# the toggle signal so we can change the enables of event loggers:

Updater.timeout.connect(UpdateLoggers)
eventlogWidget.toggle.connect(enableDisableLogger)
gui.controls().runControls().logtoggle.connect(eventlogToggle)

# Show the main window and run the app.

main_window.setCentralWidget(gui)

main_window.show()

app.exec()