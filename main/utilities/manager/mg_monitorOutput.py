'''
Provides a python substitute for  mg_monitorOutput.py

'''

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


from PyQt6.QtWidgets import (QApplication, QMainWindow, QTextEdit, QMessageBox,
    QMenuBar, QMenu, QFileDialog)
from PyQt6.QtGui import QFont, QFontMetrics, QCursor, QAction
from PyQt6.QtCore import QObject, Qt, pyqtSignal

from nscldaq import OutputMonitorQt
import sys
import os
from collections import namedtuple

Constants = namedtuple('Constants', 
    ['LOG_WIDTH', 'LOG_HEIGHT', 'LOG_BLOCKS', 'FONT_PADDING'])

CONSTANTS = Constants(
    LOG_WIDTH = 80,        # Log window 80 chars wide.
    LOG_HEIGHT = 25,       # Log window is 25 chars high.
    LOG_BLOCKS = 1000,     # Whatever a  block is..  limit the log window contents.
    FONT_PADDING=10,       # Padding for window size computation.
)

usage_text = '''
Usage:
   mg_monitorOutput host user [service]
Where:
   host - is the host in which the manager server is running.
   user - is the username of the account that started the server.
   service - is an optional service name the manager advertised
         with the port manager for its output monitor service.
         If omitted, the default service name will be used.
'''

def usage() -> None:
    global usage_text
    # Print program usage to stderr and exit.
    
    print(usage_text, file=sys.stderr)
    sys.exit(-1)

#--------------- Gui setup helpers

def setOutputWinCharacteristics(win : QTextEdit) -> None:
    #  Setup the characteristics of the output window.
    # Set the fount to courier-12 and figure out the height/width in pixels then set it.
    
    cfont = QFont('Courier')
    cfont.setPointSize(10)
    win.setFont(cfont)
    
    metrics = QFontMetrics(cfont)
    char_width = metrics.horizontalAdvance('W')     # Some big character.
    char_height = metrics.lineSpacing()
    
    widget_width = CONSTANTS.LOG_WIDTH * char_width + CONSTANTS.FONT_PADDING
    widget_height= CONSTANTS.LOG_HEIGHT * char_height + CONSTANTS.FONT_PADDING
    win.setFixedSize(widget_width, widget_height)
    
    # Turn of editing:
    
    win.setReadOnly(True)
    
    # Limit the output size:
    
    doc = win.document()
    
    if doc is None:
        raise RuntimeError('Panic!!!  Text edit has null document!!!')

    doc.setMaximumBlockCount(CONSTANTS.LOG_BLOCKS)
    
    # Set the viewport cursor to something a bit less obnoxious than the default Ibeam:
    # @todo The resulting cursor is still enormous compared with what I want but
    # it's better than if I didn't set it.
    
    viewport = win.viewport()     # This is the widget whose cursor we must change.
    cursor = QCursor(Qt.CursorShape.ArrowCursor)  #  Normal arrow cursor.
    viewport.setCursor(cursor)  
    
    
# This class handles all the file menu stuff.
# It has signals like startLogging, stopLogging
# 

class FileMenu(QObject):
    logfileChanged = pyqtSignal(str) # Passed the filename.
    startLogging = pyqtSignal()   
    stopLogging  = pyqtSignal()      # stop logging.
    
    def __init__(self, menu : QMenu, parent= None):
        super().__init__(parent)
        
        # Add the actions to the menu:
        self._menu = menu
        self._logfileName = None       # Will hold the log file name when set.
        
        self._log_file = QAction('Log File..', self._menu)
        self._menu.addAction(self._log_file)
        self._log_file.triggered.connect(self._setLogFile)
    
        self._logging_enable = QAction('Log', self._menu)
        self._logging_enable.setCheckable(True)
        self._logging_enable.setEnabled(False)    # not until a log file is set.
        self._menu.addAction(self._logging_enable)
        self._logging_enable.triggered.connect(self._changeEnable)
    
    # Slots
    
    def _setLogFile(self):
        # Called when 'Log file...' action is triggered.
        # - Prompt for a log file.
        # - if one is given, then emit logfileChanged with that as the logfile
        # and enable the 'Log' action.
        
        prompt = QFileDialog(self._menu, 'Select log file', os.getcwd(), '*.log')
        prompt.setAcceptMode(QFileDialog.AcceptMode.AcceptSave)
        prompt.setOptions(QFileDialog.Option.DontConfirmOverwrite)   # We're appending so ovewrite is ok.
        prompt.setFileMode(QFileDialog.FileMode.AnyFile)
        if prompt.exec():
            logfile = prompt.selectedFiles()
            if len(logfile) != 0:
                file = logfile[0]
                self.logfileChanged.emit(file)
                self._logging_enable.setEnabled(True)
        
        
        
        
    def _changeEnable(self):
        if self._logging_enable.isChecked():
            self.startLogging.emit()
        else:
            self.stopLogging.emit()
            
            
def setupMenus(menubar : QMenuBar) -> None:
    file_menu = menubar.addMenu('&File')
    
    
    file_menu_object = FileMenu(file_menu, menubar)
    
    return (file_menu_object,)
 
    
#---------------------- logging
class Logger(QObject):
    # This class is connected to the File menu's logging signals and
    # provides a method to append log data. 
    
    def __init__(self, menu, parent=None):
        super().__init__(parent)
        self._logging = False
        self._filename = None
        
        # Seems like I should be able to do this with lambdas but I'm evidently
        # Too stupid.
        
        menu.logfileChanged.connect(self._setLogFile)
        menu.startLogging.connect(lambda : self._setState(True))
        menu.stopLogging.connect(lambda: self._setState(False))
        
    
    #  Add to the log file if enabled:
    #  We only hold the logfile open long enough to write.
    #
    def append(self, text) :
        if self._logging:
            with open(self._filename, 'a') as f:
                f.write(text)
    
    
    #  Slots:
    def _setLogFile(self, path):
        self._filename = path
    
    def _setState(self, state):
        self._logging = state
        
        
        
        
#---------------------- end loggers.    
    
#-------------------- End of gui setup helpers.
#-------------------- slots:
    
def append_output(text : str, win : QTextEdit, logger : Logger) -> None:

    #  Slot  new output from the manager.
    #  Simply append it to the output_win:
    
    win.append(text)
    logger.append(text)   # logs if enabled.

def connection_lost(app : QApplication, output: QTextEdit) -> None:
    QMessageBox.critical(
        output,
        'Connection Failed',
        'The connection to the manager server has been lost and could not be re-established'
    )
    app.exit(-1)
    
#--------------------- end slots   
def main() -> int:
    # Entry point.
    
    
    #  Figure out the program arguments.
    
    if len(sys.argv) < 3:
        usage()
    
    host = sys.argv[1]
    user = sys.argv[2]
    service = None
    
    if len(sys.argv) == 4:
        service = sys.argv[3]
    elif len(sys.argv) > 4:
        usage()

    # Build the UI
    
    app = QApplication(sys.argv)
    main_win = QMainWindow()
    output_win = QTextEdit(main_win)
    main_win.setCentralWidget(output_win)
    setOutputWinCharacteristics(output_win)
    
    
    # Set up  the menus:
    
    menubar = main_win.menuBar()
    (file_menu,)  = setupMenus(menubar) 
    log= Logger(file_menu)     
    
    # Create the output monitor object and connect it's signals to what we need to
    # append new text to the widget:
    
    logger = OutputMonitorQt.OutputMonitorQt(host, user,  parent=output_win) if service is None else \
            OutputMonitorQt.OutputMOnitor(host, user, service)
    logger.input.connect(lambda text: append_output(text, output_win, log))
    logger.lost.connect(lambda : connection_lost(app, output_win))
    
    
    # Start the application.
    
    main_win.show()
    sys.exit(app.exec())

    
if __name__ == "__main__":
    main()