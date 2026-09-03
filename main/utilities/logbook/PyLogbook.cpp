/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  PyLogBook.cpp
 *  @brief: Python module to provide an API for the logbook subsystem.
 */
/**
 *  This module provides the Logbook module which is an API to the
 *  nscldaq supported logbook. Note that while it's tempting to build a pure
 *  python interface on top of the python Sqlite module, that would introduce
 *  maintenance problems if the logbook schema e.g. changed.  This module
 *  is therefore a compiled module built on to pof the C++ API.
 */

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif

#include <Python.h>

#include "PyLogbook.h"
#include "PyLogBookPerson.h"
#include "PyLogBookShift.h"
#include "PyLogBookRun.h"
#include "PyNote.h"

#include "LogBook.h"
#include "LogBookPerson.h"
#include "LogBookShift.h"
#include "LogBookRun.h"
#include "LogBookNote.h"


#include <stdexcept>
#include <memory>
#include <sstream>

#include <datetime.h>

// forward definitions

LogBook* PyLogBook_getLogBook(PyObject* obj);

// Exception type we'll be raising:

PyObject* logbookExceptionObject(nullptr);

/////////////////////////////////////////////////////////////////
// Utilities for PyLogBook object instances.




void freeTuple(PyObject* tuple)
{
    int n = PyTuple_Size(tuple);
    for (int i =0; i < n; i++) {
        PyObject* pobj = PyTuple_GetItem(tuple, i);
        Py_XDECREF(pobj);
    }
    Py_DECREF(tuple);
    
}

/**
 * PyLogBook_TupleFromPeople
 *    Given a logbook and a vector of LogBookPerson*
 *    returns a tuple that contains LogBook.Person objects
 *    that represent those pointers. Once this is done, the pointers
 *    are no longer needed and the _caller_ can delete them.
 *
 *  @param book  - Pointer to the logbook these people live in.
 *  @param people -References the vector containing the people pointers.
 *  @returnPyObject* - The new tuple or null if unable to make one
 *                  in which case an appropriate python exception
 *                  type will have been raised.
 */
PyObject*
PyLogBook_TupleFromPeople(
    PyObject* book, const std::vector<LogBookPerson*>& people
)
{
    PyObject* result = PyTuple_New(people.size());
    if (!result) {
        
        return nullptr;
    }
    try {
        for (int i =0; i < people.size(); i++) {
            auto person = PyPerson_newPerson(book, people[i]->id());
            Py_XINCREF(person);
            PyTuple_SET_ITEM(result, i, person);
        }
    }
    catch (LogBook::Exception& e) {
        freeTuple(result);
        result = nullptr;
        PyErr_SetString(
            logbookExceptionObject, e.what()
        );
    }
    catch (...) {
        freeTuple(result);
        result = nullptr;
        PyErr_SetString(
            logbookExceptionObject,
            "Unanticipated exception type in PyLogBook_TupleFromPeople"
        );
    }
    Py_XINCREF(result);
    return result;
}
/**
 * StringVecFromIterable
 *    Given a container object that supports iteration, and whose
 *    contents are strings (unicode objects), returns a
 *    C++ vector of those strings.
 *
 * @param[out] result   -  resulting vector.
 * @param[in]  iterable - Python object from which to extract the data.
 * @return bool - true on success, false otherwise.
 * @note on entry and on failure the vector is cleared.
 * @note on failure an exception is raised.
 */
bool
StringVecFromIterable(std::vector<std::string>& result, PyObject* iterable)
{
    result.clear();
    PyObject* iterator = PyObject_GetIter(iterable);
    if (!iterator) {
        PyErr_SetString(
            logbookExceptionObject,
            "StringVecFromIterable - supposed iterable does not support iteration"
        );
        return false;
    }
    PyObject* item;
    while ((item = PyIter_Next(iterator))) {
        const char* pText = PyUnicode_AsUTF8(item);
        if (!pText) {
            result.clear();
            Py_DECREF(item);
            Py_DECREF(iterator);
            return false;
        }
        result.emplace_back(pText);
        Py_DECREF(item);
    }
    Py_DECREF(iterator);
    return true;
}
/**
 * SizeVecFromIterable
 *   Given a Python object that is a container of integer like objects
 *   and produces a vector of size_t values.
 *
 * @param[out] result -- the vector produced. Cleared on entry and
 *                       cleared if there are errors.
 * @param[in]  items  -- The container python object.
 * @return bool - true if successful, false otherwise.
 * @note on failure an exception (LogBook.error) is raised.
 */
static bool
SizeVecFromIterable(std::vector<size_t>& result, PyObject* items)
{
    result.clear();
    PyObject* iterator = PyObject_GetIter(items);
    if (!iterator) {
        PyErr_SetString(
            logbookExceptionObject,
            "SizeVecFromIterable - object must support iteration and does not."
        );
        return false;
    }
    PyObject* item;
    while (item = PyIter_Next(iterator)) {
        size_t n = PyLong_AsSize_t(item);
        if ((n == static_cast<size_t>(-1)) && PyErr_Occurred()) {
            result.clear();
            Py_DECREF(item);
            Py_DECREF(iterator);
            return false;             // Exception raised.
        }
        result.push_back(n);
        Py_DECREF(item);
    }
    Py_DECREF(iterator);
    return true;
}
/**
 * NotesVectorToTuple
 *   Transform a vector of LogBookNote* objects into a
 *   tuple of Python LogBook.Note objects.
 *
 *  @param pyLogBook - Python object that is a log book.
 *  @param notes     - std::vector<LogBookNote*> to transform.
 *  @return PyObject* pointer to the tuple created.
 *  @note If there are errors, the result is nullptr and a
 *        an exception has been raised (LogBook.error).
 */  
static PyObject*
NotesVectorToTuple(PyObject* pyLogBook, const std::vector<LogBookNote*>& notes)
{
    PyObject* result = PyTuple_New(notes.size());
    if (!result) return nullptr;

    try {
        for (int i =0; i < notes.size(); i++) {
            PyObject* pNote = PyNote_create(pyLogBook, notes[i]);
            if (!pNote) {
                throw LogBook::Exception("Failed to make a note object");
            }
            PyTuple_SET_ITEM(result, i, pNote);
        }
    }
    catch (LogBook::Exception & e) {
        PyErr_SetString(
            logbookExceptionObject,
            e.what()
        );
        Py_DECREF(result);
        result = nullptr;
    }
    catch(...) {
        PyErr_SetString(
            logbookExceptionObject,
            "NotesVectorToTuple caught an unexpected exception"
        );
        Py_DECREF(result);
        result = nullptr;
    }
    return result;
}
///////////////////////////////////////////////////////////////
// Canonicals for PyLogBook (LogBook.LogBook) class/type.

/**
 * PyLogBook_new
 *    Create the object storage for a LogBook.LogBook type instance.
 *    Note that the m_pBook member will be initialized to nullptr
 *    so that it can be deleted without penalty in case  the deatailed
 *    construction fails.
 * @param type  - Pointer to the PyLogBook type struct.
 * @param args  - Positional arguments (ignored).
 * @param kwds  - Keyword arguments (ignored).
 * @return  PyObject* - pointer to the new object (PyLogBook struct)
 *                allocated an initialized but not constructed.
 * @retval NULL if failed.
 */
static PyObject*
PyLogBook_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    // Evidently tp_alloc will incref.
    
    PyLogBook* self = reinterpret_cast<PyLogBook*> (type->tp_alloc(type, 0));
    if (self) {
        self->m_pBook = nullptr;
    }
    return reinterpret_cast<PyObject*>(self);  
}
/**
 * PyLogBook_init
 *   Actual constructor for a LogBook.LogBook object once storage
 *   was allocated.
 *
 * @param self   - pointer to the PyLogBook object that contains object
 *                 storage
 * @param args   - Positional arguments (if no keywords this is the log filename).
 * @param kwargs - we support "filename" as the filename for the logbookl.
 * @return status of the attempt, 0 for success, -1 for failure.
 *         On failure a LogBook.error is raised if construction fails.
 */
static int
PyLogBook_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    static const char* kwlist[] = {"filename", NULL};
    const char* filename(nullptr);
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|s", const_cast<char**>(kwlist), &filename)
    ) {
        return -1;
    }
    try {
        PyLogBook* pThis = reinterpret_cast<PyLogBook*>(self);
        pThis->m_pBook = new LogBook(filename);
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        return -1;
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "Unanticipated excpetion type caught in PyLogBook_init"
        );
        return -1;
    }
    return 0;
}
/**
 * PyLogBook_dealloc
 *   Destructor for an instance of LogBook.LogBook:
 *
 * @param self - pointer to the object to delete.
 */
static void
PyLogBook_dealloc(PyObject* self)
{
    PyLogBook* pThis = reinterpret_cast<PyLogBook*>(self);
    delete pThis->m_pBook;
    pThis->m_pBook = nullptr;              // For good measure.
    Py_TYPE(self)->tp_free(self);
}
/////////////////////////////////////////////////////////////////
// LogBook object methods:

//                     Person API

/**
 * addPerson (add_person)
 *    Implements the add_person method.
 *    Creates a new person in the database and returns a LogBook.Person
 *    object that encapsulates that new person.
 *
 *  @param self - Pointer to the PyLogBook struct for this object.
 *  @param args - positional arguments.
 *  @param kwargs - Keyword parameters 'lastname', 'firstname', 'salutation'
 *  @return PyObject* - A newly created LogBook.Person object on success.
 *  @retval NULL - failed, and an exception is raised.
 */
static PyObject*
addPerson(PyObject* self, PyObject* args, PyObject* kwargs)
{
    const char* lastname(nullptr);
    const char* firstname(nullptr);
    const char* salutation(nullptr);
    const char* keywords[] = {"lastname", "firstname", "salutation", nullptr};
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|sss", const_cast<char**>(keywords),
        &lastname, &firstname, &salutation
    )) {
        return nullptr;
    }
    LogBook*   pBook = PyLogBook_getLogBook(self);
    PyObject* result = nullptr;
    try {
        LogBookPerson* person = pBook->addPerson(lastname, firstname, salutation);
        int id = person->id();
        delete person;
        
        // Now we can wrap this the Python way.
                
        result = PyPerson_newPerson(self, id);

    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "Unanticipated Exception type in LogBook.add_person"
        );
       
    }
    Py_XINCREF(result);
    return result;
}/**
  * findPeople
  *  @param self - pointer to the logbook object storage.
  *  @param args - Only one argument, a potentially null where clause.
  *  @return PyObject* Possibly empty n-tuple containing the
  *               matching person objects.
  */
static PyObject*
findPeople(PyObject* self, PyObject* args)
{
    const char* where(nullptr);
    if (!PyArg_ParseTuple(args, "|s", &where)) {
        return nullptr;
    }
    LogBook* book = PyLogBook_getLogBook(self);
    std::vector<LogBookPerson*> vResult;
    try {
        vResult = book->findPeople(where);
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        return nullptr;
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "Unanticipated excetpion type in LogBook.findPeople"
        );
        return nullptr;
    }
    PyObject* result = PyLogBook_TupleFromPeople(self, vResult);
    freeVector(vResult);
    
    return result;
}
/**
 * listPeople
 *   Returns a list of all of the people.
 *
 *  @param self - Object (logbook) requesting the operation.
 *  @param args - empty argument list.
 *  @return PyObject* see findPeople as we really just call that.
 */
static PyObject*
listPeople(PyObject* self, PyObject* args)
{
    auto a = PyTuple_New(0);
    PyObject* result =  findPeople(self, a);
    Py_DECREF(a);
    return result;
}
/**
 * getPerson
 *    Return a LogBook.Person given an id.
 * @param self - logbook object performing this method.
 * @param args - Single mandator parameter - the id.
 * @return PyObject* resulting LogBook.Person object
 * @retval   nullptr with an exception raised if an error.
 *           LogBook.error is raised for nonexistent id.
 */
static PyObject*
getPerson(PyObject* self, PyObject* args)
{
    int id(0);
    if (!PyArg_ParseTuple(args, "i", &id)) {
        return nullptr;
    }
    PyObject* result =  PyPerson_newPerson(self, id);
    Py_XINCREF(result);
    return result;
}
//               Shift api:

/**
 * getShift
 *   Given a new shift's id, returns it's shift object.
 *
 * @param PyObject* self - this logbook.
 * @param PyObject* args - parameters (shift id)
 * @return PyObject* Shift object.
 */
static PyObject*
getShift(PyObject* self, PyObject* args)
{
    int id;
    if(!PyArg_ParseTuple(args, "i", &id)) {
        return nullptr;
    }
    // Look up can throw C++:
   
    PyObject* result; 
    try {
        LogBook* book = PyLogBook_getLogBook(self);
        LogBookShift* pShift = book->getShift(id);
        if (!pShift) {
            throw LogBook::Exception("No such shift");
        }
        result = PyShift_newShift(self, pShift);
        delete pShift;              // No longer needed.
    }
    catch (LogBook::Exception & e) {
    PyErr_SetString(logbookExceptionObject, e.what());
    result =  nullptr;
  }
  catch (...) {
    PyErr_SetString(
      logbookExceptionObject,
      "Unexpected exception type caught in LogBook.get_shift"
    );
    result = nullptr;
  }
  return result;
}
/**
 * createShift
 *    Creates a new shift and encapsulates it in a PyLogBookShift
 *    object.
 * @param self - LogBook object in which the shift is being created.
 * @param args - parameters.  A shift name is mandatory
 *               if the members keyword is supplied it is a list of
 *               shift members.
 * @return PyObject*  - pointer to the new shift object.
 */
static PyObject*
createShift(PyObject* self, PyObject* args, PyObject* kwargs)
{
    const char* name(nullptr);
    PyObject*   members(nullptr);
    const char* keywords[] = {"name", "members", nullptr};
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "s|O", const_cast<char**>(keywords),
        &name, &members
    )) {
        return nullptr;
    }
    std::vector<LogBookPerson*> memberVec;
    if (members) {
        if(PyPerson_IterableToVector(memberVec, members)) {
            return nullptr;
        }
        // Note that the destructor of the logbook
        // Will also destroy the underlying member People.
        // This is undersirable, therefore,
        // We copy construct in... We do that here, rather than in
        // PyPerson_iterableToVector because that would, otherwise3,
        // be a policy decision about a function in another module:

        std::vector<LogBookPerson*> copiedPeople;
        for (auto p : memberVec) {
            copiedPeople.push_back(new LogBookPerson(*p));
        }
        memberVec = copiedPeople;
    }
    LogBook* book = PyLogBook_getLogBook(self);
    PyObject* result(nullptr);
    
    // Unique pointer below ensures that the LogBookShift* gets
    // deleted even on throws.
    try {
        std::unique_ptr<LogBookShift>
            pShift(book->createShift(name, memberVec));
        result = PyShift_newShift(self, pShift.get());
    }
    catch(LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        result = nullptr;
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "Unexpected exception in LogBook.create_shift"
        );
        return nullptr;
    }
    return result;
    
}
/**
 * listShifts
 *    Returns a  tuple containing all of the shifts
 *    known to the logbook.
 * @param self - pointer to the PyLogBook object
 * @param none - Nullptr since this is METH_NOARGS.
 * @return PyObject* tuple containing newly created PyShift object refs.
 */
static PyObject*
listShifts(PyObject* self, PyObject* none)
{
    LogBook* book = PyLogBook_getLogBook(self);
    try {
        auto shifts = book->listShifts();
        return PyShift_TupleFromVector(self, shifts);
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());

    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "An unanticipated exception type was caught in LogBook.list_shifts"
        );
    }
    
    return NULL;              // Failed if we got here.
}
/**
 * findShift
 *   Returns a shift given its name.
 * @param self - pointer to PyLogBook calling this method.
 * @param argv - Pointer to the tuple containing the string.
 * @return PyObject* shift or exception if nosuch.
 */
static PyObject*
findShift(PyObject* self, PyObject* args)
{
    const char* name;
    if (!PyArg_ParseTuple(args, "s", &name)) {
        return nullptr;
    }
    try {
        LogBook* book = PyLogBook_getLogBook(self);
        std::unique_ptr<LogBookShift> pShift(book->findShift(name));
        if (!pShift.get()) {
            throw LogBook::Exception("Shift not found");
        }
        
        PyObject* result =  PyShift_newShift(self, new LogBookShift(*pShift.get()));
        Py_XINCREF(result);
        return result;
    }
    catch (LogBook::Exception & e)
    {
        PyErr_SetString(logbookExceptionObject, e.what());
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "Unanticipated exception type throw in LogBook.find_shift"
        );
    }
    
    return nullptr;
}
/**
 * currentShift
 *   @param self - this object.
 *   @return PyObject* the current shift or None if there isn't one.
 */
static PyObject*
currentShift(PyObject* self, PyObject* none)
{
    LogBook* book = PyLogBook_getLogBook(self);
    std::unique_ptr<LogBookShift> shift(book->getCurrentShift());
    
    if (shift.get()) {
        PyObject* result(nullptr);
        try {
            result = PyShift_newShift(self, shift.get());
            Py_XINCREF(result);
        }
        catch (LogBook::Exception& e) {
            PyErr_SetString(logbookExceptionObject, e.what());
        }
        catch (...) {
            PyErr_SetString(
                logbookExceptionObject,
                "Unanticipated exception type cauth in current_shift"
            );
        }
        return result;
    } 
    Py_RETURN_NONE;

}

/////////////////// Run API ///////////////////////////////////

/**
 * currentRun
 *    Return a logbook run object that represents the
 *    current run -- if there is one.  Otherwise returns None.
 * @param self - pointer to the logbook.
 * @param unused - unused arguments.
 * @return PyObject*
 */
static PyObject*
currentRun(PyObject* self, PyObject* unused)
{
    LogBook* pBook = PyLogBook_getLogBook(self);
    LogBookRun* pResult = pBook->currentRun();
    
    if (pResult) {
        PyObject* result =  PyRun_newRun(self, pResult);
        delete pResult;
        return result;
    } else {
        Py_RETURN_NONE;             // NO current run.
    }
}
/**
 * beginRun
 *    Start a new run; returns the run object.  This can fail
 *    for a number of reasons failures result in LogBook.error
 *    being raised for the most part.
 * @param self - POinter to the logbook object.
 * @param args - position parameters.
 * @param kwargs - keyword parameters.  Number and title are mandatory.
 *                 remark is optional.
 * @return PyObject* the run object created by this begin run.
 */
static PyObject*
beginRun(PyObject* self, PyObject* args, PyObject* kwargs)
{
    int run(-1);
    const char* title(nullptr);
    const char* remark(nullptr);
    const char* keywords[] = {"number", "title", "comment", nullptr};
    
    if (!PyArg_ParseTupleAndKeywords (
        args, kwargs, "|iss", const_cast<char**>(keywords),
        &run, &title, &remark
    )) {
        return nullptr;
    }
    // We must have the run and title. Remark is optional:
    
    if ((run == -1) || (!title)) {
        PyErr_SetString(
            logbookExceptionObject,
            "For the LogBook.begin_run method only the comment is optional"
        );
        return nullptr;
    }
    PyObject* result(nullptr);
    LogBookRun* pRun(nullptr);
    try {
        // See if we can create the new run:
        
        LogBook*    pBook = PyLogBook_getLogBook(self);
        pRun = pBook->begin(run, title, remark);
        
        result = PyRun_newRun(self, pRun);
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "An unanticipated exception type was caught in LogBook.begin_run"
        );
    }
    delete pRun;
    return result;
}
/**
 * listRuns
 *    Return a vector of all the runs that are known  to the
 *    logbook.
 * @param self - pointer to our object storage.
 * @param unused
 * @return PyObject* tuple of existing runs.
 */
static PyObject*
listRuns(PyObject* self, PyObject* unused)
{
    LogBook* pBook = PyLogBook_getLogBook(self);
    std::vector<LogBookRun*> runs;
    PyObject* result(nullptr);
    try {
        runs      = pBook->listRuns();
        result    = PyRun_TupleFromVector(self, runs);
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "Unanticipated Exception type caught in LogBook.list_runs"
        );
    }
    
    freeVector(runs);
    return result;
    
}
/**
 * findRun
 *   Locates a run by run number and returns an object encapsulating it.
 * @param self - Logbook object.
 * @param args - only one argument, the run number to find.
 * @return PyObject* Encapsualted run or None if not found.
 */
static PyObject*
findRun(PyObject* self, PyObject* args)
{
    int number;
    if(!PyArg_ParseTuple(args, "i", &number)) {
        return nullptr;
    }
    
    PyObject* result = nullptr;
    LogBook*  pBook  = PyLogBook_getLogBook(self);
    LogBookRun* pRun;
    try {
        pRun = pBook->findRun(number);
        if (pRun) {
            result = PyRun_newRun(self, pRun);    
        } else {
            result = Py_None;
            Py_INCREF(Py_None);
        }
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "An unanticipated exception type was caught in LogBook.find_run"
        );
    }
    delete pRun;
    return result;
}
///////////////////////////////////////////////////////////////////
// Note API:

/**
 * createNote
 *    Create a note. This requires note 'text' and has optional parameters
 *    with the following keywords:
 *    - 'author'   - not author.
 *    - 'images'   - Iterable containing image filenames.
 *    - 'offsets'  - Iterable containing image offsets.
 *    - 'run'      - Associated run number.
 *
 *    Note that if the 'images' keywords is provided, the 'offsets'
 *    keyword must also be provided and the two objects must contain
 *    the same number of elements (although they need not be the
 *    same iterable type).  'run' is never required.
 *
 * @param self    - Pointer to the logbook. object.
 * @param args    - positional parameters.
 * @param kwargs  - Args specified by keywords.
 * @return PyObject* - A new LogBook.Note object.
 */
static PyObject*
createNote(PyObject* self, PyObject* args, PyObject* kwargs)
{
    const char* pText(nullptr);
    PyObject*   pAuthor(nullptr);
    PyObject*   pImageFiles(nullptr);
    PyObject*   pImageOffsets(nullptr);
    PyObject*   pRun(nullptr);
    
    const char* keywords[] = {
      "author", "text", "images", "offsets", "run", nullptr  
    };
    
    // Stuff we marshall from the parameters if present:
    // 
    
    LogBookRun*               pLogBookRun(nullptr);
    std::vector<std::string>  imageFilenames;
    std::vector<size_t>       imageOffsets;
    
    // Parse the arguments:
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|OsOOO", const_cast<char**>(keywords),
        &pAuthor, &pText, &pImageFiles, &pImageOffsets, &pRun
    )) {
        return nullptr;             // Exception already raised.
    }
    
    // Must have an author:
    
    if (!pAuthor) {
        PyErr_SetString(
            logbookExceptionObject,
            "Createing a note requires an author"
        );
        return nullptr;
    }
    // Must have text:
    
    if (!pText) {
        PyErr_SetString(
            logbookExceptionObject,
            "Creating a note requires note txt"
        );
        return nullptr;
    }
    
    // Things that are None, or missing. map to appropriate python types.

    bool decrefImages = false;
    bool decrefOffsets = false;
    if (!pImageFiles || (pImageFiles == Py_None)) {
        pImageFiles = PyTuple_New(0);
        decrefImages = true;
    }
    if (!pImageOffsets || (pImageOffsets == Py_None)) {
        pImageOffsets = PyTuple_New(0);
        decrefOffsets = true;
    }
    if (pRun == Py_None) {
        pRun = nullptr;
    }


    // Marshall the run if it's present.
    
    
    if (pRun) {
        if (PyRun_isRun(pRun)) {
            pLogBookRun = PyRun_getRun(pRun);
        } else {
            PyErr_SetString(
                logbookExceptionObject,
                "'run' object must be a LogBook.Run but isn't."
            );
            return nullptr;
        }
    }
    // The author must be a personb..and get it:
    
    if (!PyPerson_isPerson(pAuthor)) {
        PyErr_SetString(
            logbookExceptionObject, "'author' parameter must be a person object"
        );
        return nullptr;
    }
    LogBookPerson* pAuthorObject = PyPerson_getPerson(pAuthor);
    
    // Marshall the image filenames and offsets and validate.

    if (pImageFiles) {
        if (!StringVecFromIterable(imageFilenames, pImageFiles)) return nullptr;
        if (decrefImages) {
            Py_DECREF(pImageFiles);
        }

    }
    if (pImageOffsets) {
        if (!SizeVecFromIterable(imageOffsets, pImageOffsets)) return nullptr;
        if (decrefOffsets) {
            Py_DECREF(pImageOffsets);
        }
    }
    if (imageFilenames.size() != imageOffsets.size()) {
        PyErr_SetString(
            logbookExceptionObject,
            "There must be the same number of image filenames as offsets are nare not"
        );
        
        return nullptr;
    }
    
    
    // Try to make the note:
    
    PyObject* result(nullptr);
    try {
        LogBook* pBook = PyLogBook_getLogBook(self);
        LogBookNote* pNote = pBook->createNote(
            *pAuthorObject, pText, imageFilenames, imageOffsets, pLogBookRun
        );
        result = PyNote_create(self, pNote);
        
        delete pNote;                  // No longer needed.
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "Unexpected exception type caught in create_note"
        );
    }

    // Return the resulting note if it got made:
    
    Py_XINCREF(result);              // New ref if successful.
    return result;
}
/**
 * getNote
 *    Create a note given an integer id.
 *
 * @param self - Pointer to logbook object.
 * @param args - Positional parameters, only one, the id.
 * @return PyObject* - The note object; newly created.
 */
static PyObject*
getNote(PyObject* self, PyObject* args)
{
    int id;
    if(!PyArg_ParseTuple(
        args, "I", &id
    )) return nullptr;
    
    PyObject* result(nullptr);             // Initialized for errors.
    try {
       LogBook* pBook = PyLogBook_getLogBook(self);
       LogBookNote* pNote= pBook->getNote(id);
       result = PyNote_create(self, pNote);  // Refcount incremented.
       delete pNote;
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(
            logbookExceptionObject, e.what()
        );
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "Unexpected exception type in get_note"
        );
    }
    
    return result;
}
/**
 * listAllNotes
 *   Return a tuple containing all of the notes in the logbook.
 *   
 * @param self - Pointer to the logbook object.
 * @param unused - No parameters so this is unused.
 * @return PyObject* - pointer to a tuple containing all notes.
 */
static PyObject*
listNotes(PyObject* self, PyObject* unused)
{
    LogBook* pBook = PyLogBook_getLogBook(self);
    std::vector<LogBookNote*> notes = pBook->listAllNotes();
    PyObject* result =  NotesVectorToTuple(self, notes);   
    freeVector(notes);
    return result;
}
/**
 * listNotesForRunnum
 *    Return a tuple containing all the notes in the logbook
 *    for a run given its run number.
 * @param self - Pointer to the logbook object.
 * @param args - Positional args (must have the run number.)
 * @return PyObject* - pointer to the new tuple object.
 */
static PyObject*
listNotesForRunnum(PyObject* self, PyObject* args)
{
    int runNumber;
    
    if (!PyArg_ParseTuple(args, "i", &runNumber)) return nullptr;
    
    LogBook* pBook = PyLogBook_getLogBook(self);
    std::vector<LogBookNote*> notes = pBook->listNotesForRun(runNumber);
    PyObject* result =  NotesVectorToTuple(self, notes);   
    freeVector(notes);
    return result;
}
/**
 * listNotesForRunid
 *    Lists the notes that are associated with a run given its
 *    primary key (id).
 * @param self - pointer to the logbook object.
 * @param args - positional parameters - that's the id.
 * @return PyObject* Pointer to new tuple result.
 */
static PyObject*
listNotesForRunid(PyObject* self, PyObject* args)
{
    int id;
    if (!PyArg_ParseTuple(args, "i", &id)) return nullptr;
    
    LogBook* pBook = PyLogBook_getLogBook(self);
    std::vector<LogBookNote*> notes = pBook->listNotesForRunId(id);
    PyObject* result =  NotesVectorToTuple(self, notes);   
    freeVector(notes);
    return result;
}
/**
 * getNotesForRun
 *   Given a run object, returns the notes associated with that
 *   run.
 *
 * @param self - pointer this logbook object.
 * @param args - Positional parameters that contain the
 *               run object.
 * @return PyObject* - pointer to the resulting tuple.
 */
static PyObject*
getNotesForRun(PyObject* self, PyObject* args)
{
    PyObject* pRun;
    if (!PyArg_ParseTuple(args, "O", &pRun)) return nullptr;
    if (!PyRun_isRun(pRun)) {
        PyErr_SetString(
            logbookExceptionObject,
            "list_notes_for_run - The parameter must be a LogBook.Run  object and is not"
        );
        return nullptr;
    }
    LogBookRun* pNativeRun = PyRun_getRun(pRun);
    LogBook*    pBook = PyLogBook_getLogBook(self);
    std::vector<LogBookNote*> notes = pBook->listNotesForRun(pNativeRun);
    PyObject* result = NotesVectorToTuple(self, notes);
    freeVector(notes);
    return result;
}
/**
 * getNonrunNotes
 *   Returns a tuple containing the notes that are not associated with
 *   any run.
 * @param self  - pointer to the logbook object.
 * @param args  - unused pointer to parameters (none allowed).
 * @return PyObject* - pointer to the tuple created.
 */
static PyObject*
getNonrunNotes(PyObject* self, PyObject* args)
{
    LogBook* pBook = PyLogBook_getLogBook(self);
    std::vector<LogBookNote*> notes = pBook->listNonRunNotes();
    PyObject* result = NotesVectorToTuple(self, notes);
    freeVector(notes);
    return result;
}
/**
 * getNoteRun
 *   Return the run object associated with a run or
 *   None if the run is not associated with a run.
 *
 * @param self   - pointer to the logbook object.
 * @param args   - Note object.
 * @return PyObject* - see above.
 */
static PyObject*
getNoteRun(PyObject* self, PyObject* args)
{
    PyObject* pNoteObj;
    
    if (!PyArg_ParseTuple(args, "O", &pNoteObj)) return nullptr;
    
    // Require pNoteObj be a note object and extract the
    // logbook and note native objects.
    
    if (!PyNote_isNote(pNoteObj)) {
        PyErr_SetString(
            logbookExceptionObject,
            "get_note_run requires a note object"
        );
        return nullptr;
    }
    LogBook*     pBook = PyLogBook_getLogBook(self);
    LogBookNote* pNote = PyNote_getNote(pNoteObj);
    PyObject* result(nullptr);
    try {
        LogBookRun* pRun = pBook->getNoteRun(*pNote);
        if (!pRun) {
            result = Py_None;                  // Simplifies eventual
            Py_INCREF(Py_None);                // logic not using Py_RETURN_NONE
        } else {
            result = PyRun_newRun(self, pRun); // Does an incref.
        }
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "get_note_run: caught an unexpected exception type"
        );
    }
    return result;
}


////////////////////////////////////////////////////////////////////
// Key value store API.
//

/**
 * kvExists
 *   Determines if a key exists in the key/value store.
 * @param self  - Pointer to the logbook object storage.
 * @param args - Positional arguments... Only has the name of the key to check
 * @return PyObject* - Bool value - true if exists, false if not.
 */
static PyObject*
kvExists(PyObject* self, PyObject* args)
{
    const char* key(nullptr);
    LogBook* pBook = PyLogBook_getLogBook(self);
    
    if (!PyArg_ParseTuple(args, "s", &key)) return nullptr;  // Exception already raised.
    
    if (pBook->kvExists(key)) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
    // Should not get here so raise an exception that says that:
    
    PyErr_SetString(
        logbookExceptionObject,
        "kv_exists method - fell through without a defined value"
    );
    return nullptr;

}
/**
 * kvGet
 *   Returns the string value of a key in the key value store.
 *
 * @param self - pointer to our object storage.
 * @param args - Positional parameters that are just the keyword.
 * @return PyObject* - string object that contains the key's value.
 * @note if the key does not exist, a logbook exception is raised.
 */
static PyObject*
kvGet(PyObject* self, PyObject* args)
{
    const char* key;
    std::string value;
    LogBook* pBook = PyLogBook_getLogBook(self);
    
    if(!PyArg_ParseTuple(args, "s", &key)) return nullptr;  // Exception already raised.
    
    try {
        value = pBook->kvGet(key);
        return PyUnicode_FromString(value.c_str());
    }
    catch (LogBook::Exception& e) {
        std::stringstream msg;
        msg << "Unable to  fetch keyword " << key << "'s value: "
            << e.what();
        std::string m(msg.str());
        PyErr_SetString(logbookExceptionObject, m.c_str());
    }
    catch (...) {
        std::stringstream msg;
        msg << "Fetch of value for key  " << key
            << " failed with an unexpected exception type caught";
        std::string m(msg.str());
        PyErr_SetString(logbookExceptionObject, m.c_str());
    }
    return nullptr;        // ONly exceptions fall through
}
/**
 * kvSet
 *  Sets the value of a key.  If the key already exists in the kv store,
 *  it is modified (a new one is not created).  If it does not, then a new
 *  key value pair _is_ created.
 *
 * @param self - pointer to my object storage.
 * @param args - Pointer to position arguments which are key and value in that order.
 * @return PyObject* - none.
 */
static PyObject*
kvSet(PyObject* self, PyObject* args)
{
    const char* key;
    const char* value;
    LogBook* pBook = PyLogBook_getLogBook(self);
    std::string m;              // For exceptions
    
    if (!PyArg_ParseTuple(args, "ss", &key, &value)) return nullptr;
        
    try {
        pBook->kvSet(key, value);
        Py_RETURN_NONE;
    }
    catch (LogBook::Exception& e) {
        std::stringstream msg;   // We consider std::stringstream's construction expensive.
        msg << "Unable to set key " << key << " to " << value
            << " : " << e.what();
        m = (msg.str());
    }
    catch (...) {
        std::stringstream msg;
        msg << "Setting key " << key << " to " << value
            << " resulted in an unexpected exception type being caught";
        m = (msg.str());
    }
    PyErr_SetString(logbookExceptionObject, m.c_str());
    
    return nullptr;               // Only exceptions fall through.
}
/**
 * kvCreate
 *    Create a new key value pair.  It's an error, and will
 *    raise an exception if the key already exists.
 *    
 *  @param self - pointer to object storage.
 *  @param args - pointer to the position arguments which are the key and value
 *  @return PyObject* none on success.
 */
static PyObject*
kvCreate(PyObject* self, PyObject* args)
{
    const char* key;
    const char* value;
    LogBook* pBook = PyLogBook_getLogBook(self);
    std:: string m;
    
    if (!PyArg_ParseTuple(args, "ss", &key, &value)) return nullptr;
    
    try {
        pBook->kvCreate(key, value);
        Py_RETURN_NONE;
    }
    catch (LogBook::Exception& e) {
        std::stringstream msg;
        msg << "Unable to create new key: " << key << " with value: " << value
            << " : " << e.what();
        m = msg.str();
    }
    catch (...) {
        std::stringstream msg;
        msg << "When trying to create a new key: " << key << " with value "
            << value << " an unanticipated exception was caught";
        m = msg.str();
    }
    PyErr_SetString(logbookExceptionObject, m.c_str());
    return nullptr;
}

/**
 * kvList
 *     Return a tuple of pairs where each pair is a key value pair,
 *     listing the entire key value table.
 * 
 * @param self - pointer to this object.
 * @param unused - Pointer to the positional arguments. - unused
 * @return PyObject* tuple of key value pairs.
 */
static PyObject*
kvList(PyObject* self, PyObject* Py_UNUSED(ignored)) {
    LogBook* pBook = PyLogBook_getLogBook(self);

    std::vector<std::pair<std::string, std::string>> vresult = pBook->kvList();
    PyObject* result = PyTuple_New(vresult.size());
    for (int i = 0; i < vresult.size(); i++) {
        PyObject* key = PyUnicode_FromString(vresult[i].first.c_str());
        PyObject* value = PyUnicode_FromString(vresult[i].second.c_str());
        PyObject* item = PyTuple_New(2);
        PyTuple_SetItem(item, 0, key);
        PyTuple_SetItem(item, 1, value);

        PyTuple_SetItem(result, i, item);
    }
    return result;
}
///////////////////////////////////////////////////////////////
// Table for the PyLogBook type (LogBook.LogBook)

static PyMethodDef PyLogBook_methods [] = {   // methods
    // People api:
    {
        "add_person", (PyCFunction)addPerson,
        METH_VARARGS | METH_KEYWORDS, "Add a new person."
    },
    {
        "find_people", findPeople, METH_VARARGS, "Find matching people"
    },
    { "list_people", listPeople, METH_NOARGS, "Find all people"},
    { "get_person", getPerson, METH_VARARGS, "Create person from id"},
    
    // Shift API:
    
    {"get_shift", getShift, METH_VARARGS, "Return a new shift given id"},
    {
        "create_shift", (PyCFunction)createShift,
        METH_VARARGS | METH_KEYWORDS,
        "Create a new shift with or without people"
    },
    {"list_shifts", listShifts, METH_NOARGS, "Return a tuple with all shifts"},
    {"find_shift", findShift, METH_VARARGS, "Find a shift by name"},
    {"current_shift", currentShift, METH_NOARGS, "Return current shift"},
    
    //   Run APi:
    
    {"current_run", currentRun, METH_NOARGS, "Return current run object"},
    {
        "begin_run", (PyCFunction)beginRun, METH_VARARGS | METH_KEYWORDS,
        "Start a new run"
    },
    {"list_runs", listRuns, METH_NOARGS, "Return all runs"},
    {"find_run",  findRun, METH_VARARGS, "Find run by run number"},
    
    // Note API:
    
    {
        "create_note", (PyCFunction)createNote, METH_VARARGS | METH_KEYWORDS,
        "Create a new note"
    },
    { "get_note", getNote, METH_VARARGS, "Retrieve note by id"},
    {"list_all_notes", listNotes, METH_NOARGS, "Make a tuple of all notes"},
    {
        "list_notes_for_run_number", listNotesForRunnum, METH_VARARGS,
        "Make a tuple of all runs for a run number"
    },
    {
        "list_notes_for_run_id", listNotesForRunid, METH_VARARGS,
        "Make a tuple of all runs for a run id."
    },
    {
        "list_notes_for_run", getNotesForRun, METH_VARARGS,
        "Make a tuple of all runs for a run object"
    },
    {
        "list_nonrun_notes", getNonrunNotes, METH_NOARGS,
        "Make tuple of notes not associated with a run"
    },
    {
        "get_note_run", getNoteRun, METH_VARARGS,
        "Get run associated with a note or None if unassociated"
    },
    
    // Key value store interface:
    
    {"kv_exists", kvExists, METH_VARARGS, "Determine if a key exists"},
    {"kv_get", kvGet, METH_VARARGS, "Return the value of a key"},
    {"kv_set", kvSet, METH_VARARGS, "Create or modify a key/value"},
    {"kv_create", kvCreate, METH_VARARGS, "Create a new key/value pair"},
    {"kv_list", kvList, METH_NOARGS, "List all key value pairs."},
    
    
    // Ending sentinel:
    
     {NULL, NULL, 0, NULL}
};

// Python's Type definition for PyLogBook:

static PyTypeObject PyLogBookType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    /** g++ 4.9.2 does not support designated initializers.
    .tp_name = "LogBook.LogBook",
    .tp_basicsize = sizeof(PyLogBook),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)PyLogBook_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc  = "Logbook Class",
    .tp_methods = PyLogBook_methods,  
    .tp_init = (initproc)PyLogBook_init,
    .tp_new   = PyLogBook_new
    */
};

/////////////////////////////////////////////////////////////////
// LogBook.LogBook utilities for other type implementations:

/**
 * PyLogBook_ isLogBook
 *   @param obj - An object.
 *   @return bool -true if obj is a logbook as determined by PyObject_IsInstance
 */
bool
PyLogBook_isLogBook(PyObject* obj)
{
    return PyObject_IsInstance(
        obj, reinterpret_cast<PyObject*>(&PyLogBookType)
    ) != 0;
}
/**
 * PyLogBook_getLogBook(PyObject* obj)
 *
 *  @param obj - Object that's already been validatated to be a logbook.
 *  @return LogBook* - Pointer to the encapsulated C++ logbook o
 */
LogBook*
PyLogBook_getLogBook(PyObject* obj)
{
    PyLogBook* pbook = reinterpret_cast<PyLogBook*>(obj);
    return pbook->m_pBook;
}
///////////////////////////////////////////////////////////////////////////////
// Module level code for LogBook:

/**
 * create
 *    Create a new logbook.
 * @param self - module object.
 * @param args - parameter list.   We require four string like parameters:
 *              - filename     - Path to the logbook file.
 *              - experiment   - Experiment designation (e.g. '0400x').
 *              - spokesperson - Name of the experiment spokesperson (e.g. 'Ron Fox)
 *              - purpose      - Brief experiment purpose.
 * @return PyObject* - PyNULL if successful else an exception gets
 *                raised normally of type LogBook.error
 */
PyObject*
create(PyObject* self, PyObject* args)
{
    const char* filename;
    const char* experiment;
    const char* spokesperson;
    const char* purpose;
    
    if (!PyArg_ParseTuple(
        args, "ssss", &filename, &experiment, &spokesperson, &purpose)
    ) {
        return NULL;        // ParseTuple raise the exception
    }
    try  {
        LogBook::create(filename, experiment, spokesperson, purpose);
        Py_RETURN_NONE;
    }
    catch (std::exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "LogBook.create threw an unanticipated exception type"
        );
    }
    return NULL;
    
    
}

///////////////////////////////////////////////////////////////////////////////
// Stuff needed to make the module happen.

/**
 * logbookMethodTable
 *     This table defines LogBook module level methods - these correspond
 *     to the static methods of the C++ API's LogBook class.
 */
static PyMethodDef logbookMethodTable[] = {
    
    {"create", create, METH_VARARGS, "Create a new logbook"},

    //   End of methods sentinnel
    
    {NULL, NULL, 0, NULL}
};

/**
 * logbookModuleTable
 *    The logbook python module definition table.  Top level description
 *    of all of the module.
 */
static PyModuleDef logbookModuleTable = {
    PyModuleDef_HEAD_INIT,
    "LogBook",                       // Module name.
    nullptr,                         // at present no docstrings
    -1,                              // No per interpreter state.
    logbookMethodTable               // method table for LogBook. methods.
};

/**
 * PyInit_LogBook
 *    The logbook module initialization function.  Registers the module
 *    and the LogBook.exception exception we will use to report errors.
 *
 * Depends on:
 *    - logbookModuleTable  - the log book module table.
 *    - logbookExceptionObject - the logbook exception type.
 */

extern "C"
{
    PyMODINIT_FUNC
    PyInit_LogBook()
    {
      // This cruft is needed for C++ in Jessie e.g.:
      PyLogBookType.tp_name = "LogBook.Logbook";
      PyLogBookType.tp_basicsize = sizeof(PyLogBook);
      PyLogBookType.tp_itemsize  = 0;
      PyLogBookType.tp_dealloc   = (destructor)PyLogBook_dealloc;
      PyLogBookType.tp_flags  = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
      PyLogBookType.tp_doc   = "Logbook Class";
      PyLogBookType.tp_methods = PyLogBook_methods;
      PyLogBookType.tp_init    = (initproc)PyLogBook_init;
      PyLogBookType.tp_new     = PyLogBook_new;
      
        // Ready LogBookType.
        
        if (PyType_Ready(&PyLogBookType) < 0) {
            return NULL;
        }
        
        // Ready  Person type.
	
        PyPerson_InitType();
        if (PyType_Ready(&PyPersonType) < 0) {
            return NULL;
        }
        // Prepare the shift type.
	
	PyShift_InitType();
        if (PyType_Ready(&PyLogBookShiftType) < 0) {
            return NULL;
        }
        // Runs also have a transition type:

	PyRun_InitType();
        if (PyType_Ready(&PyRunTransitionType) < 0) {
            return NULL;
        }
	PyNote_InitType();
        if (PyType_Ready(&PyLogBookRunType) < 0) {
            return NULL;
        }
        if (PyType_Ready(&PyNoteImageType) < 0) return nullptr;
        if (PyType_Ready(&PyNoteType) < 0) return nullptr;
        
        
        // Initialize our module:
        
        PyObject* module= PyModule_Create(&logbookModuleTable);
        
        if (module) {
            // with the module initialized we need to create the exception.
            
            logbookExceptionObject =
                PyErr_NewException("LogBook.error", NULL, NULL);
            Py_XINCREF(logbookExceptionObject);
            if (PyModule_AddObject(module, "error", logbookExceptionObject) < 0) {
              Py_XDECREF(logbookExceptionObject);
              Py_CLEAR(logbookExceptionObject);
              Py_DECREF(module);                   // deletes the module.
              return nullptr;
            }
            
            // Add the log book type:
            
            Py_INCREF(&PyLogBookType);
            if (PyModule_AddObject(
                module, "LogBook",
                reinterpret_cast<PyObject*>(&PyLogBookType)
            ) < 0) {
                Py_DECREF(&PyLogBookType);
                Py_XDECREF(logbookExceptionObject);
                Py_DECREF(module);
                return NULL;
            }
            // Now add the LogBook.Person type:
            
            Py_INCREF(&PyPersonType);
            if (PyModule_AddObject(
                module, "Person",
                reinterpret_cast<PyObject*>(&PyPersonType))
                < 0
            )  {
                Py_DECREF(&PyPersonType);
                Py_DECREF(&PyLogBookType);
                Py_XDECREF(logbookExceptionObject);
                Py_DECREF(module);
                
                return NULL;
            }
            // add log bookshift
            
            Py_INCREF(&PyLogBookShiftType);
            if (PyModule_AddObject(
                module, "Shift",
                reinterpret_cast<PyObject*>(&PyLogBookShiftType)
            )
                < 0
            ) {
                Py_DECREF(&PyLogBookShiftType);
                Py_DECREF(&PyPersonType);
                Py_DECREF(&PyLogBookType);
                Py_XDECREF(logbookExceptionObject);
                Py_DECREF(module);
                return NULL;
            }
            // The PyRun complex is two types, the transition and
            // run itself:
            Py_INCREF(&PyRunTransitionType);
            if (PyModule_AddObject(
                module, "Transition",
                reinterpret_cast<PyObject*>(&PyRunTransitionType)
            ) < 0) {
                Py_DECREF(&PyRunTransitionType);
                Py_DECREF(&PyLogBookShiftType);
                Py_DECREF(&PyPersonType);
                Py_DECREF(&PyLogBookType);
                Py_XDECREF(logbookExceptionObject);
                Py_DECREF(module);
                return NULL;
            }
            Py_INCREF(&PyLogBookRunType);
            if (PyModule_AddObject(
                module, "Run",
                reinterpret_cast<PyObject*>(&PyLogBookRunType)
            ) < 0) {
                Py_DECREF(&PyLogBookRunType);
                Py_DECREF(&PyRunTransitionType);
                Py_DECREF(&PyLogBookShiftType);
                Py_DECREF(&PyPersonType);
                Py_DECREF(&PyLogBookType);
                Py_XDECREF(logbookExceptionObject);
                Py_DECREF(module);
                return NULL;
            }
            Py_INCREF(&PyNoteImageType);
            Py_INCREF(&PyNoteType);
            if (
                (PyModule_AddObject(module, "Image", reinterpret_cast<PyObject*>(&PyNoteImageType)) < 0) ||
                (PyModule_AddObject(module, "Note", reinterpret_cast<PyObject*>(&PyNoteType)) < 0)
            ) {
                Py_DECREF(&PyLogBookRunType);
                Py_DECREF(&PyRunTransitionType);
                Py_DECREF(&PyLogBookShiftType);
                Py_DECREF(&PyPersonType);
                Py_DECREF(&PyLogBookType);
                Py_DECREF(&PyNoteImageType);
                Py_DECREF(&PyNoteType);
                Py_XDECREF(logbookExceptionObject);
                Py_DECREF(module);
                return NULL;        
            } 
            
        }
        PyDateTime_IMPORT;
        return module;
    }
}                 // Extern C - required for Python interpreter linkage.
