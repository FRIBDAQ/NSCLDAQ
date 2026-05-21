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
 * @file format_eventcount.cpp
 * @brief implementation of the Python encapsulation of CRingPhysicsEventCountItem
 * @author Ron FOx
 */
static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";

#define EVENTCOUNT_IMPLEMENTATION
#include "format_eventcount.h"
#include <CRingPhysicsEventCountItem.h>


// Set up the inheritancde from the base type:

static int
init_basetype(PyObject* self, PyObject* args, PyObject* kwargs) {
    pyEventCount* pThis = reinterpret_cast<pyEventCount*>(self);

    // Init the base type:

    if (pyRingItemType.tp_init(self, args, kwargs) < 0) {
        return -1;
    }
    // Null out our  real object pointers:

    pThis->m_base.m_pItem = nullptr;
    pThis->m_pItem = nullptr;

    return 0;                        // Success.

}
// Public methods:

/**
 * timeOffset
 *    Computes the offset into the run in floating point seconds.
 * This taks into account the timestamp divisor.
 * 
 * @param self - pointer to the object calling us.
 * @param args - unused positional args.
 * @return PyObject* floating point time in seconds.
 */
static PyObject*
timeOffset(PyObject* self, PyObject* args) {
    pyEventCount* pThis = reinterpret_cast<pyEventCount*>(self);
    ufmt::CRingPhysicsEventCountItem* pItem = pThis->m_pItem;

    return PyFloat_FromDouble(pItem->computeElapsedTime());
}
/**
 *  time;
 *     Returns the absolute time at which the item was emitted.
 * This is returned an an integer.
 * 
* @param self - pointer to the object calling us.
 * @param args - unused positional args.
 * @return PyObject* time since epcoh in seconds.
 *     
 */
static PyObject*
time(PyObject* self, PyObject* args) {
    pyEventCount* pThis = reinterpret_cast<pyEventCount*>(self);
    ufmt::CRingPhysicsEventCountItem* pItem = pThis->m_pItem;

    return PyLong_FromUnsignedLongLong(pItem ->getTimestamp());
}
/**
 *  eventCount
 *    Get the number of evennts collected since the run started.
 * 
 * @param self - pointer to the object calling us.
 * @param args - unused positiona arguments.
 * @return PyObject* unsigned long long.
 */
static PyObject*
eventCount(PyObject* self, PyObject* args) {
    pyEventCount* pThis = reinterpret_cast<pyEventCount*>(self);
    ufmt::CRingPhysicsEventCountItem* pItem = pThis->m_pItem;

    return PyLong_FromUnsignedLongLong(pItem->getEventCount());

}

/**
 *  originalSourceId.
 *    Return the source id of the source that originally emitted this
 * item.  This may be different than the ring item sourceid as the
 * event builder will slap that on.
 * 
 * @param self - pointer to the object calling us.
 * @param args - unused positional params.
 * @return PyObject*  an unsigned long.
 * 
 */
static PyObject*
originalSource(PyObject* self, PyObject* args) {
    pyEventCount* pThis = reinterpret_cast<pyEventCount*>(self);
    ufmt::CRingPhysicsEventCountItem* pItem = pThis->m_pItem;

    return PyLong_FromUnsignedLong(pItem->getOriginalSourceId());
}
// Method table:

static struct PyMethodDef methods[] = {
    {"timeOffset", timeOffset, METH_NOARGS, "Get floating point time offset"},
    {"time",       time      , METH_NOARGS, "Get the clock time of the item"},
    {"eventCount", eventCount, METH_NOARGS, "Get the event count"},
    {"originalSource", originalSource, METH_NOARGS, "Get the original source id"},
    {nullptr, nullptr, 0, nullptr}       // End of table marker.
};

// The type table:

PyTypeObject pyEventCountType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "eventcountitem",
    .tp_basicsize = sizeof(pyEventCount),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible event count item. Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = methods,
    .tp_base    = &pyRingItemType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
};