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
 * @file format_eventcount.h
 * @brief Header for the encapsulation of the Physics event count ring item.
 * @author Ron Fox
 */
#ifndef FORMAT_EVENTCOUNT_H
#define FORMAT_EVENTCOUNT_H

#include "format_ringitem.h"         // Base class.
#include <Python.h>

// Formward definition of CRingPhysicsEventCountItem:

namespace ufmt {
    class CRingPhysicsEventCountItem;
}

// intance data stucture:

typedef struct {
    pyRingItem m_base;
    ufmt::CRingPhysicsEventCountItem* m_pItem;
} pyEventCount;

// extern for the type table -- can be turned off for implementation

#ifndef EVENTCOUNT_IMPLEMENTATION
extern PyTypeObject pyEventCountType;
#endif


#endif                                  // FORMAT_EVENTCOUNT_H