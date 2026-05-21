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

#include "format_textlist.h"
#include <CRingTextItem.h>
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

/**
 * The method table:
 */


 static struct PyMethodDef methods[] = {
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