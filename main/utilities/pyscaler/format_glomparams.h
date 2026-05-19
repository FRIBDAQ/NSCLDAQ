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
 * @file format_glomparams.h
 * @brief Header for the Python encapsulation of CGlomParameters
 * @author Ron Fox
 */
#ifndef FORMAT_GLOMPARAMETERS_H
#define FORMAT_GLOMPARAMETERS_H
#include <format_ringitem.h>
#include <Python.h>

// Forward reference the encapsulated class here.
namespace ufmt {
    class CGlomParameters;
}
// instance data - we are derived from CRingItem so:

typedef struct {
    pyRingItem m_base;     // Base class.
    ufmt::CGlomParameters* m_pItem;
} pyGlomParametersItem;

#ifndef GLOMPARAMS_IMPLEMENTATION
extern PyTypeObject pyGlomParametersType;
#endif

#endif