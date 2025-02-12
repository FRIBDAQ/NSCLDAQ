/**
 * @file CSIS3316EventSegment.h
 * @brief Defines the event segment that will readout a single SIS3316.
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
#ifndef CSIS3316EVENTSEGMENT_H
#define  CSIS3316EVENTSEGMENT_H
#include <CEventSegment.h>  // From SBSReadout.
#include <string>

// Forward definitions:

namespace XXUSB {
    class CConfigurableObject;
}
class sis_vmusb_interface;
class sis3316_adc;

/**
 * @class CSIS3316EventSegment
 *    The event segment for reading out a single SIS3316.
 * To be properly configured, this _must_ be part of a 
 * CSIS3316CompoundEventSegment, as that's what does the
 * configuration.
 */
class CSIS3316EventSegment : public  CEventSegment {
private
    std::string                 m_name;              // Name as per configuration.
    XXUSB::CConfigurableObject* m_pConfiguration;    // Module configuration.
    sis_vmusb_interface*        m_pVMe;              // VME interface object for SIS support.
    sis3316_adc*                m_pModule;           // SIS sis3316 support class.

    // Public canonicals:
public:
    CSIS3316EventSegment(const char* name);
    virtual ~CSIS3316EventSegment();                 // though never invoked I think.

    // Forbidden canonicals:
private:
    CSIS3316EventSegment(const CSIS3316EventSegment& rhs);
    CSIS3316EventSegment& operator=(const CSIS3316EventSegment& rhs);

    int operator==(const CSIS3316EventSegment& rhs) const;
    int operator!=(const CSIS3316EventSegment& rhs) const;

    // Selectors:

    XXUSB::CConfigurableObject* getConfiguration();
    std::string                 getName() const;

    // virtual entry points:

    virtual void initialize();
    virtual void disable();
    virtual size_t read(void* pBuffer, size_t maxwords);
    
    // Utilitye methods:

private:
    void setupConfiguration();
    size_t computeTotalWords();
    size_t sizeGroup(int group);
};


#endif