/**
 * @file CPyHelper.h
 * @brief Helper class for using the Python C++ API.
 */

#ifndef PYHELPER_HPP
#define PYHELPER_HPP

#include <Python.h>

#include <iostream>

/**
 * @class CPyHelper
 * @brief Helper class for using the Python C++ API.
 */

class CPyHelper
{
public:
    /**
     * @brief Constructor
     *
     * Initializes the Python interpreter.
     */
    CPyHelper()
	{
	    Py_Initialize();
	}
    /**
     * @brief Destructor.
     *
     * Undo initializations, destroy all (sub-)interpreters.
     */
    ~CPyHelper()
	{
	    Py_Finalize();
	}
};

/**
 * @class CPyObject
 * @brief Basic handling of Python object types.
 */

class CPyObject
{
private:
    PyObject *p; //!< The Python object managed by CPyObject.
public:
    /**
     * @brief Default constructor.
     */
    CPyObject() : p(NULL) {}
    /**
     * @brief Construct with an existing object.
     */
    CPyObject(PyObject* _p) : p(_p) {}
    /** 
     * @brief Destructor.
     * Release memory allocated to the object.
     */
    ~CPyObject()
	{
	    Release();
	}
    
    /**
     * @brief Get the object.
     * @return p  Pointer to the PyObject.
     */
    PyObject* getObject()
	{
	    return p;
	}    
    /**
     * @brief Set the object.
     * @param _p  Pointer to the PyObject to set.
     */
    PyObject* setObject(PyObject* _p)
	{
	    return (p = _p);
	}
    /**
     * @brief Increment reference count for the object.
     */
    PyObject* AddRef()
	{
	    if(p) {
		Py_INCREF(p);
	    }
	    
	    return p;
	}
    /**
     * @brief Decrement reference count for the object.
     */
    void Release()
	{
	    if(p) {
		Py_DECREF(p);
	    }
	    p = NULL;
	}
    /**
     * @brief Arrow operator.
     * @return p  Pointer to the wrapped PyObject.
     */
    PyObject* operator ->()
	{
	    return p;
	}
    /**
     * @brief Arrow operator.
     * @return bool.
     * @retval true   If the member object exists.
     * @retval false  If the member object does not exist.
     */
    bool is()
	{
	    return p ? true : false;
	}
    /**
     * @brief Conversion operator.
     * @return p  Pointer to the wrapped PyObject.
     */
    operator PyObject*()
	{
	    return p;
	}
    /**
     * @brief Conversion operator.
     * @return p  Pointer to the wrapped PyObject.
     */
    PyObject* operator = (PyObject* pp)
	{
	    p = pp;
	    return p;
	}
    /**
     * @brief Boolean operator
     * @return bool
     * @retval true   If the member object exists.
     * @retval false  If the member object does not exist.
     */
    operator bool()
	{
	    return p ? true : false;
	}
};

#endif
