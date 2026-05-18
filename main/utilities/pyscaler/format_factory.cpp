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
#include "format_factory.h"
#include "format_ringitem.h"
#include "format_abnormalend.h"
#include "format_scaler.h"
#include <map>
#include <NSCLDAQFormatFactorySelector.h>
#include <RingItemFactoryBase.h>
#include <DataFormat.h>
#include <exception>
#include <string>



// Supported version numbers.

static std::map<int, ufmt::FormatSelector:: SupportedVersions> versions = {
    {10, ufmt::FormatSelector::v10}, {11, ufmt::FormatSelector::v11}, {12, ufmt::FormatSelector::v12}
}; 

///// Utility methods;

// isRawRingItem
//   Takes a PyObject and returns false with a type error exception raised if
//   it is not a "ringitem" type.
//
static bool
isRawRingItem(PyObject* o) {
    auto otype = Py_TYPE(o);
    if (std::string(otype->tp_name) != "ringitem") {
        PyErr_SetString(PyExc_TypeError, "Object passed in was not of 'ringitem' type");
        return false;
    }
    return true;
}
/////

/**
 * init
 *    Initialize the factory object.
 * @param self - actually a pointer to a pyRingItemFactory struct.
 * @param args - positional args.  In our case, there shoulid be one parameter,
 *                         the paramter is a supported version number e.g. 10, 11, 12.
 * @param kwargs -  keyword args, unsused by us.
 * @return int      0   - for success, and -1 if not.  ValueError is raised if so.
 * @note - the main problems are that 
 *         -  The main issue is an invalid/unsupported FRIB/NSCLDAQ version.
 */
static int
init(PyObject* self, PyObject *args, PyObject* kwargs) {
    int version;
    if (!PyArg_ParseTuple(args, "i", &version)) {
        return -1; 
    }
    // Validate the version:

    if (versions.count(version) == 0) {
        // invalid:

        PyErr_SetString(PyExc_ValueError, "Invalid  or unsupported FRIB/NSCLDAQ version number");
        return -1;
    }
    // Set the object.
    // Note because factories get cached, we don't need to clean up on destrution.
    //
    pyRingItemFactory* pFactory = reinterpret_cast<pyRingItemFactory*>(self);
    auto& factory = ufmt::FormatSelector::selectFactory(versions[version]);   // We've ensured version is ok.
    pFactory->m_pfactory= &factory;
    return 0;
}

/**
 * makeRingItem
 *     Create a ring item base class from a memory buffer via the factory.
 * @param self - pointer to our object (actually a pyRingItemFactory*).
 * @param args - Only one argument is accepted, a buffer like object. This must
 *     point/contain a raw ring item suitable for being passed into the ring item factory's
 *     makeRingItem method.
 * @return  PyObject* - on success, a pointer to a ring item object.  Note that some dirtiness is done
 *            in the construction of the ring item object to set it up.  Specifically,
 *            After making the object, _we_ will set its CRingItem pointer.
 * @retval nullptr - an Error occured and an exception is raised.
 * @note I don't think that exceptions can be returned from the factory but any that are
 *      derived from std::excetpion will be caught and turned into RunTimeError.
 */
static PyObject*
makeRingItem(PyObject* self, PyObject* args) {
    Py_buffer rawItem;
    if(!PyArg_ParseTuple(args, "y*", &rawItem)) {
        return nullptr;
    }
    // I believe our buffer is a contiguous memory block so the following is ok.

    ufmt::RingItem* ringitem = reinterpret_cast<ufmt::RingItem*>(rawItem.buf);
    pyRingItemFactory* pThis = reinterpret_cast<pyRingItemFactory*>(self);
    try {
        ufmt::CRingItem* pItem = pThis->m_pfactory->makeRingItem(ringitem);
        // Make a ringitem object and stuff our item into the 
        // slot for it in the object

        PyObject* empty = PyTuple_New(0);
        PyObject* ringitem = PyObject_Call(reinterpret_cast<PyObject*>(&pyRingItemType), empty, nullptr);
        Py_DECREF(empty);                                     // Release the tuple.
        if (!ringitem) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create a CRing item object");
            return nullptr;
        }
        pyRingItem *item = reinterpret_cast<pyRingItem*>(ringitem);
        item->m_pItem = pItem;
        
        return ringitem;
    }
    catch(std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
    

    
}
/**
 * makeAbnormalEndItem
 *    Takes a raw ring item and turn it into an abnormal end ring item.
 *    The raw ring item, normally comes from makeRingItem.  The calling
 *    program then analyzes the type and determines its and
 *    ABNORMAL_END item and calls us to 'up cast' the object to an
 *    abnormal end.
 * 
 * @param self - pointer to the factory object actually.
 * @param args - Pointer to the arguments.  One parameter is expected a ring item.
 * @return PyObject* on success an abnormal end ring item, nullptr with an exception raised
 *        on faiure.
 * 
 */
static PyObject*
makeAbnormalEndItem(PyObject* self, PyObject* args) {
    PyObject* rawItemObject;
    if(!PyArg_ParseTuple(args, "O", &rawItemObject)) {
        return nullptr;
    }
    // Ensure the object we have is a raw ring item and get its pointer:
    if (! isRawRingItem(rawItemObject)) {
        return nullptr;
    }
    // Get the ring item and fatory pointers.

    pyRingItem* pRingitemObject = reinterpret_cast<pyRingItem*>(rawItemObject);
    ufmt::CRingItem*  pRingItem       = pRingitemObject->m_pItem;
    pyRingItemFactory* pFactoryObject = reinterpret_cast<pyRingItemFactory*>(self);
    ufmt::RingItemFactoryBase*   pFactory = pFactoryObject->m_pfactory;

    // Now we can try to make the abnormal end type:
    // note an std::exception could be thrown (std::bad_cast)

    try {
        ufmt::CAbnormalEndItem* item = pFactory->makeAbnormalEndItem(*pRingItem);

        // Wrap it as a python object and return it.

        PyObject* empty = PyTuple_New(0);
        PyObject* ringitem = PyObject_Call(reinterpret_cast<PyObject*>(&pyAbnormalEndItemType), empty, nullptr);
        Py_DECREF(empty);                                     // Release the tuple.
        if (!ringitem) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create a CAbnormalEndItem object");
            return nullptr;
        }
        // Init both items, base class and us to the resulting item.
        // this lets the deletion of the CRingItem take care of us too:
        pyAbnormalEndItem* itemobj = reinterpret_cast<pyAbnormalEndItem*>(ringitem);
        itemobj->m_pItem = item;
        itemobj->m_base.m_pItem = reinterpret_cast<ufmt::CRingItem*>(item);

        
        return ringitem;
    
    }
    catch(std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

}

/**
 * makeScalerItem
 *    Given an ringitem object, attempts to convert it to a scaleritem.
 * 
 * @param self - Pointer to the factory object.
 * @param args - Positional arguments. Should only be one of them,  a "ringitem"
 * @return PyObject pointer to the newly created scaler item.
 * @exception the following exceptions can be raised:
 *     - TypeError, the parameter was not a ringitem.
 *     - RunTimeError the factory failed to convert it.
 */
static PyObject*
makeScalerItem(PyObject* self, PyObject* args) {
    PyObject* rawparam;
    if (!PyArg_ParseTuple(args, "o", &rawparam)) {
        return nullptr;
    }
    // Get the ring item or raise if wwe can't

    // Ensure the object we have is a raw ring item and get its pointer:
    if (! isRawRingItem(rawparam)) {
        return nullptr;
    }
    pyRingItem* pRingitemObject = reinterpret_cast<pyRingItem*>(rawparam);
    ufmt::CRingItem*  pRingItem       = pRingitemObject->m_pItem;

    // Get the factory.

    pyRingItemFactory* pFactoryObject = reinterpret_cast<pyRingItemFactory*>(self);
    ufmt::RingItemFactoryBase*   pFactory = pFactoryObject->m_pfactory;

    // Try the conversion:

    try {
        ufmt::CRingScalerItem* pRawScaler = pFactory->makeScalerItem(*pRingItem);

        // Wrap the scaler item as a python object:

        PyObject* empty = PyTuple_New(0);
        PyObject* ringitem = PyObject_Call(reinterpret_cast<PyObject*>(&pyRingScalerItemType), empty, nullptr);
        Py_DECREF(empty);                                     // Release the tuple.
        if (!ringitem) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create a CRingScalerItem object");
            return nullptr;
        }
        // Init both items, base class and us to the resulting item.
        // this lets the deletion of the CRingItem take care of us too:

        pyRingScalerItem* itemobj = reinterpret_cast<pyRingScalerItem*>(ringitem);
        itemobj->m_pItem = pRawScaler;
        itemobj->m_base.m_pItem = reinterpret_cast<ufmt::CRingItem*>(pRawScaler);

        
        return ringitem;

    } catch(std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

/*
  Methods factories have:
*/
  static PyMethodDef factory_methods[] = {
    {"makeRingItem", makeRingItem, METH_VARARGS, "Make a ring item base object from a memory buffer"},
    {"makeAbnormalEndItem", makeAbnormalEndItem, METH_VARARGS, "Convert a ring item to an abnormal end item"},
    {"makeScalerItem", makeScalerItem, METH_VARARGS, "Convert a ring item into a scaler item"},
    {nullptr, nullptr, 0, nullptr}                             // End sentinel
};

 /**
  * Type definition block.
  */
PyTypeObject pyRingItemFactoryType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ringitemfactory",
    .tp_basicsize = sizeof(pyRingItemFactory),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("Python acessible ring item factory"),
    .tp_methods = factory_methods,
    .tp_init = init,
    .tp_new = PyType_GenericNew
    
};


