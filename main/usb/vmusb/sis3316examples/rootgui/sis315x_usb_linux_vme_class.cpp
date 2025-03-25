#include "project_system_define.h"	//define LINUX or WINDOWS
#include "project_interface_define.h"   //define Interface (sis1100/sis310x, sis3150usb or Ethnernet UDP)


#include "sis315x_usb_inux_vme_class.h"
#include "sis3150usb_vme.h"



#include <iostream>


#include <stdio.h>
//#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
//#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>


using namespace std;


// ----------------------------------------------------------------------------------------------------------------------

int call_vme_A32D32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data )
{
	  u_int32_t readdata ;
	  int return_code ;

	  return_code = sis3150Usb_Vme_Single_Read(hXDev, vme_adr, 0x9,4,&readdata)  ;

	  if (return_code < 0)  {
		  return(return_code) ;
	  }
	  *vme_data = readdata;
	  return(return_code) ;
}




int call_vme_A32D32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t vme_data )
{
	  int return_code ;
	  return_code = sis3150Usb_Vme_Single_Write(hXDev, vme_adr, 0x9, 4,  vme_data) ;
	  return(return_code) ;

}



int call_vme_A32BLT32FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data,
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
	  int return_code ;
		return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x9, 4, 1,
						      (ULONG*)vme_data, req_num_of_lwords, (ULONG*)got_num_of_lwords);
		return( return_code );
}

int call_vme_A32MBLT64FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data,
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
	  int return_code ;
		return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x8, 8, 1,
						      (ULONG*)vme_data, req_num_of_lwords & 0xfffffffe, (ULONG*)got_num_of_lwords);
		return(return_code) ;
}

// ----------------------------------------------------------------------------------------------------------------------







sis315x::sis315x( unsigned int use_device_no)
{

    this->used_device_no    = use_device_no ;
    this->nof_devices_found = 0 ;
	strcpy( this->char_messages, "sis315x device not open");
}


struct SIS3150USB_Device_Struct gl_dev_struct[1];



int sis315x::vmeopen(){

	   unsigned int numDev;
	    int rc;
	    err = -1;

	    rc = FindAll_SIS3150USB_Devices(gl_dev_struct, &numDev, 1);
	    std::cout << "FindAll_SIS3150USB_Devices': " << numDev  << "    return code = " << rc << std::endl;
	    if((rc) && (numDev > 0)) {
	        //std::cout << "Error during 'FindAll_SIS3150USB_Devices': " << rc << std::endl;
			strcpy( this->char_messages, "sis315x device open failed");
			this->nof_devices_found = 0 ;
		    err = -1;
	    }
		else {
			strcpy( this->char_messages, "sis315x device open OK");
			this->nof_devices_found = 1 ;
		    err = 0;

		    rc = Sis3150usb_OpenDriver_And_Download_FX2_Setup((char *)gl_dev_struct[0].cDName, &gl_dev_struct[0]);
		    this->usbdevice_handle = gl_dev_struct[0].hDev ;
		   // rc = Sis3150usb_OpenDriver_And_Download_FX2_Setup((char *)gl_dev_struct[0].cDName, &this->usbdevice_handle);

		    if(rc != 0){
		        std::cout << "Error during 'Sis3150usb_OpenDriver_And_Download_FX2_Setup': " << rc << std::endl;
				this->nof_devices_found = 0 ;
			    err = -1;
		    }
		}


	    return err;
}


int sis315x::vmeclose(){
	return -1;
}


int sis315x::get_vmeopen_messages(CHAR* messages, UINT* nof_found_devices){
	strcpy(messages, this->char_messages);
	*nof_found_devices = this->nof_devices_found ;
	return -1;
}




// local side register: only for sis1100
int sis315x::localRegisterRead(UINT addr, PUINT data)
{
	return -1;
}

int sis315x::localRegisterWrite(UINT addr, UINT data)
{
	return -1;
}

// vme master side register
int sis315x::controlRegisterRead(UINT addr, PUINT data)
{
	return sis3150Usb_Register_Single_Read(this->usbdevice_handle, addr, data);
}

int sis315x::controlRegisterWrite(UINT addr, UINT data)
{
	return sis3150Usb_Register_Single_Write(this->usbdevice_handle, addr, data);
}

// vme read
int sis315x::vme_A32D32_read(UINT addr, UINT* data)
{
    return call_vme_A32D32_read(this->usbdevice_handle, addr, data);
}
int sis315x::vme_A32DMA_D32_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return -1;
}
int sis315x::vme_A32BLT32_read(UINT addr, UINT* data, UINT req_num_data, UINT* got_num_data)
{
	return -1;
}
int sis315x::vme_A32MBLT64_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return -1;
}
int sis315x::vme_A32DMA_D32FIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return -1;
}
int sis315x::vme_A32BLT32FIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return call_vme_A32BLT32FIFO_read(this->usbdevice_handle, addr, data, request_nof_words, got_nof_words);
}
int sis315x::vme_A32MBLT64FIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return call_vme_A32MBLT64FIFO_read(this->usbdevice_handle, addr, data, request_nof_words, got_nof_words);
}



// here dummies
int sis315x::vme_A32_2EVME_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return -1 ;
}
int sis315x::vme_A32_2ESST160_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return -1 ;
}

int sis315x::vme_A32_2ESST267_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return -1 ;
}

int sis315x::vme_A32_2ESST320_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return -1 ;
}


int sis315x::vme_A32_2EVMEFIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return -1 ;
}
int sis315x::vme_A32_2ESST160FIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return -1 ;
}

int sis315x::vme_A32_2ESST267FIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return -1 ;
}

int sis315x::vme_A32_2ESST320FIFO_read(UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words)
{
	return -1 ;
}



// vme write
int sis315x::vme_A32D32_write(UINT addr, UINT data)
{
	return call_vme_A32D32_write(this->usbdevice_handle, addr, data);
}
int sis315x::vme_A32DMA_D32_write(UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words)
{
	return -1;
}
int sis315x::vme_A32BLT32_write(UINT addr, PUINT data, UINT request_nof_words, PUINT written_nof_words)
{
	return -1;
}
int sis315x::vme_A32MBLT64_write(UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words)
{
	return -1;
}
int sis315x::vme_A32DMA_D32FIFO_write(UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words)
{
	return -1;
}
int sis315x::vme_A32BLT32FIFO_write(UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words)
{
	return -1;
}
int sis315x::vme_A32MBLT64FIFO_write(UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words)
{
	return -1;
}


int sis315x::vme_IRQ_Status_read( UINT* data )

{
	return -1; // not implemented
}




