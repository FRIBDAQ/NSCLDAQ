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
 * @file format_scaler.h
 * @brief Header for wrapping CRingScalerItem objects for python.
 * @author Ron Fox
 */
#ifndef FORMAT_SCALER_H
#define FORMAT_SCALER_H


#include <Python.h>
#include <format_ringitem.h>

namespace ufmt {
    class CRingScalerItem;
}

// The CRingScalerItem object storage.
// Note that we are derived from the CRingItem class

typedef struct {
    pyRingItem m_base;
    ufmt::CRingScalerItem* m_pItem;
} pyRingScalerItem;

#ifndef SCLAER_IMPLEMENTATION
extern PyTypeObject pyRingScalerItemType;
#endif

#endif