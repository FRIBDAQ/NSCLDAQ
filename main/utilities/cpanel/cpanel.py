#!/usr/bin/env python



#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   cpanel.py
# @brief  Control panel for state manager
# @author <fox@nscl.msu.edu>

from nscldaq.cpanel       import cpanelWidget
from nscldaq.statemanager import StateMonitor
from nscldaq.statemanager import Utilities
from PyQt4                import QtGui, QtCore

import argparse
import getpass
import os
import sys

##
#  This program creates a cpanelWidget and connects it to the state manager.
#
#  The state manager is located by a fairly complex but determinitid set of rules:
#
#  *  Command line parameters are used to determine the host and, if supplied
#     service names used by the state manager.
#  *  If command line parameters are not supplied environment variables that
#     are supplied by the Boot program are used specifically:
#     TRANSITION_REQUEST_URI', 'TRANSITION_SUBSCRIPTION_URI'
#  *  Finally if none of these methods provide the necessary information
#     to connect to the state manager, an assumption is made that the state
#     manager is running on default services in localhost.
#
#
#  State manager transitions are used to update the state of the UI while
#  we connect to the GUI's buttonPush signal and use our slot to intiate
#  state transitions.
#


##
# parseArgs
#   Define and parse the parameters.  The we understand the following
#   options only:
#   *  --version, -v - Print version of program and exit.
#   *  --server, -n  - Host on which the state manager server is running.
#   *  --state-service, -s
#                    - Service name used for state manager's state/transition
#                      publications.
#   *  --transition-service, -t
#                    - Service name that accepts state transition requests.
#
def parseArgs():
    parser = argparse.ArgumentParser(
        description='Control panel for the experiment'
    )
    parser.add_argument(
        '-v', '--version', help='Print program version and exit',
        action='store_const', const=1
    )
    urigroup = parser.add_argument_group(
        'URI', 'Specify State manager via URIs'
    )
    urigroup.add_argument(
        '-s', '--state-uri', help='State publication URI'
    )
    urigroup.add_argument(
        '-t', '--transition-uri', help='Transition request URI'
    )
    servicegroup = parser.add_argument_group(
        'Service', 'Specify State manager via host/service-names'
    )
    servicegroup.add_argument(
        '-n', '--host', help='Host on which the state manager runs'
    )
    servicegroup.add_argument(
        '-S', '--state-service', help='Service name for state publication'
    )
    servicegroup.add_argument(
        '-T', '--transition-service', help='Service name for transition requests'
    )

    return parser.parse_args()
    
##
# connectStateManager
#
#   Figures out the URI's for the state manager and creates the state manager
#   API.
#
# @param args - Parsed command line arguments.
# @return StateMonitor - the created API to the state manager.
#
def connectStateManager(args):
    # First are both URI's defined by command parameters:
    
    stateUri = None
    transUri = None
    if (args.state_uri != None) and (args.transition_uri != None):
        stateUri = args.state_uri
        transUri = args.trans_uri
    
    # Try the service names:
    
    if (stateUri == None) and(args.host != None) and \
       (args.state_service != None) and (args.transition_service != None):
        stateUri = 'tcp://%s:%s' % (
            args.host, Utilities.getPort(
                args.host, args.state_service, getpass.getuser()
            )
        )
        transUri = 'tcp://%s:%s' % (
            args.host, Utilities.getPort(
                args.host, args.transition_service, getpass.getuser()
            )
        )
    
    # If we don't have an answer yet, try the environment variables
    # again we need both of them (TRANSITION_REQUEST_URI', 'TRANSITION_SUBSCRIPTION_URI')
    
    if stateUri == None:
        stateUri = os.getenv('TRANSITION_SUBSCRIPTION_URI')
        transUri = os.getenv('TRANSITION_REQUEST_URI')
        if (stateUri == None) or (transUri == None):
            stateUri = None
            transUri = None
    
    # If we still don't have an answer look up the URI's in the local host.
    
    if stateUri == None:    
        statePort = Utilities.getPort('localhost', 'StatePublish', getpass.getuser())
        transPort = Utilities.getPort('localhost', 'StateRequest', getpass.getuser())
        stateUri  = 'tcp://localhost:%d' % (statePort)
        transUri  = 'tcp://localhost:%d' % (transPort)
    
    #  Now create and return the state monitor
    
 
    
    sm =  StateMonitor.StateMonitor(transUri, stateUri)
    
    #  Register all state transitions to go to the UI state update:
    #
    for state in ['NotReady', 'Booting', 'Ready', 'Active']:
        sm.register(state, reportTransition, None)
    
    # Register the handler for the title and run numbers:
    
    sm.setRunNumberHandler(runChanged)
    sm.setTitleHandler(titleChanged)
    sm.setRecordingHandler(recordingChanged)
    
    return sm

##
# createUi
#   Create the user interface and connect to the buttonPush signal so button
#   presses become transition requests.
#
#
# @return app,ui - The application object and the user interface object.

def createUi():
    app = QtGui.QApplication(sys.argv)
    w   =  cpanelWidget.ControlPanel()
    w.setWindowTitle('Run Control Powered by Qt5')
    w.show()
    w.buttonPush.connect(onButtonPress)
    w.runChanged.connect(onNewRunNum)
    w.titleChanged.connect(onNewTitle)
    w.recordChanged.connect(onNewRecordingState)
    return app,w



##
# eventLoop
#
#   Interleave the event loop of the state monitor with the Qt4 eventloop.
#
# @param app - Qt4 application object.
# @param sm  - State manager object.
#
def eventLoop(app, sm):
    poller = sm.poller
    
    # TODO :   Figure out how to make the control panel exit!!
    
    app.aboutToQuit.connect(exit)
    ui.destroyed.connect(exit)
    while len(app.allWidgets()) > 0:
        poller.poll(100)
        app.processEvents(QtCore.QEventLoop.AllEvents, 100)
        app.sendPostedEvents()

## Stub
def reportTransition(sm, prior, current, cbarg):
    ui.setState(current)
    
def runChanged(newNum):
    ui.setRun(newNum)
    
def titleChanged(newTitle):
    ui.setTitle(newTitle)

def recordingChanged(state):
    ui.setRecord(state)

def onButtonPress(transition):
    smApi.requestTransition(str(transition))
def onNewRunNum(run):
    smApi.setRun(run)

def onNewTitle(title):
    smApi.setTitle(str(title).encode('ascii', errors='backslashreplace'))

def onNewRecordingState(state):
    smApi.setRecording(state)

#----------------------------------------------------------------------------
#
#  Main:
#   * Parse the arguments
#   * Connect to the state manager
#   * Create the graphical user interface.
#   * Run a hybrid event loop that swaps back and forth between the Qt and
#     State manager's event loop so both can be satisfied.

args = parseArgs()
print(args)

# --version means print version and exit:

if args.version != None:
    print("NSCLDAQ Cpanel version %s" % (args.version))
    exit(0)
    
smApi= connectStateManager(args)

app,ui   = createUi()
eventLoop (app, smApi)