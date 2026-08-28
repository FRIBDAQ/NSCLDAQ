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
from datetime import datetime, timezone

from PyQt6.QtWidgets import QTreeView, QWidget, QApplication
from PyQt6.QtGui     import QStandardItemModel, QStandardItem 

from nscldaq.LogBook import LogBook, logbookadmin, LogBookUIUtilities

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
        self.appendRow(self._noneItem)
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
            placeholder = QStandardItem('Note')
            placeholder.setData(note)                        # Col 0 has note as associated data.
            
            titleText   = note.contents.split('\n')[0]
            title       = QStandardItem(titleText)
            
            author = QStandardItem(LogBookUIUtilities.personName(note.author))
            
            timestamp = datetime.fromtimestamp(  # noqa: DTZ006
                note.time, tz=None
            )                                                  # in local time.
            datestring = timestamp.strftime('%c')
            time   = QStandardItem(datestring)
            
            self._noneItem.appendRow([placeholder, title, author, time])
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
        
       
class LogBookView(QTreeView):
    def __init__(self, parent : QWidget | None = None):
        super().__init__(parent)
        

# Test code for now
if __name__ == '__main__':
    app = QApplication(sys.argv)
    w = LogBookView()
    model = LogBookModel()
    model.setUnassociatedNotes(logbookadmin.listNonRunNotes())
    notes = model.unassociatedNotes()
    print(len(notes))
    for n in notes:
        print('note written by', LogBookUIUtilities.personName(n.author))
    
    w.setModel(model)
    
    w.show()
    h = w.height()
    w.resize(700, h)
    
    sys.exit(app.exec())

