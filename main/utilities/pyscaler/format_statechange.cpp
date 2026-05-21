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
 * @file format_statechange.cpp
 * @brief Implementation of the statechange ring buffer item type.
 * @author Ron Fox.
 */
static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";

#define STATECHANGE_IMPLEMENTATION
#include "format_statechange.h"
#include <CRingStateChangeItem.h>

// Set up the inheritance from the base type:

static int
init_basetype(PyObject* self, PyObject* args, PyObject* kwargs) {
    pyStateChange* pThis = reinterpret_cast<pyStateChange*>(self);

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

/**
 * getRunNumber
 *     Get the run number from the state change item.
 * @param self - pointer to the object that called us.
 * @param args - unused positional arguments.
 * @return PyObject* - PyInt.
 */
static PyObject*
getRunNumber(PyObject* self, PyObject* args) {
    pyStateChange* pThis = reinterpret_cast<pyStateChange*>(self);
    return PyLong_FromUnsignedLong(pThis->m_pItem->getRunNumber());
}
/**
 * getElapsedTime
 *    Floatig point seconds since the start of the run.
 * @param self - pointer to the object that called us.
 * @param args - Unused positional args.
 * @return PyObject* PyFloat
 */
static PyObject*
getElapsedTime(PyObject* self, PyObject* args) {
    pyStateChange* pThis = reinterpret_cast<pyStateChange*>(self);
    return PyFloat_FromDouble(pThis->m_pItem->computeElapsedTime());
}
/**
 * getTitle
 * 
 * @param self - pointer to the object that called us.
 * @param args - Unused positional args.
 * @return PyObject* - PyUnicode.
 * 
 */
static PyObject*
getTitle(PyObject* self, PyObject* args) {
    pyStateChange* pThis = reinterpret_cast<pyStateChange*>(self);
    return PyUnicode_FromString(pThis->m_pItem->getTitle().c_str());
}
/**
 * getTime
 *    Time the item was emitted in seconds from epoch.
 *
 * @param self - object that called us. 
 * @param args - Unused positional args.
 * @return PyObject* - PyLong time since epoch.
 * 
 */
static PyObject*
getTime(PyObject* self, PyObject* args) {
    pyStateChange* pThis = reinterpret_cast<pyStateChange*>(self);
    return PyLong_FromUnsignedLongLong(pThis->m_pItem->getTimestamp());
}
/**
 * originalSource
 *    Returns the source id of the reader that emitted us.
 * @param self - object that called us. 
 * @param args - Unused positional args.
 * @return PyObject* - PyLong original source id.
 * 
 */
static PyObject*
getOriginalSource(PyObject* self, PyObject* args) {
    pyStateChange* pThis = reinterpret_cast<pyStateChange*>(self);
    return PyLong_FromUnsignedLong(pThis->m_pItem->getOriginalSourceId());
}

// Method definition table:

static PyMethodDef methods [] = {
    {"getRunNumber", getRunNumber, METH_NOARGS, "Get the item's run number"},
    {"getElapsedTime", getElapsedTime, METH_NOARGS, "Get elapsed run time in seconds"},
    {"getTitle", getTitle, METH_NOARGS, "Get the run title"},
    {"getTime", getTime, METH_NOARGS, "Get time in seconds since epoch."},
    {"originalSource", getOriginalSource, METH_NOARGS, "Get the original source id"},

    {nullptr, nullptr, 0, nullptr}                   // End of table marker.
};

// Type table:

PyTypeObject pyStateChangeType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "statechangeitem",
    .tp_basicsize = sizeof(pyStateChange),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible Statechange item.. Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = methods,
    .tp_base    = &pyRingItemType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
};
