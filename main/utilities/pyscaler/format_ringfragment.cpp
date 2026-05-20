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
 * @file format_ringfragment.cpp
 * @brief Implementation of python wrapp of CRingFragmentItem
 * @author Ron Fox
 */

static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";

#define RINGFRAGMENT_IMPLEMENTATION
#include "format_ringfragment.h"
#include <CRingFragmentItem.h>


// init_basetype:
//   Set up  our base type so that we can inherit methods.
//   Note the factory will fill in the object data so we
//   set those pointers to null so that horrible things
//   happen to those who try to construct their own.

static int
init_basetype(PyObject* self, PyObject* args, PyObject* kwargs) {
    pyRingFragmentItem* pThis = reinterpret_cast<pyRingFragmentItem*>(self);

    // Init the base type:

    if (pyRingItemType.tp_init(self, args, kwargs) < 0) {
        return -1;
    }
    // Null out our  real object pointers:

    pThis->m_base.m_pItem = nullptr;
    pThis->m_pItem = nullptr;

    return 0;                        // Success.

}

// THe method table:

static struct PyMethodDef methods[] = {
    {nullptr, nullptr, 0, nullptr}        // End of table sentinel.
};

// Type table:

PyTypeObject pyRingFragmentType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ringfragmentitem",
    .tp_basicsize = sizeof(pyRingFragmentItem),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible Ring fragment item. Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = methods,
    .tp_base    = &pyRingItemType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
};