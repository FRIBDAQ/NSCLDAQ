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
#include "format_scaler.h"
#include "format_glomparams.h"
#include "format_event.h"
#include "format_ringfragment.h"
#include "format_eventcount.h"
#include "format_textlist.h"
#include "format_statechange.h"
#include "format_version.h"
#include <DataFormat.h>


// Some utility code:

// addConstants
//   Add module level constants to the module:
static void 
addConstants(PyObject* module) {
    // Ring item type definitions.

    PyModule_AddIntConstant(module, "BEGIN_RUN", ufmt::BEGIN_RUN);
    PyModule_AddIntConstant(module, "END_RUN", ufmt::END_RUN);
    PyModule_AddIntConstant(module, "PAUSE_RUN", ufmt::PAUSE_RUN);
    PyModule_AddIntConstant(module, "RESUME_RUN", ufmt::RESUME_RUN);
    PyModule_AddIntConstant(module, "ABNORMAL_ENDRUN", ufmt::ABNORMAL_ENDRUN);
    PyModule_AddIntConstant(module, "PERIODIC_SCALERS", ufmt::PERIODIC_SCALERS);
    PyModule_AddIntConstant(module, "INCREMENTAL_SCALERS", ufmt::INCREMENTAL_SCALERS);          // compatibility
    PyModule_AddIntConstant(module, "TIMESTAMPED_NONINCR_SCALERS", ufmt::TIMESTAMPED_NONINCR_SCALERS);  // no longer used bu..
    PyModule_AddIntConstant(module, "EVB_GLOM_INFO", ufmt::EVB_GLOM_INFO);
    PyModule_AddIntConstant(module, "PHYSICS_EVENT", ufmt::PHYSICS_EVENT);
    PyModule_AddIntConstant(module, "EVB_FRAGMENT", ufmt::EVB_FRAGMENT);
    PyModule_AddIntConstant(module, "PHYSICS_EVENT_COUNT", ufmt::PHYSICS_EVENT_COUNT);

    // Textual items:

    PyModule_AddIntConstant(module, "PACKET_TYPES", ufmt::PACKET_TYPES);
    PyModule_AddIntConstant(module, "MONITORED_VARIABLES", ufmt::MONITORED_VARIABLES);

    // Data format:

    PyModule_AddIntConstant(module, "RING_FORMAT", ufmt::RING_FORMAT);

    // dunno what it is:

    PyModule_AddIntConstant(module, "EVB_UNKNOWN_PAYLOAD", ufmt::EVB_UNKNOWN_PAYLOAD);

    
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

    if (PyType_Ready(&pyRingScalerItemType) < 0) {         // scaler item
        return nullptr;
    }
    if (PyModule_AddObjectRef(module, "scaleritem", (PyObject*)&pyRingScalerItemType) < 0) {
        return nullptr;
    }

    if (PyType_Ready(&pyGlomParametersType) < 0) {     // event builder glom parameters.
        return nullptr;
    }
    if (PyModule_AddObjectRef(module, "glomparameters", (PyObject*)&pyGlomParametersType) < 0) {
        return nullptr;
    }

    if (PyType_Ready(&pyEventType) < 0) {           // Physics event
        return nullptr;
    }
    if (PyModule_AddObjectRef(module, "physicsevent", (PyObject*)&pyEventType) < 0) {
        return nullptr;
    }

    if (PyType_Ready(&pyRingFragmentType) < 0) {     // Ring fragment item.
        return nullptr;
    }
    if (PyModule_AddObjectRef(module, "ringfragmentitem", (PyObject*)&pyRingFragmentType) < 0) {
        return nullptr;
    }

    if (PyType_Ready(&pyEventCountType) < 0) {
        return nullptr;
    }
    if (PyModule_AddObjectRef(module, "eventcountitem", (PyObject*)&pyEventCountType) < 0) {
        return nullptr;
    }

    if (PyType_Ready(&pyTextListType) < 0) {
        return nullptr;
    }
    if (PyModule_AddObjectRef(module, "stringlistitem", (PyObject*)&pyTextListType) < 0) {
        return nullptr;
        
    }

    if (PyType_Ready(&pyStateChangeType) < 0) {
        return nullptr;
    }
    if (PyModule_AddObjectRef(module, "statechangeitem", (PyObject*)&pyStateChangeType) < 0) {
        return nullptr;
    }

    if (PyType_Ready(&pyFormatVersionType) < 0) {
        return nullptr;
    }
    if (PyModule_AddObjectRef(module, "ringformatitem", (PyObject*)&pyFormatVersionType) < 0) {
        return nullptr;
    }
    return module;
}

}