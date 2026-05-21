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
 * @file format_ringitem.cpp
 * @brief implementation of Python wrapping of the ufmt::CRingItem base class.
 * @author Ron Fox
 */ 

#define  RINGITEM_IMPLEMENTATION
#include "format_ringitem.h"


static const char* Copyright = "Copyright Michigan State University 2026, All rights reserved";


/**
 *  dealloc
 *    Custom destrutor:
 *    - Need to kill off my ring item.
 */
static void
dealloc(PyObject* self) {
    pyRingItem* pThis  = reinterpret_cast<pyRingItem*>(self);
    delete pThis->m_pItem;            // destroy the encapsulated ring item.
     Py_TYPE(self)->tp_free(self);    // Free the rest of the object struct.
}

// Convenience routine - either get the pointer to the ring item
// or return nullptr and raise an runtime error:

static ufmt::CRingItem* 
getItem(PyObject* self) {
     pyRingItem* pThis = reinterpret_cast<pyRingItem*>(self);
    if (!pThis->m_pItem) {
        PyErr_SetString(PyExc_RuntimeError, "Use the ringitemfactory to create ring items!");
        return nullptr;
    }
    return pThis->m_pItem;
}
/**
 * getType
 *    Implement the type() method - Return the ring item type code.
 * @param self - pointer to our self.
 * @param args - Pointer to arguments.  Not used.
 * @return PyObject* integer type code.
 * @note If the ring item is not defined, we raise a RunTime exception.
 * @note The above assumes the struct is intialized to 0.  May need a 
 *      allocation/construction function if that's not the case.
 */
static PyObject*
getType(PyObject* self, PyObject* args) {
    ufmt::CRingItem* pItem = getItem(self);
    if( pItem) {
        return PyLong_FromLong(pItem->type());
    } else {
        return nullptr;
    }


}
/**
 * size
 *    Implement the size() method - return the number of bytes in the
 * ring item.
 * @param self - pointer to our self.
 * @param args - Pointer to arguments.  Not used.
 * @return PyObject* integer size in bytes.
 * @note If the ring item is not defined, we raise a RunTime exception.
 * @note The above assumes the struct is intialized to 0.  May need a 
 *      allocation/construction function if that's not the case.
 */
static PyObject* 
size(PyObject* self, PyObject* args) {
    ufmt::CRingItem* pItem = getItem(self);
    if (pItem) {
        return PyLong_FromLong(pItem->size());
    } else {
        return nullptr;
    }
}
/**
 * timestamp
 *   Return the item timestamp or None if there is on body header to get
 * it from.
 * 
 * @param self - pointer to our self.
 * @param args - Pointer to arguments.  Not used.
 * @return PyObject* timestamp.
 * @note If the ring item is not defined, we raise a RunTime exception.
 * @note The above assumes the struct is intialized to 0.  May need a 
 *      allocation/construction function if that's not the case.
 */
static PyObject*
timestamp(PyObject* self, PyObject* args) {
    ufmt::CRingItem* pItem  = getItem(self);
    if (pItem) {
        if (pItem->hasBodyHeader()) {
            return PyLong_FromLong(pItem->getEventTimestamp());
        } else {
            Py_RETURN_NONE;
        }
    } else {
        return nullptr;
    }
}
/**
 * sourceid
 *    Return the source id if the ring item has one else None
 * if there's no body header to get it from.
 * @param self - pointer to our self.
 * @param args - Pointer to arguments.  Not used.
 * @return PyObject* integer source id.
 * @note If the ring item is not defined, we raise a RunTime exception.
 * @note The above assumes the struct is intialized to 0.  May need a 
 *      allocation/construction function if that's not the case.
 */
static
PyObject* sourceid(PyObject* self, PyObject* args) {
    ufmt::CRingItem* pItem = getItem(self);
    if (pItem) {
        if (pItem->hasBodyHeader()) {
            return PyLong_FromLong(pItem->getSourceId());
        } else {
            Py_RETURN_NONE;
        }
    } else {
        return nullptr;
    }
}
/**
 * barriertype
 *    Return the barrier type if there is one else None if not.
 * @param self - pointer to our self.
 * @param args - Pointer to arguments.  Not used.
 * @return PyObject* barrier type code.
 * @note If the ring item is not defined, we raise a RunTime exception.
 * @note The above assumes the struct is intialized to 0.  May need a 
 *      allocation/construction function if that's not the case.
 */
static
PyObject* barriertype(PyObject* self, PyObject* args) {
    ufmt::CRingItem* pItem = getItem(self); 
    if (pItem) {
        if (pItem->hasBodyHeader()) {
            return PyLong_FromLong(pItem->getBarrierType());
        } else {
            Py_RETURN_NONE;
        }
    } else {
        return nullptr;
    }
}
/**
 * getbody
 *    Returns the body of the ring item as a byte array.
 *    By body I mean anything after the body header or longword
 *    says there isn't one.  
 * @note This also allows the ring item base class
 *       to act as an encpsulation of CRingFragmentItem, since
 *       the only useful thing you can do for it is to get the
 *       contents of the payload
 * @param self - pointer to the item calling us.
 * @param args - unused positional paramters.
 * @return PyObject* byte array containing the body.
 * 
 */
static PyObject*
getbody(PyObject* self, PyObject* args) {
    ufmt::CRingItem* pItem = getItem(self);
    if (pItem) {
        auto nBytes = pItem->getBodySize();
        const void* ptr = pItem->getBodyPointer();
        return PyBytes_FromStringAndSize(reinterpret_cast<const char*>(ptr), nBytes);
    } else {
        return nullptr;
    }
}
/*
  Methods ringitems have:
*/
static PyMethodDef ringitem_methods[] = {
    {"type", getType, METH_NOARGS, "Get ring item type"},
    {"size", size,     METH_NOARGS, "Get ring item size"},
    {"timestamp", timestamp,  METH_NOARGS, "Get timestamp"},
    {"sourceid", sourceid,   METH_NOARGS, "Get the source id"},
    {"barriertype", barriertype, METH_NOARGS, "Get barrier type"},
    {"body", getbody, METH_NOARGS, "Get the body as a byte array."},
    {nullptr, nullptr, 0, nullptr}                             // End sentinel
};



/**
  * Type definition block.
  * @todo = need a destructor to kill off the ring item.
  */
PyTypeObject pyRingItemType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ringitem",
    .tp_basicsize = sizeof(pyRingItem),
    .tp_itemsize = 0,
    .tp_dealloc   = dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = PyDoc_STR("Python acessible ring item. Should not be directly constructed.  Use ringitemfactory to make one. "),
    .tp_methods = ringitem_methods,
    .tp_new = PyType_GenericNew
    
};