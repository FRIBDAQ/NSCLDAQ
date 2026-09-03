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
@file lg_wrnote.py
@brief Write notes in the logbook, with rich text (markdown).
@author Ron Fox
'''
import sys
from collections.abc import Iterable
from PyQt6.QtWidgets import (
    QWidget, QLabel, QTextEdit, QDialog, QFileDialog, QHBoxLayout, 
    QVBoxLayout, QMenu, QApplication, QGridLayout, QLineEdit
)
from PyQt6.QtGui     import QContextMenuEvent, QAction
from PyQt6.QtCore    import Qt
from nscldaq.LogBook import LogBook, LogBookUIUtilities, logbookadmin
from nscldaq.mg_configutils import SaveDialog

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
        self.setPlaceholderText('Edit note text here')
    
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
        self._authorChooser = LogBookUIUtilities.PersonChooser(self)
        chooserBar.addWidget(self._authorChooser)

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
        self._authorChooser.textActivated.connect(self._author.setText)
        
    # Public methods:
    
    def setRuns(self, runs : Iterable[LogBook.Run]) -> None:
        '''
            Set the available runs a note can be associated with.
            @param runs runs : Iterable[LogBook.Run] - the runs that can be selected.
        '''
        self._runChooser.setRuns(runs)
    
    def selectedRun(self) -> LogBook.Run | None:
        '''
        @return runs : LogBookRun | None - The run 
        @retval  None - the user does not want the note associated with a run.
        '''
        
        return self._runChooser.selectedRun()
    
    
    def setPeople(self, people : Iterable[LogBook.Person]) -> None:
        '''
        @param people - the people that can be selected as authors.
        
        '''
        self._authorChooser.setPeople(people)
        self._author.setText(self._authorChooser.currentText())
        
    def author(self) -> LogBook.Person | None:
        '''
            @return LogBook.Person | None - the person selected as author.
            @retval None - this can only happen if setPeople was never called with a non-empty list.
        '''
        return self._authorChooser.selected()
    
    
    def noteText(self) -> str:
        '''
            @return str - the raw note text.
        '''
        return self._editor.toPlainText()      
    
    def noteImages(self) -> list[tuple[int, int, str]]:
        '''
            @return list[tuple(int, str)] - List of image information. The first
                item in each tuple is the offset into the note text of an image secification
                (location of the !) The second item in each tuple is offset to the end of the
                image specification (location of ')').  
                The final item in tuple is the image filename.
            @note Since we only allowed the display of readable files in our image selector
                dialog    
          
        '''
        text = self.noteText()
        start= 0
        result = []
        while True:
            nextimage = self._findNextImage(text, start)
            if nextimage[0] == -1:  
                break
            else:
                result.append(nextimage)
                start = nextimage[1]    # Start next search at end of the link.
        return result
    
    
    # Internal (private) slots:
    
    def _setRun(self, runText) -> None:
        #  If the run text is '' then set the value to <None>
        
        if not runText:
            runText = '<None>'
        
        self._associatedRun.setText(runText)
    
    def _findNextImage(self, text : str, start: int) -> tuple[int, int, str]:
        # Find the next image in the text or return -1,-1, '' if there isn't 
        # a next masge
        
        startIndex = text.find('![', start)
        if startIndex == -1:
            return (-1, -1, '')     # No more image links.
        
        # Locate the filename limits looking for (...)
        
        fstartIndex = text.find('(', startIndex)
        if fstartIndex == -1:
            return (-1, -1, '')
        
        fendIndex = text.find(')', fstartIndex)        
        if fendIndex == -1:
            return (-1, -1, '')
        
        # We have a valid filename construct, extract it:
        
        filename = text[fstartIndex+1:fendIndex]
        return (startIndex, fendIndex+1, filename)
    
class NoteDialog(SaveDialog):
    def __init__(self, parent : QWidget | None = None):
        super().__init__(NoteCreator(), parent)
        self.resize(500,500)
    


# Entry point:

def main() -> int:
    app = QApplication(sys.argv)
    dialog = NoteDialog()
    
    #  Populate the dialog selectors
    
    dialog.workarea().setPeople(logbookadmin.listPeople())
    dialog.workarea().setRuns(logbookadmin.listRuns())
    
    # Run the dialog if the result is accepted, fish the stuff
    # we need out of the dialog and add the note.
    
    if dialog.exec() == QDialog.DialogCode.Accepted:
        author = dialog.workarea().author()
        run    = dialog.workarea().selectedRun()
        rawText = dialog.workarea().noteText()
        image_info = dialog.workarea().noteImages()
        imageOffsets = [i[0] for i in image_info]
        imageFiles   = [i[2] for i in image_info]
        
        logbookadmin.addNote(author, rawText, imageFiles, imageOffsets, run)
    
    return 0
    


if __name__ == '__main__':
    sys.exit(main())