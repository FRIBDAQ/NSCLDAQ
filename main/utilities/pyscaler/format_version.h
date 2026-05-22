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
 * @file format_version.h
 * @brief Header for the version item Python wrapping.
 * @author Ron Fox.
 */
#ifndef FORMAT_VERSION_H
#define FORMAT_VERSION_H
#include "format_ringitem.h"
#include <Python.h>

// forward definition of ufmt::CRingFormatItem

namespace ufmt {
    class CDataFormatItem;
}

typedef struct {
    pyRingItem m_base;
    ufmt::CDataFormatItem* m_pItem;
} pyFormatVersion;


#ifndef FORMAT_IMPLEMENTATION
extern PyTypeObject pyFormatVersionType;
#endif

#endif