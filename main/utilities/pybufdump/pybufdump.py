#!/usr/bin/env python3

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
@file pybufdump.py
@brief Main program for the pyqt buffer dumper.
@author Ron Fox
'''
import pyUI
import pybufdumpController
import sys
from PyQt6.QtWidgets import QApplication


def onExit() -> None:
    QApplication.instance().exit(0)



def main() -> int:
    ''' Program entry point
    
    '''
    app = QApplication(sys.argv)
    win = pyUI.MainWindow()
    win.show()
    win.resize(800, 400)
    
    _controller = pybufdumpController.BufDumpController(win, win)
    
    # Exit when requested.
    
    win.exit.connect(onExit)
    
    return app.exec()

if __name__ == "__main__":
    sys.exit(main())
