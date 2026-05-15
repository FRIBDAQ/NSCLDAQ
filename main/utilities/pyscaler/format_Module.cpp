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

#include "format_factory.h"



// The module level methods. 
// These have to do with making the apropriate factories:

static PyMethodDef format_methods[] = {
    {nullptr, nullptr, 0, nullptr}                // End of table sentinel.
};

// Module definition table:

static PyModuleDef format_module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "daqformat",
    .m_doc  = "Provides version independent ring item format handling",
    .m_size = 0,
    .m_methods = format_methods,

};

extern "C" {
PyMODINIT_FUNC
PyInit_daqformat(void) {
    PyModuleDef_Init(&format_module);
    auto module = PyModule_Create(&format_module);
    if (PyType_Ready(&pyRingItemFactoryType) < 0) {
        return NULL;
    }
    if (PyModule_AddObjectRef(module, "ringitemfactory", (PyObject*)(&pyRingItemFactoryType)) < 0) {
        return NULL;
    }
    return module;
}

}