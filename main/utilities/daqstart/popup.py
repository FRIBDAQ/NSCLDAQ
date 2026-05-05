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
  This python script replaces the popup.tcl script to provide
  a program that can be run from inside scripts to make a popup
  dialog. A few capabilties are added:
  
  --type -t  - type of dialog to popup.  This  detrmines the type of qt dialog popped up:
              it can have any of the following values:
                about - a program about message
                critical - a critical error.
                information - an informationnal message (the default)
                question    - a questhead message.
                warning    - a warning message.
                
    The return value of the button is lost, as any non-zero return value from the program
    (the natural way to do this) is interpreted as an error.
    
    @todo figure out a way to get the button value back to the caller.
'''

import argparse
import sys
from PyQt5.QtWidgets import QMessageBox, QApplication, QMainWindow
from PyQt5.Qt import *

#  Command line option definitions:

parser = argparse.ArgumentParser(
    prog='popup', description='Make a popup dialog', 
)

parser.add_argument('message', help='The message to display')
parser.add_argument('-t', 
        '--type', choices=['about', 'critical', 'information', 'question', 'warning'],
        default='information',
        help='Type of dialog to display defaults to "informaton"'
)
parser.add_argument('-T', '--title', default='popup',
                    help='Dialog box title.  Defeaults to "popup"')

command_args = parser.parse_args()
message      = command_args.message
title        = command_args.title
type         = command_args.type


# Figure out the type of dialog to produce:
app = QApplication(sys.argv)

if type == 'about':
    icon = QMessageBox.NoIcon
elif type == 'critical':
    icon = QMessageBox.Critical
elif type == 'information':
    icon = QMessageBox.Information
elif type == 'question':
    icon = QMessageBox.Question
elif type == 'warning':
    icon = QMessageBox.Warning
else:
    print(f'Invalid dialog type {type}', file=sys.stderr)    
    sys.exit(-1)
    
win = QMessageBox(icon, title, message, QMessageBox.Ok)

win.exec()
sys.exit()


