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
 *  @file format_abnormalend.h
 *  @brief Header to define the extended object of a python abnormalend ring item type
 *  @author ron fox.
 */

 #ifndef FORMAT_ABNORMALEND_H
 #define FORMAT_ABNORMALEND_h

#include <Python.h>
#include <format_ringitem.h>
 namespace ufmt {
    class CAbnormalEndItem;              // Forward definition.
 }

// The ring abnorma, end specification - we
// inherit from CRingItem... note.
//
 typedef struct {
    pyRingItem              m_base;
    ufmt::CAbnormalEndItem* m_pItem;
 } pyAbnormalEndItem;

 #ifndef ABNORMALEND_IMPLEMENTATION
 extern PyTypeObject pyAbnormalEndItemType;
 #endif

 #endif
