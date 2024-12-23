#ifndef _SIS315x_USB_LINUX_VME_CLASS_
#define _SIS315x_USB_LINUX_VME_CLASS_

#include "project_system_define.h"		//define LINUX or WINDOWS

#include <sys/types.h>

//#include "sis3150usb_vme_calls.h"
//#include "sis3100_vme_calls.h"
//#include "sis1100_var.h"
#include "sis3150usb_vme.h"
#include "vme_interface_class.h"

typedef unsigned int UINT;
typedef unsigned int* PUINT;

#define MAX_SIS3150USB_DEVICES		4

class sis315x : public vme_interface_class
{
private:
    HANDLE usbdevice_handle;

    int err;
	struct SIS3150USB_Device_Struct device_info[MAX_SIS3150USB_DEVICES];
    UINT used_device_no ;
    UINT nof_devices_found ;
    CHAR char_messages[128] ;

public:
	sis315x(unsigned int use_device_no);

	// open
	int vmeopen(void);
	int vmeclose(void);
	int get_vmeopen_messages(CHAR* messages, UINT* nof_found_devices);


	// local side register: only for sis1100
	int localRegisterRead(UINT addr, UINT* data);
	int localRegisterWrite(UINT addr, UINT data);
	// vme master side register
	int controlRegisterRead(UINT addr, UINT* data);
	int controlRegisterWrite(UINT addr, UINT data);
	// vme read
	int vme_A32D32_read(UINT addr, UINT* data);
	int vme_A32DMA_D32_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words);
	int vme_A32BLT32_read(UINT addr, UINT* data, UINT req_num_data, UINT* got_num_data);
	int vme_A32MBLT64_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words);
	int vme_A32_2EVME_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2ESST160_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2ESST267_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2ESST320_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );

	int vme_A32DMA_D32FIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words);
	int vme_A32BLT32FIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words);
	int vme_A32MBLT64FIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words);
	int vme_A32_2EVMEFIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2ESST160FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2ESST267FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2ESST320FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );


	// vme write
	int vme_A32D32_write(UINT addr, UINT data);
	int vme_A32DMA_D32_write(UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words);
	int vme_A32BLT32_write(UINT addr, PUINT data, UINT req_num_data, PUINT put_num_data);
	int vme_A32MBLT64_write(UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words);
	int vme_A32DMA_D32FIFO_write(UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words);
	int vme_A32BLT32FIFO_write(UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words);
	int vme_A32MBLT64FIFO_write(UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words);
//	~sis315x(void);

	int vme_IRQ_Status_read( UINT* data ) ;
};
#endif // _SIS315x_USB_LINUX_VME_CLASS_
