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
import argparse




def onExit() -> None:
    QApplication.instance().exit(0)

def parse_command_line() -> argparse.Namespace:
    # Define the command line parameters, parse them
    # and return the resulting namespace.
    parser = argparse.ArgumentParser(
        prog = 'pybufdump',
        description = 'Graphical dumper of FRIB/NSCLDAQ event items',
        
    )
    parser.add_argument(
        '-F', '--format-version',  type = int, default = 12,
        choices = [10,11,12],
        help = 'Ring item format version ')  
    parser.add_argument(
           '-S', '--sourceid-definitions', type = str, default=None,
            help='Path to file containing source id textual defintions'
        )
    parser.add_argument(
            '-N', '--scaler-definitions', type = str,
            default=None,
            help = 'Path to scaler definition .toml file (see pyscaler display command)'
        )
    parser.add_argument(
        '-U', '--unpack-fragments', type = bool, action=argparse.BooleanOptionalAction, 
        help = 'Unpack event built fragments from ring item in format when present'
    )
    
    
    return parser.parse_args()

def main() -> int:
    ''' Program entry point
    
    '''
    
    parameters = parse_command_line()
    print(parameters)
    
    app = QApplication(sys.argv)
    win = pyUI.MainWindow()
    win.show()
    win.resize(800, 400)
    
    controller = pybufdumpController.BufDumpController(win, win)
    controller.setFormatVersion(parameters.format_version)
    controller.setEventBuilt(parameters.unpack_fragments)
    if parameters.scaler_definitions:
        controller.setScalerFile(parameters.scaler_definitions)
    if parameters.sourceid_definitions:
        controller.setSidFile(parameters.sourceid_definitions)
    # Exit when requested.
    
    win.exit.connect(onExit)
    
    return app.exec()

if __name__ == "__main__":
    sys.exit(main())
