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
 * @file format_selector.h
 * @brief definitions for C clients of the format selection methods.
 * @author Ron Fox
 * @note  This file defines the externals and types needed to 
 *        have a RingItemFactory.
 */
#ifndef FORMAT_SELECTOR_H
#define FORMAT_SELECTOR_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <RingItemFactoryBase.h>

// Type specific data for a ring item factory:
typedef struct {
    PyObject_HEAD
    ufmt::RingItemFactoryBase* m_pfactory;
} pyRingItemFactory;

// This is needed so the module can define this type:

#ifndef SELECTDOR_IMPLEMENTATION
extern PyTypeObject pyRingItemFactoryType;
#endif

#endif