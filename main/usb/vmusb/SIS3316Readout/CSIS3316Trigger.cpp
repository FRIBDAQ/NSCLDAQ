/**
 * @file CSIS3316Trigger.cpp
 * @brief implements the trigger based on the readability of an SIS3316 module.
 * @author Ron Fox <fox at frib dot msu dot edu>
 * 
 * 
 *  This software is Copyright by the Board of Trustees of Michigan
 *  State University (c) Copyright 2025
*
*  You may use this software under the terms of the GNU public license
*   (GPL).  The terms of this license are described at:
*
*    http://www.gnu.org/licenses/gpl.txt
*
*    Author:
*            Ron Fox
*            Facility for Rare Isotop Beams
*            Michigan State University
*            East Lansing, MI 48824-1321
*
 * 
 */
#include "CSIS3316Trigger.h"
#include "CConfigurableCompoundEventSegment.h"

/**
 *  constructor
 *     @param pSeg - the segment we'll poll
 */
CSIS3316Trigger::CSIS3316Trigger(CConfigurableCompoundEventSegment* pSeg) :
m_pSegment(pSeg) {}


/**
 *  operator()
 *     Poll the trigger once.
 * 
 * @return bool -true if triggered.
 */
bool
CSIS3316Trigger::operator()() {
    return m_pSegment->any3316Readable();
}