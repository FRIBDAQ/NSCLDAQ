/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief Header for class that accumulates lists of VME operations.
*/
#ifndef MVLC_CVMUSBREADOUTLIST_H
#define MVLC_CVMUSBREADOUTLIST_H
#include <string>
#include <vector>
#include <stdint.h>

/**
 *  @class CVMUSBReadoutList
 *     This class is used to accumulate lists of VME operations that can be
 * put into segments of MVLC configuration files.  It supplies all of the
 * operations of the VMUSB CVMUSBReadoutList to ease porting of device support software
 * to the MVLC and stores the operations as the stringified stuff expected by the yaml
 * for mvme configurations.
 * 
 * If there are operations that are not supportable, they are not implemented so that
 * calling them results in a failure at compile time.alignas
 * 
 */
class CVMUSBReadoutList {
private:
    std::vector<std::string> m_list;     // Saved operations.
public:
    // Allowed canonicals
    CVMUSBReadoutList();
    virtual ~CVMUSBReadoutList();     // just in case we derive.

private:                              // more because I'm lazy than they can't be done
    CVMUSBReadoutList(const CVMUSBReadoutList& rhs);   // copy constructor.
    CVMUSBReadoutList& operator=(const CVMUSBReadoutList& rhs); // assignment.
    int operator==(const CVMUSBReadoutList& rhs);
    int operator!=(const CVMUSBReadoutList& rhs);

    // VME operations:
public:
   
    virtual void addWrite32(uint32_t address, uint8_t amod, uint32_t datum);
    virtual void addWrite16(uint32_t address, uint8_t amod, uint16_t datum);

    virtual void addRead32(uint32_t address, uint8_t amod);
    virtual void addRead16(uint32_t address, uint8_t amod);

    virtual void addBlockRead32(uint32_t baseAddress, uint8_t amod, size_t transfers);
    virtual void addFifoRead32(uint32_t  baseAddress, uint8_t amod, size_t transfers);

    virtual void addBlockCountRead16(uint32_t address, uint32_t mask, uint8_t amod);
    virtual void addBlockCountRead32(uint32_t address, uint32_t mask, uint8_t amod);

    virtual void addMaskedCountBlockRead32(uint32_t address, uint8_t amod);
    virtual void addMaskedCountFifoRead32(uint32_t address, uint8_t amod);

    virtual void addDelay(uint32_t clocks);
    virtual void addMarker(uint32_t ms);

    // Loop on the specified 16/32 bit read until the value read masked by mask is equal to value.

    virtual void addLoopUntil32(uint32_t address, uint8_t amod, uint32_t mask, uint32_t value);
    virtual void addLoopUntil16(uint32_t addresss, uint8_t amod, uint32_t mask, uint32_t value);

    std::vector<std::string> dumpForMvlc();      // Return the operations list.
    void clear();                               // clear the list.
    void appendToStack(CVMUSBReadoutList& other); // Add out list to another list.

public:
    // Convenience definitions for amods:
    // @todo - expand for the modern amods VMUSB did not support.

    static const uint8_t a32UserData  ;
    static const uint8_t a32UserProgram  ;
    static const uint8_t a32UserBlock ;

    static const uint8_t a32PrivData ;
    static const uint8_t a32PrivProgram;
    static const uint8_t a32PrivBlock;

    static const uint8_t a16User ;
    static const uint8_t a16Priv;

    static const uint8_t a24UserData;
    static const uint8_t a24UserProgram;
    static const uint8_t a24UserBlock;
    
    static const uint8_t a24PrivData;
    static const uint8_t a24PrivProgram ;
    static const uint8_t a24PrivBlock ;
private:
    void addWrite(uint32_t address, uint8_t amod, const char* width, uint32_t data);
    void addRead(uint32_t address, uint8_t amod, const char* width);
    void readToAccumulator(uint32_t address, uint8_t amod, const char* size); 
    void maskAndShift(uint32_t mask);
    void loopUntil(uint32_t address, uint8_t amod, uint32_t mask, uint32_t value, const char* width);
};

#endif