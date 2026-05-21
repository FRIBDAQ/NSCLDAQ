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
#define SCLAER_IMPLEMENTATION
#include "format_scaler.h"
#include <CRingScalerItem.h>
#include <vector>

// true if an object is a scaler:
// A type error exception raised and false if not:
static bool isScaler(PyObject* obj) {
    auto tp = Py_TYPE(obj);
    if (std::string(tp->tp_name) == "scaleritem") {
        return true;
    } else {
        PyErr_SetString(PyExc_TypeError, "Parameter is not a scaler item");
        return false;
    }
}

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

/**
 * starttime
 *   @param self - pointer to the object requesting must be a scaleritem.
 *   @param args - parameters (ignored).
 *   @return PyObject* a PyFloat contaning the interval start time in seconds.
 *   @exception TypeError if the object passed in is not a scaler item.
 */
static PyObject*
starttime(PyObject* self, PyObject* args) {
    if (!isScaler(self)) {
        return nullptr;
    }
    pyRingScalerItem* pThis = reinterpret_cast<pyRingScalerItem*>(self);
    float start = pThis->m_pItem->computeStartTime();
    return PyFloat_FromDouble(start);
}
/**
 * endtime
 *   @param self - pointer to object, must be a scaler item.
 *   @param args - paramters, not used.
 *   @return PyObject* PyFloat containing the intervale end time in seconds.
 *   @exception TypeError if the object passed in is not a scaler item.
 */
static PyObject*
endtime(PyObject* self, PyObject* args) {
    if (!isScaler(self)) {
        return nullptr;
    }
    pyRingScalerItem* pThis = reinterpret_cast<pyRingScalerItem*>(self);
    double end = pThis->m_pItem->computeEndTime();
    return PyFloat_FromDouble(end);
}

/**
 * absoluteTime
 *    Return the absolute timestamp in seconds since the epoch.
 * 
 * @param self - pointer to the object, must be a scaler item.
 * @param args - Parmaters not used.
 * @return PyObject* a long containing the timestamp field.
 */
static PyObject*
absoluteTime(PyObject* self, PyObject *args) {
    if (!isScaler(self)) {
        return nullptr;
    }
    pyRingScalerItem* pThis = reinterpret_cast<pyRingScalerItem*>(self);
    return PyLong_FromUnsignedLongLong(pThis->m_pItem->getTimestamp());
}
/**
 * isIncremental
 * 
 *      Returns a bool that's true if  the scalers are incremental.
 * @param self -  pointer to the object calling us.
 * @param args -  unused parameters.
 * @return PyObject* - that is True if the scalers are incremtnal False otherwise.
 * 
 */
static PyObject*
isIncremental(PyObject* self, PyObject* args) {
    if (! isScaler(self)) {
        return nullptr;
    }
    pyRingScalerItem* pThis = reinterpret_cast<pyRingScalerItem*>(self);
    if (pThis->m_pItem->isIncremental()) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}
/**
 *  number
 *    Returns the number of scalers in the item.
 * @param self - pointer to the objectg calling us.
 * @param args - unused parameters.
 * @return PyObject* that is a int containing the number of scalers in the item.
 */
static PyObject*
number(PyObject* self, PyObject* args) {
    if (!isScaler(self)) {
        return nullptr;
    }
    pyRingScalerItem* pThis = reinterpret_cast<pyRingScalerItem*>(self);
    return PyLong_FromLong(pThis->m_pItem->getScalerCount());
}
/**
 * getScaler
 *    Get the value of a single scaler
 * 
 * @param self - pointer to the object calling us.
 * @param args - A single parameter, the index, is expected.
 * @return PyObject* the integer value of the selected scaler.
 * @note If the index is out of range, IndexError
 *  is raised.
 */
static PyObject*
getScaler(PyObject* self, PyObject* args) {
    int index;
    if(!PyArg_ParseTuple(args, "i", &index)) {
        return nullptr;
    }
    if (!isScaler(self)) {
        return nullptr;
    }
    pyRingScalerItem* pThis = reinterpret_cast<pyRingScalerItem*>(self);
    auto pScaler= pThis->m_pItem;         // THe scaler item.
    if (index < pScaler->getScalerCount()){
        // index ok.

        return PyLong_FromLong(pScaler->getScaler(index));
    } else {
        // Index out of range.

        PyErr_SetString(PyExc_IndexError, "Scaler index out of range");
        return nullptr;
    }
    
}
/**
 * getScalers
 *    Return a tuple of all the scaler values.
 * 
 * @param self - pointer to the object calling us.
 * @param args - unuswed positional args.
 * @return PyObject * pointer to a tuple of integers.
 */
static PyObject*
getScalers(PyObject* self, PyObject* args) {
    if (!isScaler(self)) {
        return nullptr;
    }
    pyRingScalerItem* pThis = reinterpret_cast<pyRingScalerItem*>(self);
    auto scalers = pThis->m_pItem->getScalers();

    PyObject* result = PyTuple_New(scalers.size());

    // Counteed loop becuase set item needs an index:
    for (int i =0; i < scalers.size(); i++) {
        PyTuple_SetItem(result, i, PyLong_FromLong(scalers[i]));
    }
    return result;
}

/**
 * originalSid
 *    Return the source id of the originating source.
 * This can be different fromt he body header source id as that can
 * be set by the event builder for multi-level event building.
 * 
 * @param self - Pointer to the object calling us.
 * @param args - not used.
 * @return PyObject* int with the original source id.
 */
static PyObject*
originalSid(PyObject* self, PyObject* args) {
    if (!isScaler(self)) {
        return nullptr;
    }
    pyRingScalerItem* pThis = reinterpret_cast<pyRingScalerItem*>(self);

    return PyLong_FromLong(pThis->m_pItem->getOriginalSourceId());
}
 // Method definition:

 static struct PyMethodDef methods[] =  {
    {"startTime", starttime, METH_NOARGS, "Get interval start time"},
    {"endTime", endtime, METH_NOARGS, "Get interval end time"}, 
    {"absoluteTime", absoluteTime,  METH_NOARGS, "Get absolute timetsamp in seconds since epoch"},
    {"isIncremental", isIncremental, METH_NOARGS, "True if scalers were incrementally read"}, 
    {"number", number, METH_NOARGS, "Number of scalers in the item"}, 
    {"getScaler", getScaler, METH_VARARGS, "Get a single scaler by index"},
    {"getScalers", getScalers, METH_NOARGS, "Return the array of scalers"},
    {"getOriginalSourceId", originalSid, METH_NOARGS, "Return the original source id"},
    {nullptr, nullptr, 0, nullptr}                          // table end semaphore
 };


 // Type definition:

 PyTypeObject pyRingScalerItemType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "scaleritem",
    .tp_basicsize = sizeof(pyRingScalerItem),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible scaler item Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = methods,
    .tp_base    = &pyRingItemType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
    
}; 

