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
 * @file format_ringitem.cpp
 * @brief implementation of Python wrapping of the CRingItem base class.
 * @author Ron Fox
 */ 

#define  RINGITEM_IMPLEMENTATION
#include "format_ringitem.h"


static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";


/**
 *  dealloc
 *    Custom destrutor:
 *    - Need to kill off my ring item.
 */
static void
dealloc(PyObject* self) {
    pyRingItem* pThis  = reinterpret_cast<pyRingItem*>(self);
    delete pThis->m_pItem;            // destroy the encapsulated ring item.
     Py_TYPE(self)->tp_free(self);    // Free the rest of the object struct.
}

/*
  Methods ringitems have:
*/
static PyMethodDef ringitem_methods[] = {

    {nullptr, nullptr, 0, nullptr}                             // End sentinel
};



/**
  * Type definition block.
  * @todo = need a destructor to kill off the ring item.
  */
PyTypeObject pyRingItemType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ringitem",
    .tp_basicsize = sizeof(pyRingItem),
    .tp_itemsize = 0,
    .tp_dealloc   = dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible ring item"),
    .tp_methods = ringitem_methods,
    .tp_new = PyType_GenericNew
    
};