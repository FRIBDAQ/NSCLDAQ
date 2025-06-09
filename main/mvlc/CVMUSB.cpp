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
@brief Implementation for VMUSB Stand-in class.
*/

#include "CVMUSB.h"


/** 
 * destructor -- only provided to support derivation.
 */
CVMUSB::~CVMUSB() {}


/** 
 * vmeWrite32
 *     Store up a 32 bit write operation.
 * 
 * @param address - where to write.
 * @param aModfier -address modifier to use.
 * @param data    - What to write.
 * @return int 0 - these are always successful if anybody checks.
 */
int
CVMUSB::vmeWrite32(uint32_t address, uint8_t aModifier, uint32_t data) {
    m_operationList.addWrite32(address, aModifier, data);
    return 0;
}

/**
 *  vmeWrite16
 * 
 *    Same as above but stores a 16 bit write.
 */
int
CVMUSB::vmeWrite16(uint32_t address, uint8_t aModifier, uint16_t data) {
    m_operationList.addWrite16(address, aModifier, data);
    return 0;
}
/** delay
 *    Add a delay to the stack:
 *     @paramm ms - the number of ms to delay. 
*/
void
CVMUSB::delay(uint32_t ms) {
    m_operationList.addDelay(ms);
}

/**
 * loopUntil32
 *    See CVMUSBReadoutList::addLoopUntil32
 */
void
CVMUSB::loopUntil32(uint32_t address, uint8_t amod, uint32_t mask, uint32_t value) {
    m_operationList.addLoopUntil32(address, amod, mask, value);
}
/**
 *  loopUntil16 
 *     see CVMUSBReadoutList::addLoopUntil16.
 */
void
CVMUSB::loopUntil16(uint32_t address, uint8_t amod, uint32_t mask, uint32_t value) {
    m_operationList.addLoopUntil32(address, amod, mask, value);
}
/**
 * getRecordedOperations
 *  Gets the operations we've done since the last clear in a manner suitable for
 *  inclusion in a .yaml config file for mvlc readouts. 
 * 
 * @return std::vector<string> - each element is  an MVLC .yaml stack element.
 */
std::vector<std::string>
CVMUSB::getRecordedOperations() {
    return m_operationList.dumpForMvlc();
}
/**
 *  clearRecordedOperations
 *     Clears the operations we've got recorded in our recording list.
 */
void
CVMUSB::clearRecordedOperations() {
    m_operationList.clear();
}

/**
 *  executeList
 *    Rather than executing the list, as the VMUSB implementation did, we just
 * append the operations in the input list to our 'stack'.   
 * 
 * @param list - the list of operations to append.
 * @param pReadBuffer - for compatibility with existing softwrae this won't be written into.
 * @param readBufferSize - ignored size of pRadBuffer in bytes.
 * @param bytesRead - written with zero.
 * @return int - 0 to indicate success to anyone that cares.
 */
int
CVMUSB::executeList(CVMUSBReadoutList& list, void* pReadBuffer, size_t readBufferSize, size_t* bytesRead) {
    list.appendToStack(m_operationList);
    *bytesRead = 0;
    return 0;
}