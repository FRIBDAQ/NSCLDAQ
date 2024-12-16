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

#ifndef SIS_VMUSB_INTERFACE_H
#define SIS_VMUSB_INTERFACE_H
#include <CMutex.h>
#include "vme_interface_class.h"

class CVMUSBReadoutList;

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
	static CMutex m_monitor;     // Critical section monitor.
public:
    virtual int vmeopen( void ) ;
	virtual int vmeclose( void ) ;

	virtual int get_vmeopen_messages( CHAR* messages, UINT* nof_found_devices ) ;

	virtual int vme_A32D32_read( UINT addr, UINT* data ) ;

	virtual int vme_A32DMA_D32_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;
	virtual int vme_A32BLT32_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;
	virtual int vme_A32MBLT64_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;
	virtual int vme_A32_2EVME_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;
	virtual int vme_A32_2ESST160_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;
	virtual int vme_A32_2ESST267_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;
	virtual int vme_A32_2ESST320_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;

	virtual int vme_A32DMA_D32FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;
	virtual int vme_A32BLT32FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;
	virtual int vme_A32MBLT64FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;
	virtual int vme_A32_2EVMEFIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;
	virtual int vme_A32_2ESST160FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;
	virtual int vme_A32_2ESST267FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;
	virtual int vme_A32_2ESST320FIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;


	virtual int vme_A32D32_write( UINT addr, UINT data ) ;
	virtual int vme_A32DMA_D32_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;
	virtual int vme_A32BLT32_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;
	virtual int vme_A32MBLT64_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;

	virtual int vme_A32DMA_D32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;
	virtual int vme_A32BLT32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;
	virtual int vme_A32MBLT64FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;

	virtual int vme_IRQ_Status_read( UINT* data ) ;

    // Operations that support building a list - TODO: provide list ops.

};

#endif