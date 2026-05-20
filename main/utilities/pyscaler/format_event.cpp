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
 * @file format_event.cpp
 * @brief Implement the python wrapping of an event ring item.
 * @author Ron Fox.
 */

 #include "format_event.h"
 #include <CPhysicsEventItem.h>


static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";

// Initilialize the object so that we inherit properly.  
// Note that since only the factory is allowed to create us fully,
// the data pointers are both set to null.

static int init_basetype(PyObject* self, PyObject* args, PyObject* kwargs) {
    // Initialize our base class:
    pyEventItem* pThis = reinterpret_cast<pyEventItem*>(self);
    
    if (pyRingItemType.tp_init(self, args, kwargs) < 0) {
        return -1;
    }
    // Null out the  pointers:

    pThis->m_base.m_pItem = nullptr;
    pThis->m_pItem        = nullptr;

    return 0;
}

/**
 * getbody
 *    Gives the user a copy of the eventbody as a bytearray.
 *    I recommend then using the struct module to unpack
 *    the contents of that item in user code.
 * 
 * @param self - pointer to the object calling use.
 * @param args - Unused 
 * @return PyObject* byte array containing the event body.
 */
static PyObject*
getbody(PyObject* self, PyObject* args) {
    pyEventItem* pThis = reinterpret_cast<pyEventItem*>(self);
    ufmt::CPhysicsEventItem* pEvent = pThis->m_pItem;
    const void* body = pEvent->getBodyPointer();
    size_t bodySize = pEvent->getBodySize();
    return PyBytes_FromStringAndSize(reinterpret_cast<const char*>(body), bodySize);
}

// the method table:

static struct PyMethodDef methods[] = {
    {"getbody", getbody, METH_NOARGS, nullptr},
    {nullptr, nullptr, 0, nullptr}        // End of table sentinel.
};


// Type table:

PyTypeObject pyEventType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "physicsevent",
    .tp_basicsize = sizeof(pyEventItem),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible physics event item. Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = methods,
    .tp_base    = &pyRingItemType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
    
};