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
 * @file format_ringfragment.h
 * @brief header for Python encapsulated CRingFragmentItem
 * @author Ron Fox
 */
#ifndef FORMAT_RINGFRAGMENT_H
#define FORMAT_RINGFRAGMENT_H
#include "format_ringitem.h"     // base class.
#include <Python.h>


// forward definition of the class:

namespace ufmt {
    class CRingFragmentItem;
}

// Object storage.  We need the base class
// and we need our CringFragmentItem pointer for convenience.

typedef struct {
    pyRingItem m_base;
    ufmt::CRingFragmentItem* m_pItem;
} pyRingFragmentItem;

// the type is extern for everyone but the implementation:

#ifndef RINGFRAGMENT_IMPLEMENTATION
extern PyTypeObject pyRingFragmentType;
#endif

#endif
