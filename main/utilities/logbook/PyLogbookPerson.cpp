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

/** @file:  PyLogbookPerson.cpp
 *  @brief: Implements the LogBook.Person type.
 *  
 */
#include "PyLogbook.h"
#include "PyLogBookPerson.h"
#include "LogBook.h"
#include "LogBookPerson.h"
#include <iostream>

typedef struct {
    PyObject_HEAD
    PyObject*      m_pBook;
    LogBookPerson* m_pPerson;
} PyLogBookPerson;

///////////////////////////////////////////////////////////////\
// Utilities:

/**
 * PyPerson_isPerson
 *    Determines if a python object is, in fact a person.
 * @param p - the object to check
 */
bool
PyPerson_isPerson(PyObject* p)
{
  return PyObject_IsInstance(
        p, reinterpret_cast<PyObject*>(&PyPersonType)
    ) != 0;
}
/**
 * PyPerson_IterableToVector
 *   Turns an interable that ostensibly contains PyLogBookPerson
 *   objects into a vector of pointers to LogBookPerson*
 *   This vector is made of pointers to the logbook person objects
 *   in the PyLogBookPerson objects themselves and, therefore
 *   those pointers should note be deleted by the caller.
 * @param[out] result - filled in with the vector of people.
 *                      this will be cleared before being populated
 *                      and cleared on error.
 * @param[in] iterable - a Python iterable object whose members must make
 *                PyPerson_isPerson return true.
 * @return int
 * @retval 0            - Success.
 * @retval -1           - failure with a python exception raised.
 * @note - if there's a need for the vector to persist long after the
 *         caller, the caller should incref the iterable so that it
 *         retains references to the objects it has.
 */
int
PyPerson_IterableToVector(
  std::vector<LogBookPerson*>& result, PyObject* iterable
)
{
  PyObject* iterator = PyObject_GetIter(iterable);
  result.clear();
  if (!iterator) {
    return -1;                   // Exception already raised.
  }
  PyObject* item;
  while (item = PyIter_Next(iterator)) {
    if (!PyPerson_isPerson(item)) {
      result.clear();
      PyErr_SetString(
        logbookExceptionObject,
        "PyPerson_IterableToVector - an item is not a person"
      );
      Py_XDECREF(iterator);
      return -1;            
    }
    result.push_back(PyPerson_getPerson(item));

  }
  Py_XDECREF(iterator);
  return 0;
}

/**
 * PyPerson_getPerson
 *   Get the underlying LogBookPerson* from the object
 * @param p - pointer to the object that has been verified to be
 *            a LogBook.Person.
 * @return LogBookPerson*
 */
LogBookPerson*
PyPerson_getPerson(PyObject* p)
{
  PyLogBookPerson* pThis = reinterpret_cast<PyLogBookPerson*>(p);
  return pThis->m_pPerson;
}
/**
 * PyPerson_newPerson
 *    Create a new LogBook.Person object given the logbook python object
 *    and the person id.
 * @param logbook - PyLogBook object pointer.
 * @param id      - id of the person in the database.
 * @return PyObject* Created/XINCREF's person object.
 */
PyObject*
PyPerson_newPerson(PyObject* logbook, int id)
{
  {
    PyObject* person = PyObject_CallFunction(
        reinterpret_cast<PyObject*>(&PyPersonType), "IO", id, logbook
     );
    Py_XINCREF(person);
    return person;
}

}

//////////////////////////////////////////////////////////////
//   Canonicals for PyLogBookPerson (LogBook.Person) class/type.

/**
 * PyPerson_new
 *   Create storage for a new logbook person.  This will be initialized
 *   so that it can be meaningfully deleted but will be meanigfully
 *   filled din by PyPerson_init
 * @param type   - Pointer to the PyLogBookPersonType struct.
 * @param args   - positional parameters (unused).
 * @param kwargs - keyword parameters (unused).
 * @return PyObject* - an instance of LogBook.Person not quite yet
 *                  ready for use.
 */
static PyObject*
PyPerson_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    PyLogBookPerson* self =
        reinterpret_cast<PyLogBookPerson*>(type->tp_alloc(type, 0));
    if (self) {
        self->m_pBook  = nullptr;
        self->m_pPerson = nullptr;
    }
    return reinterpret_cast<PyObject*>(self);
}
/**
 * PyPerson_init
 *    Initializes the storage for a LogBook.Person on the basis of
 *    the parameters passed to its constructor.  These are a logbook
 *    object and a person id (note that creation is done by first
 *    creating, getting the id and then constructing).
 *
 *  @Pparam self   - pointer to a PyLogBookPerson object that's been allocated
 *                   by PyPerson_new
 *  @param args    - positional parameters which are the id and the
 *                   logbook object.
 *  @param kwargs  -  "id" - the id, "logbook' - the log book argument.
 *  @return int    -  0 for success, -1 for failure with an exception raised.
 *  @note In keeping with the python storage management system, the
 *        logbook object's reference count is incremented.
 *  @note We _will_ enforce that the logbook parameter is, in fact, a logbook.
 * 
 */
static int
PyPerson_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    int id;
    PyObject* pLogBook;
    
    static const char* keywords[] = {"id", "logbook", nullptr};
    
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|iO", const_cast<char**>(keywords), &id,
        &pLogBook
    )) {
        return -1;
    }
    // If the object is not a logbook we need to complain:
    
    if (!PyLogBook_isLogBook(pLogBook)) {
        PyErr_SetString(
            logbookExceptionObject,
            "Logbook parameter is not a logbook object");
        return -1;
    }
    try {
        PyLogBookPerson* pThis = reinterpret_cast<PyLogBookPerson*>(self);
        pThis->m_pBook = pLogBook;
        
        LogBook* book  = PyLogBook_getLogBook(pLogBook);
        Py_INCREF(pLogBook);
        pThis->m_pPerson= book->getPerson(id); // Can throw.
                // We reference the book.
    }
    catch (LogBook::Exception& e) {
        PyErr_SetString(logbookExceptionObject, e.what());
        return -1;
    }
    catch (...) {
        PyErr_SetString(
            logbookExceptionObject,
            "Unanticipated exception type in PyPerson_Init"
        );
        return -1;
    }
    
    
    return 0;                        // success.
}
/**
 * PyPerson_dealloc
 *    Free storage associated with the person:
 *      - delete the person object.
 *      - deref the logbook object.
 *      - null the pointers for certain .
 *      - Free the object's storage.
 * #@param self - pointer to object storage (PyLogBookPerson*).
 */
static void
PyPerson_dealloc(PyObject* self)
{
  
    PyLogBookPerson* pThis = reinterpret_cast<PyLogBookPerson*>(self);
    Py_XDECREF(pThis->m_pBook);
    delete pThis->m_pPerson;
    pThis->m_pBook   = nullptr;
    pThis->m_pPerson = nullptr;
    Py_TYPE(self)->tp_free(self);
  
}
//////////////////////////////////////////////////////////////////
//getters:

/**
 * get_id
 *    Return the id of a person.
 * @param self -- the object storage.
 * @param closure - closure pointer (nullptr).
 * @return PyObject* - Result of the get.
 */
static PyObject*
get_id(PyObject* self, void* unused)
{
    PyLogBookPerson* pThis = reinterpret_cast<PyLogBookPerson*>(self);
    int id    = pThis->m_pPerson->id();
    return PyLong_FromLong(id);
}
/**
 * get_lastname
 *   Returns the last name of the person
 */
static PyObject*
get_lastname(PyObject* self, void* closure)
{
    PyLogBookPerson* pThis = reinterpret_cast<PyLogBookPerson*>(self);
    return PyUnicode_FromString(pThis->m_pPerson->lastName());
}
// get_firstname

static PyObject*
get_firstname(PyObject* self, void* closure)
{
    PyLogBookPerson* pThis = reinterpret_cast<PyLogBookPerson*>(self);
    return PyUnicode_FromString(pThis->m_pPerson->firstName());
}
// get_salutation

static PyObject*
get_salutation(PyObject* self, void* closure)
{
    PyLogBookPerson* pThis = reinterpret_cast<PyLogBookPerson*>(self);
    return PyUnicode_FromString(pThis->m_pPerson->salutation());
}

//////////////////////////////////////////////////////////////////
// Tables for the PyLogBookPerson type:

static PyMethodDef PyLogBookPerson_methods [] = {
    
    // END sentinel
    
    {NULL, NULL, 0, NULL}
};

static PyGetSetDef accessors[] = {
  {"id", (getter)get_id, nullptr, "Database primary key", nullptr},
  {"lastname", (getter)get_lastname, nullptr, "Last Name", nullptr},
  {"firstname", (getter)get_firstname, nullptr, "First Name", nullptr},
  {"salutation", (getter)get_salutation, nullptr, "Salutation", nullptr},
  
  // End delimeter.
  
  {NULL, NULL, NULL, NULL, NULL}  
};

PyTypeObject PyPersonType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    /** g++ 4.9.2 (jessie) does not support designated initializrs
    .tp_name = "LogBook.Person",
    .tp_basicsize = sizeof(PyLogBookPerson),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)(PyPerson_dealloc),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc  = "LogBook Person class",
    .tp_methods = PyLogBookPerson_methods,
    .tp_getset  = accessors,
    .tp_init = (initproc)PyPerson_init,
    .tp_new  = PyPerson_new
    */
};

// Initialize the person type.
// This is needed because g++-4.9.2 does not support designated initializers.
//
void PyPerson_InitType()
{
  PyPersonType.tp_name      = "LogBook.Person";
  PyPersonType.tp_basicsize = sizeof(PyLogBookPerson);
  PyPersonType.tp_itemsize  = 0;
  PyPersonType.tp_dealloc   = (destructor)(PyPerson_dealloc);
  PyPersonType.tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
  PyPersonType.tp_doc       = "LogBook Person Class";
  PyPersonType.tp_methods   = PyLogBookPerson_methods;
  PyPersonType.tp_getset    = accessors;
  PyPersonType.tp_init      = (initproc)(PyPerson_init);
  PyPersonType.tp_new       = PyPerson_new;
}
