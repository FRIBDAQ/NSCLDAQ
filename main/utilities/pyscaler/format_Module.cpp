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
 * @file format_Module.cpp
 * @brief The Module code for the the DAQ format module.
 * @author Ron Fox
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <NSCLDAQFormatFactorySelector.h>
#include <RingItemFactoryBase.h>
#include <CRingItem.h>
#include <CDataFormatItem.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>


#include "format_selector.h"

/**
 * Given a format selector number, returns the 
 * appropriate format factory object.  The format
 * factory type object is defined in the format_selector.h header.
 * 
 */
static PyObject*
select_factory(PyObject* self, PyObject* args) {
    int format;
    if(!PyArg_ParseTuple(args, "i", &format)) {
        return nullptr;                               // AN exception was raised.
    }
    return nullptr;
}


// The module level methods. 
// These have to do with making the apropriate factories:

static PyMethodDef format_methods[] = {
    {"selectFactory", select_factory, METH_VARARGS, "Select a ring item factory from the NSCLDAQ version number" },
    {nullptr, nullptr, 0, nullptr}                // End of table sentinel.
};

// Module definition table:

static PyModuleDef format_module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "format",
    .m_doc  = "Provides version independent ring item format handling",
    .m_size = 0,
    .m_methods = format_methods,

};

extern "C" {
PyMODINIT_FUNC
PyInit_format(void) {
    PyModuleDef_Init(&format_module);
    auto module = PyModule_Create(&format_module);
    if (PyType_Ready(&pyRingItemFactoryType) < 0) {
        return NULL;
    }
    if (PyModule_AddObjectRef(module, "format", (PyObject*)(&pyRingItemFactoryType)) < 0) {
        return NULL;
    }
    return module;
}

}