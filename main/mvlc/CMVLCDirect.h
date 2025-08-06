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
@brief Header for VMUSB Stand-in class that does immediate MVLC operations.
*/
#ifndef MVLC_MVLCDIRECT_H
#define MVLC_MVLCDIRECT_H
#include "CVMUSB.h"
#include <sstream>

namespace mesytec {
    namespace mvlc {
        class MVLC;
        class StackCommandBuilder;
    }
}

/**
 * @class CMVLCDirect
 *     This class is a wrapper that derives from CVMUSB and provides immediate MVLC operations.
 * This allows us to use the XLMLoader directly with an MVLC connection.  The class encapsulates
 * a reference to an MVLC that is connected to a controller.  We provide the interfaces of
 * CVMUBS and delegate to appropriate MVLC operations.
 */
class CMVLCDirect : public CVMUSB {
private: 
    mesytec::mvlc::MVLC& m_controller;
    

public:
    CMVLCDirect(mesytec::mvlc::MVLC& controller);
    virtual ~CMVLCDirect();

    // Note that default compiler generated canonicals should actualy be fine.

    // Operations:

    virtual int vmeWrite32(uint32_t address, uint8_t aModifier, uint32_t data);
    virtual int vmeWrite16(uint32_t address, uint8_t aModifier, uint16_t data);
    virtual void delay(uint32_t ms);                      // add the ability to delay in initializing.
    virtual void loopUntil32(uint32_t address, uint8_t amod, uint32_t mask, uint32_t value);
    virtual void loopUntil16(uint32_t address, uint8_t amod, uint32_t mask, uint32_t value);
    virtual int executeList(CVMUSBReadoutList& list, void* pReadBuffer, size_t readBufferSize, size_t* bytesRead);

private:
    // These methods are used in interpreting list items;

    int interp_write(std::stringstream& operation, mesytec::mvlc::StackCommandBuilder& builder);
    int interp_wait(std::stringstream& operation, mesytec::mvlc::StackCommandBuilder& builder);
    int interp_mask_shift_accu(std::stringstream& operation, mesytec::mvlc::StackCommandBuilder& builder);
    int interp_read_to_accu(std::stringstream& operation, mesytec::mvlc::StackCommandBuilder& builder);
    int interp_compare_loop_accu(std::stringstream& operation, mesytec::mvlc::StackCommandBuilder& builder);


};


#endif