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

static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";

/**
 * @file format_scaler.cpp
 * @brief Implement Python wrapping of CRingItemScaler objecvts.
 * @author Ron Fox
 */
#include "format_scaler.h"

// This initialization sets up the object's inheritance from ringitem:

static int init_basetype(PyObject* self, PyObject* args, PyObject* kwargs) {
    // Initialize our base class:
    pyRingScalerItem* pThis = reinterpret_cast<pyRingScalerItem*>(self);
    
    if (pyRingItemType.tp_init(self, args, kwargs) < 0) {
        return -1;
    }
    // Null out the  pointers:

    pThis->m_base.m_pItem = nullptr;
    pThis->m_pItem        = nullptr;

    return 0;
}



 // Method definition:

 static struct PyMethodDef methods[] =  {
    {nullptr, nullptr, 0, nullptr}                          // table end semaphore
 };


 // Type definition:

 PyTypeObject pyRingScalerItemType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "scaleritem",
    .tp_basicsize = sizeof(pyRingScalerItem),
    .tp_itemsize = 0,
//    .tp_dealloc   = dealloc,   I think the base class will deallocate the abnormal end item the way I set it up.
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible scaler item Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = methods,
    .tp_base    = &pyRingItemType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
    
}; 

