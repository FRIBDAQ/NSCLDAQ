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
#include "format_ringitem.h"
#include "format_abnormalend.h"
#include <DataFormat.h>


// Some utility code:

// addConstants
//   Add module level constants to the module:
static void 
addConstants(PyObject* module) {
    PyModule_AddIntConstant(module, "BEGIN_RUN", 1);
    PyModule_AddIntConstant(module, "END_RUN", 2);
    PyModule_AddIntConstant(module, "PAUSE_RUN", 3);
    PyModule_AddIntConstant(module, "RESUME_RUN", 4);
    PyModule_AddIntConstant(module, "ABNORMAL_ENDRUN", 5);
    PyModule_AddIntConstant(module, "PERODIC_SCALERS", 20);
    PyModule_AddIntConstant(module, "INCREMENTAL_SCALERS", 20);          // compatibility
    PyModule_AddIntConstant(module, "TIMESTAMPED_NONINCR_SCALERS", 21);  // no longer used bu..
}

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
    addConstants(module);

    // Define the factory:

    if (PyType_Ready(&pyRingItemFactoryType) < 0) {
        return NULL;
    }
    if (PyModule_AddObjectRef(module, "ringitemfactory", (PyObject*)(&pyRingItemFactoryType)) < 0) {
        return NULL;
    }
    // Add the concrete ring item types;
    if (PyType_Ready(&pyRingItemType) < 0) {  // CRingItem
        return nullptr;
    }
    if (PyModule_AddObjectRef(module, "ringitem", (PyObject*)&pyRingItemType) < 0) {
        return nullptr;
    }
    
    if (PyType_Ready(&pyAbnormalEndItemType) < 0) {         // CAbnormalEndItem
        return nullptr;
    }
    if (PyModule_AddObjectRef(module, "abnormalenditem", (PyObject*)&pyAbnormalEndItemType) < 0) {
        return nullptr;
    }

    return module;
}

}