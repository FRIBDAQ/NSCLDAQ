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
#include <map>
#include <NSCLDAQFormatFactorySelector.h>
#include <RingItemFactoryBase.h>
#include <DataFormat.h>
#include <exception>


// Supported version numbers.

static std::map<int, ufmt::FormatSelector:: SupportedVersions> versions = {
    {10, ufmt::FormatSelector::v10}, {11, ufmt::FormatSelector::v11}, {12, ufmt::FormatSelector::v12}
}; 


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

    ufmt::CRingItem* ringitem = rawItem->buf;
    pyRingitemFactory* pThis = reinterpret_cat<pyRingItemFactory*>(self);
    try {
        ufmt::CRingItem* pItem = pThis->m_pFactory->makeRingItem(ringitem);
    }
    catch(std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    return nullptr;
}

/*
  Methods factories have:
*/
  static PyMethodDef factory_methods[] = {
    {"makeRingItem", makeRingItem, METH_VARARGS, "Make a ring item base object from a memory buffer"},
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


