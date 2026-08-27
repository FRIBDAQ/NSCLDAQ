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
@file lg_wrnote.py
@brief Write notes in the logbook, with rich text (markdown).
@author Ron Fox
'''
import sys
from PyQt6.QtWidgets import (
    QWidget, QLabel, QTextEdit, QDialog, QFileDialog, QHBoxLayout, 
    QVBoxLayout, QMenu, QApplication, QGridLayout, QLineEdit
)
from PyQt6.QtGui     import QContextMenuEvent, QAction
from PyQt6.QtCore    import Qt
from nscldaq.LogBook import LogBook, LogBookUIUtilities, logbookadmin

class ImagePromptDialog(QFileDialog):
    ''' Extend the file dialog with a caption
        prompt
        To do this we must 
        1. Turn off the OS file prompt dialog.
        2. Get our layout and add our widgets to that layout.
        
    '''
    def __init__(self, parent : QWidget | None):
        super().__init__(parent)
        
        self.setOption(QFileDialog.Option.DontUseNativeDialog, True)
        
        layout = self.layout()
        rows   = layout.rowCount()
        
        extralayout = QHBoxLayout()
        extralayout.addWidget(QLabel('Caption:', self))
        self._caption = QLineEdit(self)
        extralayout.addWidget(self._caption)
        
        layout.addLayout(extralayout, rows, 0, 1, -1)

        # only allow one, existing file:
        
        self.setFileMode(QFileDialog.FileMode.ExistingFile)

        # Let's set the filters to support some interesting image file
        # types
        
        self.setNameFilters(['Image Files (*.png *.jpg *.bmp *.svg, *.gif)', 'All Files (*)'])
        
    def caption(self) -> str:
        return self._caption.text()
    
    
class NoteEditor(QTextEdit):
    '''
        A Textedit with an extended menu that can prompt for the
        insertion of an image and caption at the current position
    '''
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        self.setContextMenuPolicy(Qt.ContextMenuPolicy.DefaultContextMenu)
    
    def contextMenuEvent(self, event : QContextMenuEvent) -> None:
        '''
            Called on the right click to bring up a context menu.
            We get the standard context menu and add an 'insert image'
            action.
        '''

        menu : QMenu = self.createStandardContextMenu(event.pos())
        imgaction = QAction('Insert Image...', menu)
        menu.addAction(imgaction)
        imgaction.triggered.connect(self._promptImage)
        menu.exec(event.globalPos())
    
    def _promptImage(self, _ : bool) -> None:
        prompt = ImagePromptDialog(self)
        if prompt.exec() == QDialog.DialogCode.Accepted:
            file = prompt.selectedFiles()
            if len(file) > 0:
                file = file[0]
                caption = prompt.caption()
                self.insertPlainText(f'![{caption}]({file})')
            
       

class NoteCreator(QWidget):
    '''
        Editor for notes includes PersonChooser for the author
        and a run chooser for the associated runs,
        a banner indicating the current author and associated run.
        Below this banner is a QTextEdit for editing the note. The
        QTextEdit has a context menu with the standard editor
        context menu items as well as an 'Inser Image...' menu entry
        that allows image files and links to them selected.
        
        Methods of interest:
        
        setRuns    - Set the runs that can be selected
        setPeople  - Set the authors that can be selected.
        selectedRun -Return the selected run (could be None)
        author     - Return the seleced author.
        noteText   - Return the raw note text.
        noteImages - Return the images associated with the note and their offsets.
        
    '''
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        
        self._layout = QVBoxLayout()
        self.setLayout(self._layout)
        
        # The choosers:
        
        chooserBar = QHBoxLayout()
        
        chooserBar.addWidget(QLabel('Run: ', self))
        self._runChooser = LogBookUIUtilities.RunChooser(self)
        chooserBar.addWidget(self._runChooser)
        
        chooserBar.addWidget(QLabel('Author: ', self))
        self._author = LogBookUIUtilities.PersonChooser(self)
        chooserBar.addWidget(self._author)

        self._layout.addLayout(chooserBar)
        
        # The note information bar:
    
        infobar = QHBoxLayout();
        
        infobar.addWidget(QLabel('Author:', self))
        self._author = QLabel('<Not Chosen>', self)
        infobar.addWidget(self._author)
        
        infobar.addWidget(QLabel('Associated with run: ', self))
        self._associatedRun = QLabel('<None>', self)
        infobar.addWidget(self._associatedRun)
        
        self._layout.addLayout(infobar)
        
        # THe text editor:
        
        self._editor = NoteEditor(self)
        self._layout.addWidget(self._editor)
        
        
        # Autonomous/internal signal handling:
        
        self._runChooser.textActivated.connect(self._setRun)  # Cook the run a bit.
        
    # Public methods:
    
    def setRuns(self, runs : LogBook.Run) -> None:
        '''
            Set the available runs a note can be associated with.
            
        '''
        self._runChooser.setRuns(runs)
            
    # Internal (private) slots:
    
    def _setRun(self, runText) -> None:
        #  If the run text is '' then set the value to <None>
        
        if not runText:
            runText = '<None>'
        
        self._associatedRun.setText(runText)
        
        
# Test code for now:

if __name__ == '__main__':
    app = QApplication(sys.argv)
    
    w = NoteCreator()
    w.show()
    
    w.setRuns(logbookadmin.listRuns())
    
    sys.exit(app.exec())