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
This module provides graphical user interface elements for the
pybufdump utility a graphical buffer dumper.
'''
from PyQt6.QtWidgets import (
    QMainWindow,  QToolBar, QTextEdit, QWidget, QStyle, QFileDialog, QMessageBox
)
from PyQt6.QtGui     import QFont, QAction
from PyQt6.QtCore    import pyqtSignal, Qt

import importlib.machinery

class DumpWidget(QTextEdit):
    '''
    This is a text widget that:
    -  Has a fixed font
    -  Is readonly.
    -  Provides a convenience function to set its contents that
       is a bit kinder to the eye than setPlainText.
    '''
    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)
        self.setReadOnly(True)
        
        fixed_font = QFont('Monospace')
        fixed_font.setStyleHint(QFont.StyleHint.Monospace)
        
        self.setFont(fixed_font)
        
    def setText(self, text : str) -> None:
        '''
            Erase the current text and set it to 
            the text parameters
            
            @param text : str - new contents of the
                               widget.
        '''
        self.setPlainText(text)

class MainWindow(QMainWindow):
    '''
    This is the main window for the dumper.  It has:
    1. A menubar with pre-stocked menus.
      - File menu with:
         Open... to open an event file.
         Plugin... to load a formatting plugin.
         Exit...   to exit the application
     - A filter menu with:
        Filter Types...  to request that only some ring item types are shown.
        Remove Filter    to remove any existing filter.
    2. A toolbar with a Next event button which gets the next event in the filtered set.
    3. A DumpWidget which provides the view of an event (the central widget).
    
    Signals:
      open   - Open the requested file for the data source, closing an existing sourcde.
      plugin - Load the requested plugin file.
      exit   - Exit requested and confirmed,  do any needed cleanup.
      filter - Set a new event filter.
      clearfilter - clear any existing event filter.
    
    '''        
    open = pyqtSignal(str)
    plugin = pyqtSignal(str)
    exit  = pyqtSignal()
    filter = pyqtSignal(list)
    clearfilter = pyqtSignal

    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        
        self._dump = DumpWidget(self)
        self.setCentralWidget(self._dump)
        
        self._makeMenus()
        self._makeToolbars()
        
    
    # Construction Utiltities:
    
    def _makeMenus(self) -> None:
        menubar = self.menuBar()
        
        # File menu:
        
        file = menubar.addMenu('File')
        
        open = QAction('Open...', file)
        open.triggered.connect(self._open)
        file.addAction(open)
        
        plugin = QAction('Load Plugin...', file)
        plugin.triggered.connect(self._load)
        file.addAction(plugin)
        
        
        file.addSeparator()
        
        exit = QAction('Exit...', file)
        exit.triggered.connect(self._exit)
        file.addAction(exit)
        
        # Filter menu:
        
        filter = menubar.addMenu('Filters')
        
        types = QAction('Filter Types...', filter)
        filter.addAction(types)
        clear = QAction('Clear Filters', filter)
        filter.addAction(clear)
        
        

    def _makeToolbars(self) -> None:
        toolbar = QToolBar(self)
        
        nextpixmap = QStyle.StandardPixmap.SP_MediaPlay
        nexticon   = self.style().standardIcon(nextpixmap)
        next    = QAction(nexticon, 'Next', toolbar)
        toolbar.addAction(next)
        self.addToolBar(Qt.ToolBarArea.BottomToolBarArea, toolbar)
    
    # Internal slots:
    
    def _open(self) -> None:
        # Handle the File->Open by prompting for 
        # an event file and emitting the
        # open signal if one is chosen.
        
        file,_ = QFileDialog.getOpenFileName(
            self, 'Choose Event File', '.', 
            'Event Files (*.evt);;All Files (*)', "*.evt"
        )
        if file.strip():
            self.open.emit(file)
    
    def _load(self) -> None:
        # Handle the File->Load menu optino prompting for
        # a python or so file and emitting plugin for the file
        # specified.
        compiled_suffixes =  importlib.machinery.EXTENSION_SUFFIXES
        compiled_suffixepatterns = [f'*.{s}' for s in compiled_suffixes]
        csuffixes = ', '.join(compiled_suffixepatterns)
        file, _ = QFileDialog.getOpenFileName(
            self, 'Choose plugin file', '.',
            f'Python module (*.py);;Compiled Python Module ({csuffixes};;All Files (*))'
            '*.py'
        )
        if file.strip():
            self.plugin.emit(file)
    
    def _exit(self) -> None:
        # if the user confirms the exit,
        # emit the exit signal.
        
        response = QMessageBox.question(
            self, 'Exit?', 'Do you really want to exti?'
        )
        if response == QMessageBox.StandardButton.Yes:
            self.exit.emit()
        
# Test code

if __name__ == '__main__':
    import sys
    from PyQt6.QtWidgets import QApplication
    
    def open(path):
        print('Open', path)
    def load(path):
        print('Load', path)
    app = QApplication(sys.argv)
    win = MainWindow()
    win.show()
    win.centralWidget().setText('line1\nline2\nanother line')
    win.open.connect(open)
    win.exit.connect(sys.exit)
    
    win.plugin.connect(load)
    sys.exit(app.exec())
    
        