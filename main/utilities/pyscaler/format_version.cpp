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
 * @file format_version.cpp
 * @brief Implementation of Python wrapping of CRingFormatItem.
 * @author Ron Fox.
 */

static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";
#define FORMAT_IMPLEMENTATION
#include "format_version.h"
#include <CDataFormatItem.h>

// Set up the inheritance from the base type:

static int
init_basetype(PyObject* self, PyObject* args, PyObject* kwargs) {
    pyFormatVersion* pThis = reinterpret_cast<pyFormatVersion*>(self);

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

/** getMajor
 *    @param self - pointer to the format  item object.
 *    @param args - unused positional arguments.
 *    @return PyObject* integer major version number.
 */
static PyObject*
getMajor(PyObject* self, PyObject* args) {
    pyFormatVersion* pThis = reinterpret_cast<pyFormatVersion*>(self);
    return PyLong_FromUnsignedLong(pThis->m_pItem->getMajor());
}


// The method table:

struct PyMethodDef methods []  = {
    {"getMajor", getMajor, METH_NOARGS, "Get the major version from the format item"},
    {nullptr, nullptr, 0, nullptr}                    // end table sentinel.
};

// The type table:

PyTypeObject pyFormatVersionType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ringformatitem",
    .tp_basicsize = sizeof(pyFormatVersion),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible Ring Format item... Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = methods,
    .tp_base    = &pyRingItemType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
};