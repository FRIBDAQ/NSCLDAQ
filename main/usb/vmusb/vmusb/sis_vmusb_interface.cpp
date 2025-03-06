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
#include <Globals.h>    // Accdess to the interface.
#include <string.h>
#include <stdexcept>

#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"

// I'm lazy so:

#define VME sis_vmusb_interface

// Static storage - a class level mutex used to
// serialize access to the controller.

CMutex VME::m_monitor;

/**
 * vmeopen
 *    Open the vme interface.
 * @return int - 0 on success, -1 on failure.
 * @note only fails if Globals::pUSBController is null.
 */
int 
VME::vmeopen(void) {
    if (Globals::pUSBController) {
        return 0;
    } else {
        return -1;
    }
}
/**
 * vmeclose
 *   This is a no-op. 
 * @return 0 - success.
 */
int 
VME::vmeclose(void) { 
    return 0;
}
/**
 * get_vme_open_messages
 *   Returns failure messages for vme open?
 *  
 * @param messages - buffer into which to load the message(?)
 * @param nof_found_devices - 1 or 0 depending on pUSBController.
 * @returns 0 - always succdess (I guess).
 */
int
VME::get_vmeopen_messages(CHAR* messages, UINT* nof_found_devices) {
    if (Globals::pUSBController) {
        *nof_found_devices = 1;
        strcpy(messages, "Open will succeed");
    } else {
        *nof_found_devices = 0;
        strcpy(messages, "There's no underlying VMUSB object to use");
    }
    return 0;
}
/** 
 * vme_A32_D32_read
 *  Reads a single 32 bit word from the VME note that we use the a32UserData address
 * modifier.
 * @param addr - VME address.
 * @param data - Where to put the data.
 * @return value returned from the underlying CVMUSB::vmeread32.
 * 
 */
int 
VME::vme_A32D32_read(UINT addr, UINT* data) {
    if (Globals::pUSBController) {
        CriticalSection s(m_monitor);
        return Globals::pUSBController->vmeRead32(
            addr, CVMUSBReadoutList::a32UserData, data
        );
    } else {
        // there's no open interface:

        return -1;
    }
}
/**
 *  vme_A32DMA_D32_read
 *     block read from a32/d32 (extended user data) space:
 *  @param addr - VME address from which to read.
 *  @param data - Where to put the read data.
 *  @param request_nof_words - number of words requested.
 *  @param got_nof_words - pointer to a place to put the actual transfer count.
 *  @return value from CMUSB::vemBlockRead
 */
int
VME::vme_A32DMA_D32_read(
    UINT addr, UINT* data, UINT req_nof_words, UINT* got_nof_words
) {
    // note the sizes are size_t in the VMUSB library.

    if (Globals::pUSBController) {
        CriticalSection s(m_monitor);
        size_t xfercount(0);
        int result = Globals::pUSBController->vmeBlockRead(
            addr, CVMUSBReadoutList::a32UserData,
            data, req_nof_words, &xfercount
        );
        *got_nof_words = xfercount;
        return result < 0 ? result : 0;
    } else {
        *got_nof_words = 0;
        return -1;
    }
}
/**
 * vme_A32BLT32_D32_read
 *    Read a block of data uing a BLT transfer (AM=CVMUSB::a32UserBlock)
 * 
 *  @param addr - VME address from which to read.
 *  @param data - Where to put the read data.
 *  @param request_nof_words - number of words requested.
 *  @param got_nof_words - pointer to a place to put the actual transfer count.
 *  @return value from CMUSB::vemBlockRead
 */
int
VME::vme_A32BLT32_read(
    UINT addr, UINT* data, UINT req_nof_words, UINT* got_nof_words
) {
// note the sizes are size_t in the VMUSB library.

    if (Globals::pUSBController) {
        CriticalSection s(m_monitor);
        size_t xfercount(0);
        int result = Globals::pUSBController->vmeBlockRead(
            addr, CVMUSBReadoutList::a32UserBlock,
            data, req_nof_words, &xfercount
        );
        *got_nof_words = xfercount;
        return result < 0 ? result : 0;
    } else {
        *got_nof_words = 0;
        return -1;
    }
}
/**
 * vme_A32MBLT64_read
 *     This transfer mode is supposed to be supported by he
 * VMUSB -- we'll see.  The address modifier will be hard coded to
 * 0xc which is the  which is the non-privileged MBLT64 transfer AM.
 * 
 *  @param addr - VME address from which to read.
 *  @param data - Where to put the read data.
 *  @param request_nof_words - number of words requested.
 *  @param got_nof_words - pointer to a place to put the actual transfer count.
 *  @return value from CMUSB::vmeBlockRead
 */
int
VME::vme_A32MBLT64_read(
    UINT addr, UINT* data, UINT req_nof_words, UINT* got_nof_words
) {
// note the sizes are size_t in the VMUSB library.

    if (Globals::pUSBController) {
        CriticalSection s(m_monitor);
        size_t xfercount(0);
        int result = Globals::pUSBController->vmeBlockRead(
            addr, 0xc,
            data, req_nof_words, &xfercount
        );
        *got_nof_words = xfercount;
        return result < 0 ? result : 0;
    } else {
        *got_nof_words = 0;
        return -1;
    }
}

/**
 * The remainig types of block read transfers are not supported
 * by the VMUSB and just become MBLT64 transfers.
 */
int
VME::vme_A32_2EVME_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) {
    return vme_A32MBLT64_read(addr, data, request_nof_words, got_nof_words);
}

int
VME::vme_A32_2ESST160_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) {
    return vme_A32MBLT64_read(addr, data, request_nof_words, got_nof_words);
}

int
VME::vme_A32_2ESST267_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) {
    return vme_A32MBLT64_read(addr, data, request_nof_words, got_nof_words);
}

int
VME::vme_A32_2ESST320_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) {
    return vme_A32MBLT64_read(addr, data, request_nof_words, got_nof_words);
}

/**
 * vme_A32DMA_D32FIFO_read
 *   Do a block transfer from a FIFO- the transfers take place from the same
 * VME address.  
 * 
 *
 *  @param addr - VME address from which to read.
 *  @param data - Where to put the read data.
 *  @param request_nof_words - number of words requested.
 *  @param got_nof_words - pointer to a place to put the actual transfer count.
 *  @return value from CMUSB::vemFifoRead. 
 */
int
VME::vme_A32DMA_D32FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) {
    if (Globals::pUSBController) {
        CriticalSection s(m_monitor);
        size_t got;
        int result = Globals::pUSBController->vmeFifoRead(
            addr, CVMUSBReadoutList::a32UserData, data, request_nof_words, &got
        );
        *got_nof_words = got;
        return result < 0 ? result : 0;

    } else {
        *got_nof_words =0;
        return -1;
    }
}
/**
 * vme_A32BLT32FIFO_read
 *    same as above but with a block transfer adrfess modifier.  The
 * slave does not assume an address increment within each transfer block and the
 * master asserts the same address at the start of each block.
 * 
 *  @param addr - VME address from which to read.
 *  @param data - Where to put the read data.
 *  @param request_nof_words - number of words requested.
 *  @param got_nof_words - pointer to a place to put the actual transfer count.
 *  @return value from CMUSB::vemFifoRead.   
 */
int
VME::vme_A32BLT32FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) {
    if (Globals::pUSBController) {
        CriticalSection s(m_monitor);
        size_t got;
        int result = Globals::pUSBController->vmeFifoRead(
            addr, CVMUSBReadoutList::a32UserBlock, data, request_nof_words, &got
        );
        *got_nof_words = got;
        return result < 0 ? result : 0;

    } else {
        *got_nof_words =0;
        return -1;
    }
}
/**
 *  vme_A32MBLT64FIFO_read
 * 
 *    Same as above but the address modifier is 0xC this is supported by
 *    the VMUSB (supposedly).
 */
int
VME::vme_A32MBLT64FIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) {
    if (Globals::pUSBController) {
        CriticalSection s(m_monitor);
        size_t got;
        int result = Globals::pUSBController->vmeFifoRead(
            addr, 0xc, data, request_nof_words, &got
        );
        *got_nof_words = got;
        return result < 0 ? result : 0;

    } else {
        *got_nof_words =0;
        return -1;
    }
}

// The remaining FIFO transfer methods are not supported by the VMUSB
// and will be mapped to A32MBLT64 reads:

int
VME::vme_A32_2EVMEFIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )  {
    return vme_A32MBLT64FIFO_read(addr, data, request_nof_words, got_nof_words ) ;
}

int
VME::vme_A32_2ESST160FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) {
    return vme_A32MBLT64FIFO_read(addr, data, request_nof_words, got_nof_words ) ;
}

int
VME::vme_A32_2ESST267FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) {
    return vme_A32MBLT64FIFO_read(addr, data, request_nof_words, got_nof_words ) ;
}

int
VME::vme_A32_2ESST320FIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) {
    return vme_A32MBLT64FIFO_read(addr, data, request_nof_words, got_nof_words ) ;
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
    if(Globals::pUSBController) {
        CriticalSection s(m_monitor);
        return Globals::pUSBController->vmeWrite32(addr, CVMUSBReadoutList::a32UserData, data);
    } else {
        // no open controller

        return -1;
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
    return vme_A32DMA_D32_write(addr, data, request_nof_words, written_nof_words);
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

// All other FIFO writes are in t4erms of the above:

int
VME::vme_A32BLT32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words )  {
    return vme_A32DMA_D32FIFO_write(addr, data, request_nof_words, written_nof_words);

}

int
VME::vme_A32MBLT64FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words )  {
    return vme_A32DMA_D32FIFO_read(addr, data, request_nof_words, written_nof_words);
    
}

/** vme_IRQ_Status_read
 *   Not possible in the VMUSB - we throw.
 *   std::runtime_error.
 */
int 
VME::vme_IRQ_Status_read(UINT* data) {
    throw std::runtime_error("VMUSB does not support vme_IRQ_Status_read operations");
}