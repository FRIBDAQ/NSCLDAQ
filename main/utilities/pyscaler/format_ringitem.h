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
 *  @file format_ringitem.h
 *  @brief Header to provide the object structure for  the python ringitem type.
 *  @author Ron Fox
 */

#ifndef FORMAT_RINGITEM_H
#define FORMAT_RINGITEM_H

#include <CRingItem.h>
#include <Python.h>



// the object struct 

typedef struct {
    PyObject_HEAD
    ufmt::CRingItem* m_pItem;
} pyRingItem;

#ifndef RINGITEM_IMPLEMENTATION
extern PyTypeObject pyRingItemType;
#endif

#endif
