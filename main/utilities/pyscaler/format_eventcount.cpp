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
 * @file format_eventcount.cpp
 * @brief implementation of the Python encapsulation of CRingPhysicsEventCountItem
 * @author Ron FOx
 */
static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";

#define EVENTCOUNT_IMPLEMENTATION
#include "format_eventcount.h"
#include <CRingPhysicsEventCountItem.h>


// Set up the inheritancde from the base type:

static int
init_basetype(PyObject* self, PyObject* args, PyObject* kwargs) {
    pyEventCount* pThis = reinterpret_cast<pyEventCount*>(self);

    // Init the base type:

    if (pyRingItemType.tp_init(self, args, kwargs) < 0) {
        return -1;
    }
    // Null out our  real object pointers:

    pThis->m_base.m_pItem = nullptr;
    pThis->m_pItem = nullptr;

    return 0;                        // Success.

}


// Method table:


static struct PyMethodDef methods[] = {

    {nullptr, nullptr, 0, nullptr}       // End of table marker.
};

// The type table:

PyTypeObject pyEventCountType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "eventcountitem",
    .tp_basicsize = sizeof(pyEventCount),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible event count item. Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = methods,
    .tp_base    = &pyRingItemType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
};