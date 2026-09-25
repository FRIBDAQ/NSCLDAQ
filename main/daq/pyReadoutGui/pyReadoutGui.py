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
@file pyReadoutGui.py
@brief THe main program for the python Readout GUI.
@author Ron Fox
'''
import sys

from nscldaq.readoutgui.ReadoutGuiView import ReadoutGuiMainWindow
from nscldaq.readoutgui.StateMachine import ReadoutStateMachine
from PyQt6.QtWidgets import QApplication
    

def main():
    app = QApplication(sys.argv)
    main_window = ReadoutGuiMainWindow()
    
    # Set the initial state of the buttons:
    
    main_window.centralWidget().StateControls().setState(ReadoutStateMachine.instance().state())
    
    main_window.show()
    
    
    return app.exec()

if __name__ == '__main__':
    sys.exit(main())
else:
    print('This is a main program, not intended to be loaded as a module!!!')
    sys.exit(-1)