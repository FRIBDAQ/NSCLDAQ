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
 * @file format_event.h
 * @brief header for Phython encapsulation of CEvent.
 * @author Ron Fox.
 */
#ifndef FORMAT_EVENT_H
#define FORMAT_EVENT_H

#include "format_ringitem.h"
#include <Python.h>
// forward reference for ufmt::CPhysicsEventItem.

namespace ufmt {
    class CPhysicsEventItem;
}


// Object storage is such that we inherit from CRingItem:

typedef struct {
    pyRingItem m_base;
    ufmt::CPhysicsEventItem* m_pItem;
} pyEventItem;

#ifndef EVENT_IMPLEMENTATION
extern PyTypeObject pyEventType;
#endif

#endif