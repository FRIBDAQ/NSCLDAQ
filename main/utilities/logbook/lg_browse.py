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
@file lg_browse.py
@brief Logbook browser application (replaces lg_browse.tcl)
@author Ron Fox
'''
import os
import pathlib
import subprocess
import sys
from collections.abc import Iterable

from nscldaq.LogBook import LogBook, LogBookUIUtilities, logbookadmin
from PyQt6.QtCore import QFileInfo, QModelIndex, QPoint, Qt, pyqtSignal
from PyQt6.QtGui import (
    QAction,
    QBrush,
    QColor,
    QPalette,
    QStandardItem,
    QStandardItemModel,
)
from PyQt6.QtWidgets import (
    QApplication,
    QFileDialog,
    QMenu,
    QMessageBox,
    QTabWidget,
    QTreeView,
    QWidget,
)


def _timestring(stamp) -> str:
    return LogBookUIUtilities.timestring(stamp)   # Factored out.
    
#---------------------------------    LogBook tree browser -------------------------------------------    
class LogBookModel(QStandardItemModel):
    '''
        This model provides a hierarchical view of the
        items in a logbook.  The top level always has a 'None' item which
        can have, as children notes that are not associated with any
        specific run.
        
        Other top level items can be Runs, which will contain their
        transitions and associated notes.
        
        attributes:
           unassociatedNotes - The unassociated notes.
           runs              - Runs and their associated notes.
                               
        
        Runs are described by a tuple of the form:
        tuple[LogBook.Run, list[LogBook.Note]] where
        the list, the notes associated with the run.  Note as well that for each run, there will be
        'invisible' elements that log the transitions of the run.   The notes in the run will be
        inserted into the underlying model interspersed with the transitions so that everything
        is time ordered, within a run.  The runs are listed in order of run number.

        Internals notes: 
        * Extensive use is made of associated item data so that we don't need
          to maintain an parallel data structures.
        * Init creates the 'None' top level and maintains a reference to it.
        * Notes added to 'None' are children of None and the col 0 item has the note
          as associated data.
        * Runs have the Run as associated data and generate children as follows
          a time ordered list is made of transitions and associated notes.  These are inserted
          in order and 
              - for transitions, the associated data are the Transition object, chidren are
                the shift members at the time of the transition
              - For notes, the associated data is the note itself.

    '''
    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)
        
        # Create the none top level:

        self.clear()
    
    def clear(self) -> None:
        super().clear()
        self._noneItem = QStandardItem('None')
        self._setFlagsAndAppendRow(self, [self._noneItem,])
        self.setColumnCount(5)
    
        self.setHorizontalHeaderLabels(['Run', 'Title', 'Person/Shift', 'State/Time', 'Remark'])
            
    def setUnassociatedNotes(self, notes : Iterable[LogBook.Note]):
        '''
            Set the notes that are below the None item....that is notes that are
            not associated with any run.  These will be sorted by time.
            
            @param notes : Iterable[LogBook.Note] - the notes to put under None
            @note  the children of notes are first destroyed.
        '''
        # Clear the existing children:
        
        while self._noneItem.hasChildren():
            self._noneItem.takeRow(0)
            
        # Make a time sorted list of rows:
        
        sortedNotes = sorted(notes, key=lambda n: n.time)
        for note in sortedNotes:
            self._addNote(self._noneItem, note)
           
    def unassociatedNotes(self) -> list[LogBook.Note]:
        '''
            Return the unassociated logbook notes.
        '''
        result : list[LogBook.Note] = []
        if self._noneItem.hasChildren():
            childCount = self._noneItem.rowCount()
            for row in range(childCount):
                child = self._noneItem.child(row)   # The placeholder.
                note = child.data()
                result.append(note)
            
        return result
    
    def setRuns(self, runs :Iterable[tuple[LogBook.Run, list[LogBook.Note]]]):
        # First we need to get rid of all but the 'None' placeholder and its children.
        
        if self.rowCount() > 1:
            self.removeRows(1, self.rowCOunt-1)
            
        for run in runs:
            # We need a place holder with the run number that has, as data, the run:
            
            runInfo = run[0]
            notes   = run[1]
            
            # The run line:
            
            number = QStandardItem(str(runInfo.number))
            number.setData(runInfo)
            
            title  = QStandardItem(runInfo.title)
            shift  = QStandardItem('')
            state  = QStandardItem(runInfo.last_transition())
            self._setFlagsAndAppendRow(self, [number, title, shift, state])
            
            events = self._mergeTransitionsAndNotes(runInfo, notes)
            
            for event in events:
                # Event is either a LogBook.Transition or a LogBookNote.
                
                match type(event):
                    case LogBook.Note:
                        # Insert a note:
                        self._addNote(number, event)
                    case LogBook.Transition:
                        # Insert a transition child:
                        
                        self._addTransition(number, event)
                    case _:
                        raise TypeError(
                            f'BUG: LogBookModel.setRuns event list unexpected type:  {type(event).__name__} please report this'
                        )
    
    # Utilities
    def _addTransition(self, parent : QStandardItem, transition : LogBook.Transition) -> None:
        # Add a transition to a parent:
        
        
        
        text = QStandardItem(transition.transition_name)
        text.setData(transition)              # THe transition text has the full transition as data.
        shiftName = QStandardItem(transition.shift.name)
        shiftName.setData(transition.shift)
        transitionTime = QStandardItem(_timestring(transition.time))
        
        self._setFlagsAndAppendRow(parent,[text, QStandardItem(''), shiftName, transitionTime])
                         
        # Add the shift members as children to 'text':
        
        for member in transition.shift.members:
            memberName = QStandardItem(LogBookUIUtilities.personName(member))
            self._setFlagsAndAppendRow(text, [QStandardItem(''), QStandardItem(''), memberName])
        
    def _addNote(self, parent : QStandardItem, note: LogBook.Note) -> None:
        # Add a note child to a parent item:
        
        placeholder = QStandardItem('Note')
        placeholder.setData(note)                        # Col 0 has note as associated data.
        
        titleText   = note.contents.split('\n')[0]
        title       = QStandardItem(titleText)
        
        author = QStandardItem(LogBookUIUtilities.personName(note.author))
        
        datestring = _timestring(note.time)
        
        time   = QStandardItem(datestring)
        
        self._setFlagsAndAppendRow(parent,[placeholder, title, author, time])
    def _mergeTransitionsAndNotes(
        self,
        run : LogBook.Run, notes : Iterable[LogBook.Note]) -> list[LogBook.Transition | LogBook.Note] :
        # make a time ordered merged list of transitions and notes.
         
        # first just make the merged list:
        
        merged_list = []
        for tnum in range(run.transition_count()):
            merged_list.append(run.get_transition(tnum))
        merged_list.extend(notes)
        
        # now some fancy footwork to return the sorted one:
        # both notes and transitions have time attributes so just:
        
        merged_list.sort(key=lambda item: item.time)
        return merged_list
        
        
    def _setFlagsAndAppendRow(self, parent: QStandardItem | QStandardItemModel, row : list[QStandardItem]) -> None:
        # Set item appropriate flags (mostly we want to turn off editing)
        # Then append the row in the appropriate parent:
        
        for item in row:
            flags = item.flags()
            flags = flags & (~Qt.ItemFlag.ItemIsEditable)
            item.setFlags(flags)
        parent.appendRow(row)
class LogBookView(QTreeView):
    # The view for LogBookModel.
    # Signals:
    
    noteWritten = pyqtSignal()      # A new note was written
    refresh     = pyqtSignal()      # Refresh context menu item selected.
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        
        # Double clicking a note will render it as a
        # in a web browser as html:
        
        self.doubleClicked.connect(self._onDoubleClick)
        
        # We want to supply a context menu.
        
        self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.customContextMenuRequested.connect(self._contextMenu)
    def _compose(self) -> None:
        # Just run $DAQBIN/lg_wrnote
        # Note the edit is modal so that when done, the noteWritten signal can be emitted.
        if 'DAQBIN' not in os.environ:
            QMessageBox.warning(
                self, 'DAQBIN undefined', 
                'The DAQBIN environment variable is not defined, use a daqsetup.bash script to set it up'
            )
        else:
            wrnote = pathlib.Path(os.environ['DAQBIN']) / 'lg_wrnote'
            subprocess.call([wrnote,])     # edit as model.
            self.noteWritten.emit()       # Probably the app wants to refresht the tree.
            
        
    def _itemToPdf(self, point : QPoint) -> None:
        items = self._pointToItems(point)
        if len(items) == 0:
            return
        filename = self._getPdfFile()
        if filename:
            
            markdown = LogBookUIUtilities.generateMarkdownFromItemList(items)
            LogBookUIUtilities.markdownToPdf(markdown, filename)
            
            
    def _bookToPdf(self) -> None:
        items = self._allData()
        if len(items) == 0:
            return    # Nothing to render.
        filename = self._getPdfFile()
        if filename:
            markdown = LogBookUIUtilities.generateMarkdownFromItemList(items)
            LogBookUIUtilities.markdownToPdf(markdown, filename)
        
        
    # internal/private signal handlers:
    
    def _onDoubleClick(self, index : QModelIndex) -> None:
        # Get the first column as that has the assocviated data
    
        col0Index = index.siblingAtColumn(0)
        item = self.model().itemFromIndex(col0Index)
        associatedData = item.data()
        if type(associatedData) == LogBook.Note:
            self._renderNoteInBrowser(associatedData)
    
    def _contextMenu(self, where : QPoint) -> None:
        context_menu = QMenu(self)
        
        compose = QAction('Compose note...', context_menu)
        compose.triggered.connect(self._compose)
        
        pdf_selected = QAction('Make PDF from selected...', context_menu)
        pdf_selected.triggered.connect(lambda : self._itemToPdf(where))
        
        pdf_book     = QAction('Make PDF from entire book...', context_menu)
        pdf_book.triggered.connect(self._bookToPdf)
        
        refresh      = QAction('Refresh', context_menu)
        refresh.triggered.connect(self.refresh.emit)
        
        context_menu.addAction(compose)
        context_menu.addAction(pdf_selected)
        context_menu.addAction(pdf_book)
        context_menu.addAction(refresh)
        
        pos = self.mapToGlobal(where)
        context_menu.exec(pos)
            
    
    # Utility methods.
    
    def _getPdfFile(self) -> str | None:
        # Prompt for and get a PDF filename:
        # if the user cancels the prompting dialog, then returns None.
        
        file, filter = QFileDialog.getSaveFileName(
            self, 'PDF file',
            '.', 'PDF files (*.pdf) ;; All Files (*)'
            '*.pdf'
        )
        if not file.strip():
            return None
        else:
            # Enforce the filter if needed.
            fileinfo = QFileInfo(file)
            if (not fileinfo.suffix()) and ('*.pdf' in filter):
                file += '.pdf'
            return file
        
    
    def _pointToItems(self, where : QPoint) -> list[LogBook.Run | LogBook.Transition | LogBook.Note]:
        #  Given where a context menu was popped up get the item associated with it.
        #  Note that if what we have is a transition, we return the run that the
        #  transition is part of.
        
        point_index = self.indexAt(where)
        
        # But it's column 0's data that has the item as data:
        
        col0Index = point_index.siblingAtColumn(0)
        col0Item = self.model().itemFromIndex(col0Index)
        if col0Item is None:
                return []
        
        if col0Item.text() == 'None':
            
            # Return the list of unassociated notes:
            
            
            return self._childData(col0Item)
        
        data     = col0Item.data()
        if type(data) == LogBook.Note:
            print('note')
            return [data,]       # just return notes
        elif type(data) == LogBook.Run:
            result = [data,]
            result.extend(self._childData(col0Item))
            return result
        elif type(data) == LogBook.Transition:
            
            runItem = col0Item.parent() 
            result  = [runItem.data(),]
            result.extend(self._childData(runItem))
            return result
            
        return []
    
    def _allData(self) -> list[LogBook.Run | LogBook.Transition | LogBook.Note]:
        # Return all of the renderable data from the model:
        
        # Start with row 0 which are the unassociated notes:
        result = self._childData(self.model().item(0))
        
        # Now the rest:
        
        for row in range(1, self.model().rowCount()):
            item = self.model().item(row)
            result.append(item.data())
            result.extend(self._childData(item))
        
        return result
    def _childData(self, item : QStandardItem) -> list[LogBook.Run | LogBook.Transition | LogBook.Note]:
        # Get the data associated with the children of the specified item index:
        
        result = [list]
        for row in range(item.rowCount()):
            result.append(item.child(row).data())
        
        return result 
    def _renderNoteInBrowser(self, note : LogBook.Note) -> None:
        # Render the note as html and pop up the system browser
        # in the background to view it:
        
        # Create the markdown text:
        
        markdown = LogBookUIUtilities.genNoteMarkdown(note)
        
        # Write a file in our tempdir with the HTML rendering of the
        # markdown.
        
        noteFile = LogBookUIUtilities.makeNoteHtmlFilename(note.id)
        LogBookUIUtilities.markdownToHtml(markdown, noteFile)
        
        
        subprocess.Popen(['xdg-open', noteFile])
        
#-------------------------- Code for shift tab ----------------------------------------------------------

class ShiftModel(QStandardItemModel):
    '''
        This model provides a container for the shifts which, in turn can be displayed in the
        ShiftView widget.  The model is organized as a tree where top level nodes are
        shift names and child nodes are the people inside the shift.
    '''
    def __init__(self, parent : QWidget | None = None):
        super().__init__()
        self.clear()
    
    def clear(self) -> None:
        '''
            Clear the model and setup the initial state of the widget, specifically
            column count and header labels.
        '''
        super().clear()
        
        self.setColumnCount(4)
        self.setHorizontalHeaderLabels(['Shift', 'Last Name', 'First Name', 'Salutation'])
    
    def addShifts(self, shifts : Iterable[LogBook.Shift]) -> None:
        '''
        Add a set of shifts to the model.
        @param shifts: Iterable[LogBook.Shift] The shifts to add.
        '''
        
        for shift in shifts:
            self.addShift(shift)
    def addShift(self, shift : LogBook.Shift) -> None:
        '''
        Add a single shift to the logbook.   
        @param shift : LogBook.Shift - the shift to add.
        '''
        # Top level item is the shift name:
        name = QStandardItem(shift.name)
        self._setFlagsAndAppendRow(self, [name, ])
        for member in shift.members:
            last = QStandardItem(member.lastname)
            first = QStandardItem(member.firstname)
            sal   = QStandardItem(member.salutation)
            empty = QStandardItem('')
            self._setFlagsAndAppendRow(name, [empty, last, first, sal])

    
    
    #  Utiltities:
    def _setFlagsAndAppendRow(self, parent: QStandardItem | QStandardItemModel, row : list[QStandardItem]) -> None:
        # Set item appropriate flags (mostly we want to turn off editing)
        # Then append the row in the appropriate parent:
        
        for item in row:
            flags = item.flags()
            flags = flags & (~Qt.ItemFlag.ItemIsEditable)
            item.setFlags(flags)
        parent.appendRow(row)    

class ShiftView(QTreeView):
    '''
    Tree intended to view the shifts that are in a ShiftModel.  Note external 
    forces must set the model.
    
    Signals:
      addShift - The context menu was used to request an add a shift.
      editShfit(str) - The context menu was used to request an edit of a shift.
      selectShift(str) - The named shift should be made current. The name
                   of the shift is the clicked shift which should be made current.
      refresh    - The refresh context menu was clicked.
    '''
    addShift = pyqtSignal()
    editShift = pyqtSignal(str)
    selectShift = pyqtSignal(str)
    refresh    = pyqtSignal()
    
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        
        # Suport a custopm context menu:
        
        self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.customContextMenuRequested.connect(self._contextMenu)

    def setCurrentShift(self, name : str) -> None:
        '''
            Visually indicate the current shift.
            @param name -name of the shift.
            
            This has to be done in the view to get current colors.
        '''
        # Get the colors:
        
        defaultBg = self.palette().color(QPalette.ColorRole.Base)
        currentBg = QColor('cyan')
        
        notCurrent = QBrush(defaultBg)
        current    = QBrush(currentBg)
        
        for row in range(self.model().rowCount()):
            item = self.model().item(row, 0)
            if item.text() == name:
                item.setData(current, Qt.ItemDataRole.BackgroundRole)
            else:
                item.setData(notCurrent, Qt.ItemDataRole.BackgroundRole)
        
        
            
    
    # Context menu handling:
    
    def _addShift(self) -> None:
        # Add a new shift:  just signal and let the outside world
        # deal with it:
        
        self.addShift.emit()
            
    
    def _contextMenu(self, where : QPoint) -> None:    
        # Handles the right click.  The where parameter is where the
        # click occured.  We'll use that to figure out a shift for the 
        # set current and edit signals.
        
        context_menu = QMenu(self)
        shiftName = self._pointToShift(where)
        
        
        add = QAction('Add Shift...', context_menu)
        context_menu.addAction(add)
        add.triggered.connect(self._addShift)
        
        edit = QAction('Edit...', context_menu)
        context_menu.addAction(edit)
        edit.triggered.connect(lambda: self.editShift.emit(shiftName))
        
        setCurrent = QAction('Set Current', context_menu)
        context_menu.addAction(setCurrent)
        setCurrent.triggered.connect(lambda : self.selectShift.emit(shiftName))
        
        refresh = QAction('Refresh', context_menu)
        context_menu.addAction(refresh)
        refresh.triggered.connect(self.refresh.emit)
        
        pos = self.mapToGlobal(where)     # Wher we want the context menu posted.
        context_menu.exec(pos)
        
    def _pointToShift(self, where : QPoint) -> str :
        # Map the where point to a shift name.
        
        point_index = self.indexAt(where)     # QItemIndex.
        item        = self.model().itemFromIndex(point_index)
        parent = item.parent()
        if parent:
            return parent.text()
        else:
            return item.text()
#--------------------------- Code for the person tab ----------------------------------------------------

class PersonModel(QStandardItemModel):
    '''
        Model for people in a table.  THis is a flat model.
    '''
    def __init__(self, parent : QWidget | None):
        super().__init__(parent)
        self.clear()
        
    def clear(self):
        super().clear()
        self.setHorizontalHeaderLabels(['Last Name', 'First Name', 'Salutation'])
    
    #  Utiltities:
    def _setFlagsAndAppendRow(self,  row : list[QStandardItem]) -> None:
        # Set item appropriate flags (mostly we want to turn off editing)
        # Then append the row in the appropriate parent:
        
        for item in row:
            flags = item.flags()
            flags = flags & (~Qt.ItemFlag.ItemIsEditable)
            item.setFlags(flags)
        self.appendRow(row)            
#--------------------------    Main program logic --------------------------------------------------------

# ---- Logbook logic.

def loadLogBookTab(model : LogBookModel) -> None:
    model.clear()
    
    model.setUnassociatedNotes(logbookadmin.listNonRunNotes())
    notes = model.unassociatedNotes()
    
    runs = logbookadmin.listRuns()
    runAndNotes = []
    for run in runs:
        notes = logbookadmin.listNotesForRun(run.number)
        runAndNotes.append((run, notes))
    model.setRuns(runAndNotes)

# ---- Shift logic.

def loadShiftTab(model : ShiftModel) -> None:
    model.clear()
    shifts = []
    for name in logbookadmin.listShifts():
        shifts.append(logbookadmin.getShift(name))  
    model.addShifts(shifts)

def addShift(model : ShiftModel) -> None:
    # Just run lg_mkShift and reload the model:
    
    program = LogBookUIUtilities.programPath('lg_mkshift')  
    if program is None:
        return
    else:
        subprocess.call([program,])
        loadShiftTab(model)

def editShift(name : str, model : ShiftModel) -> None:
    # Edit the named shift.
    
    program = LogBookUIUtilities.programPath('lg_mgshift')
    if not program is None:
        subprocess.call([program, 'edit', name])
        loadShiftTab(model)
    
def main() -> int:
    '''
    Program entry point.
    '''
    
    app = QApplication(sys.argv)
    win = QTabWidget()
    
    # The logbook tab:
    
    lbview = LogBookView(win)
    lbmodel = LogBookModel()
    loadLogBookTab(lbmodel)
    
    # Bot the signals for notes written and update refresh go to loadLogBookTab
    refreshLb = lambda: loadLogBookTab(lbmodel)
    lbview.noteWritten.connect(refreshLb)
    lbview.refresh.connect(refreshLb)
    
    lbview.setModel(lbmodel)
    win.addTab(lbview, 'LogBook')
    
    # The shifts tab:
    
    shiftview = ShiftView(win)
    shiftmodel = ShiftModel(shiftview)
    shiftview.setModel(shiftmodel)
    win.addTab(shiftview, 'Shifts')
    
    refreshShifts = lambda: loadShiftTab(shiftmodel)
    refreshShifts()
    
    shiftview.refresh.connect(refreshShifts)
    shiftview.addShift.connect(lambda : addShift(shiftmodel))
    shiftview.editShift.connect(lambda name: editShift(name, shiftmodel) )
    shiftview.selectShift.connect(logbookadmin.setCurrentShift)
    shiftview.selectShift.connect(shiftview.setCurrentShift)
    
    shiftview.setCurrentShift(logbookadmin.currentShift().name)
    
    ##
    win.show()
    h = win.height()
    win.resize(700, h)
    
    return app.exec()    

    

if __name__ == '__main__':
    sys.exit(main())