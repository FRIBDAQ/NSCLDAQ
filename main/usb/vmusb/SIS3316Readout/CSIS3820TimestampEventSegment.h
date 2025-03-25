/**
 * @file CSIS3820TimestampEventSegment.h
 * @brief Header for using the CSIS3820 scaler as a timstamper.
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
*
 */
#ifndef     CSIS3820TIMESTAMPEVENTSEGMENT_H
#define     CSIS3820TIMESTAMPEVENTSEGMENT_H
#include <CEventSegment.h>
#include <string>
namespace XXUSB {
    class CConfigurableObject;
}


/**
 *  @class CSIS3820TimestampEventSegment
 *  
 * The purpose of this event segment is to read a timestamp from the SIS3820 scaler.
 * This scaler has the ability to make two channels, channel 0 and channel 16 
 * 48 channel counters that are suitable for timestamping.  Normally this is done
 * by putting a  pulses to be counted into one or both of channel 0 and 16 and
 * then the trigger signal fanned into input 1 which latches the scalers for readout.
 * 
 * The set of configuration options are, therefore minimal.  Note that LNE means the Load Next Event
 * signal (input 1)
 * 
 * *  -base - base address of the module.
 * *  -outputmode - defines what comes from the scaler outputs one of:
 *     * clock50MHz output 5 echoes the latch request and output 7 is a 50MHz clock.
 *     * LNEndLed -Output 5 is the LNE and 8 the user output (not used).
 *     * Clock2x10MHz - Output 5 is LNE, outputs 6, 7 are 10MHz clocks.
 *     * Clock1x10MHz - Output 5 echose the LNE and output 6 only is a 10MHz clock.
 */
class CSIS3820TimestampEventSegment : public CEventSegment {
private:
    std::string                 m_name;
    XXUSB::CConfigurableObject* m_pConfiguration;

    // Allowed canonicals:
public:
    CSIS3820TimestampEventSegment(const char* name);
    virtual ~CSIS3820TimestampEventSegment();

    // forbidden canonicals:
private:
    CSIS3820TimestampEventSegment(const CSIS3820TimestampEventSegment& rhs);
    CSIS3820TimestampEventSegment& operator=(const CSIS3820TimestampEventSegment& rhs);
    int operator==(const CSIS3820TimestampEventSegment& rhs) const;
    int operator!=(const CSIS3820TimestampEventSegment& rhs) const;
    // selectors:
public:
    XXUSB::CConfigurableObject* getConfiguration();
    std::string getName() const;

    // Implement virtual entries we care about.

    virtual void initialize();
    virtual size_t read(void* pBuffer, size_t maxwords);
    
    // Utilitye methods:

private:
    void setupConfiguration();
    
};




#endif
