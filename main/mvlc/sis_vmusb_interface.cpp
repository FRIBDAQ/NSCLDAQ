/**
 *  @file sis_vmusb_interface.cpp
 *  @brief Implement the class described in sis_vmusb_interface.h
 *  @author Ron Fox<rfoxkendo@gmail.com>
 * 
 *  
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
 */

#include "sis_vmusb_interface.h"
#include <string.h>
#include <stdexcept>

#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"     // For the adress modifier defs.

// I'm lazy so:

#define VME sis_vmusb_interface

/**
 * constructor just nulls out the interface pointer:
 */
sis_vmusb_interface::sis_vmusb_interface() : m_pInterface(0) {}

/**
 * vmeopen
 *    Open the vme interface.
 * @param interface - an open CVMUSB object reference we'll squirel away.
 * @return int - 0 on success, -1 on failure.
 * @note only fails if Globals::pUSBController is null.
 */
int 
VME::vmeopen(void* interface) {
    m_pInterface = reinterpret_cast<CVMUSB*>(interface);
    return 0;
}
/**
 * vmeclose
*     null out the interface pointer. we can't decomission an interface.
 * @return 0 - success.
 */
int 
VME::vmeclose(void) { 
    m_pInterface = nullptr;
    return 0;
}
/**
 * get_vmeopen_messages
 *   Returns failure messages for vme open?
 *  
 * @param messages - buffer into which to load the message(?)
 * @param nof_found_devices - 1 or 0 depending on pUSBController.
 * @returns 0 - always success (I guess).
 */
int
VME::get_vmeopen_messages(CHAR* messages, UINT* nof_found_devices) {
    if (m_pInterface) {
        *nof_found_devices = 1;
        strcpy(messages, "Open will succeed");
    } else {
        *nof_found_devices = 0;
        strcpy(messages, "Must vme open before use");
        return -1;
    }
    return 0;
}
/**
 * vme_A32D32_write
 * 
 * Write a 32 bit data item to a VME address.  We use A32 bit user data for the
 * AM.
 *   @param addr - the addres to which the write is done.
 *   @param data - data to write.
 * 
 */
int
VME::vme_A32D32_write( UINT addr, UINT data ) {
    if (m_pInterface) {
        m_pInterface->vmeWrite32(addr, CVMUSBReadoutList::a32UserData, data);
        return 0;
    } else {
        return -1;        // Can't, the device isn't open.
    }

}

/**
 * vme_A32DMA_write
 *   @note - The CVMUSB class has not implemented support for block write transfers.
 *    These will all be simulated  with a write loop.
 *   @note since block writes during DAQ are rare, this is probably not a big
 * performance issue, however:
 * TODO:  Implement block writes in the CVMUSB support class.
 * 
 * @param addr - first VME address written to.
 * @param data - Pointer to the data to write.
 * @param request_nof_words - number of int32_t's to transfer.
 * @param written_nof_words - number of in32_t's that were transfered.
 * @return 0 on success.
 */
int
VME::vme_A32DMA_D32_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) {
    // assumption:  UINT is uint32_t or some such.


    *written_nof_words = 0;
    for (int i =0; i < request_nof_words; i++) {
        int status = vme_A32D32_write(addr, *data);
        if (status < 0) {
            return status;
        }
        *written_nof_words++;
        data++;
        addr += sizeof(UINT);
    }
    return 0;
}

// All of the other block writes are in terms of this:

int
VME::vme_A32BLT32_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) {
    return vme_A32DMA_D32_write(addr, data, request_nof_words, written_nof_words);
}

int
VME::vme_A32MBLT64_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) {
    int status =  vme_A32DMA_D32_write(addr, data, request_nof_words*2, written_nof_words);
    *written_nof_words /=2;

    return status;
}


// FIFO writes are just like vme_A32DMA_write but the taget address isn't
// incremented.

int
VME::vme_A32DMA_D32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) {
    // assumption:  UINT is uint32_t or some such.

    *written_nof_words = 0;
    for (int i =0; i < request_nof_words; i++) {
        int status = vme_A32D32_write(addr, *data);
        if (status < 0) {
            return status;
        }
        *written_nof_words++;
        data++;
        
    }
    return 0;
}

// All other FIFO writes are in terms of the above:

int
VME::vme_A32BLT32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words )  {
    return vme_A32DMA_D32FIFO_write(addr, data, request_nof_words, written_nof_words);

}

int
VME::vme_A32MBLT64FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words )  {
    int status = vme_A32DMA_D32FIFO_write(addr, data, request_nof_words*2, written_nof_words);
    *written_nof_words /= 2;
    return status;

    
}

/** vme_IRQ_Status_read
 *   Not possible in the VMUSB - we throw.
 *   std::runtime_error.
 */
int 
VME::vme_IRQ_Status_read(UINT* data) {
    throw std::runtime_error("VMUSB does not support vme_IRQ_Status_read operations");
}



