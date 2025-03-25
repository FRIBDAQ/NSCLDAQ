/**
 * @file CVMUSBBusy.h
 * @brief Header for a busy class using the VMUSB.
 * 
  * @author Ron Fox <fox at frib dot msu dot edu>
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
 */
#ifndef CVMUSBBUSY_H
#define CVMUSBBUSY_H
#include <CBusy.h>

/**
 * @class CVMUSBBusy
 * 
 * This is a class that can provide end of event information in the SBS readout
 * skeleton used with the VMUSB in interactive mode.
 * 
 * We don't provide an actual busy but, instead, provide a pulse on O2 when the system can go 'clear'.
 * this is done by writing bit 1 of the action register (USB Trigger) which, by default sends a pulse out O2.
 * The CVMUSBEventSegment leaves the selector for O2 as zero which is what we need.
 */
class CVMUSBBusy : public CBusy {
public:
    CVMUSBBusy() {}
    virtual ~CVMUSBBusy() {}
    // No data so the default copy constructor, assignment, etc. are just fine.
public:
    virtual void GoBusy();
    virtual void GoClear();
};


#endif