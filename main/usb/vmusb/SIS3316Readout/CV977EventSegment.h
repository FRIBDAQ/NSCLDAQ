/**
 * @file3 CV977EventSegment.h
 * @brief Header for an event segment to read a V977 pattern register.
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
*
 * This is part of the TCL configurable support for D. Bazin's SIS3316 
 * readout.   The event segment is configurable much like it is
 * for the VMUSBReadout with a few fewer parameters.
 * 
 * Specifically we don't support interrupts.
 */
#ifndef CV977EVENTSEGMENT_H
#define CV977EVENTSEGMENT_H
#include "CEventSegment.h"
#include <string>
namespace XXUSB {
    class CConfigurableObject;
}



/**
 *  @class CV977EventSegemnt
 *     Class definition. Configuration parameters  are:
 * 
 * *  -base - base address of the module.
 * *  -inputmask - the input mask for the module.
 * *  -outputmask - the output mask for the module.
 * *  -readmode - singlehit or multihit.
 * *  -readandcler - bool - true if the register is cleared on read.
 * *  -pattern - bool - set or don't set the pattern bit in the control register.
 * *  -gate  - bool - set or dont' set the gate bit in the control register.
 * *  -ormask - bool do or don't set the ormask in the control register.
 */
class CV977EventSegment : public CEventSegment {
private:
    std::string                 m_name;
    XXUSB::CConfigurableObject* m_pConfiguration;

        // public canonicals:
public:
    CV977EventSegment(const char* name);
    virtual ~CV977EventSegment();

    // forbidden canonicals:

private:
    CV977EventSegment(const CV977EventSegment& rhs); // can't copy construct.
    CV977EventSegment& operator=(const CV977EventSegment& rhs);  // Can't assign.

    int operator==(const CV977EventSegment& rhs) const;   // Can't compare.
    int operator!=(const CV977EventSegment& rhs) const;   //  ""

    // selectors:

    XXUSB::CConfigurableObject* getConfiguration();
    std::string getName() const;

    // Virtual entry points.

    virtual void initialize();
    virtual size_t read(void* pBuffer, size_t maxwords);
    
    // Utilitye methods:

private:
    void setupConfiguration();
    
};

#endif