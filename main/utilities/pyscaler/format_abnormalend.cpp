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
 * @file format_abnormalend.cpp
 * @brief implement the Python wrapping of a ufmt::CAbnormalEnd item.
 */
static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";

#define ABNORMALEND_IMPLEMENTATION
#include "format_abnormalend.h"
#include "format_ringitem.h"
#include <CAbnormalEndItem.h>


//
// We set up the inheritance from the 
// ring item base type.  Both object pointers
// (ring item and ours) will be nulled out
// they are filled in by the factory.  
// That implies that creation should
// only be through the factory.
static int init_basetype(PyObject* self, PyObject* args, PyObject* kwargs) {
    // Initialize our base class:
    pyAbnormalEndItem* pThis = reinterpret_cast<pyAbnormalEndItem*>(self);
    
    if (pyRingItemType.tp_init(self, args, kwargs) < 0) {
        return -1;
    }
    // Null out the  pointers:

    pThis->m_base.m_pItem = nullptr;
    pThis->m_pItem        = nullptr;

    return 0;
}


// The abnormalend dispatch table 
// note that all actual methods are inherited from the 'ringitem' base class.

static struct PyMethodDef methods[] = {
    {nullptr, nullptr, 0, nullptr}
};

// The type object for abnormalend:

PyTypeObject pyAbnormalEndItemType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "abnormalenditem",
    .tp_basicsize = sizeof(pyAbnormalEndItem),
    .tp_itemsize = 0,
//    .tp_dealloc   = dealloc,   I think the base class will deallocate the abnormal end item the way I set it up.
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible abnormal end run item. Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = methods,
    .tp_base    = &pyRingItemType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
    
}; 

