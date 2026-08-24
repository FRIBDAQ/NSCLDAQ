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
from collections.abc import Iterable


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
    return book.current_shift()
    
def getShift(name : str) -> LogBook.Shift | None:
    '''
      Return the named shift object.
      @param name - shift name.
      @return LogBook.Shift | None - The named shift.
      @retval None -there is no matching shift.
    '''
    book = _currentLogBookOrError()
    try:
        return book.find_shift(name)
    except LogBook.error:
        return None
    
def beginRun(run : int, title : str, remark :str | None = None) -> LogBook.Run:
    '''
        Creates a new run, logging a begin.
        @param run - new run number.
        @param title - the run title.
        @param remark - Comment to associate with the run. None if no comment is desired.
    '''
    book = _currentLogBookOrError()
    if remark:
        return book.begin_run(run, title, remark)
    else:
        return book.begin_run(run, title)
    
    
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


def addNote(
    author : LogBook.Person, text : str, 
    image_paths : Iterable[str] | None = None, image_offsets : Iterable[int] | None = None, 
    run : LogBook.Run | None = None
) -> LogBook.Note:
    '''
    Create a new note that is optionally bound to a run.
    @param author        : LogBook.Person - Person who is contributing the note.
    @param text          : str           - Text of the note (markdown).
    @param image_paths   : Iterable[str] - Paths to any filenames of images included in the note.
    @param image_offsets : Iterable[int] - Offsets in the text at which the images occur.
    @param run           : LogBookRun    - Optional run the note should be associated with.
    @return LogBook.Note - The note that was added.
    
    '''
    book = _currentLogBookOrError()
    return book.create_note(
        author, text, 
        image_paths,  image_offsets,
        run)

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
    return book.list_notes_for_run_number(num)

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
        Set a key's value.  Note that if the
        key does not exist, it is created.
        
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
    import sys

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
    
    class PersonTests(unittest.TestCase):
        def setUp(self):
            # Delete the current logbook file.
            
            try:
                CURRENT_LOGBOOK_PATH.unlink()
            except FileNotFoundError:
                pass                # Might not exist.
            
            
            # This is a bit odd... we're just making
            # a tempfile to get a nice
        
            self._tempfile = tempfile.NamedTemporaryFile().name  # noqa: SIM115
            
            # Make that a logbook and make it the current logbook:
            
            createLogBook(self._tempfile, 'e0400x', 'Ron Fox', 'Test Experiment', True)
        def tearDown(self):
            try:
                pathlib.Path(self._tempfile).unlink()
            except FileNotFoundError:
                pass
            
            try:
                CURRENT_LOGBOOK_PATH.unlink()
            except FileNotFoundError:
                pass                # Might not exist.

        def test_listNoPeople(self):
            people = listPeople()   #SB empty list.
            self.assertEqual(0, len(people))
            
        def test_createPerson(self):
            addPerson('Fox', 'Ron', 'Mr.')    
            people = listPeople()
            self.assertEqual(1, len(people))
            fox = people[0]
            self.assertEqual('Fox', fox.lastname)
            self.assertEqual('Ron', fox.firstname)
            self.assertEqual('Mr.', fox.salutation)
        
        def test_createPeple(self):
            addPerson('Fox', 'Ron', 'Mr.')    
            addPerson('Mouse', 'Minnie', 'Ms.')
            people = listPeople();
            self.assertEqual(2, len(people))  # just check the size.
        
    class ShiftTests(unittest.TestCase):
        def setUp(self):
            # Delete the current logbook file.
            
            try:
                CURRENT_LOGBOOK_PATH.unlink()
            except FileNotFoundError:
                pass                # Might not exist.
            
            
            # This is a bit odd... we're just making
            # a tempfile to get a nice
        
            self._tempfile = tempfile.NamedTemporaryFile().name  # noqa: SIM115
            
            # Make that a logbook and make it the current logbook:
            
            createLogBook(self._tempfile, 'e0400x', 'Ron Fox', 'Test Experiment', True)
            
            # Any semblance to people living or dead is coincidental.
            
            addPerson('Mouse', 'Mickey', 'Mr.')
            addPerson('Duck', 'Daffy', 'Mr.')
            addPerson('Oil', 'Olive', 'Ms.')
            addPerson('Stooge', 'Larry', 'Mr.')
            addPerson('DuMont', 'Margaret', 'Ms.')
            self._people = listPeople()     # I'll probably be doing this no matter what.
        def tearDown(self):
            try:
                pathlib.Path(self._tempfile).unlink()
            except FileNotFoundError:
                pass
            
            try:
                CURRENT_LOGBOOK_PATH.unlink()
            except FileNotFoundError:
                pass                # Might not exist.  
            
        def test_listNoShifts(self):
            shifts = listShifts()
            self.assertEqual(0, len(shifts))
        def test_createShift(self):
            # select the members: Mickey Mouse, Olive Oil and Larry Stooge.
            
            members = [p for p in self._people if p.lastname in ['Mouse', 'Oil', 'Stooge']]
            createShift('ashift', members)
            
            # The shift is in the list:
            
            shifts = listShifts()
            self.assertEqual(1, len(shifts))
            sname = shifts[0]
            self.assertEqual('ashift', sname)
            
            # It has the right people in it:
            
            shift_members = listShiftMembers(sname)
            self.assertEqual(3, len(shift_members))
            
            # I don't know the order of the members....
            
            shift_lastnames = [p.lastname for p in shift_members]
            self.assertIn('Mouse', shift_lastnames)
            self.assertIn('Oil', shift_lastnames)
            self.assertIn('Stooge', shift_lastnames)

        def test_noCurrentShift(self):
            self.assertIsNone(currentShift())
        
        def test_haveCurrentShift(self):
            createShift('ashift', self._people)    #  just shove everyone in.
            setCurrentShift('ashift')
            
            shift = currentShift()
            self.assertIsNotNone(shift)
            self.assertEqual('ashift', shift.name)
            self.assertEqual(5, len(shift.members))
            member_names = [p.lastname for p in shift.members]
            for lname in ['Mouse', 'Duck', 'Oil', 'Stooge', 'DuMont']:
                self.assertIn(lname, member_names)
        
        def test_removeMember(self):
            createShift('ashift', self._people)
            setCurrentShift('ashift')    # Makes it easier to get.
            # Now remove micky mouse:
            
            mickey = next(p for p in listPeople() if p.lastname == 'Mouse')
            removeMemberFromShift('ashift', mickey)
            
            self.assertNotIn('Mouse', [p.lastname for p in currentShift().members])
        
        def test_fineNone(self):
            sbnone = getShift('ashift')
            self.assertIsNone(sbnone)
        
        def test_addMember(self):
            createShift('ashift', [])   # No people in the shift (I think that's ok).
            people = listPeople()
            mickey = next(p for p in people if p.lastname == 'Mouse')
            dumont = next(p for p in people if p.lastname == 'DuMont')
            
            addMembersToShift('ashift', [mickey, dumont])
            
            shift = getShift('ashift')
            self.assertIn('Mouse', [p.lastname for p in shift.members])
            self.assertIn('DuMont', [p.lastname for p in shift.members])
            
            
    class TransitionTests(unittest.TestCase):
        def setUp(self):
            # Delete the current logbook file.
            
            try:
                CURRENT_LOGBOOK_PATH.unlink()
            except FileNotFoundError:
                pass                # Might not exist.
            
            
            # This is a bit odd... we're just making
            # a tempfile to get a nice
        
            self._tempfile = tempfile.NamedTemporaryFile().name  # noqa: SIM115
            
            # Make that a logbook and make it the current logbook:
            
            createLogBook(self._tempfile, 'e0400x', 'Ron Fox', 'Test Experiment', True)
            
            # Need to establish a current shift.
            addPerson('Mouse', 'Mickey', 'Mr.')
            addPerson('Duck', 'Daffy', 'Mr.')
            addPerson('Oil', 'Olive', 'Ms.')
            addPerson('Stooge', 'Larry', 'Mr.')
            addPerson('DuMont', 'Margaret', 'Ms.')
            people = listPeople()     # I'll probably be doing this no matter what.
            createShift('ashift', people)
            setCurrentShift('ashift')
        def tearDown(self):
            try:
                pathlib.Path(self._tempfile).unlink()
            except FileNotFoundError:
                pass
            
            try:
                CURRENT_LOGBOOK_PATH.unlink()
            except FileNotFoundError:
                pass                # Might not exist.  
        
        def test_emptyList(self):
            self.assertEqual(0, len(listRuns()))  # no runs have been created.
            self.assertIsNone(currentRun())       # There is no current run.
            
        def test_beginRunNoRemark(self):
            beginRun(124, 'A title')  
            #there should now be a run and a current run and they should be the same:
            
            runs    = listRuns()
            self.assertEqual(1, len(runs))
            therun  = runs[0]
            current = currentRun()
            self.assertIsNotNone(current)
            
            # 'both' runs are active
            
            self.assertTrue(therun.is_current())
            self.assertTrue(current.is_current())
            
            # Each has only one transition:
            
            self.assertEqual(1, therun.transition_count())
            self.assertEqual(1, current.transition_count())
            
            # Check the title and number
            
            self.assertEqual(124, therun.number)
            self.assertEqual(124, current.number)
            self.assertEqual('A title', therun.title)
            self.assertEqual(current.title, current.title)
            
            # by now I believe they're the same so :
            
            t = current.get_transition(0)   # transition to begin run:
            self.assertEqual('', t.comment)
            self.assertEqual('BEGIN', t.transition_name)
        
        def test_beginRunRemark(self):
            beginRun(1234, 'A title', 'A remark')
            
            # Assuming the test_beginRunNoRemark works....
            
            current = currentRun()
            transition = current.get_transition(0)
            self.assertEqual('A remark', transition.comment)
         
        def test_endrunInactive(self):
            with self.assertRaises(LogBook.error):
                endRun()
        def test_endRunNoRemark(self):
            beginRun(1234, 'A title')
            endRun()
            # Not current so:
            
            run = findRun(1234)
            self.assertIsNotNone(run)
            
            self.assertEqual(2, run.transition_count())
            
            end = run.get_transition(1)    # End run transition.
            self.assertEqual('', end.comment)
            self.assertEqual('END', end.transition_name)
            
        def test_endWithRemark(self):
            beginRun(1234, 'A title')
            endRun('A remark')
            
            run = findRun(1234)
            self.assertIsNotNone(run)
            
            end = run.get_transition(1)
            self.assertEqual('A remark', end.comment)
        
        def test_pauseNotActive(self):
            with self.assertRaises(LogBook.error):
                pauseRun()
        
        def test_pause_noRemark(self):
            beginRun(1234, 'a title')
            pauseRun()
            
            # There's still a current run:
            
            self.assertIsNotNone(currentRun())
            
            run = findRun(1234)
            pause = run.get_transition(1)
            self.assertEqual('PAUSE', pause.transition_name)
            self.assertEqual('', pause.comment)
            
        def test_pause_Remark(self):
            beginRun(1234, 'a title')
            pauseRun('a comment')
            
            run = findRun(1234)
            pause = run.get_transition(1)
            self.assertEqual('a comment', pause.comment)
        def test_resume_norun(self):
            with self.assertRaises(LogBook.error):
                resumeRun()
        def test_resume_noRemark(self):
            # Start the run first:
            
            beginRun(1234, 'a title')
            pauseRun()
            
            
            resumeRun()
            self.assertIsNotNone(currentRun())    # There's still a current run.
            run = findRun(1234)
            resume = run.get_transition(2)
            self.assertEqual('RESUME', resume.transition_name)
            self.assertEqual('', resume.comment)
            
            
        def test_resume_remark(self):
            
            beginRun(1234, 'a title')
            pauseRun()
            resumeRun('A comment')
            run = findRun(1234)
            resume = run.get_transition(2)
            self.assertEqual('RESUME', resume.transition_name)
            self.assertEqual('A comment', resume.comment)
            
        def test_emergencyend_noRun(self):
            with self.assertRaises(LogBook.error):
                emergencyEndRun()
        def test_emergencyend_NoComment(self):
            beginRun(1234,' a title')    
            emergencyEndRun()
            
            # Should not  be a current run:
            
            self.assertIsNone(currentRun())
            
            run = findRun(1234)
            end = run.get_transition(1)
            self.assertEqual('', end.comment)
            self.assertEqual('EMERGENCY_END', end.transition_name)
        def test_emergencyend_Comment(self):
            beginRun(1234, 'a title')
            emergencyEndRun('a comment')
            run = findRun(1234)
            end = run.get_transition(1)
            self.assertEqual('a comment', end.comment)
            self.assertEqual('EMERGENCY_END', end.transition_name)
        
        def test_list_none(self):
            self.assertEqual(0, len(listRuns()))
        def test_list_one(self):
            beginRun(1, 'title for run 1')
            endRun()
            runs = listRuns()
            self.assertEqual(1, len(runs))
            
            # The run has all the right attributes:
            run = runs[0]
            self.assertEqual(1, run.number)
            self.assertEqual('title for run 1', run.title)
            self.assertFalse(run.is_current())
            self.assertEqual('END', run.last_transition())
            self.assertFalse(run.is_active())
            self.assertEqual(2, run.transition_count())
        
        def test_list_some(self):
            # Make three runs
            
            for run in range(1,4):
                beginRun(run, f'title for run {run}')
                endRun()
                
            runlist = listRuns()
            
            self.assertEqual(3, len(runlist))
            
            # get the run numbers:
            run_numbers = [r.number for r in runlist]
            run_numbers.sort()
            self.assertEqual(3, len(run_numbers))
            for i,r in enumerate(run_numbers):
                self.assertEqual(i+1, r)  # Runs start from 1.
    class NotesTests(unittest.TestCase):
        def setUp(self):
            # Delete the current logbook file.
            
            try:
                CURRENT_LOGBOOK_PATH.unlink()
            except FileNotFoundError:
                pass                # Might not exist.
            
            
            # This is a bit odd... we're just making
            # a tempfile to get a nice
        
            self._tempfile = tempfile.NamedTemporaryFile().name  # noqa: SIM115
            
            # Make that a logbook and make it the current logbook:
            
            createLogBook(self._tempfile, 'e0400x', 'Ron Fox', 'Test Experiment', True)
            
            # Need to establish a current shift.
            addPerson('Mouse', 'Mickey', 'Mr.')
            addPerson('Duck', 'Daffy', 'Mr.')
            addPerson('Oil', 'Olive', 'Ms.')
            addPerson('Stooge', 'Larry', 'Mr.')
            addPerson('DuMont', 'Margaret', 'Ms.')
            self._people = listPeople()     # I'll probably be doing this no matter what.
            createShift('ashift', self._people)
            setCurrentShift('ashift')
            
            # Make some runs we can hang notes off:
            for run in range(1,11): 
                beginRun(run , f'Title for run{run}')
                endRun()
        def tearDown(self):
            try:
                pathlib.Path(self._tempfile).unlink()
            except FileNotFoundError:
                pass
            
            try:
                CURRENT_LOGBOOK_PATH.unlink()
            except FileNotFoundError:
                pass                # Might not exist.  

        def  test_list_noNotes(self):
            self.assertEqual(0, len(listAllNotes()))
        def test_list_noRunNotes(self):
            for run in range(1, 11):
                self.assertEqual(0, len(listNotesForRun(run)))
        def test_list_noNonRunNotes(self):
            self.assertEqual(0, len(listNonRunNotes()))
            
        def test_addNote_norun(self):
            actual_author = self._people[0]
            
            note = addNote(actual_author, 'This is a note')
            self.assertIsNone(note.run)
            self.assertEqual('This is a note', note.contents)
            note_author = note.author   # LogBook.Person.
            
            self.assertEqual(actual_author.lastname, note_author.lastname)
            self.assertEqual(actual_author.firstname, note_author.firstname)
            self.assertEqual(actual_author.salutation, note_author.salutation)
            
        def test_addNote_run(self):
            actual_author = self._people[2]  
            run           = findRun(3)
            self.assertIsNotNone(run)
            note = addNote(actual_author, 'Some harmelss note text', run=run)
            
            # Assuming the other stuff is fine:
            
            note_run = note.run
            self.assertIsNotNone(note_run)
            
            self.assertEqual(run.number, note_run.number)
            self.assertEqual(run.title, note_run.title)
        
        def test_getNote_error(self):
            self.assertIsNone(getNote(1234))       # no such note.
            
        def test_getNote(self):
            note = addNote(self._people[0], 'Thisi s a note')
            got_note = getNote(note.id)    # Shoulid get the same note.
            self.assertIsNotNone(got_note)
            
            # Note should be properly filled out:
            
            self.assertEqual(note.id, got_note.id)
            self.assertEqual(note.run, got_note.run)   # None actually in both cases.
            self.assertEqual(note.time, got_note.time)
            self.assertEqual(note.contents, got_note.contents)
            self.assertEqual(self._people[0].lastname, note.author.lastname)
        
        def test_gettitle(self):
            note = addNote(self._people[1], "This is the title\nAnother line")    
            
            self.assertEqual('This is the title', getNoteTitle(note))
            
        
        def test_listall_one(self):
            note = addNote(self._people[0], 'This is is the only note')
            notes = listAllNotes()
            self.assertEqual(1, len(notes))
            
            # and notes[0] is the same id as note meaning it is the same.
            
            self.assertEqual(note.id, notes[0].id)
            
        def test_listall_several(self):
            added_notes = []
            for i in range(10):
                added_notes.append(addNote(self._people[0], f'Note number {i}'))
            
            # Added_notes is in id order but the listed notes may not be:
            
            listed_notes = list(listAllNotes())    #listAllNotes returns a tuple.
            listed_notes.sort(key=lambda item: item.id)
            
            self.assertEqual(len(added_notes), len(listed_notes))
            for i, note in enumerate(added_notes):
                self.assertEqual(note.id, listed_notes[i].id)
        
        def test_listRunNotes_one(self):
            run = findRun(1)      #We'll attache a note to this one.
            
            note = addNote(self._people[2], 'Note text', run=run)
            run_notes = listNotesForRun(1)
            self.assertEqual(1, len(run_notes))            
            
            self.assertEqual(note.id, run_notes[0].id)   # Same note.
            
        def test_listRunNotes_serveral(self):
            run = findRun(2)
            allNotes = []
            for i in range(10):
                if i % 2:     # every other is a run note.
                    note = addNote(self._people[1], f'Note number {i}')
                else:
                    note = addNote(self._people[1], f'Note number {i}', run = run)
                allNotes.append(note)
            
            
            run_notes = listNotesForRun(2)
            
            self.assertEqual(5, len(run_notes))
            for i,rn in enumerate(run_notes):
                self.assertEqual(allNotes[i*2].id, rn.id)
                
        def test_listNonRunNote_one(self):
            note = addNote(self._people[1], 'A note')
            notes = listNonRunNotes()
            self.assertEqual(1, len(notes))
            self.assertEqual(note.id, notes[0].id)
            
        def test_listNonRunNotes_several(self):
            run = findRun(2)
            allNotes = []
            for i in range(10):
                if i % 2:     # every other is a run note.
                    note = addNote(self._people[1], f'Note number {i}', run = run)
                else:
                    note = addNote(self._people[1], f'Note number {i}')
                    
                allNotes.append(note)
            # Even notes are non run:
            
            notes= listNonRunNotes()
            
            self.assertEqual(5, len(notes))
            
            for i,n  in enumerate(notes):
                self.assertEqual(allNotes[i*2].id, n.id)
        
    class KvTests(unittest.TestCase):
        def setUp(self):
            # Delete the current logbook file.
            
            try:
                CURRENT_LOGBOOK_PATH.unlink()
            except FileNotFoundError:
                pass                # Might not exist.
            
            
            # This is a bit odd... we're just making
            # a tempfile to get a nice
        
            self._tempfile = tempfile.NamedTemporaryFile().name  # noqa: SIM115
            
            # Make that a logbook and make it the current logbook:
            
            createLogBook(self._tempfile, 'e0400x', 'Ron Fox', 'Test Experiment', True)       
        def tearDown(self):
            try:
                pathlib.Path(self._tempfile).unlink()
            except FileNotFoundError:
                pass
            
            try:
                CURRENT_LOGBOOK_PATH.unlink()
            except FileNotFoundError:
                pass                # Might not exist.
        
        def test_kvCreateOk(self):
            # This should not raise an exception so no assert is needed:
            
            kvCreate('a', 'b')
        def test_kvCreateDup(self):
            
            kvCreate('a', 'b') # ok.
            with self.assertRaises(LogBook.error):
                kvCreate('a', 'c')    # Duplicate raises.
        
        def test_kvGetOk(self):
            kvCreate('a', 'b') 
            self.assertEqual('b', kvGet('a'))
        def test_kvGetError(self):
            with self.assertRaises(LogBook.error):
                kvGet('a')
                
        def test_kvSetOk(self):
            kvCreate('a', 'b')
            kvSet('a', 'c')
            self.assertEqual('c', kvGet('a'))
        def test_kvSetCreates(self):
            kvSet('a', 'b')
            self.assertEqual('b', kvGet('a'))
        
    unittest.main()
    