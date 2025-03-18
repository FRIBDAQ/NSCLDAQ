/**
 *  @file CVMUSBEventSegment.h
 * @brief Header for event segment to read VMUSB scalers as an event segment.
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
* 
 */
#ifndef CVMUSBEVENTSEGMENT_H
#define CVMUSBEVENTSEGMENT_H
#include <CEventSegment.h>
#include <string>


namespace XXUSB {
    class CConfigurableObject;
}
/**
 * @class CVMUSBEventSegment
 * 
 * The configuration will provide the following initially:alignas
 * 
 * * -incremental (bool) - if true clear scalers after read - this is lossy
 * * -scalera (enum) - Determine what increments scalera:  
 *      dgga, nimi1, nimi2.  Note at present we don't define DGG programming but later may
 * * -scalerb (enum) - Determinet what incdremetns scalerb:
 *      carry,, nimi1, nimi2
 */
class CVMUSBEventSegment : public CEventSegment {
private:
    std::string m_name;                         // Name of the object.
    XXUSB::CConfigurableObject* m_pConfiguration; 

    // Usable canonicals:
public:
    CVMUSBEventSegment(const char* pName);
    ~CVMUSBEventSegment();

    // forbidden canonicals:
private:
    CVMUSBEventSegment(const CVMUSBEventSegment& rhs);
    CVMUSBEventSegment& operator=(const CVMUSBEventSegment& rhs);
    int operator==(CVMUSBEventSegment& rhs) const;
    int operator!=(CVMUSBEventSegment& rhs) const;

    // Selectors:
public:
    XXUSB::CConfigurableObject* getConfiguration();
    std::string getName() const;

    // Overrides to virtuals:

public:
    virtual void initialize();
    virtual size_t read(void * pBuffer, size_t maxwords);

    // Utilities

private:
    void defineConfiguration();

};



#endif