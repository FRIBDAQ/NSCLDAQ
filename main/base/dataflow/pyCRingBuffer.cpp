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
 * @file pyCRingBuffer.cpp
 * @brief implement access to selected CRingBuffer static methods for Python clients.
 * @author Ron FOx.
 */
static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";

#include "pyCRingBuffer.h"  // Note this includes Python.h as well.
#include <CRingBuffer.h>
#include <Exception.h>
#include <string>


/**
 * create
 *    Creat a ring buffer.
 * @param self - the module pointer.
 * @param argv - the positional arguments - only one, the ringbuffer name is acceptable.
 * @param kwargs the conditional args.  We accept the following keywords:
 *               'databytes' - size of the data section of the ringbuffer.
 *               'maxconsumers' - maximum number of consumers that can connect.
 * @note suitable defauls are provided if a keyword arg is not provided.
 * @return None - Nothing meaningful is returned.
 *   
 */
static PyObject*
create(PyObject* self, PyObject* argv, PyObject* kwargs) {
    const char* ring_name;
    int databytes = CRingBuffer::getDefaultRingSize();
    int maxconsumers = CRingBuffer::getDefaultMaxConsumers();
    const char* keywords[] = {
        "name", "databytes", "maxconsumers", nullptr
    };
    if(!PyArg_ParseTupleAndKeywords(
        argv, kwargs, "s|$ii", const_cast<char**>(keywords), 
        &ring_name, &databytes, &maxconsumers
    )) {
        return nullptr;             // Presumably this raises the right exception.
    }

    try {
        CRingBuffer::create(ring_name, databytes, maxconsumers);
    } catch (CException& e) {
        PyErr_SetString(PyExc_RuntimeError, e.ReasonText());
        return nullptr;
    }
    Py_RETURN_NONE;
}

/**
 *  remove
 *     Remove an existing ring buffer.  Use wisely because the ringmaster
 * will attempt to force exits on all clients of the ringbuffer producers
 * and consumers alike.
 * 
 * @parm self - Module pointer.
 * @param argv - Positional args... we should have only one,
 *      The name of the ringbuffer.
 * @return None.
 */
static PyObject*
remove(PyObject* self, PyObject* argv) {
    const char* name;
    if (!PyArg_ParseTuple(argv, "s", &name)) {
        return nullptr;
    }

    try {
        CRingBuffer::remove(name);
    } catch(CException& e) {
        PyErr_SetString(PyExc_RuntimeError, e.ReasonText());
        return nullptr;
    }
    Py_RETURN_NONE;
}
/**
 * format
 *    Format a ring buffer.  This resets all of the pointers to an initial
 * state It should be used very sparingly, only when there's good reason to believe
 * the ringbuffer has become corrupted. It's safer to remove/receate the ringbuffer.
 * 
 * @param self - pointer to the module object.
 * @param argv  - Pointer to the list of positional args.  We expect only the name of the ringbuffer.
 * @return None;
 * @note - the max consumers is reset to the default.
 * @todo - Support keyword args for 'maxconsumers', an optional parameter to set that.
 */
static PyObject*
format(PyObject* self, PyObject* argv) {
    const char* name;
    if (!PyArg_ParseTuple(argv, "s", &name)) {
        return nullptr;
    }
    try {
        CRingBuffer::format(name);
    }
    catch (CException& e) {
        PyErr_SetString(PyExc_RuntimeError, e.ReasonText());
        return nullptr;
    }
    Py_RETURN_NONE;
}

/**
 * exists:
 *    Determines if a ringbuffer exists.
 * 
 * @param self - pointer to the module object.
 * @param argv - pointer to the list of parameters.  We expect only one, the name of the
 *               ringbuffer.
 * @return Bool - Py_TRUE - if the ringbuffer exists and Py_FALSE if not.
 */
static PyObject*
exists(PyObject* self, PyObject* argv) {
    const char* name;
    if(!PyArg_ParseTuple(argv, "s", &name)) {
        return nullptr;
    }
    bool result;
    try {
        result = CRingBuffer::isRing(name);
    } 
    catch (CException& e) {
        PyErr_SetString(PyExc_RuntimeError, e.ReasonText());
        return nullptr;
    }
    if (result ) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}
/**
 * default_name
 *    Returns the default ring buffer name for the current user.
 *    This shouild be the logged in user name.
 * 
 * @param self - pointer to the module object.
 * @param argv - positional arguments, we've declared this so there are none.
 * @return A unicode string. Note that we interpret the return value from the
 * CRingBuffer::defaultRing  call as a UTF-8 string...hopefully that works properly
 * in international environments.... Since the convention is to use UTF8 for filenames.
 * 
 */
static PyObject*
default_name(PyObject* self, PyObject* argv) {
    try {
        std::string cppname = CRingBuffer::defaultRing();
        const char* error="Converting ring name";
        return PyUnicode_DecodeUTF8(const_cast<char*>(cppname.c_str()), cppname.size(), error);
    }
    catch (CException& e) {
        PyErr_SetString(PyExc_RuntimeError, e.ReasonText());
        return NULL;
    }
}
/**
 * define the methods.  We're going to support:
 * - create - create a ring buffer.
 * - remove - remove an existing ringbuffer.
 * - format - format a ring buffer.
 * - exists - checks the existence of a ring by name.
 * - default_name - get the default ringbuffer name for this user.
 * - 
 */
static PyMethodDef ringbuffer_methods[] = {
    {"create", (PyCFunction)(create), METH_VARARGS |  METH_KEYWORDS, "Create a new ringbuffer."},
    {"remove", remove, METH_VARARGS, "Delete an existing ringbuffer"},
    {"format", format, METH_VARARGS, "Reformat an existing ringbuffer"},
    {"exists", exists, METH_VARARGS, "Check the existence of a ringbuffer"},
    {"default_name", default_name, METH_NOARGS, "Provide the user's default ringbuffer name"}, 
    {nullptr, nullptr, 0, nullptr}
};

static PyModuleDef ringbuffer_module {
    PyModuleDef_HEAD_INIT,
    "RingBuffer", 
    "Provide low level access to FRIBDAQ ringbuffers",
    -1,
    ringbuffer_methods
};

/**
 * PyInit_RingBuffer
 *    Initializes the module.  Note that we dont' define any types.
 * 
 * 
 */
extern "C" {
   PyMODINIT_FUNC
    PyInit_RingBuffer() {

        return PyModule_Create(&ringbuffer_module);
        
        
    }
}