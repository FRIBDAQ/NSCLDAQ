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
@brief Implementation of VMUSB Stand-in class that does immediate MVLC operations.
*/
#include "CMVLCDirect.h"
#include "CVMUSBReadoutList.h"
#include <mesytec-mvlc/mesytec-mvlc.h>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>

#include <vector>


using  mesytec::mvlc::MVLC;
using  mesytec::mvlc::VMEDataWidth;

// @note  When the C++ standard changes to 20 or higher we can use std::rotl here
// rather than this left rotate method.

// value - value to rotate left.
// n     - number of bits to rotate .. |n| must be < 32. Best if it's positive but
//           I think this will work even if negative.
// It is important that the parameter value is unsigned else C
// is allowed to fill the right shift with the sign bit which is not desired.
//
static inline uint32_t rol(uint32_t value, int n) {
    return (value << n) | (value >> 32-n);
}
/**
 *  constructor
 *     @param controller - reference to the MVLC we will encapsulate
 *    The caller continues to own the object.
 */
CMVLCDirect::CMVLCDirect(MVLC& controller) :
    m_controller(controller) {}

/**
 *  destructor - chain to base class.
 */
CMVLCDirect::~CMVLCDirect() {}

/////////////////////////////// VME operations /////////////////////////////////////////////

/**
 * vmeWrite32
 *    Do a 32 bit write to the VME.  Delegates to m_controller.vmWrite
 * 
 * @param address - vme address.
 * @param aModifier - address modifier that selects that address space for  address.
 * @param data    - Data to write.
 * @return int    - 0 on success, on failure, errno is set to EIO rather than analyzing the ec:
 * 
 */
int
CMVLCDirect::vmeWrite32(uint32_t address, uint8_t aModifier, uint32_t data) { 
    if(m_controller.vmeWrite(address, data, aModifier, VMEDataWidth::D32)) {
        errno = EIO;
        return -1;
    }
    return 0;
}
/**
 * vmeWrite16
 *    Do a 16 bit write to a vme address:
 * 
 * @param address - vme address.
 * @param aModifier - address modifier that selects that address space for  address.
 * @param data    - Data to write.
 * @return int    - 0 on success, on failure, errno is set to EIO rather than analyzing the ec: 
 */
int
CMVLCDirect::vmeWrite16(uint32_t address, uint8_t aModifier, uint16_t data) { 
    if(m_controller.vmeWrite(address, (uint32_t)(data), aModifier, VMEDataWidth::D16)) {
        errno = EIO;
        return -1;
    }
    return 0;
}
/**
 *  delay
 *     Delay for at least the requested number of 200ns units. The MVLC class does not support
 * using the controller to do the delay - we will us usleep to delay for the number usec that is
 * at least enough:
 * 
 * @param amount - number of 200ns units desired.
 */
void 
CMVLCDirect::delay(uint32_t amount) {
    useconds_t usec = (double)amount/5.0 + 1;    // Usecs to delay (5 200ns units in a microseconds).

    usleep(usec);
}
/**
 *  loopUntil32
 *    Loops until the value gotten by reading a 32 bit value anded with a mask is the same as a value:
 * 
 * @param address - address to read.
 * @param amod    - address modifier.
 * @param mask    - the mask to and the read value with.
 * @param value   - the value to compare with.
 * 
 *
 * @note we ignore read errors....which could cause infinite loops.
 */
void
CMVLCDirect::loopUntil32(uint32_t address, uint8_t amod, uint32_t mask,  uint32_t value) {
    while (true) {
        uint32_t data(0);
        m_controller.vmeRead(address, data, amod, VMEDataWidth::D32);
        if ((data & mask) == value) return;
    }
}
/**
 *  loopUntil16
 *    Same as loopUntil32 but a 16 bit read is done:
 */
void
CMVLCDirect::loopUntil16(uint32_t address, uint8_t amod, uint32_t mask, uint32_t value) {
    while (true) {
        uint32_t data(0);
        m_controller.vmeRead(address, data, amod, VMEDataWidth::D16);
        if ((data & mask) == value) return;
    }
}
/**
 * executeList
 *    Since I don't yet know/understand how to do an MVLC immediate list like I do with a VMUSB,
 * this method interprets the list...which is just a set of operations encoded as for the Yaml
 * configuration file.   We *know* this list only contains writes loopuntils and delay operations.
 * 
 * @param  list - Referencde to the VMUSB Readout list.
 * @param  pReadBuffer - pointer to a buffer that will never get anything put in it.
 * @param  readBufferSize - Size of the read buffer.
 * @param  Pointer to a cell to hold the number of bytes read (always gets 0 stored in it).
 * @return 0 on success else -1 and errno has: EIO If an operation failed or ENOTSUP if the list contains
 * an unsupported operation.
 */
int
CMVLCDirect::executeList(CVMUSBReadoutList& list, void* pReadBuffer, size_t readBufferSize, size_t * bytesRead) {
    auto operations = list.dumpForMvlc();      // get the list operations.

    try {
        for (size_t pc = 0; pc < operations.size(); ) {
            std::string opcode;
            std::stringstream sop(operations[pc]);                 // Makes parsing easy.
            sop >> opcode;                             // operation to do:

            if (opcode == "vme_write") {
                interp_write(sop);
                pc++;
            } else if (opcode == "wait") {
                interp_wait(sop);
                pc++;
            } else if (opcode == "mask_shift_accu") {
                interp_mask_shift_accu(sop);
                pc++;

            } else if (opcode == "read_to_accu") {
                interp_read_to_accu(sop);
                m_prevReadIndex = pc;                  // Save the potential loop dest.
                pc++;
            } else if (opcode == "compare_loop_accu") {
                if (interp_compare_loop_accu(sop)) {
                    pc++;
                } else {
                    pc = m_prevReadIndex;             // no match so loop.
                }
            } else {
                errno = ENOTSUP;
                return -1;
            }
        }
    } catch (std::runtime_error& error) {
        errno = EIO;
        return -1;
    }
    return 0;
}
/////////////////////////////////////// Private methods ///////////////////////////////////////////

/// Methods for interpreting  CVMUSReadoutlists.
/// Note for all of these, the caller has pulled the opcdoe
/// out of the string stream.  It's the responsibility of the
/// method to obtain the remaining parameters.

/**
 *  interp_write
 *     Interpret a write operation.  The full format of this is 
 * 
 * vme_write amod d16|d32 address value
 * 
 * Note that CVMUSBReadoutList has encode numbers as 0xhexnumbers.
 * We count on that if that's not the case the decode for  numbers will fail.
 * 
 * @param operation the operation with the "vme_write" already removed from it.
 * @return 0  - on success.  Throws std::runtime_error if the write failed.
 */
int
CMVLCDirect::interp_write(std::stringstream& operation) {
    uint8_t amod;
    std::string width;
    uint32_t address;
    uint32_t data;

    operation >> std::hex >> amod >> width >> address >> data;

    if (m_controller.vmeWrite(address, data, amod, width == "d32" ? VMEDataWidth::D32 : VMEDataWidth::D16)) {
        throw std::runtime_error("VMEWrite failed");
    }
    return 0;
}
/**
 *  interp_wait
 * 
 *      Wait for at least n 62.5usec  The full format of this is:
 * 
 * wait decimal-cycles
 * 
 * @param operation - the operation with "wait" already removed.
 * @note the actual delay will be the last number of usec that is at least as long as requested.
 */
int
CMVLCDirect::interp_wait(std::stringstream& operation) {
    uint32_t cycles;
    operation >> cycles;
    uint32_t vmusbdelay = (double)cycles*62.5/200.0 + 1.0;   // in 200ns units

    delay(vmusbdelay);


    return 0;                   // Always works.
}
/**
 * interp_mask_shift_accu
 *    Interpret the mask and shift accumulator operation.  This looks like this:
 * 
 * mask_shift_accu hex-mask dec-rotate-left-count.
 * 
 * @param operation - the operation with mask_shift_accu already removed.
 * @return int  - always 0, this operation cannot fail.
 * @note m_accumulator contains the accumulator value
 */
int
CMVLCDirect::interp_mask_shift_accu(std::stringstream& operation) {
    uint32_t mask;
    int      shift;


    operation >> std::hex >> mask >> std::dec >> shift;

    m_accumulator &= mask;
    m_accumulator = rol(m_accumulator, shift);

    return 0;
}
/**
 * interp_read_to_accu
 *     Interpret a read_to_accu operation which should look like:
 * 
 * 
 * read_to_accu amod d16|d32 address
 * 
 * where amod and address are hex.
 * The result of the read on success is put in m_accumulator.
 * 
 * @param operation - the operation string with read_to_accu already stripped out.
 * @return int 0 on success else throws std::runtime_error.
 */
int
CMVLCDirect::interp_read_to_accu(std::stringstream& operation) {
    uint8_t amod;
    std::string width;
    uint32_t addr;

    uint32_t data(0);
    //Decode the operands:

    operation >> std::hex >> amod >> width >>addr;

    // do the read:

    auto ec = m_controller.vmeRead(addr, data, amod, width == "d32" ? VMEDataWidth::D32 : VMEDataWidth::D16 );
    if (ec) {
        throw std::runtime_error("vmeRead in interp_read_to_accu failed");
    }

    m_accumulator = data;

    return 0;
}
/**
 *  interp_comare_loop_accu
 * 
 *     Compares the accumulator to a value and returns non-zero if the comparison matches.
 * The form of this is:
 * 
 * compare_loop_accu cmp hex-value
 * 
 * Where cmp is a -1, 0, 1, depending on if we want the accumulator to be less, equal or greater than hex-value.
 * In practicde the CVMUSBReadoutList will always use 0 but we'll make this general just in case things
 * change.
 * 
 * @param operation - the operation with compare_loop_accu stripped off.
 * @return int - 0 comparison failed, 1 comparison worked.  It's up to the caller to implement the loop.
 */
int
CMVLCDirect::interp_compare_loop_accu(std::stringstream& operation) {
    int compare;
    uint32_t value;

    operation >> compare >> std::hex >>value;

    if (compare < 0) {
        return m_accumulator < value;
    } else if (compare > 0) {
        return m_accumulator > value;
    } else {   // 0
        return m_accumulator == value;
    }
    // should not vall here.
    // this will kill the program as it's not caught.
    throw std::logic_error("interp_compare_loop_accu - control fell through the if chain");
}
