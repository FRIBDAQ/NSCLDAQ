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
 * @file format_textlist.h
 * @brief Header for ring item types that contain lists of strings.
 * @author Ron Fox
 */
#ifndef FORMAT_TEXTLIST_H
#define FORMAT_TEXTLIST_H

#include "format_ringitem.h"    // Base class
#include <Python.h> 

// Forward reference to ufmt::CRingTextItem:

namespace ufmt {
    class CRingTextItem;
}

// define the shape of the object instance data:

typedef struct {
    pyRingItem m_base;    // base class storage.
    ufmt::CRingTextItem* m_pItem;
} pyTextList;


// If we are not the implementation, we need to get to the type table:

#ifndef TEXTLIST_IMPLEMENTATION
extern PyTypeObject pyTextListType;
#endif


#endif