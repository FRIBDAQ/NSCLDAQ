/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_eth_access_rate_test.cpp                             */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 18.02.2015                                       */
/*  last modification:    24.07.2024                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*                                                                         */
/*  https://www.struck.de                                                  */
/*                                                                         */
/*  � 2024                                                                 */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


#include "project_system_define.h"		//define LINUX or WINDOWS
#include "project_interface_define.h"   //define Interface (Ethnernet UDP)


#ifdef LINUX

#include <iostream>
using namespace std;

//#include <iostream>
//#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <sys/time.h>

typedef int BOOL ;
#define TRUE  1
#define FALSE 0

#endif

#ifdef WINDOWS
	#include <iostream>
	#include <iomanip>
	using namespace std;
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <tchar.h>
	#include <winsock2.h>

	#include <stdlib.h>
	#include <string.h>
	//#include <math.h>
	#include "wingetopt.h" 

	#include <time.h>


#endif
/******************************************************************************************************/


#ifdef ETHERNET_UDP_INTERFACE
	#include "sis3316_ethernet_access_class.h"
	sis3316_eth *gl_virtual_vme_crate ;

	#ifdef LINUX
		#include <sys/types.h>
		#include <sys/socket.h>
	#endif

	#ifdef WINDOWS
		#pragma comment(lib, "ws2_32.lib") 
		typedef int socklen_t;
		char  gl_sis3316_ip_addr_string[32] ;

		long WinsockStartup()
		{
		  long rc;
		  WORD wVersionRequested;
		  WSADATA wsaData;
		  wVersionRequested = MAKEWORD(2, 1);

		  rc = WSAStartup( wVersionRequested, &wsaData );
		  return rc;
		}
	#endif

#endif
/******************************************************************************************************/


/******************************************************************************************************/


#include "sis3316_class.h"
#include "sis3316.h"

/*===========================================================================*/
/* Defines					  			     */
/*===========================================================================*/

#define MAX_NUMBER_LWORDS_64MBYTE			0x1000000       /* 64MByte */


/*===========================================================================*/
/* Globals					  			     */
/*===========================================================================*/

BOOL gl_stopReq = FALSE;

char gl_cmd_ip_string[64];


/*===========================================================================*/
/* Prototypes			                               		  			     */
/*===========================================================================*/


void program_stop_and_wait(void);
void get_time_clock(double* double_second) ;

#ifdef WINDOWS
BOOL CtrlHandler( DWORD ctrlType );
void usleep(unsigned int uint_usec) ;
#endif



/*===========================================================================*/


int main(int argc, char *argv[])
{

	int int_ch ;
	char ch_string[256] ;
	unsigned int uint_jumbo_frame_enable_flag ;

	unsigned int *uint_data_buffer ;

CHAR char_messages[128];
UINT nof_found_devices ;
unsigned int sis3316_not_OK;

int return_code ;
//unsigned int first_mod_base, nof_modules   ;
unsigned int module_base_addr=0   ; // dummy
unsigned int data;
unsigned int i;
unsigned int loop_counter;
//unsigned int error_loop_counter;

//unsigned int nof_lwords ;


	unsigned int speed_loop ;
	double time_clock_start, time_clock_finish;
	double  double_cycles_duration;
	double  double_Byte_Per_Second;
	double  double_KByte_Per_Second;
	double  double_MByte_Per_Second;

	unsigned int write_register_length ;
	unsigned int write_address_array[64] ;
	unsigned int write_data_array[64] ;
	unsigned int read_register_length ;
	unsigned int read_address_array[64] ;
	unsigned int read_data_array[64] ;


	unsigned int sub_write_block_length ;
	unsigned int sub_write_nof_blocks ;
	unsigned int write_fifo_length ;

	unsigned int udp_register_read_bad_return_code_counter = 0 ;
	unsigned int sis3316_register_write_bad_return_code_counter = 0 ;
	unsigned int sis3316_register_read_bad_return_code_counter = 0 ;

	unsigned int sis3316_fifo_write_bad_return_code_counter = 0 ;

	unsigned int sis3316_fifo_read_bad_return_code_WRONG_ACK_counter = 0 ;
	unsigned int sis3316_fifo_read_bad_return_code_WRONG_NOF_RECEVEID_BYTES_counter = 0 ;
	unsigned int sis3316_fifo_read_bad_return_code_WRONG_PACKET_IDENTIFIER_counter = 0 ;
	unsigned int sis3316_fifo_read_bad_return_code_WRONG_RECEIVED_PACKET_ORDER_counter = 0 ;
	unsigned int sis3316_fifo_read_bad_return_code_TIMEOUT_counter = 0 ;
	unsigned int written_nof_words ;

	unsigned int got_nof_words ;
	unsigned int read_fifo_length ;
	unsigned int read_block_length ;
	unsigned int j ;
	unsigned i_jumbo_mode;
	unsigned i_jumbo_max_mode;
	unsigned i_nof_packet_max_mode;
	unsigned i_nof_packet_mode;
	unsigned nofPacketsPerRequest_arry[5];

	unsigned cycle_error_flag;
	

/******************************************************************************************/
/*                                                                                        */
/*  SIS3316 program parameter                                                             */
/*                                                                                        */
/******************************************************************************************/
	// Ethernet UDP IP address
	strcpy(gl_cmd_ip_string,"192.168.1.2") ; // SIS3316 IP address
	//strcpy(gl_cmd_ip_string,"sis3316-0158") ; // SIS3316 IP address
	//strcpy(gl_cmd_ip_string,"212.60.16.204") ; // SIS3316 IP address
	//strcpy(gl_cmd_ip_string,"212.60.16.7") ; // SIS3316 IP address

	uint_jumbo_frame_enable_flag = 0 ;

/******************************************************************************************************************************/


	if (argc > 1) {

		while ((int_ch = getopt(argc, argv, "?hJI:")) != -1) {
			switch (int_ch) {
				//printf("ch %c    \n", int_ch );

				case 'I':
					sscanf(optarg,"%s", ch_string) ;
					//printf("-I %s    \n", ch_string );
					strcpy(gl_cmd_ip_string,ch_string) ;
					break;

				case 'J':
					uint_jumbo_frame_enable_flag = 1 ;
					break;

				case '?':
				case 'h':
				default:
					printf("Usage: %s  [-?h] [-I ip] [-J]  ", argv[0]);
					printf("   \n");
					printf("   \n");
					printf("   -I string  ......  SIS3316 IP Address  Default = %s\n", gl_cmd_ip_string);
					printf("   \n");
					printf("   -J     ..........  enable Jumbo packets \n");
					printf("   \n");
					printf("   \n");
					printf("   -h     ..........  print this message only\n");
					printf("   \n");
					printf("   \n");
					printf("   date: 24.07.2024 \n");
					printf("   \n");
					printf("   \n");
					exit(1);
			}
		}
    } // if (argc > 2)

	printf("\n");




/******************************************************************************************************************************/
/* VME Master Create, Open and Setup                                                                                          */
/******************************************************************************************************************************/
 

#ifdef ETHERNET_UDP_INTERFACE

	char  pc_ip_addr_string[32] ;
	char  sis3316_ip_addr_string[32] ;

	strcpy(sis3316_ip_addr_string, gl_cmd_ip_string) ; // SIS3316 IP address
	//strcpy(sis3316_ip_addr_string,"192.168.1.100") ; // SIS3316 IP address
	#ifdef WINDOWS
    //return_code = WSAStartup();
    return_code = WinsockStartup();
	#endif

	gl_virtual_vme_crate = new sis3316_eth;  //sis3316_ethernet_access_class

	// increase read_buffer size
	// SUSE needs following command as su: >sysctl -w net.core.rmem_max=33554432
	int	sockbufsize = 335544432 ; // 0x2000000
	return_code = gl_virtual_vme_crate->set_UdpSocketOptionBufSize(sockbufsize) ;
	if(return_code != 0x0) {
		printf("ERROR  set_UdpSocketOptionBufSize: return_code = 0x%08x\n\n", return_code);
		program_stop_and_wait();
	}

	//strcpy(pc_ip_addr_string,"212.60.16.49") ; // Window example: secocnd Lan interface IP address is 212.60.16.49
	strcpy(pc_ip_addr_string,"") ; // empty if default Lan interface (Window: use IP address to bind in case of 2. 3. 4. .. LAN Interface)
	return_code = gl_virtual_vme_crate->set_UdpSocketBindMyOwnPort( pc_ip_addr_string);

	gl_virtual_vme_crate->set_UdpSocketSIS3316_IpAddress( sis3316_ip_addr_string);



	gl_virtual_vme_crate->udp_reset_cmd();
	gl_virtual_vme_crate->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000); // kill request and grant from other vme interface
	gl_virtual_vme_crate->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x1); // request access to SIS3316 from UDP interface

    return_code = gl_virtual_vme_crate->vme_A32D32_read(SIS3316_MODID,&data);
	if(return_code != 0x0) {
		printf("ERROR: gl_virtual_vme_crate->vme_A32D32_read: return_code = 0x%08x\n\n", return_code);
		program_stop_and_wait();
		return -1 ;
	}
	printf("return_code = 0X%08x   Module ID = 0X%08x \n",return_code, data);
    return_code = gl_virtual_vme_crate->vme_A32D32_read(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL,&data);
	printf("return_code = 0X%08x   ACCESS    = 0X%08x \n",return_code, data);

#endif

// open Vme Interface device
	return_code = gl_virtual_vme_crate->vmeopen ();  // open Vme interface
	gl_virtual_vme_crate->get_vmeopen_messages (char_messages, &nof_found_devices);  // open Vme interface
	printf("\n%s    (found %d vme interface device[s])\n\n",char_messages, nof_found_devices);

	if(return_code != 0x0) {
		//printf("ERROR: gl_virtual_vme_crate->vmeopen: return_code = 0x%08x\n\n", return_code);
		program_stop_and_wait();
		return -1 ;
	}


/******************************************************************************************/


/********************************************************************************************************************************************/
/*  malloc buffer                                                                                                                            */
/********************************************************************************************************************************************/

	uint_data_buffer = (unsigned int*)malloc(MAX_NUMBER_LWORDS_64MBYTE);
	if(uint_data_buffer == NULL) {
		printf("Error: malloc uint_data_buffer\n");
		exit(1);
	}

/******************************************************************************************/
#ifdef WINDOWS
	if( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE)CtrlHandler, TRUE ) ){
		printf( "Error setting Console-Ctrl Handler\n" );
		return -1;
	}
#endif
/******************************************************************************************/

/******************************************************************************************/

	return_code = gl_virtual_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_MODID, &data);  
	//printf("vme_A32D32_read: data = 0x%08x     return_code = 0x%08x\n", data, return_code);


	// kill request and grant from vme interface (in case of use using etehrnet interface)
	gl_virtual_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000);
	// arbitrate
	gl_virtual_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 1);


	sis3316_not_OK = 0 ;
	if (return_code == 0) {
		printf("SIS3316_MODID                    = 0x%08x\n\n", data);

		gl_virtual_vme_crate->vme_A32D32_write( module_base_addr + SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits
		gl_virtual_vme_crate->vme_A32D32_write( module_base_addr + SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits
		gl_virtual_vme_crate->vme_A32D32_write( module_base_addr + SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits
		gl_virtual_vme_crate->vme_A32D32_write( module_base_addr + SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits

		gl_virtual_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH1_4_FIRMWARE_REG, &data);  
		printf("SIS3316_ADC_CH1_4_FIRMWARE_REG   = 0x%08x \n", data);
		gl_virtual_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH5_8_FIRMWARE_REG, &data);  
		printf("SIS3316_ADC_CH5_8_FIRMWARE_REG   = 0x%08x \n", data);
		gl_virtual_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH9_12_FIRMWARE_REG, &data);  
		printf("SIS3316_ADC_CH9_12_FIRMWARE_REG  = 0x%08x \n", data);
		gl_virtual_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH13_16_FIRMWARE_REG, &data);  
		printf("SIS3316_ADC_CH13_16_FIRMWARE_REG = 0x%08x \n\n", data);

#ifdef TEST_DEBUG
		gl_virtual_vme_crate->vme_A32D32_write( module_base_addr + SIS3316_VME_FPGA_LINK_ADC_PROT_STATUS, 0xE0E0E0E0);  // clear error Latch bits 
		gl_virtual_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_VME_FPGA_LINK_ADC_PROT_STATUS, &data);  
		printf("SIS3316_VME_FPGA_LINK_ADC_PROT_STATUS: data = 0x%08x     return_code = 0x%08x\n", data, return_code);
		if (data != 0x18181818) { sis3316_not_OK = 1 ; }

		gl_virtual_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH1_4_STATUS_REG, &data);  
		printf("SIS3316_ADC_CH1_4_STATUS_REG     = 0x%08x \n", data);
		if (data != 0x130018) { sis3316_not_OK = 1 ; }

		gl_virtual_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH5_8_STATUS_REG, &data);  
		printf("SIS3316_ADC_CH5_8_STATUS_REG     = 0x%08x \n", data);
		if (data != 0x130018) { sis3316_not_OK = 1 ; }

		gl_virtual_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH9_12_STATUS_REG, &data);  
		printf("SIS3316_ADC_CH9_12_STATUS_REG    = 0x%08x \n", data);
		if (data != 0x130018) { sis3316_not_OK = 1 ; }

		gl_virtual_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH13_16_STATUS_REG, &data);  
		printf("SIS3316_ADC_CH13_16_STATUS_REG   = 0x%08x \n\n", data);
		if (data != 0x130018) { sis3316_not_OK = 1 ; }
#endif
	}
	else {
		printf("SIS3316_MODID                  = 0x%08x     return_code = 0x%08x\n", data, return_code);
		program_stop_and_wait() ;
		return -1 ;
	}



	sis3316_adc  *sis3316_adc1 ;
	sis3316_adc1 = new sis3316_adc( gl_virtual_vme_crate, module_base_addr); // sis3316_class
	if (sis3316_adc1->adc_125MHz_flag == 1) {
		printf("SIS3316-125MHz-16bit");
	}
	else {
		printf("SIS3316-250MHz-14bit");
	}
	printf("\n\n");
	printf("\n");


	printf("\n\n");
	printf("\n\n");
	printf("Info Counter:  read_udp_register_receive_ack_retry        .......    =  %d     \n", gl_virtual_vme_crate->read_udp_register_receive_ack_retry_counter);
	printf("Info Counter:  read_sis3316_register_receive_ack_retry    .......    =  %d     \n", gl_virtual_vme_crate->read_sis3316_register_receive_ack_retry_counter);
	printf("Info Counter:  write_sis3316_register_receive_ack_retry   .......    =  %d     \n", gl_virtual_vme_crate->write_sis3316_register_receive_ack_retry_counter);
	printf("Info Counter:  read_sis3316_fifo_receive_ack_retry        .......    =  %d     \n", gl_virtual_vme_crate->read_sis3316_fifo_receive_ack_retry_counter);
	printf("Info Counter:  write_sis3316_fifo_receive_ack_retry       .......    =  %d     \n", gl_virtual_vme_crate->write_sis3316_fifo_receive_ack_retry_counter);
	printf("\n");
	printf("Info Counter:  udp_register_read_bad_return_code          .......    =  %d     \n", udp_register_read_bad_return_code_counter);
	printf("Info Counter:  sis3316_register_write_bad_return_code     .......    =  %d     \n", sis3316_register_write_bad_return_code_counter);
	printf("Info Counter:  sis3316_register_read_bad_return_code      .......    =  %d     \n", sis3316_register_read_bad_return_code_counter);
	printf("\n");
	printf("Info Counter:  fifo_write_bad_return_code                 .......    =  %d     \n", sis3316_fifo_write_bad_return_code_counter);
	printf("\n");
	printf("Info Counter:  fifo_read_bad_return_code_TIMEOUT          .......    =  %d     \n", sis3316_fifo_read_bad_return_code_TIMEOUT_counter);
	printf("Info Counter:  fifo_read_bad_return_code_WRONG_ACK                   =  %d     \n", sis3316_fifo_read_bad_return_code_WRONG_ACK_counter);
	printf("Info Counter:  fifo_read_bad_return_code_WRONG_NOF_RECEVEID_BYTES    =  %d     \n", sis3316_fifo_read_bad_return_code_WRONG_NOF_RECEVEID_BYTES_counter);
	printf("Info Counter:  fifo_read_bad_return_code_WRONG_PACKET_IDENTIFIER     =  %d     \n", sis3316_fifo_read_bad_return_code_WRONG_PACKET_IDENTIFIER_counter);
	printf("Info Counter:  fifo_read_bad_return_code_WRONG_RECEIVED_PACKET_ORDER =  %d     \n", sis3316_fifo_read_bad_return_code_WRONG_RECEIVED_PACKET_ORDER_counter);
	printf("\n\n\n");



/******************************************************************************************/
	loop_counter = 0 ;
	do {



	/* single internal UDP Register write cycles (without handshake)   */
		speed_loop = 10000 ;
		get_time_clock(&time_clock_start);
		for (i = 0;i<speed_loop;i++) {
			return_code = gl_virtual_vme_crate->udp_register_write( 0x0, 0x10001); // toogle led U ; using the sis3316_ethernet_access_class
		}
		get_time_clock(&time_clock_finish);
		double_cycles_duration = (double)(time_clock_finish - time_clock_start) ;
		//printf( "start = %2.6f seconds\n", (double)(time_clock_start));
		//printf( "finish = %2.6f seconds\n", (double)(time_clock_finish)  );
		//printf( "%2.3f seconds\n", double_cycles_duration );
		double_Byte_Per_Second =  (double)(speed_loop * 4) / double_cycles_duration ;
		double_KByte_Per_Second =  double_Byte_Per_Second / 1024.0 ;
		printf( "->Single udp_register_write cycle rate:                      %4.3f KByte/sec \n", double_KByte_Per_Second );
		//printf( "%2.1f transfer speed \n", double_KByte_Per_Second );



	/* single internal UDP Register read cycles (handshake)  ->  SIS3316_ETH_UDP_RegisterRead */
		speed_loop = 5000 ;
		get_time_clock(&time_clock_start);
		for (i = 0;i<speed_loop;i++) {
			return_code = gl_virtual_vme_crate->udp_register_read(  0x0, &data); //   using the sis3316_ethernet_access_class
			if (return_code != 0) {
				printf("Single udp_register_read: i = 0x%08x   return_code = 0x%08x   \n",i, return_code);
				udp_register_read_bad_return_code_counter++ ;
			}
			if(gl_stopReq == TRUE) {break ;}
		}
		get_time_clock(&time_clock_finish);
		double_cycles_duration = (double)(time_clock_finish - time_clock_start)  ;
		double_Byte_Per_Second =  (double)(speed_loop * 4) / double_cycles_duration ;
		double_KByte_Per_Second =  double_Byte_Per_Second / 1024.0 ;
		printf( "->Single udp_register_read cycle rate:                       %4.3f KByte/sec \n", double_KByte_Per_Second );
		printf("\n");

		if(gl_stopReq == TRUE) {break ;}

	/* single SIS3316 Register write cycles (with handshake)    */
		write_register_length = 1 ;
		write_address_array[0] = 0x78 ;
		write_data_array[0] = 0x80000000 ;  // pulse on Lemo UO

		speed_loop = 20000 ;
		get_time_clock(&time_clock_start);
		for (i = 0;i<speed_loop;i++) {
			return_code = gl_virtual_vme_crate->udp_sis3316_register_write( write_register_length, write_address_array, write_data_array); // using the sis3316_ethernet_access_class
			if (return_code != 0) {
				printf("Single udp_sis3316_register_write: i = 0x%08x   return_code = 0x%08x   \n",i, return_code);
				sis3316_register_write_bad_return_code_counter++ ;
				
			}
			if(gl_stopReq == TRUE) {break ;}
		}
		get_time_clock(&time_clock_finish);
		double_cycles_duration = (double)(time_clock_finish - time_clock_start) ;
		double_Byte_Per_Second =  (double)(speed_loop * 4) / double_cycles_duration ;
		double_KByte_Per_Second =  double_Byte_Per_Second / 1024.0 ;
		printf( "->Single udp_sis3316_register_write cycle rate:              %4.3f KByte/sec \n", double_KByte_Per_Second );

		if(gl_stopReq == TRUE) {break ;}


	/* single SIS3316 Register read cycles (with handshake)  ->  SIS3316_ETH_UDP_SIS3316_RegisterRead */
		read_register_length = 1 ;
		read_address_array[0] = 0x20 ;
		speed_loop = 10000 ;
		get_time_clock(&time_clock_start);
		for (i = 0;i<speed_loop;i++) {
			return_code = gl_virtual_vme_crate->udp_sis3316_register_read( read_register_length, read_address_array, read_data_array); // using the sis3316_ethernet_access_class
			if (return_code != 0) {
				printf("Single udp_sis3316_register_read: i = 0x%08x   return_code = 0x%08x   \n",i, return_code);
				sis3316_register_read_bad_return_code_counter++ ;
			}
			if(gl_stopReq == TRUE) {break ;}
		}
		get_time_clock(&time_clock_finish);
		double_cycles_duration = (double)(time_clock_finish - time_clock_start) ;
		double_Byte_Per_Second =  (double)(speed_loop * 4) / double_cycles_duration ;
		double_KByte_Per_Second =  double_Byte_Per_Second / 1024.0 ;
		printf( "->Single udp_sis3316_register_read cycle rate:               %4.3f KByte/sec \n", double_KByte_Per_Second );

		printf("\n");

	/* burst of 16 SIS3316 Register read cycles (with handshake)  ->  SIS3316_ETH_UDP_SIS3316_RegisterRead */
		read_register_length = 16 ;
		for (i = 0;i<4;i++) {
			read_address_array[i]    = 0x1120 + (4*i) ; // Previous Bank Sample address ch 1-4
			read_address_array[4+i]  = 0x2120 + (4*i) ; // Previous Bank Sample address ch 5-8
			read_address_array[8+i]  = 0x3120 + (4*i) ; // Previous Bank Sample address ch 9-12
			read_address_array[12+i] = 0x4120 + (4*i) ; // Previous Bank Sample address ch 13-16
		}
		speed_loop = 10000 ;
		get_time_clock(&time_clock_start);
		for (i = 0;i<speed_loop;i++) {
			return_code = gl_virtual_vme_crate->udp_sis3316_register_read( read_register_length, read_address_array, read_data_array); // using the sis3316_ethernet_access_class
			if (return_code != 0) {
				printf("Burst of 16  udp_sis3316_register_read: i = 0x%08x   return_code = 0x%08x   \n",i, return_code);
				sis3316_register_read_bad_return_code_counter++ ;
			}
			if(gl_stopReq == TRUE) {break ;}
		}
		get_time_clock(&time_clock_finish);
		double_cycles_duration = (double)(time_clock_finish - time_clock_start)  ;
		double_Byte_Per_Second =  (double)(speed_loop * 4 * read_register_length) / double_cycles_duration ;
		double_KByte_Per_Second =  double_Byte_Per_Second / 1024.0 ;
		printf( "->Burst of 16 udp_sis3316_register_read cycle rate:          %4.3f KByte/sec \n", double_KByte_Per_Second );

		printf("\n");
		if(gl_stopReq == TRUE) {break ;}


// Memory write (test feature)
		write_register_length  = 1 ;
		write_address_array[0] = 0x80 ;
		write_data_array[0]    = 0xC0000000 ; // start write transfer FSM
		return_code = gl_virtual_vme_crate->udp_sis3316_register_write( write_register_length, write_address_array, write_data_array); // using the sis3316_ethernet_access_class
		if (return_code != 0) {
			printf("Single udp_sis3316_register_write: i = 0x%08x   return_code = 0x%08x   \n",i, return_code);
			sis3316_register_write_bad_return_code_counter++ ;
		}
		for (i=0;i<0x10000;i++) {
			uint_data_buffer[i] = 0x11000000 + i;
		}

		sub_write_block_length = 0x100; // max 0x100
		sub_write_nof_blocks   = 0x100; // 0x100 blocks -> 0x10000 -> 64K 32-bit words
		write_fifo_length = sub_write_nof_blocks * sub_write_block_length ;

		cycle_error_flag = 0 ;
		speed_loop = 20 ;
		get_time_clock(&time_clock_start);
		for (i = 0;i<speed_loop;i++) {

			return_code = gl_virtual_vme_crate->udp_sis3316_fifo_write( write_fifo_length, 0x100000, uint_data_buffer, &written_nof_words); // using the sis3316_ethernet_access_class
			if (return_code != 0) {
				printf("Single udp_sis3316_fifo_write: i = 0x%08x   return_code = 0x%08x   \n",i, return_code);
				sis3316_fifo_write_bad_return_code_counter++ ;
				cycle_error_flag = 1 ;
			}
			if(gl_stopReq == TRUE) {break ;}
		}
		get_time_clock(&time_clock_finish);
		double_cycles_duration = (double)(time_clock_finish - time_clock_start)  ;
		double_Byte_Per_Second =  (double)(speed_loop * 4 * write_fifo_length) / double_cycles_duration ;
		//double_KByte_Per_Second =  double_Byte_Per_Second / 1024.0 ;
		double_MByte_Per_Second =  double_Byte_Per_Second / 1048576.0 ; // 1M
		printf( "->udp_sis3316_fifo_write cycle rate:                         %4.3f MByte/sec ", double_MByte_Per_Second );
		if(cycle_error_flag != 0) {
			printf("  -->  detected error !");
		}
		printf("\n\n");
		printf("\n");
		if(gl_stopReq == TRUE) {break ;}


	// Memory read
		write_register_length  = 1 ;
		write_address_array[0] = 0x80 ;
		write_data_array[0]    = 0x80000000 ; // start read transfer FSM
		return_code = gl_virtual_vme_crate->udp_sis3316_register_write( write_register_length, write_address_array, write_data_array); // using the sis3316_ethernet_access_class
		if (return_code != 0) {
			printf("Single udp_sis3316_register_write: i = 0x%08x   return_code = 0x%08x   \n",i, return_code);
			sis3316_register_write_bad_return_code_counter++ ;
		}

		read_fifo_length  = 0x10000 ;
		read_block_length  = 0x40 ;

		i_nof_packet_max_mode = 5 ;
		nofPacketsPerRequest_arry[0] = 1 ;
		nofPacketsPerRequest_arry[1] = 5 ;
		nofPacketsPerRequest_arry[2] = 10 ;
		nofPacketsPerRequest_arry[3] = 20 ;
		nofPacketsPerRequest_arry[4] = UDP_MAX_PACKETS_PER_REQUEST ; 

		i_jumbo_max_mode = 1 ;
		if(uint_jumbo_frame_enable_flag==1) {
			i_jumbo_max_mode = 2 ;
		}
		//printf("i_jumbo_max_mode = %d     \n",i_jumbo_max_mode);



		for (i_jumbo_mode = 0; i_jumbo_mode < i_jumbo_max_mode; i_jumbo_mode++ ) {	
			if(i_jumbo_mode == 0) { gl_virtual_vme_crate->set_UdpSocketDisableJumboFrame() ; }
			if(i_jumbo_mode == 1) { gl_virtual_vme_crate->set_UdpSocketEnableJumboFrame() ; }

			for (i_nof_packet_mode = 0; i_nof_packet_mode < i_nof_packet_max_mode; i_nof_packet_mode++ ) {	
					printf( "                                                              \tmaxNofPackets  \n");
					printf( "                                       read_fifo_32bit_length \tPerRequest \tjumbo_mode  \n");


				gl_virtual_vme_crate->set_UdpSocketReceiveNofPackagesPerRequest(nofPacketsPerRequest_arry[i_nof_packet_mode]);
				for (j=3;j<13;j++) {
					read_fifo_length  = read_block_length << j  ;
					speed_loop = 500 ;
					if(j > 4) {speed_loop = 200 ; }
					if(j > 7) {speed_loop = 100 ; }
					if(j > 9) {speed_loop = 30 ; }
					get_time_clock(&time_clock_start);
					cycle_error_flag = 0 ;
					for (i = 0;i<speed_loop;i++) {
						return_code = gl_virtual_vme_crate->udp_sis3316_fifo_read( read_fifo_length, 0x100000, uint_data_buffer, &got_nof_words); // using the sis3316_ethernet_access_class
						if (return_code != 0) {
							//printf("udp_sis3316_fifo_read: i = 0x%08x   return_code = 0x%08x   \n",i, return_code);
							cycle_error_flag = 1 ;
							if (return_code == PROTOCOL_ERROR_CODE_WRONG_ACK) {
								sis3316_fifo_read_bad_return_code_WRONG_ACK_counter++ ;
							} 
							else {
								if (return_code == PROTOCOL_ERROR_CODE_WRONG_NOF_RECEVEID_BYTES) {
									sis3316_fifo_read_bad_return_code_WRONG_NOF_RECEVEID_BYTES_counter++ ;
								} 
								else {
									if (return_code == PROTOCOL_ERROR_CODE_WRONG_PACKET_IDENTIFIER) {
										sis3316_fifo_read_bad_return_code_WRONG_PACKET_IDENTIFIER_counter++ ;
									} 
									else {
										if (return_code == PROTOCOL_ERROR_CODE_WRONG_RECEIVED_PACKET_ORDER) {
											sis3316_fifo_read_bad_return_code_WRONG_RECEIVED_PACKET_ORDER_counter++ ;
										} 
										else {
											sis3316_fifo_read_bad_return_code_TIMEOUT_counter++ ;
										}
									}
								}
							}
						}
						if(gl_stopReq == TRUE) {break ;}
					}
					if(gl_stopReq == TRUE) {break ;}
					get_time_clock(&time_clock_finish);
					double_cycles_duration = (double)(time_clock_finish - time_clock_start) ;
					double_Byte_Per_Second =  (double)(speed_loop * 4 * read_fifo_length) / double_cycles_duration ;
					double_MByte_Per_Second =  double_Byte_Per_Second / 1048576.0 ; // 1M
					//printf( "start = %2.6f seconds\n", (double)(time_clock_start));
					//printf( "finish = %2.6f seconds\n", (double)(time_clock_finish)  );
					//printf( "->udp_sis3316_fifo_read cycle rate:    read_fifo_32bit_length = %d (0x%06x)   \tmaxNofPacketsPerRequest = %d  \tjumbo_mode = %d\t  %4.3f MByte/sec  ",
					//		read_fifo_length, read_fifo_length, nofPacketsPerRequest_arry[i_nof_packet_mode], i_jumbo_mode, double_MByte_Per_Second );
					printf( "->udp_sis3316_fifo_read cycle rate:        %d (0x%06x)   \t     %d      \t    %d       \t  %4.3f MByte/sec  ",
							read_fifo_length, read_fifo_length, nofPacketsPerRequest_arry[i_nof_packet_mode], i_jumbo_mode, double_MByte_Per_Second );
					if(cycle_error_flag != 0) {
						printf("  -->  detected error !");
					}
					printf("\n");

				}
				printf("\n");
				if(gl_stopReq == TRUE) {break ;}
			}
		}


		gl_virtual_vme_crate->set_UdpSocketReceiveNofPackagesPerRequest(1);
		gl_virtual_vme_crate->set_UdpSocketDisableJumboFrame() ;


		printf("\n\n\n");
		printf("loop_counter = %d     \n", loop_counter);
		printf("\n\n");
		printf("Info Counter:  read_udp_register_receive_ack_retry        .......    =  %d     \n", gl_virtual_vme_crate->read_udp_register_receive_ack_retry_counter);
		printf("Info Counter:  read_sis3316_register_receive_ack_retry    .......    =  %d     \n", gl_virtual_vme_crate->read_sis3316_register_receive_ack_retry_counter);
		printf("Info Counter:  write_sis3316_register_receive_ack_retry   .......    =  %d     \n", gl_virtual_vme_crate->write_sis3316_register_receive_ack_retry_counter);
		printf("Info Counter:  read_sis3316_fifo_receive_ack_retry        .......    =  %d     \n", gl_virtual_vme_crate->read_sis3316_fifo_receive_ack_retry_counter);
		printf("Info Counter:  write_sis3316_fifo_receive_ack_retry       .......    =  %d     \n", gl_virtual_vme_crate->write_sis3316_fifo_receive_ack_retry_counter);
		printf("\n");
		printf("Info Counter:  udp_register_read_bad_return_code          .......    =  %d     \n", udp_register_read_bad_return_code_counter);
		printf("Info Counter:  sis3316_register_write_bad_return_code     .......    =  %d     \n", sis3316_register_write_bad_return_code_counter);
		printf("Info Counter:  sis3316_register_read_bad_return_code      .......    =  %d     \n", sis3316_register_read_bad_return_code_counter);
		printf("\n");
		printf("Info Counter:  fifo_write_bad_return_code                 .......    =  %d     \n", sis3316_fifo_write_bad_return_code_counter);
		printf("\n");
		printf("Info Counter:  fifo_read_bad_return_code_TIMEOUT          .......    =  %d     \n", sis3316_fifo_read_bad_return_code_TIMEOUT_counter);
		printf("Info Counter:  fifo_read_bad_return_code_WRONG_ACK                   =  %d     \n", sis3316_fifo_read_bad_return_code_WRONG_ACK_counter);
		printf("Info Counter:  fifo_read_bad_return_code_WRONG_NOF_RECEVEID_BYTES    =  %d     \n", sis3316_fifo_read_bad_return_code_WRONG_NOF_RECEVEID_BYTES_counter);
		printf("Info Counter:  fifo_read_bad_return_code_WRONG_PACKET_IDENTIFIER     =  %d     \n", sis3316_fifo_read_bad_return_code_WRONG_PACKET_IDENTIFIER_counter);
		printf("Info Counter:  fifo_read_bad_return_code_WRONG_RECEIVED_PACKET_ORDER =  %d     \n", sis3316_fifo_read_bad_return_code_WRONG_RECEIVED_PACKET_ORDER_counter);
		printf("\n\n\n\n");
		loop_counter++;
		if(gl_stopReq == TRUE) {break ;}


	} while(gl_stopReq == FALSE);


	// wait until Ctrl C
	gl_stopReq = FALSE;
	printf("press CTRL-C to exit\n");
	do {
		//printf("gl_stopReq    = %d     \n", gl_stopReq);
		usleep(100000);
	} while((gl_stopReq == FALSE) );


	return 0;
}

/***********************************************************************************************************************************************/
/***********************************************************************************************************************************************/
/***********************************************************************************************************************************************/
/**********************************************************************************************************************************/


/***********************************************************************************************************************************************/
/***********************************************************************************************************************************************/
/***********************************************************************************************************************************************/


void program_stop_and_wait(void)
{
	gl_stopReq = FALSE;
	printf( "\n\nProgram stopped");
	printf( "\n\nEnter ctrl C");
	do {
		//printf("program_stop_and_wait : gl_stopReq    = %d     \n", gl_stopReq);
		usleep(1000);
	} while (gl_stopReq == FALSE) ;
	
	//		result = scanf( "%s", line_in );
}


/***************************************************/
#ifdef WINDOWS

BOOL CtrlHandler( DWORD ctrlType ){
	switch( ctrlType ){
	case CTRL_C_EVENT:
		printf( "\n\nCTRL-C pressed. finishing current task \n\n");
		gl_stopReq = TRUE;
		//printf("CtrlHandler : gl_stopReq    = %d     \n", gl_stopReq);
		return( TRUE );
		break;
	default:
		printf( "\n\ndefault pressed. \n\n");
		return( FALSE );
		break;
	}
}
/* ***************************************************************************************************************** */
#endif
void get_time_clock(double* double_second)
{
#ifdef WINDOWS
	volatile clock_t clock_t_time ;
	clock_t_time = clock()  ;
	*double_second  =  ((double) clock_t_time /  (double) CLOCKS_PER_SEC) ;

#endif
#ifdef LINUX
	timeval time ;
	gettimeofday(&time, 0);
	*double_second  = (double) (time.tv_sec + (time.tv_usec / 1000000.0));
#endif

}
