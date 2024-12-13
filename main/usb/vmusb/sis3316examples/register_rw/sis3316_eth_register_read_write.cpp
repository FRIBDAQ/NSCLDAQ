/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_eth_register_read_write.cpp                          */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 20.09.2019                                       */
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



/*===========================================================================*/
/* Globals					  			     */
/*===========================================================================*/

char gl_cmd_ip_string[64];


/*===========================================================================*/
/* Prototypes			                               		  			     */
/*===========================================================================*/




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
	unsigned int uint_address ;
	unsigned int uint_read_data ;
	unsigned int uint_write_data ;
	unsigned int uint_write_enable_flag ;

	return_code            = 0 ;
	uint_address           = 0 ;
	uint_read_data         = 0 ;
	uint_write_data        = 0 ;
	uint_write_enable_flag = 0 ;

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

		while ((int_ch = getopt(argc, argv, "?hJI:A:W:")) != -1) {
			switch (int_ch) {
				//printf("ch %c    \n", int_ch );

				case 'I':
					sscanf(optarg,"%s", ch_string) ;
					//printf("-I %s    \n", ch_string );
					strcpy(gl_cmd_ip_string,ch_string) ;
					break;

				case 'A':
					sscanf(optarg,"%X", &uint_address) ;
					break;

				case 'W':
					sscanf(optarg,"%X", &uint_write_data) ;
					uint_write_enable_flag = 1 ;
					break;

				case '?':
				case 'h':
				default:
					printf("Usage: %s  [-?h]  -I ip -A address  [-W write data] ", argv[0]);
					printf("   \n");
					printf("   \n");
					printf("   -I string  ......  SIS3316 IP Address  Default = %s\n", gl_cmd_ip_string);
					printf("   \n");
					printf("   -A Address   ....  Read/Write Address  (hex)\n");
					printf("   -W Wirite Data ..  Read/Write Address (hex) \n");
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
	}

	//strcpy(pc_ip_addr_string,"212.60.16.49") ; // Window example: secocnd Lan interface IP address is 212.60.16.49
	strcpy(pc_ip_addr_string,"") ; // empty if default Lan interface (Window: use IP address to bind in case of 2. 3. 4. .. LAN Interface)
	return_code = gl_virtual_vme_crate->set_UdpSocketBindMyOwnPort( pc_ip_addr_string);

	gl_virtual_vme_crate->set_UdpSocketSIS3316_IpAddress( sis3316_ip_addr_string);

	gl_virtual_vme_crate->udp_reset_cmd();
	gl_virtual_vme_crate->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000); // kill request and grant from other vme interface
	gl_virtual_vme_crate->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x1); // request access to SIS3316 from UDP interface

#endif




// open Vme Interface device
	return_code = gl_virtual_vme_crate->vmeopen ();  // open Vme interface
	gl_virtual_vme_crate->get_vmeopen_messages (char_messages, &nof_found_devices);  // open Vme interface
	//printf("\n%s    (found %d vme interface device[s])\n\n",char_messages, nof_found_devices);

	if(return_code != 0x0) {
		//printf("ERROR: gl_virtual_vme_crate->vmeopen: return_code = 0x%08x\n\n", return_code);
		return -1 ;
	}


	printf("\n");

	if(uint_write_enable_flag == 1) {
		gl_virtual_vme_crate->vme_A32D32_write(uint_address, uint_write_data);
		printf("write: address = 0x%08x     read_data = 0x%08x     return_code = 0x%08x\n",uint_address,  uint_write_data, return_code);
	}
	else {
		return_code = gl_virtual_vme_crate->vme_A32D32_read ( uint_address, &uint_read_data);
		printf("read:  address = 0x%08x     read_data = 0x%08x     return_code = 0x%08x\n",uint_address,  uint_read_data, return_code);
	}
	printf("\n");
/******************************************************************************************/



	return 0;
}
