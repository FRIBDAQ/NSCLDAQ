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
    This program is a simple dispatcher for the various configuration editors
    in the managed experiment environment in FRIB/NSCLDAQ
    
    @file mg_config.py
    @brief Configure the managed experiment environment
    @author Ron Fox
'''

from PyQt6.QtWidgets import QApplication, QListWidget, QPushButton, QWidget, QVBoxLayout, QListWidgetItem
import sys
import os
import pathlib
import subprocess

# List of applications.  Each list item is a pair.
# apps[i][0] is the list widget text to display.
# double clicking that item will run $DAQBIN/apps[i][1] appending
# the name of the database file to the program.
#
apps = [
    ('Container Wizard', 'mg_container_wizard'),
    ('Readout Wizard',  'mg_readout_wizard'),
    ('Program Wizard',  'mg_program_wizard'),
    ('Event Log Wizard', 'mg_logwizard'),
    ('Containers',       'mg_cfgcontainers'),
    ('Programs',        'mg_cfgprogram'),
    ('Event Logging',    'mg_cfgEvlog'),
    ('Sequence Definition', 'mg_seqedit'),
    ('State Machine',    'mg_stateedit'),
    ('Users and Roles',  'mg_authedit'),
    ('Key/Value store',  'mg_kvedit')
    
]

def stockApplications(l : QListWidget) -> None:
    global apps
    # fill the list widget with the application list.
    # 
    appnames = [x[0] for x in apps]
    l.addItems(appnames)

def runApplication(_idx : QListWidgetItem, l : QListWidget, config: str) -> None:
    global apps
    program = apps[l.currentRow()][1]
    daqbin = os.environ['DAQBIN']
    
    program_path = pathlib.Path(daqbin)
    program_path = program_path / program

    subprocess.call([program_path, config])

def usage() -> None:
    ''' Print program usage to stderr:'''
    
    print('''
Configure the managed environment for FRIB/NSCLDAQ
Usage:  
    $DAQBIN/mg_config config_path
Where:
    config_path - is the filesystem path to the configuration database file.
    ''', file=sys.stderr)

def main() -> int:
    if len(sys.argv) != 2:
        usage()
        return -1

    config_file = sys.argv[1]
    
    app = QApplication(sys.argv)
    win = QWidget()
    layout = QVBoxLayout(win)
    win.setLayout(layout)
    
    applist = QListWidget(win)
    stockApplications(applist)
    layout.addWidget(applist)
    applist.itemDoubleClicked.connect(lambda idx : runApplication(idx, applist, config_file))
    
    exit    = QPushButton('Exit', win)
    layout.addWidget(exit)
    exit.clicked.connect(lambda: app.exit(0))
    
    
    win.show()
    
    return app.exec()
     

if __name__ == '__main__':
    sys.exit(main())


