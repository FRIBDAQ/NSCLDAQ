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
 * @file format_glomparams.cpp
 * @brief implement the python encapsulation of CGlomParameters
 * @author Ron Fox
 */
static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";
#define GLOMPARAMS_IMPLEMENTATION
#include "format_glomparams.h"
#include <CGlomParameters.h>

static const char* PolicyTable[] = {"first", "last", "average"};


// This initialization sets up the object's inheritance from ringitem:

static int init_basetype(PyObject* self, PyObject* args, PyObject* kwargs) {
    // Initialize our base class:
    pyGlomParametersItem* pThis = reinterpret_cast<pyGlomParametersItem*>(self);
    
    if (pyRingItemType.tp_init(self, args, kwargs) < 0) {
        return -1;
    }
    // Null out the  pointers:

    pThis->m_base.m_pItem = nullptr;
    pThis->m_pItem        = nullptr;

    return 0;
}

// Methods:

/**
 * coincidenceTicks
 *    Return the number of ticks that define Glom's coincidence
 * interval.
 * 
 * @param self - pointer to the object calling us. 
 * @param args - unused positional args.
 * @return PyObjecst* integer PyLong value.
 */
static PyObject*
coincidenceTicks(PyObject* self, PyObject* args) {
    pyGlomParametersItem* pThis = 
        reinterpret_cast<pyGlomParametersItem*>(self);
    
    return PyLong_FromUnsignedLongLong(pThis->m_pItem->coincidenceTicks());
}

/**
 * isBuilding
 *   @param self - pointer this object.
 *   @param args - unused positional args.
 *   @return PyObject* True if glom was building false if not.
 */
static PyObject*
isBuilding(PyObject* self, PyObject* args) {
    pyGlomParametersItem* pThis = 
        reinterpret_cast<pyGlomParametersItem*>(self);
    if (pThis->m_pItem->isBuilding()) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}
/**
 * policy
 * 
 * @param self -pointer to the object calling us.
 * @param args - unused positional arguments.
 * @return PyObject* a string containing one of:
 *      'first', 'last' or 'average'.
 * @note the lookup table relies on knowledge of the TimestampPolicy 
 * enum.
 */

static PyObject*
policy(PyObject* self, PyObject* args) {
    pyGlomParametersItem* pThis = 
        reinterpret_cast<pyGlomParametersItem*>(self);
    const char* p = PolicyTable[pThis->m_pItem->timestampPolicy()];
    return PyUnicode_FromString(p);

}
// Method dispatch table:

static struct PyMethodDef methods[] =  {
    {"coincidenceTicks", coincidenceTicks, METH_NOARGS, "Get GLOM coincidence ticks"},
    {"isBuilding", isBuilding, METH_NOARGS, "True if Glom was building"},
    {"policy", policy, METH_NOARGS, "The policy Glom used to assign timestamps"},
    {nullptr, nullptr, 0, nullptr}          // End of table sentinel.
};

// Type table.
PyTypeObject pyGlomParametersType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "glomparameters",
    .tp_basicsize = sizeof(pyGlomParametersItem),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible event builder glom parameters item. Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = methods,
    .tp_base    = &pyRingItemType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
    
};