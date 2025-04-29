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
 * addWrite32
 *    Add a 32 bit write to the stack.  This looks like
 * \verbatim
 * vme_write	amod d32 address value	
 * \endverbatim
 * 
 * @param addresss - the address to write to.
 * @param amod     - the address modifier to use... CVMUSBReadoutList has some constant defs so you
 *                   don't need to memorize them from the VME spec.
 * @param datum    - The data to write.
 */
void
CVMUSBReadoutList::addWrite32(uint32_t address, uint8_t amod, uint32_t datum) {
    addWrite(address, amod, "d32", datum);
}
/**
 *  addWrite16
 *     Same as addWrite32 above but the write is a d16 write.
 */
void
CVMUSBReadoutList::addWrite16(uint32_t address, uint8_t amod, uint16_t datum) {
    addWrite(address, amod, "d16", datum);
}

/**
 * addRead32
 *     Add a 32 bit read to the stack.  This produces something like
 * \verbatim
 *  vme_read amod d32 address
 * \endverbatim
 *
 * 
 * @param address - the address to read from.
 * @param amod    - The address modifier to use... CVMUSBReadoutList supplies some symbolics to help.
 */
void
CVMUSBReadoutList::addRead32(uint32_t address, uint8_t amod) {
    addRead(address, amod, "d32");
}
/**
 * addRead16 - Same as addRead32 but the read is a 16 bit read.
 * 
 */
void
CVMUSBReadoutList::addRead16(uint32_t address, uint8_t amod) {
    addRead(address, amod, "d16");
}

/**
 *  addBlockRead32
 *     Adds a block read of 32 bit items.  The caller must ensure that the address modifier is
 * a block transfer modifier such as CVMUSBReadoutList::a32UserBlock or else the results of the
 * operation are not defined.alignas
 *
 * @param baseAddress- address of the first read.
 * @param amod   -- Address modifier (see cautiolnabove).
 * @param transfers - Number of transfers to occur.
 * 
 * VME note:  normally a read requires two vme bus cycles one to assert the transfer address and
 * the second to actually clock in the data.  VME added block transfers where the target of the read is 
 * responsible for 'knowing' the addresss of subsequent transfers after the first in a block.  The
 * number of transfers in e.g. an MBLT is limited so the master must still perform an address cycle from
 * time to time, however this address cycle is now amortized over several data transfers improving performance
 *  
 * THe resulting stack line is of the form:alignas
 * \verbatim
 *   vme_block_read_mem	amod transfers address
 * \endverbatim 
 */
void
CVMUSBReadoutList::addBlockRead32(uint32_t baseAddress, uint8_t amod, size_t transfers) {
    std::stringstream stack_line;
    stack_line << "vme_block_read_mem 0x" << std::hex << unsigned(amod) << std::dec << " " << transfers
        << std::hex << " 0x" << baseAddress;

    std::string mvlc_op(stack_line.str());
    m_list.push_back(mvlc_op);
}
/**
 * addFifoRead32
 *     THis is identical to addBlockRead32, _but_  the slave does not internally increment the
 * transfer address  within a VME block transfer and the master always outputs the same base address
 * for each VME block.   As before, the caller is responsible for using a valid block transfer
 * address modifier.
 * 
 * @param baseAddress - address of the FIFO.
 * @param amod        - Address modifier of the transfer.
 * @param transfers   - number of transfers to perform.
 */
void
CVMUSBReadoutList::addFifoRead32(uint32_t baseAddress, uint8_t amod, size_t transfers) {
    std::stringstream stack_line;
    stack_line << "vme_block_read 0x" << std::hex << unsigned(amod) << std::dec << " " << transfers
        << std::hex << " 0x" << baseAddress;

    std::string mvlc_op(stack_line.str());
    m_list.push_back(mvlc_op);
}

/**
 * addBlockCountRead16
 *   For the MVLC, the counted block read (that is the block read where the count comes from
 * a bit field in a value read from the VME), this requires
 * 1.  A read from memory to the 'accumulator'
 * 2.  A mask/shift of the accumulator to generate the count.
 * 3.  Either a vme_read_mem for a block transfer from memory or vme_read for a FIFO transer
 * 
 * This method does step 1 and 2 of the above.  The addMaskedCountBLockxxxxRead32 methods do other steps.
 * The utility maskAndShift generates step 2 in a common way for those methods.
 * 
 * @param address - address from which the count is read.
 * @param mask    - mask that defines the field containing the count.
 * @param amod    - address modifier.
 * 
 * @note, if the user's next operation is not a masked Count block read, the results will be unexpected.
 */
void
CVMUSBReadoutList::addBlockCountRead16(uint32_t address, uint32_t mask, uint8_t amod) {
    readToAccumulator(address, amod, "d16");    // Step 1.
    maskAndShift(mask);                         // Step 2.
}
/**
 *  Same as addBlockCountRead16 but the read to the accumulator is 32 bits.
 */
void
CVMUSBReadoutList::addBlockCountRead32(uint32_t address, uint32_t mask, uint8_t amod) {
    readToAccumulator(address, amod, "d32");    // Step 1.
    maskAndShift(mask);                         // Step 2.
}
/**
 * addMaskedCountBlockRead32 
 *   This assumes the accumulator already has the transfer counnt via a clall to AddBlockCountReadxx
 *   When that's the case, a vme_read_mem ( addRead32) is a block read that does the specified
 *   number of transfers.
 * 
 * @param addresss - base address for hte read (this gets incremente4d by vme_read_mem as needed).
 * @param amod     - Address modifier for the read  an MBLT amod is suggested.
 *    
 */
void
CVMUSBReadoutList::addMaskedCountBlockRead32(uint32_t address, uint8_t amod) {
    addRead32(address, amod);                 // Generates the right stuff.
}

/**
 *  addMaskedCountFifoRead32
 *     Same as addMaskedCountBlockRead32 but we use a vme_read so that the address does not 
 * increment, as for a FIFO.
 */
void 
CVMUSBReadoutList::addMaskedCountFifoRead32(uint32_t address, uint8_t amod) {
    // Don't have a utility and this is the only use so, shamelessly copy/pasted and
    // edited from the addRead utility, we have:

    std::stringstream stack_line;
    stack_line << "vme_read 0x" << std::hex << unsigned(amod)
        << " " << "d32" << " 0x" << address;
    
    std::string mvlc_op(stack_line.str());
    m_list.push_back(mvlc_op);
}
/**
 *  addDelay
 
 * 
 * @param value  - The number delay length in 200ns units.  
 * 
 * This will be transparently converted to the 62.5 cycle units supported
 * by the mvlc.  The caller  must ensure that this value is less than 0xffffff cycles.
 * In general this is not a problem as the VMUSB only allowed up to 255 cycles in a delay
 * which is easily accomodated.
 * 
 * In general, the waits will be as much as 62.5ns longer than requested.
 */
void
CVMUSBReadoutList::addDelay(uint32_t value) {
    
    int cycles = (float)(value)*200.0/62.5  + 1.0;   // MVLC cycles.
    std::stringstream stack_line;
    stack_line << "wait " << cycles;
    std::string mvlc_op(stack_line.str());

    m_list.push_back(mvlc_op);


}
/**
 * addLoopUntil32:
 *     Loop over a 32 bit read until a condition is met.
 * @param address - address of the read.
 * @param amod    - address modifier.
 * @param mask    - comparison mask.
 * @param value   - comarison value.
 * 
 * The read is done over and over again until when anded with the mask, it matches value.
 * Suppose for example you need to delay until bit 1 (numbered from 0) is set in some register.
 * You can 
 *   addLoopUntil32(address, amod, 0x02, 0x02); and the stack will wait for that to happen.
 */
void
CVMUSBReadoutList::addLoopUntil32(uint32_t address, uint8_t amod, uint32_t mask, uint32_t value) {
    loopUntil(address, amod, mask, value, "d32");

    

}
/**
 * addLoopUntil16 - Same as addLoopUntil32 but the read is 16 bits wide
 */
void 
CVMUSBReadoutList::addLoopUntil16(uint32_t address, uint8_t amod, uint32_t mask, uint32_t value) {
    loopUntil(address, amod, mask, value, "d16");
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

/** 
 *  clear
 *    Just clear's the vector
 */
void
CVMUSBReadoutList::clear() {
    m_list.clear();
}
////////////////////////// Private utilities ////////////////////////////////////////////


/** 
 * addWrite  - add a write to the stack.  The additional parameter from the addWritexx is the
 * textual width of the write ("d32" or "d16").
 */
void
CVMUSBReadoutList::addWrite(uint32_t address, uint8_t amod, const char* width, uint32_t data) {
    std::stringstream stack_line;
    stack_line << "vme_write 0x" << std::hex   << unsigned(amod) <<  " " << width 
     << " 0x" << address << " 0x" << data;
    std::string mvlc_op(stack_line.str());
    m_list.push_back(mvlc_op);
}

/**
 *  addRead - add a read to the stack.  
 * 
 */
void
CVMUSBReadoutList::addRead(uint32_t address, uint8_t amod, const char* width) {
    std::stringstream stack_line;
    stack_line << "vme_read_mem 0x" << std::hex << unsigned(amod)
        << " " << width << " 0x" << address;
    
    std::string mvlc_op(stack_line.str());
    m_list.push_back(mvlc_op);
}

/** 
 * generate a read_to_accu	amod data_width address stack line:
 * 
 */
void
CVMUSBReadoutList::readToAccumulator(uint32_t address, uint8_t amod, const char* size) {
    std::stringstream stack_line;
    stack_line << "read_to_accu 0x" << std::hex << unsigned(amod) << " " << size    
        << " 0x" << address;

    std::string mvlc_op(stack_line.str());
    m_list.push_back(mvlc_op);
}
/**
 *  generate a mask_shift_accu	mask shift 
 * operation.  This assumes the mask is never 0.
 * 
 */
void
CVMUSBReadoutList::maskAndShift(uint32_t mask) {
    // need to figure out the shift count:

    uint32_t scmask = 1;
    unsigned scount = 0;
    while(!(scmask & mask)) {
        scount++;
        scmask = scmask << 1;
    }
    // The shift is a rotate (circular)  left on a 32 bit accumulator so :
    // Don't need any shifting if it's 0.
    if (scount != 0) scount = 32 - scount;  // Corresopnds to right shift of scount

    // Generate the stack line:

    std::stringstream stack_line;
    stack_line << "mask_shift_accu 0x" << std::hex << mask << " " << std::dec << scount;

    std::string mvlc_op(stack_line.str());
    m_list.push_back(mvlc_op);

}

/** looputil make a generic loop until */

void CVMUSBReadoutList::loopUntil(uint32_t address, uint8_t amod, uint32_t mask, uint32_t value, const char* width) {
    readToAccumulator(address, amod, width);
    std::stringstream stack_line;
    stack_line << "mask_shift_accu 0x" << std::hex << mask << " 0" <<  std::dec;
    std::string op = stack_line.str();
    m_list.push_back(op);
    
    // Clear the stream I think.

    stack_line.seekp(0);
    stack_line.str("");
    // cmp is like from strcomp -1 less than, 0 equal, +1 > than according to Florian
    stack_line << "compare_loop_accu 0 0x" << std::hex << value;
    op = stack_line.str();

    m_list.push_back(op);
}