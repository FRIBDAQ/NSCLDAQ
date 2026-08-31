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
import sys
from collections.abc import Iterable

from nscldaq.LogBook import LogBook, LogBookUIUtilities, logbookadmin
from PyQt6.QtCore import QModelIndex, Qt
from PyQt6.QtGui import QStandardItem, QStandardItemModel
from PyQt6.QtWidgets import QApplication, QTreeView, QWidget
import subprocess


def _timestring(stamp) -> str:
    return LogBookUIUtilities.timestring(stamp)   # Factored out.
    
    
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
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        
        # Double clicking a note will render it as a
        # in a web browser as html:
        
        self.doubleClicked.connect(self._onDoubleClick)
    
    # internal/private signal handlers:
    
    def _onDoubleClick(self, index : QModelIndex) -> None:
        # Get the first column as that has the assocviated data:
        
        col0Index = index.siblingAtColumn(0)
        item = self.model().itemFromIndex(col0Index)
        associatedData = item.data()
        if type(associatedData) == LogBook.Note:
            self._renderNoteInBrowser(associatedData)
    
    def _renderNoteInBrowser(self, note : LogBook.Note) -> None:
        # Create the markdown text:
        
        markdown = LogBookUIUtilities.genNoteMarkdown(note)
        
        noteFile = LogBookUIUtilities.makeNoteHtmlFilename(note.id)
        LogBookUIUtilities.markdownToHtml(markdown, noteFile)
        status = subprocess.call(['xdg-open', noteFile])
        print('web browser open status: ', status)
        

# Test code for now
if __name__ == '__main__':
    app = QApplication(sys.argv)
    w = LogBookView()
    model = LogBookModel()
    model.setUnassociatedNotes(logbookadmin.listNonRunNotes())
    notes = model.unassociatedNotes()
    
    runs = logbookadmin.listRuns()
    runAndNotes = []
    for run in runs:
        notes = logbookadmin.listNotesForRun(run.number)
        runAndNotes.append((run, notes))
    
    model.setRuns(runAndNotes)
    
    w.setModel(model)
    
    w.show()
    h = w.height()
    w.resize(700, h)
    
    sys.exit(app.exec())

