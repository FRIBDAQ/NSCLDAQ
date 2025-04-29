/**
 *  @file sis_vmusb_interface.h
 *  @brief Define class to rehost the SIS vme interface class on VMUSB.
 *  @author Ron Fox<rfoxkendo@gmail.com>
 * Intent of this is to provide a plaform on which the 
 * SIS class for handling the SIS3316 can be ported.
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

#ifndef MVLC_SIS_VMUSB_INTERFACE_H
#define MVLC_SIS_VMUSB_INTERFACE_H

#include "vme_interface_class.h"

class CVMUSB;

/**
 *  @class sis_vmusb_interface
 *    This is mostly  just a copy of what's in the base class,
 *   however we add support as well for building lists.
 * 
 * Note that this is threadsafe to the extent that bracketing all
 * VMUSB operations in a critical section is threadsafe.
 * Clearly it's possible for interleaved accesses on the same
 * module to fail in some way.  It's also the
 * caller's responsibility to ensure that prior to calling any methods,
 * the interface is not in autonomous data taking mode.
 */
class sis_vmusb_interface : public vme_interface_class {
private:
	CVMUSB* m_pInterface;
public:
	sis_vmusb_interface();
    virtual int vmeopen(void* interface) ;
	virtual int vmeclose( void ) ;

	virtual int get_vmeopen_messages( CHAR* messages, UINT* nof_found_devices ) ;

	// Kill off the reads as the non-interactive nature of the translator makse them impractical.
	
	
	virtual int vme_A32D32_write( UINT addr, UINT data ) ;
	virtual int vme_A32DMA_D32_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;
	virtual int vme_A32BLT32_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;
	virtual int vme_A32MBLT64_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;

	virtual int vme_A32DMA_D32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;
	virtual int vme_A32BLT32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;
	virtual int vme_A32MBLT64FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;

	virtual int vme_IRQ_Status_read( UINT* data ) ;

    

};

#endif