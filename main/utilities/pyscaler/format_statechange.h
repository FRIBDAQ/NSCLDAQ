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
 * @file format_statechange.h
 * @brief Header for encapsulation of CRingStateChangeItem in pytyhon.
 * @author Ron Fox
 */
#ifndef FORMAT_STATECHANGE_H
#define FORMAT_STATECHANGE_H

#include "format_ringitem.h"
#include <Python.h>

// forward definition of ufmt::CRingStateChangeItem

namespace ufmt {
    class CRingStateChangeItem;
}

// Shape of instance storage:

typedef struct {
    pyRingItem m_base;
    ufmt::CRingStateChangeItem * m_pItem;
} pyStateChange;

// Non implementation file need to get to the type table:

#ifndef STATECHANGE_IMPLEMENTATION
extern PyTypeObject pyStateChangeType;
#endif


#endif