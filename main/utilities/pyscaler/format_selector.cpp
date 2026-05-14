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

#define SELECTOR_IMPLEMENTATION
#include "format_selector.h"

PyTypeObject pyRingItemFactoryType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "format.ringitemfactory",
    .tp_basicsize = sizeof(pyRingItemFactory),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible ring item factory"),
    .tp_new = PyType_GenericNew,
};


