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
 * @file format_ringfragment.cpp
 * @brief Implementation of python wrapp of CRingFragmentItem
 * @author Ron Fox
 */

static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";

#define RINGFRAGMENT_IMPLEMENTATION
#include "format_ringfragment.h"
#include <CRingFragmentItem.h>


// init_basetype:
//   Set up  our base type so that we can inherit methods.
//   Note the factory will fill in the object data so we
//   set those pointers to null so that horrible things
//   happen to those who try to construct their own.

static int
init_basetype(PyObject* self, PyObject* args, PyObject* kwargs) {
    pyRingFragmentItem* pThis = reinterpret_cast<pyRingFragmentItem*>(self);

    // Init the base type:

    if (pyRingItemType.tp_init(self, args, kwargs) < 0) {
        return -1;
    }
    // Null out our  real object pointers:

    pThis->m_base.m_pItem = nullptr;
    pThis->m_pItem = nullptr;

    return 0;                        // Success.

}

/////// Method implementations:

/**
 * timestamp
 *    Return the timestamp of the fragment.
 * @param self - pointer to the object that called us.
 * @param args - position parameters, not used.
 * @return PyObject* object containing the timestamp.
 */
static PyObject*
timestamp(PyObject* self, PyObject* args) {
    pyRingFragmentItem* pThis = reinterpret_cast<pyRingFragmentItem*>(self);
    return PyLong_FromUnsignedLongLong(pThis->m_pItem->timestamp());
}
/**
 * source
 *    Return the source id of the source that contributed the fragment.
 * @param self -pointer to the object that called us.
 * @param args - unused position arguments.
 * @return PyObject* object containing the source id.
 * 
 */
static PyObject*
source(PyObject* self, PyObject* args) {
    pyRingFragmentItem* pThis = reinterpret_cast<pyRingFragmentItem*>(self);
    return PyLong_FromUnsignedLong(pThis->m_pItem->source());
}
/**
 * payload
 *    Returns the fragment payload as a bytearra.
 * 
 * @param self - pointer to the object calling us.
 * @param args - unused positional args.
 * @return PyObject* - pointer to the byte array we produce.
 */
static PyObject*
payload(PyObject* self, PyObject* args) {
    pyRingFragmentItem* pThis = reinterpret_cast<pyRingFragmentItem*>(self);
    ufmt::CRingFragmentItem* pItem = pThis->m_pItem;    // For notational simplicty.
    size_t nBytes = pItem->payloadSize();
    return PyBytes_FromStringAndSize(
        reinterpret_cast<char*>(pItem->payloadPointer()), nBytes
    );
}
/**
 * barrierType
 *  return the fragment barrier type.
 * @param self -pointer to the object calling us.
 * @param args - unused positional arg.
 * @return PyObject* long containing the barrier.
 * 
 */
static PyObject*
barrierType(PyObject* self, PyObject* args) {
    pyRingFragmentItem* pThis = reinterpret_cast<pyRingFragmentItem*>(self);
    return PyLong_FromUnsignedLong(pThis->m_pItem->barrierType());
}
// THe method table:

static struct PyMethodDef methods[] = {
    {"timestamp", timestamp, METH_NOARGS, "Get the fragment timestamp"},
    {"source", source, METH_NOARGS, "Get the fragment source id"}, 
    {"payload", payload, METH_NOARGS, "Get the fragment payload as a byte array."},
    {"barrierType", barrierType, METH_NOARGS, "Get the barrier type of the fragment"},
    {nullptr, nullptr, 0, nullptr}        // End of table sentinel.
};

// Type table:

PyTypeObject pyRingFragmentType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ringfragmentitem",
    .tp_basicsize = sizeof(pyRingFragmentItem),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible Ring fragment item. Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = methods,
    .tp_base    = &pyRingItemType,
    .tp_init = init_basetype,
    .tp_new = PyType_GenericNew
};