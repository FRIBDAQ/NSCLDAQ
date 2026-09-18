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
@file testRunStateController.py
@brief Test of the run state view, and timer widgets working together.
@author Ron Fox
@note  This is not installed but does get to go int he tarball.
@note  Intended to be run after installation 
'''

from nscldaq.readoutgui import StateMachine
from nscldaq.readoutgui import StateView
from nscldaq.readoutgui import RunTimerView


from PyQt6.QtWidgets import QApplication, QWidget, QHBoxLayout
from PyQt6.QtCore    import pyqtSignal, QTimer

import sys


class Gui(QWidget):
    '''
    Megawidget that provides a runtime view, a stateview and
    laid out horizontally.  This is for test code so the 
    subwidgets are public.
    
    '''
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        
        self._layout = QHBoxLayout()
        self.setLayout(self._layout)
        
        self.buttons = StateView.StateButtons(self)
        self._layout.addWidget(self.buttons)
        
        self.timer   = RunTimerView.RunTimerView(self)
        self._layout.addWidget(self.timer)

# typing savers.

def getState() -> str:
    return StateMachine.ReadoutStateMachine.instance().state()  
    
def transition(newState : str) -> None:
    StateMachine.ReadoutStateMachine.instance().transition(newState)
    win.buttons.setState(getState())          

# signal handlers.

def completeBoot() -> None:
    transition('Halted')

def boot() -> None:
    transition('Starting')
    bootTimer.start()
    
def begin() -> None:
    transition('Active')
    win.timer.start() 

def end() -> None:
    transition('Halted')
    win.timer.stop()
    
def pause() -> None:
    transition('Paused')
    win.timer.stop()
    
def resume() -> None:
    transition('Active')
    win.timer.resume()

#  Entry point:


app = QApplication(sys.argv)    
win = Gui()


win.buttons.setState(getState())

# Connect to the button signals:

win.buttons.start.connect(boot)
win.buttons.begin.connect(begin)
win.buttons.end.connect(end)
win.buttons.pause.connect(pause)
win.buttons.resume.connect(resume)

# Connect to the end of run signal.

win.timer.end.connect(end)

# Simulate the boot timing.

bootTimer = QTimer(win)
bootTimer.setSingleShot(True)
bootTimer.setInterval(1000)
bootTimer.timeout.connect(completeBoot)

win.show()
sys.exit(app.exec())
