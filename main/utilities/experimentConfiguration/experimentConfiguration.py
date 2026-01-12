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
# @file   <filename>
# @brief  <short description>
# @author <fox@nscl.msu.edu>
import sys
from PyQt4 import QtGui

from nscldaq.expconfiguration import filemenu, projectdisplay


##
# @class MainWindow
#
#    Subclass of the QMainWindow widget which implements the top levels of the
#    application views.
#
class MainWindow(QtGui.QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setupUi()
    

        
    ##
    # setupUi
    #   Drives the setup of the gui, and hooks us into a controller.
    #   The controller will be responsible for hooking us up to a model.#
    #   
    def setupUi(self):
        
        # Set up the menus.
        
        self._fileMenu = filemenu.FileMenu(self)
        self._projectUi= projectdisplay.ProjectDisplay(self)
        
        # Enable the statusbar:
        
        statusbar = self.statusBar()
        statusbar.showMessage('Ready')
        
        self.setGeometry(300,300, 500, 450)
        self.setWindowTitle('Experiment configuration editor')
        self.show()
        
        
def main():
    app = QtGui.QApplication(sys.argv)
    ex  = MainWindow()
    ex.setWindowTitle("Experiment configuration editor --  Powered by Qt5")
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()