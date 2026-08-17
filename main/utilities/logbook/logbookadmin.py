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
    @file logbookadmin.py
    @brief Provide simple log book administrative functions.
    @author Ron Fox
'''
from nscldaq.LogBook import LogBook
import pathlib

CURRENT_LOGBOOK_PATH=pathlib.Path('~/.nscl-logbook-current').expanduser().absolute()

def _currentLogBookOrError() -> LogBook:
    # Internal function to get the current  log book or aise
    # a LogBook.error if there isn't one.
    
    book = currentLogBook()
    if book:
        return book
    else:
        raise LogBook.error('There is no current logbook.')

def setCurrentLogBook(path: str) -> None:
    '''
        Sets the current logbook by writing its absolute path into
        the CURRENT_LOGBOOK_PATH file.
        
        @param path : str - path to the logbook to become current. 
        
    '''
    absolute_path = pathlib.Path(path).expanduser().absolute()
    CURRENT_LOGBOOK_PATH.write_text(str(absolute_path), encoding='utf-8')
        
def currentLogBook() -> LogBook.LogBook | None:
    '''
      @return LogBook - If a current logbook is set, it is returned, else None is returned.
      
    '''
    if CURRENT_LOGBOOK_PATH.is_file():
        
        current_path = CURRENT_LOGBOOK_PATH.read_text(encoding='utf-8')
            
        return LogBook.LogBook(current_path)
    else:
        return None
    
def createLogBook(path : str, experiment : str, spokesperson : str, purpose : str, select : bool = False) -> LogBook:
    '''
    Create a new logbook and, optionally, make it the current one. The current logbook is the one that
    the lg_* commands operate on.
      @param path         : str- name of the the file that will be created for the logbook.
      @param experiment   : str - The experiment for which the logbook is being created e.g. 'e14014'.
      @param spokesperson : str - The experiment spokesperson name.
      @param purpose      : str -  Purpose of the experiment
      @param select       : bool = False - if provided and True, the logbook becomes the current logbook.
      @return LogBook - A logbook object that's open on the path given.
    '''
    LogBook.create(path, experiment, spokesperson, purpose)
    result = LogBook.LogBook(path)
    
    if select:
        setCurrentLogBook(path)
    
    return result

def addPerson(last : str, first: str, salutation: str) -> None:
    '''
    Add a person to he current logbook.
    @param last       : str - last name,
    @param first      : str - first name
    #param salutation : str - salutation they prefer
    '''
    book = _currentLogBookOrError()
    book.add_person(last, first, salutation)
    
def listPeople() -> tuple[LogBook.Person]:
    '''
        Lists all of the people in the current logbook.
        @return tuple[LogBook.Person]   - Person objects for all people in the logbook.
    '''
    book = _currentLogBookOrError()
    return book.list_people()
    
def createShift(name : str, members : list[LogBook.Person]) -> None:
    '''
        Create a new shift in the current logbook from an iterable of 
        persons in that logbook.
        
        @param name : str  -name of the new shift.
        @param members : list[LogBook.Person] - members in the shift.
    '''
    book = _currentLogBookOrError()
    book.create_shift(name, members)
    
    
def addMembersToShift(shiftName : str, newMembers : list[LogBook.Person]) -> None:
    '''
        Adds members to a named shift in the current logbook.
        @param shiftName  : str - the name of the shift to add people to.
        @param newMembers : list[LogBook.Person]- the people to add to the shift
    '''
    book = _currentLogBookOrError()
    shift = book.find_shift(shiftName)
    for member in newMembers:
        shift.add_member(member)
    
    
def removeMemberFromShift(shiftName : str, member : LogBook.Person) -> None:
    '''
        Removes a member from an existing shift.
        @param shiftName    : str - name of the shift.
        @param member       : LogBook.Person - person to remove.
    '''
    
    book = _currentLogBookOrError()
    shift = book.find_shift(shiftName)
    shift.remove_member(member)
    
    
def setCurrentShift(shiftName : str) -> None:
    '''
    Set the current shift:
    @param shiftName  : str - new current shift name.
    
    '''
    book = _currentLogBookOrError()
    shift = book.find_shift(shiftName)
    shift.set_current()
    

def listShiftMembers(shiftName : str) -> tuple[LogBook.Person]:
    '''
    Returns a list of people in the named shift of the current logbook.
    @param shiftName : str - the name of the shift to list.
    @return tuple[LogBook.Person] the people in that shift.
    '''
    book = currentLogBook()
    shift = book.find_shift(shiftName)
    return shift.members

def listShifts() -> list[str]:
    '''
        Provides the name of all shifts.
        @return list[str]: - list of the shift names.
    '''
    book = _currentLogBookOrError()
    shifts = book.list_shifts()
    result = []
    for shift in shifts:
        result.append(shift.name)
        return result
    

def currentShift() -> LogBook.Shift:
    '''
        Returns the current shift
        
        @return LogBook.Shift - current shift object.
    '''
    book = _currentLogBookOrError()
    return book.currentShift()
    

def beginRun(run : int, title : str, remark :str | None = None) -> LogBook.Run:
    '''
        Creates a new run, logging a begin.
        @param run - new run number.
        @param title - the run title.
        @param remark - Comment to associate with the run. None if no comment is desired.
    '''
    book = _currentLogBookOrError()
    if remark:
        return book.beginRun(run, title, remark)
    else:
        return book.beginRun(run, title)
    
    
def endRun(remark : str | None = None)  -> None:
    '''
    Pause the active run.
    @param remark : str - if supplied, this is a remark that will be associated with the run.
                    If omitted, or None, no remark will be associated with the run.
    @throws LogBook.error if there is no current log book or there is but there's no
        active run.
    '''
    
    book = _currentLogBookOrError()
    run = book.current_run()
    if run:
        if remark:
            run.end(remark)
        else:
            run.end()
    else:
        raise LogBook.error("There is no active run")
    

def pauseRun(remark : str | None = None) -> None:
    '''
    Logs a pause run transition.  Note that it's up to external forces
    to maintain the full state diagram of the system.  We only care that there
    is a current log book and a current run in that logbook.
    
    @param remark : str - Optional remark to associate with the log entry.
         If omitted or None, no remark is sassociated with the run.
    @throws LogBook.error if there is no current log book or no active run.
    '''
    book = _currentLogBookOrError()
    run = book.current_run()
    if run:
        if remark:
            run.pause(remark)
        else:
            run.pause()
    else:
        raise LogBook.error('There is no active run to pause.')

def resumeRun(remark : str | None = None) -> None:
    '''
    Resume a paused run.  It's up to the external program to enforce that the
    run is paused before this is called.
    
    @param remark : str - optional remark to associated with the end of the run.
        If omitted or None, no remark is associated with the end of the run.
    @throws  LogBook.error if there is no current log book or no active run.
    '''
    book = _currentLogBookOrError()
    run = book.current_run()
    if run:
        if remark:
            run.resume(remark)
        else:
            run.resume()
    else:
        raise LogBook.error('There is no active run to resume.')

def emergencyEndRun(remark : str | None = None) -> None:
    '''
    Indicate to the log book the run ended abnormally.
    @param remark : str optional remark to associate with the log entry.
       If this is omitte or None, no remark is associated with the state transition.
    @throws  LogBook.error if there is no current log book or no active run.
    '''
    book = _currentLogBookOrError()
    run = book.current_run()
    if run:
        if remark:
            run.emergency_end(remark)
        else:
            run.emergency_end()
    else:
        raise LogBook.error('There is no active run to end abnormally.')

def listRuns() -> tuple[LogBook.Run]:
    '''
    @return a tuple containnig all of the runs in the logbook.  
          If there is a current run, that will also be included.
    @throws LogBook.error if there is no current logbook.
    '''
    
    book = _currentLogBookOrError()
    return book.list_runs()
    

def currentRun() -> LogBook.Run | None:
    '''
    @return LogBook.Run | None: If there is a current run it's run object
        is returned, if not, None is returned.
    @throws LogBook.error if there is no current logbook.
    '''
    book = _currentLogBookOrError()
    return book.current_run()
    
def findRun(number : int) -> LogBook.Run | None:
    ''''
    @param number - the run number to find.
    @return LogBook.Run - if there is a matching run, the run object is returned.
    @retval None - There is no matching run.
    @throws LogBook.error if there is no current logbook.
    '''
    book = _currentLogBookOrError()
    return book.find_run(number)


def getNote(id : int) -> LogBook.Note | None :
    '''
    @param id : int - a note id.
    @return LogBook.Note - the note with the matching id.
    @retval None there's not matching ote.
    @throws LogBook.error  if there is not current logbook
    '''
    book = _currentLogBookOrError()
    try:
        return book.get_note(id)
    except LogBook.error:
        return None

def getNoteText(note : LogBook.Note) -> str:
    '''
    Returns the note as mark up suitable for rendering.
    This may include exporting images that are referenced by the note
    to the image cace (~/.nscl-logbook) so that references in the note
    text will properly render those images.
    
    @param note : LogBook.Note - the note to process.
    @return str - The logbook note as renderable markdown.
    '''
    return note.substitute_images()

def getNoteTitle(note : LogBook.Note) -> str:
    '''
    @param note : LogBook.Note - A logbook note object.
    @return str - the first line of the note text.
    @retval ''  - in the pathalogical case the note is empty.
    '''
    lines = note.contents.split("\n")
    # Note splitting an empty string gives a one element list with
    # [0] an empty string.
    if len(lines):
        return lines[0]
def listAllNotes() -> tuple[LogBook.Note]:
    '''
    @return tuple[LogBook.Note] - tuple containing all of the notes in the logbook.
    @throws LogBook.error if there is no current logbook.
    
    '''
    book = _currentLogBookOrError()
    return book.list_all_notes()
    
def listNotesForRun(num : int) -> tuple[LogBook.Note]:
    '''
    Return a list of all of the notes associated with the specified run number.
    @param num : id - the run number to return the notes for.
    @return tuple[LogBook.Note] - The notes for the run
    @throws LogBook.error if  there is no current logbook.
    '''
    book = _currentLogBookOrError()
    return book.list_all_notes_for_run_number(num)

def listNonRunNotes() -> tuple[LogBook.Note]:
    '''
    @return tuple[LogBook.Note] - all of the notes that are not associated
        with a run.
    @throws LogBook.error if  there is no current logbook.
    '''
    book = _currentLogBookOrError()
    return book.list_nonrun_notes()
    

def kvGet(key :str) -> str:
    '''
    Gets the value of  key in the key value store.
    @param key : str - the key whose value we're fetching.
    @return str - The value associated with the key.
    @throws LogBook.error - there is no current logbook or there's no key 'key'.
    '''
    book = _currentLogBookOrError()
    return book.kv_get(key)
    
def kvSet(key : str, value: str) ->None:
    '''
        Set a key's value.  Note the key must already exist, see
        kvCreate below... so this means that if you don't
         knwow that a key exists but you do know there's a current
         logbook you should probably:
         
        try:
            kvSet(key, value)
        except LogBook.error:
            kvCreate(key, value)
    @param key - the key to set.
    @param value - the new value to give it.
    '''
    book = _currentLogBookOrError()
    book.kv_set(key, value)

def kvCreate(key : str, value : str) -> None:
    '''
    Attempts to create a new key with an initial value.  See kvSet above.
    @param key - key to create.
    @param value - initial value.
    @throws LogBook.error - if there's  no current logbook or the key already exists.
    '''
    book = _currentLogBookOrError()
    book.kv_create(key, value)
    

# Self contained unit tests.

if __name__ == '__main__':
    import unittest
    import tempfile

    class AdminCreateOpenTests(unittest.TestCase):
        def setUp(self):
            # Delete the current logbook file.
            
            try:
                CURRENT_LOGBOOK_PATH.unlink()
            except FileNotFoundError:
                pass                # Might not exist.
            
            
            # This is a bit odd... we're just making
            # a tempfile to get a nice
        
            self._tempfile = tempfile.NamedTemporaryFile().name  # noqa: SIM115
        def tearDown(self):
            try:
                pathlib.Path(self._tempfile).unlink()
            except FileNotFoundError:
                pass
        
        def test_noCurrentLogbook(self):
            self.assertIsNone(currentLogBook())
        
        def test_create_notCurrent(self):
            
            createLogBook(self._tempfile, 'e0400x', 'Ron Fox', 'Test experiment' ) 
            self.assertIsNone(currentLogBook())
        
        def test_create_Current(self):
            createLogBook(self._tempfile, 'e0400x', 'Ron Fox', 'Test Experiment', True)
            self.assertIsNotNone(currentLogBook())
    unittest.main()