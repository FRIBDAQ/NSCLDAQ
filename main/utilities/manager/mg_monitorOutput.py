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


from PyQt6.QtWidgets import QApplication, QMainWindow, QTextEdit
from PyQt6.QtGui import QFont, QFontMetrics, QCursor
from PyQt6.QtCore import Qt

from nscldaq import OutputMonitorQt
import sys
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
    
    
def append_output(text : str, win : QTextEdit) -> None:

    #  Signal handler for new output from the manager.
    #  Simply append it to the output_win:
    
    win.append(text)
    
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
    
    # Create the output monitor object and connect it's signals to what we need to
    # append new text to the widget:
    
    logger = OutputMonitorQt.OutputMonitorQt(host, user) if service is None else \
            OutputMonitorQt.OutputMOnitor(host, user, service)
    logger.input.connect(lambda text: append_output(text, output_win))
    
    
    # Start the application.
    
    main_win.show()
    sys.exit(app.exec())

    
if __name__ == "__main__":
    main()