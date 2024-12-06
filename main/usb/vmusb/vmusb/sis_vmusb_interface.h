/**
 *  @file sis_vmusb_interface.h
 *  @brief Define class to rehost the SIS vme interface class on VMUSB.
 * 
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
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef SIS_VMUSB_INTERFACE_H
#define SIS_VMUSB_INTERFACE_H
#include "vme_interface_class.h"

class CVMUSBReadoutList;

/**
 *  @class sis_vmusb_interface
 *    This is mostly  just a copy of what's in the base class,
 *   however we add support as well for building lists.
 */
class sis_vmusb_interface:: public vme_interface_class {
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
	virtual int vme_A32_2ESST320FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words ) ;


	virtual int vme_A32D32_write( UINT addr, UINT data ) ;
	virtual int vme_A32DMA_D32_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;
	virtual int vme_A32BLT32_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;
	virtual int vme_A32MBLT64_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;

	virtual int vme_A32DMA_D32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;
	virtual int vme_A32BLT32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;
	virtual int vme_A32MBLT64FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words ) ;

	virtual int vme_IRQ_Status_read( UINT* data ) ;

    // Operations that support building a list - TODO:

};

#endif