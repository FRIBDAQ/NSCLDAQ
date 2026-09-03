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

/** @file:  PyLogBookShift.cpp
 *  @brief: Implement the logbook shift.
 */
#include "PyLogBookShift.h"
#include "PyLogbook.h"
#include "PyLogBookPerson.h"
#include "LogBookShift.h"
#include "LogBook.h"
#include "LogBookPerson.h"


///////////////////////////////////////////////////////////////////
// Internal utilities

/**
 * getPerson
 *   Validate a python object to be a person and return the LogBookPerson*
 *   it encapsulates.
 * @param person - Stipulated to be a person.
 * @return LogBookPerson*
 * @retval nullptr - not a person object.
 */
static LogBookPerson*
getPerson(PyObject* person)
{
  if (!PyPerson_isPerson(person)) return nullptr;
  return PyPerson_getPerson(person);
}

//////////////////////////////////////////////////////////////////////
// Exported utilities:
/**
 * PyPerson_isShift
 *    @param p - the object to test.
 *    @reeturn bool - true if the object is a PyLogBookShift object.
 */
bool
PyPerson_isShift(PyObject* p)
{
  return PyObject_IsInstance(
    p, reinterpret_cast<PyObject*>(&PyLogBookShiftType)
  ) != 0;
}

/**
 * PyShift_newShift
 *    Creat a shift from a logbook python object and a LogBookShift.
 * @param book - PyObject* known to be a logbook.
 * @param shift - LogBookShift* to be tunrned into a PyObject shift.
 * @return PyObject* - new Log Book Shift object.
 * @note this involves creating a new shift object so the
 *       one passed in can be deleted if no longer needed.
 */
PyObject*
PyShift_newShift(PyObject* book, LogBookShift* shift)
{
  int id = shift->id();
  PyObject* pyShift =  PyObject_CallFunction(
    reinterpret_cast<PyObject*>(&PyLogBookShiftType), "IO", id, book
  );
  Py_XINCREF(pyShift);
  return pyShift;
}
/**
 * PyShift_TupleFromVector
 *   Given a vector of shift object pointers turns them into a
 *   tuple of PyLogBookShift objects.
 * @param book - Object verified to be a logbook.
 * @param shifts - shifts to convert.
 * @return PyObject* resulting new tuple object.
 * @retval nullptr - Unable to do that a python exception has been raised.
 */
PyObject*
PyShift_TupleFromVector(PyObject* book, const std::vector<LogBookShift*>& shifts)
{
  PyObject* result = PyTuple_New(shifts.size());
  if (!result) return nullptr;
  
  try {
    for (int i = 0; i < shifts.size(); i++) {
      PyTuple_SET_ITEM(result, i, PyShift_newShift(book, shifts[i]));
    }
  }
  catch (LogBook::Exception& e) {
    freeTuple(result);
    result = nullptr;
    PyErr_SetString(logbookExceptionObject, e.what());
  }
  catch (...) {
    freeTuple(result);
    result = nullptr;
    PyErr_SetString(
      logbookExceptionObject,
      "Unable to turn a shift vector into a tuple in PyShift_TupleFromVector"
    );
  }
  return result;
}

////////////////////////////////////////////////////////////////////
// Shift type canonicals.

/**
 * PyShift_new
 *    Allocates storage for a new logbook shift object.
 *    The storage is filled in as follows:
 *    * m_book <- nullptr.
 *    * m_shift <- nullptr
 *
 *   This allows those objects to be freed via Py_XDECREF and delete
 *   in case the creation fails and PyShift_dealloc is called before
 *   construction is complete.
 * @param type   - Pointer to the type object for LogBook.Shift.
 * @param args   - Positional parameters (ignored).
 * @param kwargs - Keyword parmeters (ignored).
 * @return PyObject* the allocated and initialized storage.
 */
static PyObject*
PyShift_new(PyTypeObject* type, PyObject* aregs, PyObject* kwargs)
{
  pPyLogBookShift self =
    reinterpret_cast<pPyLogBookShift>(type->tp_alloc(type, 0));
  if (self) {
    self->m_book  = nullptr;
    self->m_shift = nullptr;
  }
  return reinterpret_cast<PyObject*>(self);
}
/**
 * PyShift_init
 *    Initialize contents of a shift object created by PyShift_new
 *    given a logbook and the id of the shift.
 *
 * @param self  - Pointer to the PyLogBooKShift struct.
 * @param args  - Positional parameters.
 * @param kwargs - Keywords parameters: 'logbook', and 'id'
 * @return int   - -1 on error, 0 on success.
 *                an exception (often LogBook.error) is raised if -1 is
 *                returned.
 */
static int
PyShift_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
  const char* keywords[] = {"id", "logbook", nullptr};
  PyObject* logbook;
  int       id;
  
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "|iO", const_cast<char**>(keywords), &id, &logbook
  )) {
    return -1;
  }
  // Ensure the logbook is a logbook:
  
  if (!PyLogBook_isLogBook(logbook)) {
    PyErr_SetString(logbookExceptionObject, "PyShift construction not a logbook object");
    return -1;
  }
  
  try {
    pPyLogBookShift pThis = reinterpret_cast<pPyLogBookShift>(self);
    LogBook* pBook = PyLogBook_getLogBook(logbook);
    pThis->m_shift = pBook->getShift(id);
    pThis->m_book  = logbook;
    Py_INCREF(logbook);            // we reference the book.
  }
  catch (LogBook::Exception& e) {
    PyErr_SetString(logbookExceptionObject, e.what());
    return -1;
  }
  catch (...) {
    PyErr_SetString(
      logbookExceptionObject,
      "Unexpected exception type in PyShift_init"
    );
    return -1;
  }
  return 0;
  
}
/**
 * PyShift_dealloc
 *    Free storage/resources associated with a shift (destructor)
 *
 *  @param self  - Pointer to the object.
 */
void
PyShift_dealloc(PyObject* self)
{
  pPyLogBookShift pThis = reinterpret_cast<pPyLogBookShift>(self);
  Py_XDECREF(pThis->m_book);
  delete pThis->m_shift;
  
  Py_TYPE(self)->tp_free(self);
}

////////////////////////////////////////////////////////////////////
// Attribute getters.

/**
 * get_id
 *   Return the id of the shift.
 * @param self - pointer to the object's storage.
 * @param closure - unused closure object.
 * @return PyObject* - integer id object.
 */
static PyObject*
get_id(PyObject* self, void* closure)
{
  pPyLogBookShift pThis = reinterpret_cast<pPyLogBookShift>(self);
  return PyLong_FromLong(pThis->m_shift->id());
}
/**
 * get_name
 *    Return the name of the shift.
 * @param self - pointer to the object's storage.
 * @param closure - unused closure object.
 * @return PyObject* - integer id object.
 */
static PyObject*
get_name(PyObject* self, void* closure)
{
  pPyLogBookShift pThis = reinterpret_cast<pPyLogBookShift>(self);
  return PyUnicode_FromString(pThis->m_shift->name());

}
/**
 * get_members
 *    Produces an ntuple containing the members of the shift.
 *    Each ntuple member is a Python person object.
 * @param self   - pointer to my storage.
 * @param closure - unused pointer to per getter data.
 * @return PyObject* see above
 */
static PyObject*
get_members(PyObject* self, void* closure)
{
    pPyLogBookShift pThis = reinterpret_cast<pPyLogBookShift>(self);
    // Need to copy the shift members to avoid premature deletion:

    std::vector<LogBookPerson*> people = pThis->m_shift->members();
    std::vector<LogBookPerson*> people_copy;
    for (auto p : people) {
      people_copy.push_back(new LogBookPerson(*p));
    }
    return PyLogBook_TupleFromPeople(
      pThis->m_book, people_copy
    );
}
////////////////////////////////////////////////////////////////////
// Methods:

/**
 * addMember
 *    Adds a member to this shift.
 *
 * @param PyObject* self - pointer to my storage.
 * @param PyObject* args - only one allowed - a python person object.
 * @return PyObject* self - support member chaining for the heck of it.
 */
static PyObject*
addMember(PyObject* self, PyObject* args)
{
  PyObject* pyPerson;
  if (!(PyArg_ParseTuple(args, "O", &pyPerson))) {
    return nullptr;
  }
  LogBookPerson* person = getPerson(pyPerson);
  if (!person) {
    PyErr_SetString(
      logbookExceptionObject,
      "The parameter to LogBook.Shift.add_member must be a LogBook.Person object"
    );
    return nullptr;
  }
  pPyLogBookShift pThis = reinterpret_cast<pPyLogBookShift>(self);
  LogBook* book = PyLogBook_getLogBook(pThis->m_book);
  LogBookPerson* personCopy = new LogBookPerson(*person);   // Need to feed a copy so the caller'rs
  try {                                            // won't get deleted.
    book->addShiftMember(pThis->m_shift, personCopy);      
  }
  catch (LogBook::Exception & e) {
    PyErr_SetString(logbookExceptionObject, e.what());
    return nullptr;
  }
  catch (...) {
    PyErr_SetString(
      logbookExceptionObject,
      "Unexpected exception type caught in LogBook.Shift.add_member"
    );
    return nullptr;
  }
  
  return self;
}
/**
 * removeMember
 *   Removes a member from the shift.
 * @param PyObject* self  - pointer to the shift object.
 * @param PyObject* args  - positional args -contains PyPerson object.
 * @return self - to support object chaining.
 */
static PyObject*
removeMember(PyObject* self, PyObject* args)
{
  PyObject* PyPerson;
  if (!PyArg_ParseTuple(args, "O", &PyPerson)) {
    return nullptr;
  }
  LogBookPerson* person = getPerson(PyPerson);
  if (!person) {
    PyErr_SetString(
      logbookExceptionObject,
      "Parameter to remove_member must be a LogBook.Person but wasn't"
    );
    return nullptr;
  }
  pPyLogBookShift pThis = reinterpret_cast<pPyLogBookShift>(self);
  LogBook* book = PyLogBook_getLogBook(pThis->m_book);
  try {
    book->removeShiftMember(pThis->m_shift, person);
  }
  catch (LogBook::Exception & e) {
    PyErr_SetString(logbookExceptionObject, e.what());
    return nullptr;
  }
  catch (...) {
    PyErr_SetString(
      logbookExceptionObject,
      "Unexpected exception type caught in LogBook.Shift.remove_member"
    );
    return nullptr;
  }
  
  return self;
}
/**
 * setCurrent
 *    Set the object to be the current shift.
 * @param self - pointer to the object storage.
 * @param argv - nullptr not used.
 * @return self - to support object chaining.
 */
static PyObject*
setCurrent(PyObject* self, PyObject* args)
{
  pPyLogBookShift pThis = reinterpret_cast<pPyLogBookShift>(self);
  LogBook* book = PyLogBook_getLogBook(pThis->m_book);
  try {
    book->setCurrentShift(pThis->m_shift);
  }
  catch (LogBook::Exception & e) {
    PyErr_SetString(logbookExceptionObject, e.what());
    return nullptr;
  }
  catch (...) {
    PyErr_SetString(
      logbookExceptionObject,
      "Unexpected exception type caught in LogBook.Shift.set_current"
    );
    return nullptr;
  }
  return self;
}

////////////////////////////////////////////////////////////////////
// Shift type tables for python.
/*
 *  Shift accessor table.
 */
static PyGetSetDef accessors[] = {
    // End delimeter.
  {"id", get_id, nullptr, "Get database primary key", nullptr},
  {"name", get_name, nullptr, "Get name of shift", nullptr},
  {"members", get_members, nullptr, "Get ntuple of shift members", nullptr},
  
  // name getter setter doc closure
  {NULL, NULL, NULL, NULL, NULL}  
};

/**
 * shift method table
 */

static PyMethodDef PyLogBookShift_methods[]  = {
     // End delimeter.
  
  {"add_member", addMember, METH_VARARGS, "Add member to shift"},
  {"remove_member", removeMember, METH_VARARGS, "Remove member from shift"},
  {"set_current", setCurrent, METH_NOARGS, "Set shift as current"},
  
  
  // name function, flags docstring
  {NULL, NULL,  0,  NULL}  
};


PyTypeObject PyLogBookShiftType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    /** g++ version 4.9.2 e.g. does not supporte designated initializers 
    .tp_name = "LogBook.Shift",
    .tp_basicsize = sizeof(PyLogBookShift),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)(PyShift_dealloc),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc  = "LogBook Shift class; collection of people",
    .tp_methods = PyLogBookShift_methods,
    .tp_getset  = accessors,
    .tp_init = (initproc)PyShift_init,
    .tp_new  = PyShift_new    
    */
};

/**
 *  This is necessary because earlier versions of g++ do not support
 *  designated initializers menaing we must actively init the struct prior to use.
 */
void
PyShift_InitType()
{
  PyLogBookShiftType.tp_name      = "LogBook.Shift";
  PyLogBookShiftType.tp_basicsize = sizeof(PyLogBookShiftType);
  PyLogBookShiftType.tp_itemsize  = 0;
  PyLogBookShiftType.tp_dealloc   = (destructor)(PyShift_dealloc);
  PyLogBookShiftType.tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
  PyLogBookShiftType.tp_doc       = "Log book shift class - collection of people";
  PyLogBookShiftType.tp_methods   = PyLogBookShift_methods;
  PyLogBookShiftType.tp_getset    = accessors;
  PyLogBookShiftType.tp_init      = (initproc)(PyShift_init);
  PyLogBookShiftType.tp_new       = PyShift_new;
}
