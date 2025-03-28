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
@brief Implementation for class that accumulates lists of VME operations.
*/

#include "CVMUSBReadoutList.h"
#include <sstream>                   // Used to format operations.

// Static members of the class:

const uint8_t CVMUSBReadoutList::a32UserData = 0x09;
const uint8_t CVMUSBReadoutList::a32UserProgram = 0xa;
const uint8_t CVMUSBReadoutList::a32UserBlock = 0x0b;

const uint8_t CVMUSBReadoutList::a24PrivData(0x3d);
const uint8_t CVMUSBReadoutList::a24PrivProgram = 0x3e;
const uint8_t CVMUSBReadoutList::a24PrivBlock = 0x3f;

const uint8_t CVMUSBReadoutList::a16Priv(0x2d);
const uint8_t CVMUSBReadoutList::a16User = 0x29;

const uint8_t CVMUSBReadoutList::a32PrivData = 0x0d;
const uint8_t CVMUSBReadoutList::a32PrivProgram = 0x0e;
const uint8_t CVMUSBReadoutList::a32PrivBlock = 0x0f;

const uint8_t CVMUSBReadoutList::a24UserData = 0x39;
const uint8_t CVMUSBReadoutList::a24UserProgram = 0x3a;
const uint8_t CVMUSBReadoutList::a24UserBlock = 0x3b;


///////////////////////////////////// Canonicals ///////////////////////////////

/**
 *  constructor
 */
CVMUSBReadoutList::CVMUSBReadoutList() {}
CVMUSBReadoutList::~CVMUSBReadoutList() {}

/////////////////////////////////// operation ////////////////////////////////////////

/**
 * addMarker
 *   Add a marker to the  data stream. Note that for the mvlc, markers are 32 bits wide;
 * while the VMUSB generates 16 bit markers.
 * @param value  - The value to write to the event stream.
 */
void 
CVMUSBReadoutList::addMarker(uint32_t value) {
    std::stringstream stack_line;
    stack_line << "write_marker 0x" <<  std::hex << value;
    std::string mvlc_op = stack_line.str();
    m_list.push_back(mvlc_op);
}




/**
 *  dumpForMvlc
 *    Dump the stack.
 * 
 * @return std::vector<std::string> - the stack as textual commands.
 */
std::vector<std::string>
CVMUSBReadoutList::dumpForMvlc() {
    return m_list;
}