/**
 * @file CSIS3316Trigger.h
 * @brief Defines the trigger based on the readability of an SIS3316 module.
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
#ifndef CSIS3316TRIGGER_H
#define  CSIS3316TRIGGER_H

#include <CTrigger.h>
class CConfigurableCompoundEventSegment;


/** 
 * @class CSIS3316Trigger
 *   COntains a configurable compound event segment and check its any3316Readable method.
 */
class CSIS3316Trigger : public CTrigger {
private:
    CConfigurableCompoundEventSegment* m_pSegment;
public
    CSIS3316Trigger(CConfigurableCompoundEventSegment* pSeg);

    virtual bool operator();
}


#endif