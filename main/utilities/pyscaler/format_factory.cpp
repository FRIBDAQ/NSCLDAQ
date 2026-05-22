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
#include "format_glomparams.h"
#include "format_event.h"
#include "format_ringfragment.h"
#include "format_eventcount.h"
#include "format_textlist.h"
#include "format_statechange.h"
#include "format_version.h"

#include <CAbnormalEndItem.h>
#include <CRingItem.h>
#include <CRingScalerItem.h>
#include <CGlomParameters.h>
#include <CPhysicsEventItem.h>
#include <CRingFragmentItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingTextItem.h>
#include <CRingStateChangeItem.h>
#include <CDataFormatItem.h>

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
            delete pItem;
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
            delete item;
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
    if (!PyArg_ParseTuple(args, "O", &rawparam)) {
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
            delete pRawScaler;
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
/**
 * makeGlomParameters
 *   Take a raw ring item and turn it into a gloimparameters
 * object.
 * 
 * @param self - pointer to the factory object.
 * @param args - Command arguments, the first and only must be a
 *               ringitem to converty.
 * @return PyObject* - pointer to a new glomparameters object.
 * 
 */
static PyObject*
makeGlomParameters(PyObject* self, PyObject* args) {
    PyObject* rawparam;
    if (!PyArg_ParseTuple(args, "O", &rawparam)) {
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
        ufmt::CGlomParameters* pRawGlomItem = pFactory->makeGlomParameters(*pRingItem);
        // Wrap the object in a ring item python object

        PyObject* empty = PyTuple_New(0);
        PyObject* ringitem = PyObject_Call(
            reinterpret_cast<PyObject*>(&pyGlomParametersType), empty, nullptr);
        Py_DECREF(empty);                                     // Release the tuple.
        if (!ringitem) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create a CRingScalerItem object");
            delete pRawGlomItem;
            return nullptr;
        }
        // We're the ones that init the data structur:

        pyGlomParametersItem* itemobj =
             reinterpret_cast<pyGlomParametersItem*>(ringitem);
        itemobj->m_pItem = pRawGlomItem;
        itemobj->m_base.m_pItem = reinterpret_cast<ufmt::CRingItem*>(pRawGlomItem);
        return ringitem;
    }
    catch (std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}
/**
 * makephysicsevent
 *    Take a ring item parameter and return a physics item.
 * 
 * @param self- pointer to the factory calling us.
 * @param args - Positional args, one ring item object.
 * @return PyObject* - physics event item as python object.
 * 
 */
static PyObject*
makephysicsevent(PyObject* self, PyObject* args)  {
    PyObject* rawparam;
    if (!PyArg_ParseTuple(args, "O", &rawparam)) {
        return nullptr;
    }
    if (!isRawRingItem(rawparam)) {
        return nullptr;                    // Parameter isn't a ring item.
    }
    // Get the raw ring item:

    pyRingItem* pRingItemObject = reinterpret_cast<pyRingItem*>(rawparam);
    ufmt::CRingItem* pRingItem = pRingItemObject->m_pItem;

    // get the factory.
    pyRingItemFactory* pFactoryObject = reinterpret_cast<pyRingItemFactory*>(self);
    ufmt::RingItemFactoryBase* pFactory = pFactoryObject->m_pfactory;

    // Now try the conversion.

    try {
        ufmt::CPhysicsEventItem* pRawEvent = pFactory->makePhysicsEventItem(*pRingItem);
        
        // Ok, now wrap the event in a pyEventType:

        PyObject* empty = PyTuple_New(0);
        PyObject* eventObj = PyObject_Call(
            reinterpret_cast<PyObject*>(&pyEventType), empty, nullptr
        );
        Py_DECREF(empty);
        if (!eventObj) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create a physics event item object");
            delete pRawEvent;
            return nullptr;
        }
        pyEventItem* pEvent = reinterpret_cast<pyEventItem*>(eventObj);
        pEvent->m_pItem = pRawEvent;
        pEvent->m_base.m_pItem = reinterpret_cast<ufmt::CRingItem*>(pRawEvent);
        return eventObj;

    } catch (std::exception& e) {                    // Conversion threw.
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}
/**
 * makefragment
 *    Take a CRingItem wrapped object and return a CRingFragmentItem wrapped object.
 * 
 * @param self - pointer to the factory object.
 * @param args - Positional arguments should be one ring item object parameter.
 * @return PyObject* -  pointer to the wrapped wring item fragment.
 * 
 */
static PyObject*
makefragment(PyObject* self, PyObject* args) {
    // Get and verify the ring item parameter:

    PyObject* itemobj;
    if (!PyArg_ParseTuple(args, "O", &itemobj)) {
        return nullptr;
    }
    if (!isRawRingItem(itemobj)) {
        return nullptr;
    }

    // Get a pointer to the CRingItem to convert:

    pyRingItem* pRingItemObj = reinterpret_cast<pyRingItem*>(itemobj);
    ufmt::CRingItem*  pActualItem  = pRingItemObj->m_pItem;

    // Now get a pointer to the actual factory:

    pyRingItemFactory* pThis = reinterpret_cast<pyRingItemFactory*>(self);
    ufmt::RingItemFactoryBase* pFactory = pThis->m_pfactory;

    // The conversion can result in an exception so it's all in a try block.

    try {
        ufmt::CRingFragmentItem* rawitem = 
            pFactory->makeRingFragmentItem(*pActualItem);
        
        // Now wrap in the python object.. on fail we do need to delete the rawitem:

        PyObject* empty = PyTuple_New(0);
        PyObject* rawresult =  PyObject_Call(
            reinterpret_cast<PyObject*>(&pyRingFragmentType), empty, nullptr
        );
        Py_DECREF(empty);
        if (!rawresult) {
            delete rawitem;
            PyErr_SetString(PyExc_RuntimeError, "Failed to wrap CRingFragmentItem in python object");
            return nullptr;
        }
        // Now fill in the object data as only we can do:

        pyRingFragmentItem* pFragment = reinterpret_cast<pyRingFragmentItem*>(rawresult);
        pFragment->m_pItem = rawitem;
        pFragment->m_base.m_pItem = reinterpret_cast<ufmt::CRingItem*>(rawitem);

        return rawresult;

    }
    catch(std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
    
}

/**
 * makePhysicsEventCountItem
 *    Given a ringitem base object return the corresponding a physics event count item.
 * 
 * @param self - pointer to the object that called us.
 * @param args - Positional parameters, - I expect only a ring item base object.
 * @return PyObject* - pointe4r tot he encapsulated CRingPhysicsEventCountItem I created
 * 
 */
static PyObject*
makePhysicsEventCountItem(PyObject* self, PyObject* args) {
    // We expect an object as a parameter it must be a pyRingItem:

    PyObject* itemobj;
    if (!PyArg_ParseTuple(args, "O", &itemobj)) {
        return nullptr;
    }
    if (!isRawRingItem(itemobj)) {
        return nullptr;
    }
    pyRingItem* pRingItemObj = reinterpret_cast<pyRingItem* >(itemobj);
    ufmt::CRingItem*  pActualItem = pRingItemObj->m_pItem;

    // Get the factory pointer.

    pyRingItemFactory* pThis = reinterpret_cast<pyRingItemFactory*>(self);
    ufmt::RingItemFactoryBase* pFactory = pThis->m_pfactory;

    // The factory can throw an exception if .e.g. the ring item is of the 
    // wrong type so:

    try {
        ufmt::CRingPhysicsEventCountItem* rawItem = 
            pFactory->makePhysicsEventCountItem(*pActualItem);

        // Now wrap the item in a phython object:

        PyObject* empty = PyTuple_New(0);    // Emtpy position args for:
        PyObject* rawresult = PyObject_Call(
            reinterpret_cast<PyObject*>(&pyEventCountType), empty, nullptr
        );
        Py_DECREF(empty);
        if(!rawresult) {
            delete rawItem;
            PyErr_SetString(PyExc_RuntimeError, "Failed to wrap a physics event count item in python");
            return nullptr;
        }
        // Now set the object data:

        pyEventCount* result = reinterpret_cast<pyEventCount*>(rawresult);
        result->m_pItem = rawItem;
        result->m_base.m_pItem = reinterpret_cast<ufmt::CRingItem*>(rawItem);

        return rawresult;

    } catch (std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}
/**
 * makeTextItem
 *     Take a raw ring item and entice the factory to create a CRingTextItem which
 * is then encapsulated into a Python object.
 * 
 * @param self - Pointer to the factory object.
 * @param args - Positional arguments.  There must be one - a ringitem object.
 * @return PyObject* - a 'stringlist' object created and wrapped by us.
 */
static PyObject*
makeTextItem(PyObject* self, PyObject* args) {
    // get and validate the argument.. in the end we need the pointer to the
    // encapsulated raw ring item.

    PyObject* itemobj;
    if (!PyArg_ParseTuple(args, "O", &itemobj)) {
        return nullptr;
    }
    if (!isRawRingItem(itemobj)) {
        return nullptr;
    }
    pyRingItem* pRingItemObj = reinterpret_cast<pyRingItem*>(itemobj);
    ufmt::CRingItem*  pRingItem    = pRingItemObj->m_pItem;

    // Get the Ring Item factory object pointer as well

    pyRingItemFactory* pFactoryObj = reinterpret_cast<pyRingItemFactory*>(self);
    ufmt::RingItemFactoryBase* pFactory = pFactoryObj->m_pfactory;

    // The factory method could throw so wrap this in a try/catch
    // and map the exception to a python runtime error 

    try {
        // Try to make the text item
        ufmt::CRingTextItem* rawTextitem = pFactory->makeTextItem(*pRingItem); // Could throw.

        // COnstruct a 'stringlistitem' and set its object data properly:

        PyObject* empty = PyTuple_New(0);      // Constructor takes no argumetns
        PyObject* wrappedTextItem = PyObject_Call(
            reinterpret_cast<PyObject*>(&pyTextListType), empty, nullptr
        );
        Py_DECREF(empty);                     // Free the argument list.
        if (!wrappedTextItem) {
            delete rawTextitem;              // no memory  leak here.
            PyErr_SetString(PyExc_RuntimeError, "Failed to wrapp a CRingTextItem in a python object");
            return nullptr;
        }
        pyTextList* pTextItem = reinterpret_cast<pyTextList*>(wrappedTextItem);
        pTextItem->m_pItem = rawTextitem;
        pTextItem->m_base.m_pItem = reinterpret_cast<ufmt::CRingItem*>(rawTextitem);
        return wrappedTextItem;
    }
    catch (std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}
/**
 * makeStateChangeItem
 *    Create a state change item from a base ring item.
 * 
 * @param self - pointer to the fatory object that called us.
 * @param args - Positional paramters. should have one ringitem object.
 * @return PyObject* Pointer to a statechangeitem object.
 */
static PyObject*
makeStateChangeItem(PyObject* self, PyObject* args) {
    // Pull out the CRingItem that was passed :

    PyObject* obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) {
        return nullptr;
    }
    if (!isRawRingItem(obj)) {
        return nullptr;
    }
    pyRingItem* pRingItemObj = reinterpret_cast<pyRingItem*>(obj);
    ufmt::CRingItem*  pRingItem = pRingItemObj->m_pItem;

    // Get the factory:

    pyRingItemFactory* pFactoryObj = reinterpret_cast<pyRingItemFactory*>(self);
    ufmt::RingItemFactoryBase* pFactory = pFactoryObj->m_pfactory;

    // wrap the creation in a try/catch that converts exceptions thrown by the
    // factory to runtime errors:
    
    try {
        ufmt::CRingStateChangeItem* pRawItem = pFactory->makeStateChangeItem(*pRingItem);

        // Make a python statechange item and wrap our item in it:

        PyObject* empty = PyTuple_New(0);
        PyObject* wrapper = PyObject_Call(
            reinterpret_cast<PyObject*>(&pyStateChangeType), empty, nullptr
        );
        Py_DECREF(empty);

        if(!wrapper) {
            delete pRawItem;
            PyErr_SetString(PyExc_RuntimeError, "Failed to create a python state change item type");
            return nullptr;
        }

        pyStateChange* pStateChange = reinterpret_cast<pyStateChange*>(wrapper);
        pStateChange->m_pItem = pRawItem;
        pStateChange->m_base.m_pItem = reinterpret_cast<ufmt::CRingItem*>(pRawItem);

        return wrapper;
    }
    catch (std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }



}

/**
 * makeDataFormatItem
 *    GIven a base ring item, creates, if possible, a format version item from it.
 * 
 * @param self - Pointer to the object that called us (factory).
 * @param args - Pointer to the positional paramters, in this case, a single ring item object.
 */
static PyObject*
makeDataFormatItem(PyObject* self, PyObject* args) {
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) {
        return nullptr;
    }
    if (!isRawRingItem(obj)) {
        return nullptr;
    }

    // Get the CRingItem pointer:

    pyRingItem* pRingItemObject = reinterpret_cast<pyRingItem*>(obj);
    ufmt::CRingItem* pRingItem  = pRingItemObject->m_pItem;

    // Get the factory:

    pyRingItemFactory* pFactoryObj = reinterpret_cast<pyRingItemFactory*>(self);
    ufmt::RingItemFactoryBase* factory = pFactoryObj->m_pfactory;


    //  The factory conversion can throw - this converts any such throw
    // to raising a RuntimeError:

    try {
        ufmt::CDataFormatItem* pRawItem = factory->makeDataFormatItem(*pRingItem);

        PyObject* empty = PyTuple_New(0);
        PyObject* wrappedItem = PyObject_Call(
            (PyObject*)(&pyFormatVersionType), empty, nullptr
        );
        Py_DECREF(empty);
        if (!wrappedItem) {
            delete pRawItem;
            PyErr_SetString(PyExc_RuntimeError, "Failed to create a ringformatitem");
            return nullptr;
        }

        // Set the object data and return the wrapped object.

        pyFormatVersion* pWrapper = reinterpret_cast<pyFormatVersion*>(wrappedItem);
        pWrapper->m_pItem = pRawItem;
        pWrapper->m_base.m_pItem = reinterpret_cast<ufmt::CRingItem*>(pRawItem);

        return wrappedItem;
    }
    catch (std::exception& e) {
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
    {"makeGlomParameters", makeGlomParameters, METH_VARARGS, "Convert ring item into a glom parameters item"},
    {"makePhysicsEventItem", makephysicsevent, METH_VARARGS, "Convert ring item into a physics event"},
    {"makeRingFragmentItem", makefragment, METH_VARARGS, "Convert ring item to a ring fragment item"},
    {"makePhysicsEventCountItem", makePhysicsEventCountItem, METH_VARARGS, "Convert ring item into an event count item"},
    {"makeTextItem", makeTextItem, METH_VARARGS, "Convert ring item into a stringlist item"},
    {"makeStateChangeItem", makeStateChangeItem, METH_VARARGS, "Convert a ring item into a state chane."},
    {"makeDataFormatItem", makeDataFormatItem, METH_VARARGS, "Convert a ring item into a data format item"},
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


