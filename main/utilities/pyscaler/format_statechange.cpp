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


// Method definition table:

static PyMethodDef methods [] = {

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
    .tp_base    = &pyStateChangeType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
};
