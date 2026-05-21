/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014-2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file format_textlist.cpp
 * @brief implemention of the textlist class - ring items with lists of strings.
 * @author Ron Fox.
 */
static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";
#define TEXTLIST_IMPLEMENTATION
#include "format_textlist.h"
#include <CRingTextItem.h>
#include <vector>
// Set up the inheritance from the base type:

static int
init_basetype(PyObject* self, PyObject* args, PyObject* kwargs) {
    pyTextList* pThis = reinterpret_cast<pyTextList*>(self);

    // Init the base type:

    if (pyRingItemType.tp_init(self, args, kwargs) < 0) {
        return -1;
    }
    // Null out our  real object pointers, they will be set by the factory
    // when an object is created.

    pThis->m_base.m_pItem = nullptr;
    pThis->m_pItem = nullptr;

    return 0;                        // Success.

}
/////////////////////////  Public methods available to scripts:

/**
 * getStrings
 *     Returns  tuple of strings that are in the item.
 * @param self - Pointer to the object calling us.
 * @param args - Unused positional arguments.
 * @return PyObject* tuple of strings.
 */
static PyObject*
getStrings(PyObject* self, PyObject* args) {
    pyTextList* pThis = reinterpret_cast<pyTextList*>(self);
    auto strings = pThis->m_pItem->getStrings();

    // Make and load up the reslting tuple:

    PyObject* result = PyTuple_New(strings.size());
    for (int i = 0; i < strings.size() ; i++) {    // We need the tuple index...
        PyTuple_SetItem(result, i, PyUnicode_FromString(strings[i].c_str()));
    }

    return result;

}
/**
 * getElapsedTime
 *    Return the elapsed run time (in floating point seconds) at which the 
 * item was emitted fromt he readout.
 * 
 * @param self - Pointer to the object calling us.
 * @param args - unused positional arguments.
 * @return PyObject - Floating point seconds.
 * @note - we use computeElapsed time so that the user doesn't have to worry
 *         about divisors and such.
 */
static PyObject*
getElapsedTime(PyObject* self, PyObject* args) {
    pyTextList* pThis = reinterpret_cast<pyTextList*>(self);
   
    return PyFloat_FromDouble(pThis->m_pItem->computeElapsedTime());

}
/**
 * getTime
 *    Get the time in seconds after the epoch at which this item was
 * emitted.
 * 
 * @param self - pointer to the object that called us.
 * @param args - unused positional arguments.
 * @return PyObject* - Long object.
 */
static PyObject*
getTime(PyObject* self, PyObject* args) {
    pyTextList* pThis = reinterpret_cast<pyTextList*>(self);

    return PyLong_FromUnsignedLongLong(pThis->m_pItem->getTimestamp());
}

/**
 * getOriginalSource
 * 
 *    Get the id of the readout that emitted this item.  Note that glom e.g.
 * can rewrite the source id in the body header to support hierarchical event building.
 * 
 * @param self - pointer to the object calling us.
 * @param args - unused positional parameters.
 * @return PyObject* - PyLong result.
 */
static PyObject*
getOriginalSource(PyObject* self, PyObject* args) {
    pyTextList* pThis = reinterpret_cast<pyTextList*>(self);
    return PyLong_FromUnsignedLong(pThis->m_pItem->getOriginalSourceId());
}
/**
 * The method table:
 */


 static struct PyMethodDef methods[] = {
    {"getStrings", getStrings, METH_NOARGS, "Get strings from the item."},
    {"getElapsedTime", getElapsedTime, METH_NOARGS, "Get the elapsed run time from the item."}, 
    {"getTime", getTime, METH_NOARGS, "Get the wall clock time from the item"},
    {"originalSource", getOriginalSource, METH_NOARGS, "Get the original source id of the item"},
    {nullptr, nullptr, 0, nullptr}                 // End of table marker.
 };

 /**
  * The type table:
  */

  PyTypeObject pyTextListType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "stringlistitem",
    .tp_basicsize = sizeof(pyTextList),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible event count item. Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = methods,
    .tp_base    = &pyRingItemType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
  };